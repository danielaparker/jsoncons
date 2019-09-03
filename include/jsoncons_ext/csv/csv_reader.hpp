// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_READER_HPP
#define JSONCONS_CSV_CSV_READER_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <memory> // std::allocator
#include <utility> // std::move
#include <istream> // std::basic_istream
#include <jsoncons/source.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons_ext/csv/csv_error.hpp>
#include <jsoncons_ext/csv/csv_parser.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons_ext/csv/csv_options.hpp>

namespace jsoncons { namespace csv {

template<class CharT,class Src=jsoncons::stream_source<CharT>,class Allocator=std::allocator<char>>
class basic_csv_reader 
{
    struct stack_item
    {
        stack_item()
           : array_begun_(false)
        {
        }

        bool array_begun_;
    };
    typedef CharT char_type;
    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT> char_allocator_type;

    basic_csv_reader(const basic_csv_reader&) = delete; 
    basic_csv_reader& operator = (const basic_csv_reader&) = delete; 

    basic_null_json_content_handler<CharT> default_content_handler_;

    basic_json_content_handler<CharT>& handler_;

    basic_csv_parser<CharT,Allocator> parser_;
    Src source_;
    std::vector<CharT,char_allocator_type> buffer_;
    size_t buffer_length_;
    bool eof_;
    bool begin_;
public:
    // Structural characters
    static const size_t default_max_buffer_length = 16384;
    //!  Parse an input stream of CSV text into a json object
    /*!
      \param is The input stream to read from
    */

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_content_handler<CharT>& handler)

       : basic_csv_reader(std::forward<Source>(source), 
                          handler, 
                          basic_csv_options<CharT>::get_default_options(), 
                          default_csv_parsing())
    {
    }

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_content_handler<CharT>& handler,
                     const basic_csv_options<CharT>& options)

        : basic_csv_reader(std::forward<Source>(source), 
                           handler, 
                           options, 
                           default_csv_parsing())
    {
    }

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_content_handler<CharT>& handler,
                     std::function<bool(csv_errc,const ser_context&)> err_handler)
        : basic_csv_reader(std::forward<Source>(source), 
                           handler, 
                           basic_csv_options<CharT>::get_default_options(), 
                           err_handler)
    {
    }

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_content_handler<CharT>& handler,
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler,
                     typename std::enable_if<!std::is_constructible<basic_string_view<CharT>,Source>::value>::type* = 0)
       : handler_(handler),
         parser_(options, err_handler),
         source_(std::forward<Source>(source)),
         buffer_length_(default_max_buffer_length),
         eof_(false),
         begin_(true)
    {
        buffer_.reserve(buffer_length_);
    }

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_content_handler<CharT>& handler,
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler,
                     typename std::enable_if<std::is_constructible<basic_string_view<CharT>,Source>::value>::type* = 0)
       : handler_(handler),
         parser_(options, err_handler),
         buffer_length_(0),
         eof_(false),
         begin_(false)
    {
        basic_string_view<CharT> sv(std::forward<Source>(source));
        auto result = unicons::skip_bom(sv.begin(), sv.end());
        if (result.ec != unicons::encoding_errc())
        {
            throw ser_error(result.ec,parser_.line(),parser_.column());
        }
        size_t offset = result.it - sv.begin();
        parser_.update(sv.data()+offset,sv.size()-offset);
    }

    ~basic_csv_reader()
    {
    }

    void read()
    {
        std::error_code ec;
        read(ec);
        if (ec)
        {
            throw ser_error(ec,parser_.line(),parser_.column());
        }
    }

    void read(std::error_code& ec)
    {
        try
        {
            read_internal(ec);
        }
        catch (const ser_error& e)
        {
            ec = e.code();
        }
    }

    bool eof() const
    {
        return eof_;
    }

    size_t buffer_length() const
    {
        return buffer_length_;
    }

    void buffer_length(size_t length)
    {
        buffer_length_ = length;
        buffer_.reserve(buffer_length_);
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    JSONCONS_DEPRECATED_MSG("Instead, use buffer_length()")
    size_t buffer_capacity() const
    {
        return buffer_length_;
    }

    JSONCONS_DEPRECATED_MSG("Instead, use buffer_length(size_t)")
    void buffer_capacity(size_t length)
    {
        buffer_length_ = length;
        buffer_.reserve(buffer_length_);
    }
#endif
private:

    void read_internal(std::error_code& ec)
    {
        if (source_.is_error())
        {
            ec = csv_errc::source_error;
            return;
        }   
        parser_.reset();
        while (!parser_.finished())
        {
            if (parser_.source_exhausted())
            {
                if (!source_.eof())
                {
                    read_buffer(ec);
                    if (ec)
                    {
                        return;
                    }
                }
                else
                {
                    parser_.update(buffer_.data(),0);
                    eof_ = true;
                }
            }
            parser_.parse_some(handler_, ec);
            if (ec) return;
        }
    }
    void read_buffer(std::error_code& ec)
    {
        buffer_.clear();
        buffer_.resize(buffer_length_);
        size_t count = source_.read(buffer_.data(), buffer_length_);
        buffer_.resize(static_cast<size_t>(count));
        if (buffer_.size() == 0)
        {
            eof_ = true;
        }
        else if (begin_)
        {
            auto result = unicons::skip_bom(buffer_.begin(), buffer_.end());
            if (result.ec != unicons::encoding_errc())
            {
                ec = result.ec;
                return;
            }
            size_t offset = result.it - buffer_.begin();
            parser_.update(buffer_.data()+offset,buffer_.size()-offset);
            begin_ = false;
        }
        else
        {
            parser_.update(buffer_.data(),buffer_.size());
        }
    }

};

typedef basic_csv_reader<char> csv_reader;
typedef basic_csv_reader<wchar_t> wcsv_reader;

#if !defined(JSONCONS_NO_DEPRECATED)
JSONCONS_DEPRECATED_MSG("Instead, use csv_reader") typedef csv_reader csv_string_reader;
JSONCONS_DEPRECATED_MSG("Instead, use wcsv_reader") typedef wcsv_reader wcsv_string_reader;
#endif

}}

#endif

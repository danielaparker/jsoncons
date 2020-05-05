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
#include <jsoncons/json_visitor.hpp>
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
    using char_type = CharT;
    using temp_allocator_type = Allocator;
    typedef typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<CharT> char_allocator_type;

    basic_csv_reader(const basic_csv_reader&) = delete; 
    basic_csv_reader& operator = (const basic_csv_reader&) = delete; 

    basic_default_json_visitor<CharT> default_visitor_;

    basic_json_visitor<CharT>& visitor_;

    basic_csv_parser<CharT,Allocator> parser_;
    Src source_;
    std::size_t buffer_length_;
    bool eof_;
    bool begin_;
    std::vector<CharT, char_allocator_type> buffer_;
public:
    // Structural characters
    static constexpr size_t default_max_buffer_length = 16384;
    //!  Parse an input stream of CSV text into a json object
    /*!
      \param is The input stream to read from
    */

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_visitor<CharT>& visitor, 
                     const Allocator& alloc = Allocator())

       : basic_csv_reader(std::forward<Source>(source), 
                          visitor, 
                          basic_csv_decode_options<CharT>(), 
                          default_csv_parsing(), 
                          alloc)
    {
    }

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_visitor<CharT>& visitor,
                     const basic_csv_decode_options<CharT>& options, 
                     const Allocator& alloc = Allocator())

        : basic_csv_reader(std::forward<Source>(source), 
                           visitor, 
                           options, 
                           default_csv_parsing(),
                           alloc)
    {
    }

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_visitor<CharT>& visitor,
                     std::function<bool(csv_errc,const ser_context&)> err_handler, 
                     const Allocator& alloc = Allocator())
        : basic_csv_reader(std::forward<Source>(source), 
                           visitor, 
                           basic_csv_decode_options<CharT>(), 
                           err_handler,
                           alloc)
    {
    }

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_visitor<CharT>& visitor,
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler, 
                     const Allocator& alloc = Allocator(),
                     typename std::enable_if<!std::is_constructible<basic_string_view<CharT>,Source>::value>::type* = 0)
       : visitor_(visitor),
         parser_(options, err_handler, alloc),
         source_(std::forward<Source>(source)),
         buffer_length_(default_max_buffer_length),
         eof_(false),
         begin_(true),
         buffer_(alloc)
    {
        buffer_.reserve(buffer_length_);
    }

    template <class Source>
    basic_csv_reader(Source&& source,
                     basic_json_visitor<CharT>& visitor,
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler, 
                     const Allocator& alloc = Allocator(),
                     typename std::enable_if<std::is_constructible<basic_string_view<CharT>,Source>::value>::type* = 0)
       : visitor_(visitor),
         parser_(options, err_handler, alloc),
         buffer_length_(0),
         eof_(false),
         begin_(false),
         buffer_(alloc)
    {
        jsoncons::basic_string_view<CharT> sv(std::forward<Source>(source));
        auto result = unicons::skip_bom(sv.begin(), sv.end());
        if (result.ec != unicons::encoding_errc())
        {
            JSONCONS_THROW(ser_error(result.ec,parser_.line(),parser_.column()));
        }
        std::size_t offset = result.it - sv.begin();
        parser_.update(sv.data()+offset,sv.size()-offset);
    }

    ~basic_csv_reader() noexcept = default;

    void read()
    {
        std::error_code ec;
        read(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,parser_.line(),parser_.column()));
        }
    }

    void read(std::error_code& ec)
    {
        read_internal(ec);
    }

    bool eof() const
    {
        return eof_;
    }

    std::size_t buffer_length() const
    {
        return buffer_length_;
    }

    void buffer_length(std::size_t length)
    {
        buffer_length_ = length;
        buffer_.reserve(buffer_length_);
    }

#if !defined(JSONCONS_NO_DEPRECATED)

    JSONCONS_DEPRECATED_MSG("Instead, use buffer_length()")
    std::size_t buffer_capacity() const
    {
        return buffer_length_;
    }

    JSONCONS_DEPRECATED_MSG("Instead, use buffer_length(std::size_t)")
    void buffer_capacity(std::size_t length)
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
            parser_.parse_some(visitor_, ec);
            if (ec) return;
        }
    }
    void read_buffer(std::error_code& ec)
    {
        buffer_.clear();
        buffer_.resize(buffer_length_);
        std::size_t count = source_.read(buffer_.data(), buffer_length_);
        buffer_.resize(static_cast<std::size_t>(count));
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
            std::size_t offset = result.it - buffer_.begin();
            parser_.update(buffer_.data()+offset,buffer_.size()-offset);
            begin_ = false;
        }
        else
        {
            parser_.update(buffer_.data(),buffer_.size());
        }
    }

};

using csv_reader = basic_csv_reader<char>;
using wcsv_reader = basic_csv_reader<wchar_t>;

#if !defined(JSONCONS_NO_DEPRECATED)
JSONCONS_DEPRECATED_MSG("Instead, use csv_reader") typedef csv_reader csv_string_reader;
JSONCONS_DEPRECATED_MSG("Instead, use wcsv_reader") typedef wcsv_reader wcsv_string_reader;
#endif

}}

#endif

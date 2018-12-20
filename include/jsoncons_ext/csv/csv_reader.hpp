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
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons_ext/csv/csv_error.hpp>
#include <jsoncons_ext/csv/csv_parser.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons_ext/csv/csv_options.hpp>

namespace jsoncons { namespace csv {

template<class CharT,class Source,class Allocator=std::allocator<char>>
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

    default_parse_error_handler default_err_handler_;

    basic_csv_parser<CharT,Allocator> parser_;
    Source source_;
    std::vector<CharT,char_allocator_type> buffer_;
    size_t buffer_length_;
    size_t buffer_position_;
    bool eof_;
public:
    // Structural characters
    static const size_t default_max_buffer_length = 16384;
    //!  Parse an input stream of CSV text into a json object
    /*!
      \param is The input stream to read from
    */

    basic_csv_reader(Source source,
                     basic_json_content_handler<CharT>& handler)

       : basic_csv_reader(std::move(source), 
                          handler, 
                          basic_csv_options<CharT>(), 
                          default_err_handler_)
    {
    }

    basic_csv_reader(Source source,
                     basic_json_content_handler<CharT>& handler,
                     const basic_csv_options<CharT>& options)

        : basic_csv_reader(std::move(source), 
                           handler, 
                           options, 
                           default_err_handler_)
    {
    }

    basic_csv_reader(Source source,
                     basic_json_content_handler<CharT>& handler,
                     parse_error_handler& err_handler)
        : basic_csv_reader(std::move(source), 
                           handler, 
                           basic_csv_options<CharT>(), 
                           err_handler)
    {
    }

    basic_csv_reader(Source source,
                     basic_json_content_handler<CharT>& handler,
                     const basic_csv_options<CharT>& options,
                     parse_error_handler& err_handler)
       :
         parser_(handler, options, err_handler),
         source_(std::move(source)),
         buffer_length_(default_max_buffer_length),
         buffer_position_(0),
         eof_(false)
    {
        buffer_.reserve(buffer_length_);
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
            throw serialization_error(ec,parser_.line_number(),parser_.column_number());
        }
    }

    void read(std::error_code& ec)
    {
        try
        {
            read_internal(ec);
        }
        catch (const serialization_error& e)
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

    size_t buffer_capacity() const
    {
        return buffer_length_;
    }

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
        if (ec)
        {
            return;
        }
        while (!parser_.stopped())
        {
            if (parser_.source_exhausted())
            {
                if (!source_.eof())
                {
                    buffer_.clear();
                    buffer_.resize(buffer_length_);
                    size_t count = source_.read(buffer_.data(), buffer_length_);
                    buffer_.resize(count);
                    if (buffer_.size() == 0)
                    {
                        eof_ = true;
                    }
                    parser_.update(buffer_.data(),buffer_.size());
                }
                else
                {
                    parser_.update(buffer_.data(),0);
                    eof_ = true;
                }
            }
            parser_.parse_some(ec);
            if (ec) return;
        }
    }

};

template <class Json>
Json decode_csv(typename Json::string_view_type s)
{
    typedef typename Json::char_type char_type;

    json_decoder<Json> decoder;

    basic_csv_reader<char_type,jsoncons::string_source<char_type>> reader(s,decoder);
    reader.read();
    return decoder.get_result();
}

template <class Json,class Allocator=std::allocator<char>>
Json decode_csv(typename Json::string_view_type s, const basic_csv_options<typename Json::char_type>& options)
{
    typedef typename Json::char_type char_type;

    json_decoder<Json> decoder;

    basic_csv_reader<char_type,jsoncons::string_source<char_type>> reader(s,decoder,options);
    reader.read();
    return decoder.get_result();
}

template <class Json>
Json decode_csv(std::basic_istream<typename Json::char_type>& is)
{
    typedef typename Json::char_type char_type;

    json_decoder<Json> decoder;

    basic_csv_reader<char_type,jsoncons::text_stream_source<char_type>> reader(is,decoder);
    reader.read();
    return decoder.get_result();
}

template <class Json,class Allocator = std::allocator<char>>
Json decode_csv(std::basic_istream<typename Json::char_type>& is, const basic_csv_options<typename Json::char_type>& options)
{
    typedef typename Json::char_type char_type;

    json_decoder<Json,Allocator> decoder;

    basic_csv_reader<char_type,jsoncons::text_stream_source<char_type>,Allocator> reader(is,decoder,options);
    reader.read();
    return decoder.get_result();
}

typedef basic_csv_reader<char,jsoncons::text_stream_source<char>> csv_reader;
typedef basic_csv_reader<char,jsoncons::string_source<char>> csv_string_reader;
typedef basic_csv_reader<wchar_t,jsoncons::text_stream_source<wchar_t>> wcsv_reader;
typedef basic_csv_reader<wchar_t,jsoncons::string_source<wchar_t>> wcsv_string_reader;

}}

#endif

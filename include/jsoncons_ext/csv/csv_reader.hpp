// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_READER_HPP
#define JSONCONS_CSV_CSV_READER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons_ext/csv/csv_error_category.hpp>
#include <jsoncons_ext/csv/csv_parser.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons_ext/csv/csv_serializing_options.hpp>

namespace jsoncons { namespace csv {

template<class CharT,class Allocator=std::allocator<char>>
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
    std::basic_istream<CharT>& is_;
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

    basic_csv_reader(std::basic_istream<CharT>& is,
                     basic_json_content_handler<CharT>& handler)

       : basic_csv_reader(is, 
                          handler, 
                          basic_csv_serializing_options<CharT,Allocator>(), 
                          default_err_handler_)
    {
    }

    basic_csv_reader(std::basic_istream<CharT>& is,
                     basic_json_content_handler<CharT>& handler,
                     const basic_csv_serializing_options<CharT,Allocator>& options)

        : basic_csv_reader(is, 
                           handler, 
                           options, 
                           default_err_handler_)
    {
    }

    basic_csv_reader(std::basic_istream<CharT>& is,
                     basic_json_content_handler<CharT>& handler,
                     parse_error_handler& err_handler)
        : basic_csv_reader(is, 
                           handler, 
                           basic_csv_serializing_options<CharT,Allocator>(), 
                           err_handler)
    {
    }

    basic_csv_reader(std::basic_istream<CharT>& is,
                     basic_json_content_handler<CharT>& handler,
                     basic_csv_serializing_options<CharT,Allocator> options,
                     parse_error_handler& err_handler)
       :
         parser_(handler, options, err_handler),
         is_(is),
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
            throw parse_error(ec,parser_.line_number(),parser_.column_number());
        }
    }

    void read(std::error_code& ec)
    {
        parser_.reset();
        while (!eof_ && !parser_.done())
        {
            if (parser_.source_exhausted())
            {
                if (!is_.eof())
                {
                    buffer_.clear();
                    buffer_.resize(buffer_length_);
                    is_.read(buffer_.data(), buffer_length_);
                    buffer_.resize(static_cast<size_t>(is_.gcount()));
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
            if (!eof_)
            {
                parser_.parse(ec);
            }
        }
        parser_.end_parse();
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
};

template <class Json>
Json decode_csv(typename Json::string_view_type s)
{
    json_decoder<Json> decoder;

    basic_csv_parser<typename Json::char_type> parser(decoder);
    parser.reset();
    parser.parse(s.data(), 0, s.size());
    parser.end_parse();
    return decoder.get_result();
}

template <class Json,class Allocator>
Json decode_csv(typename Json::string_view_type s, const basic_csv_serializing_options<typename Json::char_type,Allocator>& options)
{
    json_decoder<Json,Allocator> decoder;

    basic_csv_parser<typename Json::char_type,Allocator> parser(decoder, options);
    parser.reset();
    parser.update(s.data(), s.size());
    parser.parse();
    parser.end_parse();
    return decoder.get_result();
}

template <class Json>
Json decode_csv(std::basic_istream<typename Json::char_type>& is)
{
    json_decoder<Json> decoder;

    basic_csv_reader<typename Json::char_type> reader(is,decoder);
    reader.read();
    return decoder.get_result();
}

template <class Json,class Allocator>
Json decode_csv(std::basic_istream<typename Json::char_type>& is, const basic_csv_serializing_options<typename Json::char_type,Allocator>& options)
{
    json_decoder<Json,Allocator> decoder;

    basic_csv_reader<typename Json::char_type,Allocator> reader(is,decoder,options);
    reader.read();
    return decoder.get_result();
}

typedef basic_csv_reader<char> csv_reader;
typedef basic_csv_reader<wchar_t> wcsv_reader;

}}

#endif

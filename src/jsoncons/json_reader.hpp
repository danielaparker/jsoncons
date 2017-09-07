// Copyright 2015 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_READER_HPP
#define JSONCONS_JSON_READER_HPP

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include <ios>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_input_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons/json_parser.hpp>

namespace jsoncons {

template<class CharT>
class basic_json_reader 
{
    static const size_t default_max_buffer_length = 16384;

    basic_json_parser<CharT> parser_;
    std::basic_istream<CharT>& is_;
    bool eof_;
    std::vector<CharT> buffer_;
    size_t buffer_length_;
    size_t buffer_capacity_;
    bool begin_;

    // Noncopyable and nonmoveable
    basic_json_reader(const basic_json_reader&) = delete;
    basic_json_reader& operator=(const basic_json_reader&) = delete;

public:

    basic_json_reader(std::basic_istream<CharT>& is)
        : parser_(),
          is_(is),
          eof_(false),
          buffer_length_(0),
          buffer_capacity_(default_max_buffer_length),
          begin_(true)
    {
        buffer_.resize(buffer_capacity_);
    }

    basic_json_reader(std::basic_istream<CharT>& is,
                      basic_parse_error_handler<CharT>& err_handler)
       : parser_(err_handler),
         is_(is),
         eof_(false),
         buffer_length_(0),
         buffer_capacity_(default_max_buffer_length),
         begin_(true)
    {
        buffer_.resize(buffer_capacity_);
    }

    basic_json_reader(std::basic_istream<CharT>& is,
                      basic_json_input_handler<CharT>& handler)
        : parser_(handler),
          is_(is),
          eof_(false),
          buffer_length_(0),
          buffer_capacity_(default_max_buffer_length),
          begin_(true)
    {
        buffer_.resize(buffer_capacity_);
    }

    basic_json_reader(std::basic_istream<CharT>& is,
                      basic_json_input_handler<CharT>& handler,
                      basic_parse_error_handler<CharT>& err_handler)
       : parser_(handler,err_handler),
         is_(is),
         eof_(false),
         buffer_length_(0),
         buffer_capacity_(default_max_buffer_length),
         begin_(true)
    {
        buffer_.resize(buffer_capacity_);
    }

    size_t buffer_capacity() const
    {
        return buffer_capacity_;
    }

    void buffer_capacity(size_t capacity)
    {
        buffer_capacity_ = capacity;
        buffer_.resize(buffer_capacity_);
    }

    size_t max_nesting_depth() const
    {
        return parser_.max_nesting_depth();
    }

    void max_nesting_depth(size_t depth)
    {
        parser_.max_nesting_depth(depth);
    }

    void read_next()
    {
        std::error_code ec;
        read_next(ec);
        if (ec)
        {
            throw parse_error(ec,parser_.line_number(),parser_.column_number());
        }
    }

    void read_next(std::error_code& ec)
    {
        parser_.reset();
        while (!eof_ && !parser_.done())
        {
            if (parser_.source_exhausted())
            {
                if (!is_.eof())
                {
                    if (is_.fail())
                    {
                        ec = json_parser_errc::source_error;
                        return;
                    }        
                    is_.read(buffer_.data(), buffer_capacity_);
                    buffer_length_ = static_cast<size_t>(is_.gcount());
                    if (buffer_length_ == 0)
                    {
                        eof_ = true;
                    }
                    else if (begin_)
                    {
                        auto result = unicons::skip_bom(buffer_.data(), buffer_.data()+buffer_length_);
                        switch (result.first)
                        {
                        case unicons::encoding_errc::expected_u8_found_u16:
                            ec = json_parser_errc::expected_u8_found_u16;
                            return;
                        case unicons::encoding_errc::expected_u8_found_u32:
                            ec = json_parser_errc::expected_u8_found_u32;
                            return;
                        case unicons::encoding_errc::expected_u16_found_fffe:
                            ec = json_parser_errc::expected_u16_found_fffe;
                            return;
                        case unicons::encoding_errc::expected_u32_found_fffe:
                            ec = json_parser_errc::expected_u32_found_fffe;
                            return;
                        default: // ok
                            break;
                        }
                        size_t offset = result.second - buffer_.data();
                        parser_.set_source(buffer_.data()+offset,buffer_length_-offset);
                        parser_.set_column_number(offset+1);
                        begin_ = false;
                    }
                    else
                    {
                        parser_.set_source(buffer_.data(),buffer_length_);
                    }

                }
                else
                {
                    eof_ = true;
                }
            }
            if (!eof_)
            {
                parser_.parse(ec);
                if (ec) return;
            }
        }
        if (eof_)
        {
            parser_.end_parse(ec);
            if (ec) return;
        }
    }

    void check_done()
    {
        std::error_code ec;
        check_done(ec);
        if (ec)
        {
            throw parse_error(ec,parser_.line_number(),parser_.column_number());
        }
    }

    size_t line_number() const
    {
        return parser_.line_number();
    }

    size_t column_number() const
    {
        return parser_.column_number();
    }

    void check_done(std::error_code& ec)
    {
        if (eof_)
        {
            parser_.check_done(ec);
            if (ec) return;
        }
        else
        {
            while (!eof_)
            {
                if (parser_.source_exhausted())
                {
                    if (!is_.eof())
                    {
                        if (is_.fail())
                        {
                            ec = json_parser_errc::source_error;
                            return;
                        }        
                        is_.read(buffer_.data(), buffer_capacity_);
                        buffer_length_ = static_cast<size_t>(is_.gcount());
                        parser_.set_source(buffer_.data(),buffer_length_);
                        if (buffer_length_ == 0)
                        {
                            eof_ = true;
                        }
                    }
                    else
                    {
                        eof_ = true;
                    }
                }
                if (!eof_)
                {
                    parser_.check_done(ec);
                    if (ec) return;
                }
            }
        }
    }

    bool eof() const
    {
        return eof_;
    }

    void read()
    {
        read_next();
        check_done();
    }

    void read(std::error_code& ec)
    {
        read_next(ec);
        if (!ec)
        {
            check_done(ec);
        }
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    size_t max_depth() const
    {
        return parser_.max_nesting_depth();
    }

    void max_depth(size_t depth)
    {
        parser_.max_nesting_depth(depth);
    }
#endif

private:
};

typedef basic_json_reader<char> json_reader;
typedef basic_json_reader<wchar_t> wjson_reader;

}

#endif


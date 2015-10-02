// Copyright 2015 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

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
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/parse_error_handler.hpp"
#include "jsoncons/json_parser.hpp"

namespace jsoncons {

template<typename Char>
class basic_json_reader 
{
    static const size_t default_max_buffer_length = 16384;
public:
    basic_json_reader(std::basic_istream<Char>& is,
                      basic_json_input_handler<Char>& handler)
        : parser_(handler),
          is_(std::addressof(is)),
          buffer_capacity_(default_max_buffer_length),
          err_handler_(std::addressof(default_basic_parse_error_handler<Char>::instance())),
          eof_(false),
          buffer_length_(0),
          index_(0)
    {
        buffer_.resize(buffer_capacity_);
    }

    basic_json_reader(std::basic_istream<Char>& is,
                      basic_json_input_handler<Char>& handler,
                      basic_parse_error_handler<Char>& err_handler)
       : parser_(handler,err_handler),
         is_(std::addressof(is)),
         buffer_capacity_(default_max_buffer_length),
         err_handler_(std::addressof(err_handler)),
         eof_(false),
         buffer_length_(0),
         index_(0)
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

    size_t max_depth() const
    {
        return parser_.max_depth();
    }

    void max_depth(size_t depth)
    {
        parser_.max_depth(depth);
    }

    void read_next()
    {
        parser_.begin_parse();
        while (!eof_ && !parser_.done())
        {
            if (!(index_ < buffer_length_))
            {
                if (!is_->eof())
                {
                    is_->read(&buffer_[0], buffer_capacity_);
                    buffer_length_ = static_cast<size_t>(is_->gcount());
                    if (buffer_length_ == 0)
                    {
                        eof_ = true;
                    }
                    index_ = 0;
                }
                else
                {
                    eof_ = true;
                }
            }
            if (!eof_)
            {
                parser_.parse(&buffer_[0],index_,buffer_length_);
                index_ = parser_.index();
            }
        }
        parser_.end_parse();
    }

    void check_done()
    {
        while (!eof_)
        {
            if (!(index_ < buffer_length_))
            {
                if (!is_->eof())
                {
                    is_->read(&buffer_[0], buffer_capacity_);
                    buffer_length_ = static_cast<size_t>(is_->gcount());
                    if (buffer_length_ == 0)
                    {
                        eof_ = true;
                    }
                    index_ = 0;
                }
                else
                {
                    eof_ = true;
                }
            }
            if (!eof_)
            {
                parser_.check_done(&buffer_[0],index_,buffer_length_);
                index_ = parser_.index();
            }
        }
    }

    bool eof() const
    {
        return eof_;
    }

    // Deprecated
    void read()
    {
        read_next();
    }
private:
    basic_json_parser<Char> parser_;
    std::basic_istream<Char> *is_;
    basic_parse_error_handler<Char> *err_handler_;
    bool eof_;
    std::vector<Char> buffer_;
    size_t buffer_length_;
    size_t buffer_capacity_;
    size_t index_;
};

typedef basic_json_reader<char> json_reader;
typedef basic_json_reader<wchar_t> wjson_reader;

}

#endif


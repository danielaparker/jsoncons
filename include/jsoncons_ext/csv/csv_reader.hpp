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
#include <jsoncons/json_input_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons_ext/csv/csv_error_category.hpp>
#include <jsoncons_ext/csv/csv_parser.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons_ext/csv/csv_parameters.hpp>

namespace jsoncons { namespace csv {

template<class CharT>
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
    basic_csv_reader(const basic_csv_reader&) = delete; 
    basic_csv_reader& operator = (const basic_csv_reader&) = delete; 

    basic_csv_parser<CharT> parser_;
    std::basic_istream<CharT>& is_;
    std::vector<CharT> buffer_;
    size_t buffer_length_;
    size_t buffer_position_;
    bool eof_;
    size_t index_;
public:
    // Structural characters
    static const size_t default_max_buffer_length = 16384;
    //!  Parse an input stream of CSV text into a json object
    /*!
      \param is The input stream to read from
    */

    basic_csv_reader(std::basic_istream<CharT>& is,
                     basic_json_input_handler<CharT>& handler)

       : parser_(handler),
         is_(is),
         buffer_length_(default_max_buffer_length),
         buffer_position_(0),
         eof_(false),
         index_(0)
    {
        buffer_.reserve(buffer_length_);
    }

    basic_csv_reader(std::basic_istream<CharT>& is,
                     basic_json_input_handler<CharT>& handler,
                     basic_csv_parameters<CharT> params)

       : parser_(handler,params),
         is_(is),
         buffer_length_(default_max_buffer_length),
         buffer_position_(0),
         eof_(false),
         index_(0)
    {
        buffer_.reserve(buffer_length_);
    }

    basic_csv_reader(std::basic_istream<CharT>& is,
                     basic_json_input_handler<CharT>& handler,
                     parse_error_handler& err_handler)
       :
         parser_(handler,err_handler),
         is_(is),
         buffer_length_(default_max_buffer_length),
         buffer_position_(0),
         eof_(false),
         index_(0)
    {
        buffer_.reserve(buffer_length_);
    }

    basic_csv_reader(std::basic_istream<CharT>& is,
                     basic_json_input_handler<CharT>& handler,
                     parse_error_handler& err_handler,
                     basic_csv_parameters<CharT> params)
       :
         parser_(handler,err_handler,params),
         is_(is),
         buffer_length_(default_max_buffer_length),
         buffer_position_(0),
         eof_(false),
         index_(0)
    {
        buffer_.reserve(buffer_length_);
    }

    ~basic_csv_reader()
    {
    }

    void read()
    {
        parser_.reset();
        while (!eof_ && !parser_.done())
        {
            if (!(index_ < buffer_.size()))
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
                    index_ = 0;
                }
                else
                {
                    eof_ = true;
                }
            }
            if (!eof_)
            {
                parser_.parse(buffer_.data(),index_,buffer_.size());
                index_ = parser_.index();
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

typedef basic_csv_reader<char> csv_reader;

}}

#endif

// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_CSV_CSV_READER_HPP
#define JSONCONS_CSV_CSV_READER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/parse_error_handler.hpp"
#include "jsoncons_ext/csv/csv_error_category.hpp"
#include "jsoncons_ext/csv/csv_parser.hpp"
#include "jsoncons/json.hpp"

namespace jsoncons { namespace csv {

template<typename Char,class Alloc>
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
public:
    // Structural characters
    static const size_t default_max_buffer_length = 16384;
    //!  Parse an input stream of CSV text into a json object
    /*!
      \param is The input stream to read from
    */

    basic_csv_reader(std::basic_istream<Char>& is,
                     basic_json_input_handler<Char>& handler)

       : parser_(handler),
         is_(std::addressof(is)),
         buffer_(default_max_buffer_length),
         buffer_capacity_(default_max_buffer_length),
         buffer_position_(0),
         buffer_length_(0),
         eof_(false),
         index_(0)
    {
    }

    basic_csv_reader(std::basic_istream<Char>& is,
                     basic_json_input_handler<Char>& handler,
                     basic_csv_parameters<Char> params)

       : parser_(handler,params),
         is_(std::addressof(is)),
         buffer_(default_max_buffer_length),
         buffer_capacity_(default_max_buffer_length),
         buffer_position_(0),
         buffer_length_(0),
         eof_(false),
         index_(0)
    {
    }

    basic_csv_reader(std::basic_istream<Char>& is,
                     basic_json_input_handler<Char>& handler,
                     basic_parse_error_handler<Char>& err_handler)
       :
         parser_(handler,err_handler),
         is_(std::addressof(is)),
         buffer_(),
         buffer_capacity_(default_max_buffer_length),
         buffer_position_(0),
         buffer_length_(0),
         eof_(false),
         index_(0)


    {
    }

    basic_csv_reader(std::basic_istream<Char>& is,
                     basic_json_input_handler<Char>& handler,
                     basic_parse_error_handler<Char>& err_handler,
                     basic_csv_parameters<Char> params)
       :
         parser_(handler,err_handler,params),
         is_(std::addressof(is)),
         buffer_(),
         buffer_capacity_(default_max_buffer_length),
         buffer_position_(0),
         buffer_length_(0),
         eof_(false),
         index_(0)
    {
    }

    ~basic_csv_reader()
    {
    }

    void read()
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

    bool eof() const
    {
        return eof_;
    }

    size_t buffer_capacity() const
    {
        return buffer_capacity_;
    }

    void buffer_capacity(size_t buffer_capacity)
    {
        buffer_capacity_ = buffer_capacity;
    }

private:
    basic_csv_reader(const basic_csv_reader&); // noop
    basic_csv_reader& operator = (const basic_csv_reader&); // noop

    std::basic_istream<Char>* is_;
    std::vector<Char> buffer_;
    size_t buffer_capacity_;
    size_t index_;
    size_t buffer_position_;
    size_t buffer_length_;
    bool eof_;
    basic_csv_parser<Char> parser_;
};

typedef basic_csv_reader<char,std::allocator<void>> csv_reader;

}}

#endif

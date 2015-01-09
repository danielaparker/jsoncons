// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_EXT_CSV_CSV_READER_HPP
#define JSONCONS_EXT_CSV_CSV_READER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/parse_error_handler.hpp"
#include "jsoncons/json.hpp"

namespace jsoncons_ext { namespace csv {

template<typename Char,class Alloc>
class basic_csv_reader : private jsoncons::basic_parsing_context<Char>
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
                     jsoncons::basic_json_input_handler<Char>& handler)

       :
         minimum_structure_capacity_(0),
         column_(0),
         line_(0),
         string_buffer_(),
         stack_(),
         is_(std::addressof(is)),
         buffer_(default_max_buffer_length),
         buffer_capacity_(default_max_buffer_length),
         buffer_position_(0),
         buffer_length_(0),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(jsoncons::default_basic_parse_error_handler<Char>::instance())),
         assume_header_(),
         field_delimiter_(),
         quote_char_(),
         quote_escape_char_(),
         comment_symbol_(),
         bof_(true),
         eof_(false)
    {
        init(jsoncons::json::an_object);
    }

    basic_csv_reader(std::basic_istream<Char>& is,
                     jsoncons::basic_json_input_handler<Char>& handler,
                     const jsoncons::basic_json<Char,Alloc>& params)

       :
         minimum_structure_capacity_(0),
         column_(0),
         line_(0),
         string_buffer_(),
         stack_(),
         is_(std::addressof(is)),
         buffer_(default_max_buffer_length),
         buffer_capacity_(default_max_buffer_length),
         buffer_position_(0),
         buffer_length_(0),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(jsoncons::default_basic_parse_error_handler<Char>::instance())),
         assume_header_(),
         field_delimiter_(),
         quote_char_(),
         quote_escape_char_(),
         comment_symbol_(),
         bof_(true),
         eof_(false)
    {
        init(params);
    }

    basic_csv_reader(std::basic_istream<Char>& is,
                     jsoncons::basic_json_input_handler<Char>& handler,
                     jsoncons::basic_parse_error_handler<Char>& err_handler)
       :

         minimum_structure_capacity_(),
         column_(0),
         line_(0),
         string_buffer_(),
         stack_(),
         is_(std::addressof(is)),
         buffer_(),
         buffer_capacity_(default_max_buffer_length),
         buffer_position_(0),
         buffer_length_(0),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(err_handler)),
         assume_header_(),
         field_delimiter_(),
         quote_char_(),
         quote_escape_char_(),
         comment_symbol_(),
         bof_(true),
         eof_(false)


    {
        init(jsoncons::json::an_object);
    }

    basic_csv_reader(std::basic_istream<Char>& is,
                     jsoncons::basic_json_input_handler<Char>& handler,
                     jsoncons::basic_parse_error_handler<Char>& err_handler,
                     const jsoncons::basic_json<Char,Alloc>& params)
       :
         minimum_structure_capacity_(),
         column_(0),
         line_(0),
         string_buffer_(),
         stack_(),
         is_(std::addressof(is)),
         buffer_(),
         buffer_capacity_(default_max_buffer_length),
         buffer_position_(0),
         buffer_length_(0),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(err_handler)),
         assume_header_(),
         field_delimiter_(),
         quote_char_(),
         quote_escape_char_(),
         comment_symbol_(),
         bof_(true),
         eof_(false)
    {
        init(params);
    }

    void init(const jsoncons::basic_json<Char,Alloc>& params)
    {
        field_delimiter_ = params.get("field_delimiter",",").as_char();

        assume_header_ = params.get("has_header",false).as_bool();

        quote_char_ = params.get("quote_char","\"").as_char();

        quote_escape_char_ = params.get("quote_escape_char","\"").as_char();

        comment_symbol_ = params.get("comment_symbol","\0").as_char();
    }

    ~basic_csv_reader()
    {
    }

    void read();

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

    void read_array_of_arrays();
    void read_array_of_objects();

    void parse_string();
    void parse_quoted_string();
    void ignore_single_line_comment();

    void read_some()
    {
        if (buffer_position_ < buffer_length_)
        {
            return; // exhaust buffer first
        }
        buffer_position_ = 0;
        if (!is_->eof())
        {
            is_->read(&buffer_[0], buffer_capacity_);
            buffer_length_ = static_cast<size_t>(is_->gcount());
            if (bof_)
            {
                bof_ = false;
                if (buffer_length_ == 0)
                {
                    eof_ = true;
                }
            }
        }
        else
        {
            buffer_length_ = 0;
            eof_ = true;
        }
    }

    Char read_ch()
    {
        if (buffer_position_ >= buffer_length_)
        {
            read_some();
        }
        Char c = 0;

        //std::cout << "buffer_position = " << buffer_position_ << ", buffer_length=" << buffer_length_ << std::endl;
        if (buffer_position_ < buffer_length_)
        {
            c = buffer_[buffer_position_++];
        }
        else
        {
            buffer_position_++;
        }
        ++column_;

        return c;
    }

    Char peek()
    {
        if (buffer_position_ >= buffer_length_)
        {
            read_some();
        }
        Char c = 0;
        if (buffer_position_ < buffer_length_)
        {
            c = buffer_[buffer_position_];
        }

        return c;
    }

    void skip_ch()
    {
        read_ch();
    }

    void unread_last_ch()
    {
        if (buffer_position_ > 0)
        {
            --buffer_position_;
            --column_;
        }
    }

    virtual Char do_peek()
    {
        if (buffer_position_ >= buffer_length_)
        {
            read_some();
        }
        return buffer_[buffer_position_];
    }

    virtual bool do_eof() const
    {
        return eof_;
    }

    virtual Char do_last_char() const
    {
        return 0;
    }

    virtual unsigned long do_line_number() const
    {
        return line_;
    }

    virtual unsigned long do_column_number() const
    {
        return column_;
    }

    virtual size_t do_minimum_structure_capacity() const
    {
        return minimum_structure_capacity_;
    }

    size_t minimum_structure_capacity_;
    unsigned long column_;
    unsigned long line_;
    std::basic_string<Char> string_buffer_;
    std::vector<stack_item> stack_;
    std::basic_istream<Char>* is_;
    std::vector<Char> buffer_;
    size_t buffer_capacity_;
    size_t buffer_position_;
    size_t buffer_length_;
    jsoncons::basic_json_input_handler<Char>* handler_;
    jsoncons::basic_parse_error_handler<Char>* err_handler_;
    bool assume_header_;
    Char field_delimiter_;
    Char quote_char_;
    Char quote_escape_char_;
    Char comment_symbol_;
    bool bof_;
    bool eof_;
};

template<typename Char,class Alloc>
void basic_csv_reader<Char,Alloc>::read()
{
    if (is_->bad())
    {
        JSONCONS_THROW_EXCEPTION("Input stream is invalid");
    }
    buffer_.resize(buffer_capacity_);
    buffer_position_ = 0;
    buffer_length_ = 0;

    line_ = 1;
    column_ = 0;

    handler_->begin_json();
    stack_.push_back(stack_item());
    handler_->begin_array(*this);
    stack_.back().array_begun_ = true;
    if (assume_header_)
    {
        read_array_of_objects();
    }
    else
    {
        read_array_of_arrays();
    }
    handler_->end_array(*this);
    stack_.pop_back();
    handler_->end_json();
}

template<typename Char,class Alloc>
void basic_csv_reader<Char,Alloc>::read_array_of_arrays()
{
    size_t row_capacity = 0;
    stack_.push_back(stack_item());
    while (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            continue;
        }

        // Check new line
        if (c == '\r' && peek() == '\n')
        {
            skip_ch();
            ++line_;
            column_ = 0;
        }
        else if (c == '\n' || c == '\r')
        {
            ++line_;
            column_ = 0;
        }

        if (c == field_delimiter_)
        {
            //skip_ch();
			continue;
        }
        if (column_ == 0) // Just got newline
        {
            if (stack_.back().array_begun_)
            {
                handler_->end_array(*this);
            }
            stack_.pop_back();
            stack_.push_back(stack_item());
        }
        else if (column_ == 1 && c == comment_symbol_)
        {
            skip_ch();
            if (eof())
            {
                err_handler_->error(std::error_code(jsoncons::json_parser_errc::unexpected_eof, jsoncons::json_parser_category()), *this);
            }
            ignore_single_line_comment();
        }
        else
        {
            if (stack_.size() > 0)
            {
                if (line_ == 1)
                {
                    ++row_capacity;
                }
                if (c == quote_char_)
                {
                    parse_quoted_string();
                    if (!stack_.back().array_begun_)
                    {
                        minimum_structure_capacity_ = row_capacity;
                        handler_->begin_array(*this);
                        minimum_structure_capacity_ = 0;
                        stack_.back().array_begun_ = true;
                    }
                    handler_->value(string_buffer_.c_str(),string_buffer_.length(),*this);
                }
                else
                {
                    unread_last_ch();
                    parse_string();
                    if (!stack_.back().array_begun_)
                    {
                        minimum_structure_capacity_ = row_capacity;
                        handler_->begin_array(*this);
                        minimum_structure_capacity_ = 0;
                        stack_.back().array_begun_ = true;
                    }
                    handler_->value(string_buffer_.c_str(),string_buffer_.length(),*this);
                }
            }
        }
    }

    if (stack_.size() > 1)
    {
        if (stack_.back().array_begun_)
        {
            handler_->end_array(*this);
        }
        stack_.pop_back();
    }
}

template<typename Char,class Alloc>
void basic_csv_reader<Char,Alloc>::read_array_of_objects()
{
    std::vector<std::basic_string<Char>> header;
    size_t row_index = 0;
    size_t column_index = 0;

    stack_.push_back(stack_item());
    while (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            continue;
        }

        // Check new line
        if (c == '\r' && peek() == '\n')
        {
            skip_ch();
            ++line_;
            column_ = 0;
        }
        else if (c == '\n' || c == '\r')
        {
            ++line_;
            column_ = 0;
        }
        if (c == field_delimiter_)
        {
            skip_ch();
        }
        if (column_ == 0) // Just got newline
        {
            if (stack_.back().array_begun_)
            {
                handler_->end_object(*this);
            }
            stack_.pop_back();
            stack_.push_back(stack_item());
            ++row_index;
            column_index = 0;
        }
        else if (c == quote_char_)
        {
            parse_quoted_string();
            if (!stack_.back().array_begun_)
            {
                minimum_structure_capacity_ = header.size();
                handler_->begin_object(*this);
                minimum_structure_capacity_ = 0;
                stack_.back().array_begun_ = true;
            }
            if (column_index < header.size())
            {
                handler_->name(header[column_index],*this);
                handler_->value(string_buffer_.c_str(),string_buffer_.length(),*this);
            }
            ++column_index;
        }
        else
        {
            unread_last_ch();
            parse_string();
            if (row_index == 0)
            {
                header.push_back(string_buffer_);
            }
            else
            {
                if (!stack_.back().array_begun_)
                {
                    minimum_structure_capacity_ = header.size();
                    handler_->begin_object(*this);
                    minimum_structure_capacity_ = 0;
                    stack_.back().array_begun_ = true;
                }
                if (column_index < header.size())
                {
                    handler_->name(header[column_index],*this);
                    handler_->value(string_buffer_.c_str(),string_buffer_.length(),*this);
                }
            }
            ++column_index;
        }

    }

    if (stack_.size() > 1)
    {
        if (stack_.back().array_begun_)
        {
            handler_->end_object(*this);
        }
        stack_.pop_back();
    }
}

template<typename Char,class Alloc>
void basic_csv_reader<Char,Alloc>::parse_string()
{
    string_buffer_.clear();

    bool done = false;
    while (!done)
    {
        Char c = read_ch();
        if (eof())
        {
            done = true;
        }
        // Check new line
        if (c == '\r' && peek() == '\n')
        {
            done = true;
            unread_last_ch();
        }
        else if (c == '\n' || c == '\r')
        {
            done = true;
            unread_last_ch();
        }
        else if (c == field_delimiter_)
        {
            done = true;
            unread_last_ch();
        }
        else
        {
            string_buffer_.push_back(c);
        }
    }
}

template<typename Char,class Alloc>
void basic_csv_reader<Char,Alloc>::parse_quoted_string()
{
    string_buffer_.clear();

    bool done_string = false;
    //std::cout << "start quoted string" << std::endl;
    while (!done_string)
    {
        Char c = read_ch();
        if (eof())
        {
            err_handler_->error(std::error_code(jsoncons::json_parser_errc::eof_reading_string_value, jsoncons::json_parser_category()), *this);
        }
        else if (c == quote_escape_char_ && peek() == quote_char_)
        {
            string_buffer_.push_back(quote_char_);
            skip_ch();
        }
        else if (c == quote_char_)
        {
            done_string = true;
        }
        else
        {
            //std::cout << c;
            string_buffer_.push_back(c);
        }
    }
    //std::cout << std::endl << "end string" << std::endl;
    bool done = false;
    while (!done)
    {
        Char c = read_ch();
        if (eof())
        {
            done = true;
        }
        // Check new line
        if (c == '\r' && peek() == '\n')
        {
            done = true;
            unread_last_ch();
        }
        else if (c == '\n' || c == '\r')
        {
            done = true;
            unread_last_ch();
        }
        else if (c == field_delimiter_)
        {
            done = true;
            unread_last_ch();
        }
    }
}

template<typename Char,class Alloc>
void basic_csv_reader<Char,Alloc>::ignore_single_line_comment()
{
    bool done = false;
    while (!done)
    {
        Char c = read_ch();
        if (eof())
        {
            done = true;
        }
        // Check new line
        if (c == '\r' && peek() == '\n')
        {
            done = true;
            unread_last_ch();
        }
        else if (c == '\n' || c == '\r')
        {
            done = true;
            unread_last_ch();
        }
        else if (c == field_delimiter_)
        {
            done = true;
            unread_last_ch();
        }
    }

}

typedef basic_csv_reader<char,std::allocator<void>> csv_reader;

}}

#endif

// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_CSV_READER_HPP
#define JSONCONS_CSV_READER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include "jsoncons/jsoncons_config.hpp"
#include "jsoncons/json_char_traits.hpp"
#include "jsoncons/json_listener.hpp"
#include "jsoncons/error_handler.hpp"
#include "jsoncons/json_reader.hpp"

namespace jsoncons {

template<class Char>
class basic_csv_reader : private basic_parsing_context<Char>
{
    static default_error_handler default_err_handler;

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
    //!  Parse an input stream of JSON text into a json object
    /*!
      \param is The input stream to read from
    */
    basic_csv_reader(const basic_json<Char>& params,
                     std::basic_istream<Char>& is,
                     basic_json_listener<Char>& handler,
                     basic_error_handler<Char>& err_handler)
       : is_(is), 
         handler_(handler), 
         err_handler_(err_handler),
         input_buffer_(0), 
         buffer_position_(0), 
         buffer_length_(0)
    {
        input_buffer_ = new Char[buffer_capacity_];

        const basic_json<Char>& vs = params.get("field_separator",",");
        value_separator_ = vs.is_empty() ? ',' : vs.as_string()[0];

        assume_header_ = params.get("assume_header",false).as_bool();

        const basic_json<Char>& qc = params.get("quote_char","\"");
        quote_char_ = qc.is_empty() ? '\"' : qc.as_string()[0];
    }
    basic_csv_reader(const basic_json<Char>& params,
                     std::basic_istream<Char>& is,
                     basic_json_listener<Char>& handler)
        
       : is_(is), 
         handler_(handler), 
         err_handler_(default_err_handler), 
         input_buffer_(0), 
         buffer_position_(0), 
         buffer_length_(0)
    {
        input_buffer_ = new Char[buffer_capacity_];

        const basic_json<Char>& vs = params.get("field_separator",",");
        value_separator_ = vs.is_empty() ? ',' : vs.as_string()[0];

        assume_header_ = params.get("assume_header",false).as_bool();

        const basic_json<Char>& qc = params.get("quote_char","\"");
        quote_char_ = qc.is_empty() ? '\"' : qc.as_string()[0];
    }

    ~basic_csv_reader()
    {
        delete[] input_buffer_;
    }

    void read();

    bool eof() const
    {
        return buffer_position_ > buffer_length_ && is_.eof();
    }

    size_t buffer_capacity() const
    {
        return buffer_capacity_;
    }

    void buffer_capacity(size_t buffer_capacity)
    {
        buffer_capacity_ = buffer_capacity;
    }

    virtual unsigned long line_number() const
    {
        return line_;
    }

    virtual unsigned long column_number() const
    {
        return column_;
    }

    virtual const std::basic_string<Char>& buffer() const
    {
        return string_buffer_;
    }

private:
    basic_csv_reader(const basic_csv_reader&); // noop
    basic_csv_reader& operator = (const basic_csv_reader&); // noop

    void read_array_of_arrays();
    void read_array_of_objects();

    void skip_separator();
    void parse_string();
    void parse_quoted_string();
    void ignore_single_line_comment();
    void ignore_multi_line_comment();
    void fast_ignore_single_line_comment();
    void fast_ignore_multi_line_comment();
    void fast_skip_white_space();
    unsigned int decode_unicode_codepoint();
    unsigned int decode_unicode_escape_sequence();

    void read_data_block()
    {
        buffer_position_ = 0;
        if (!is_.eof())
        {
            is_.read(input_buffer_, buffer_capacity_);
            buffer_length_ = static_cast<int>(is_.gcount());
        }
        else
        {
            buffer_length_ = 0;
        }
    }

    Char read_ch()
    {
        if (buffer_position_ >= buffer_length_)
        {
            read_data_block();
        }
        Char c = 0;

        //std::cout << "buffer_position = " << buffer_position_ << ", buffer_length=" << buffer_length_ << std::endl;
        if (buffer_position_ < buffer_length_)
        {
            c = input_buffer_[buffer_position_++];
            if (c == '\n')
            {
                ++line_;
                column_ = 0;
            }
            ++column_;
        }
        else
        {
            buffer_position_++;
        }

        return c;
    }

    Char peek()
    {
        if (buffer_position_ >= buffer_length_)
        {
            read_data_block();
        }
        Char c = 0;
        if (buffer_position_ < buffer_length_)
        {
            c = input_buffer_[buffer_position_];
        }

        return c;
    }

    void skip_ch()
    {
        read_ch();
    }

    void unread_ch(Char ch)
    {
        if (buffer_position_ > 0)
        {
            --buffer_position_;
            --column_;
            if (ch == '\n')
            {
                --line_;
                column_ = 0;
            }
        }
    }

    unsigned long column_;
    unsigned long line_;
    std::basic_string<Char> string_buffer_;
    std::vector<stack_item> stack_;
    std::basic_istream<Char>& is_;
    Char *input_buffer_;
    size_t buffer_capacity_;
    int buffer_position_;
    int buffer_length_;
    basic_json_listener<Char>& handler_;
    basic_error_handler<Char>& err_handler_;
    char value_separator_;
    bool assume_header_;
    char quote_char_;
};

template<class Char>
default_error_handler basic_csv_reader<Char>::default_err_handler;

template<class Char>
void basic_csv_reader<Char>::read()
{
    line_ = 1;
    column_ = 0;

    handler_.begin_json();
    stack_.push_back(stack_item());
    handler_.begin_array(*this);
    stack_.back().array_begun_ = true;
    if (assume_header_)
    {
        read_array_of_objects();
    }
    else
    {
        read_array_of_arrays();
    }
    handler_.end_array(*this);
    stack_.pop_back();
    handler_.end_json();
}

template<class Char>
void basic_csv_reader<Char>::read_array_of_arrays()
{
    stack_.push_back(stack_item());
    while (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            continue;
        }
        switch (c)
        {
        case '\n':
            if (stack_.back().array_begun_)
            {
                handler_.end_array(*this);
            }
            stack_.pop_back();
            stack_.push_back(stack_item());
            break;
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            fast_skip_white_space();
            continue;
        case '/':
            {
                Char next = peek();
                if (eof())
                {
                    err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
                }
                if (next == '/')
                {
                    skip_ch();
                    if (eof())
                    {
                        err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
                    }
                    ignore_single_line_comment();
                }
                if (next == '*')
                {
                    skip_ch();
                    if (eof())
                    {
                        err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
                    }
                    ignore_multi_line_comment();
                }
            }
            continue;
        default:
            if (stack_.size() > 0)
            {
                if (c == quote_char_)
                {
                    parse_quoted_string();
                    if (!stack_.back().array_begun_)
                    {
                        handler_.begin_array(*this);
                        stack_.back().array_begun_ = true;
                    }
                    handler_.value(string_buffer_,*this);
                }
                else
                {
                    unread_ch(c);
                    parse_string();
                    if (!stack_.back().array_begun_)
                    {
                        handler_.begin_array(*this);
                        stack_.back().array_begun_ = true;
                    }
                    handler_.value(string_buffer_,*this);
                }
            }
        }
    }

    if (stack_.size() > 1)
    {
        if (stack_.back().array_begun_)
        {
            handler_.end_array(*this);
        }
        stack_.pop_back();
    }
}

template<class Char>
void basic_csv_reader<Char>::read_array_of_objects()
{
    std::vector<std::string> header;
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
        switch (c)
        {
        case '\n':
            ++row_index;
            column_index = 0;
            if (stack_.back().array_begun_)
            {
                handler_.end_object(*this);
            }
            stack_.pop_back();
            stack_.push_back(stack_item());
            break;
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            fast_skip_white_space();
            continue;
        case '/':
            {
                Char next = peek();
                if (eof())
                {
                    err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
                }
                if (next == '/')
                {
                    skip_ch();
                    if (eof())
                    {
                        err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
                    }
                    ignore_single_line_comment();
                }
                if (next == '*')
                {
                    skip_ch();
                    if (eof())
                    {
                        err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
                    }
                    ignore_multi_line_comment();
                }
            }
            continue;
        default:
            if (stack_.size() > 0)
            {
                if (c == quote_char_)
                {
                    parse_quoted_string();
                    if (!stack_.back().array_begun_)
                    {
                        handler_.begin_object(*this);
                        stack_.back().array_begun_ = true;
                    }
                    handler_.value(string_buffer_,*this);
                }
                else
                {
                    unread_ch(c);
                    parse_string();
                    if (row_index == 0)
                    {
                        header.push_back(string_buffer_);
                    }
                    else
                    {
                        if (!stack_.back().array_begun_)
                        {
                            handler_.begin_object(*this);
                            stack_.back().array_begun_ = true;
                        }
                        if (column_index < header.size())
                        {
                            handler_.name(header[column_index],*this);
                            handler_.value(string_buffer_,*this);
                        }
						++column_index;
                    }
                }
            }
        }
    }

    if (stack_.size() > 1)
    {
        if (stack_.back().array_begun_)
        {
            handler_.end_object(*this);
        }
        stack_.pop_back();
    }
}

template<class Char>
void basic_csv_reader<Char>::skip_separator()
{
    while (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            err_handler_.fatal_error("JPE101", "Unexpected EOF, expected :", *this);
        }
        switch (c)
        {
        case '\n':
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            fast_skip_white_space();
            continue;
        case '/':
            {
                if (!eof())
                {
                    Char next = peek();
                    if (next == '/')
                    {
                        ignore_single_line_comment();
                    }
                    if (next == '/')
                    {
                        ignore_multi_line_comment();
                    }
                }
            }
            break;
        default:
            err_handler_.fatal_error("JPE106", "Expected :", *this);
        }
    }

    err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
}

template<class Char>
void basic_csv_reader<Char>::parse_string()
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
        else if (c == '\r')
        {
        }
        else if (c == '\n')
        {
            done = true;
            unread_ch(c);
        }
        else if (c == value_separator_)
        {
            done = true;
        }
        else 
        {
            string_buffer_.push_back(c);
        }
    }
}

template<class Char>
void basic_csv_reader<Char>::parse_quoted_string()
{
    string_buffer_.clear();

    bool done_string = false;
    while (!done_string)
    {
        Char c = read_ch();
        if (eof())
        {
            err_handler_.fatal_error("JPE101", "EOF, expected quote character", *this);
        }
        else if (c == quote_char_)
        {
            if (peek() == quote_char_)
            {
                string_buffer_.push_back(quote_char_);
                skip_ch();
            }
            else
            {
                done_string = true;
            }
        } 
        else
        {
            string_buffer_.push_back(c);
        }
    }
    bool done = false;
    while (!done)
    {
        Char c = read_ch();
        if (eof())
        {
            done = true;
        }
        else if (c == '\n')
        {
            done = true;
            unread_ch(c);
        }
    }
}

template<class Char>
void basic_csv_reader<Char>::ignore_single_line_comment()
{
    bool done = false;
    while (!done)
    {
        Char c = read_ch();
        if (eof())
        {
            err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
        }
        if (c == '\n')
        {
            done = true;
            break;
        }
    }
}

template<class Char>
void basic_csv_reader<Char>::fast_ignore_single_line_comment()
{
    while (buffer_position_ < buffer_length_)
    {
        if (input_buffer_[buffer_position_] == '\n')
        {
            break;
        }
        ++buffer_position_;
        ++column_;
    }
}

template<class Char>
void basic_csv_reader<Char>::fast_ignore_multi_line_comment()
{
    while (buffer_position_ < buffer_length_)
    {
        if (input_buffer_[buffer_position_] == '*')
        {
            break;
        }
        if (input_buffer_[buffer_position_] == '\n')
        {
            ++line_;
            column_ = 0;
        }
        ++buffer_position_;
        ++column_;
    }
}

template<class Char>
void basic_csv_reader<Char>::ignore_multi_line_comment()
{
    bool done = false;
    while (!done)
    {
        Char c = read_ch();
        if (eof())
        {
            err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
        }
        if (c == '*')
        {
            Char next = peek();
            if (eof())
            {
                err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
            }
            if (next == '/')
            {
                done = true;
            }
            break;
        }
    }
}

template<class Char>
void basic_csv_reader<Char>::fast_skip_white_space()
{
    bool done = false;
    while (!done && buffer_position_ < buffer_length_)
    {
        switch (input_buffer_[buffer_position_])
        {
        case '\n':
            ++line_;
            column_ = 0;
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            ++buffer_position_;
            ++column_;
            break;
        default:
            done = true;
            break;
        }
    }
}

template<class Char>
unsigned int basic_csv_reader<Char>::decode_unicode_codepoint()
{

    unsigned int cp = decode_unicode_escape_sequence();
    if (cp >= 0xD800 && cp <= 0xDBFF)
    {
        // surrogate pairs
        if (read_ch() == '\\' && read_ch() == 'u')
        {
            if (eof())
            {
                err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
            }
            unsigned int surrogate_pair = decode_unicode_escape_sequence();
            cp = 0x10000 + ((cp & 0x3FF) << 10) + (surrogate_pair & 0x3FF);
        }
        else
        {
            err_handler_.fatal_error("JPE202", "expecting another \\u token to begin the second half of a cp surrogate pair.", *this);
        }
    }
    return cp;
}

template<class Char>
unsigned int basic_csv_reader<Char>::decode_unicode_escape_sequence()
{
    unsigned int cp = 0;
    size_t index = 0;
    while (!eof() && index < 4)
    {
        Char c = read_ch();
        if (eof())
        {
            err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
        }
        const unsigned int u(c >= 0 ? c : 256 + c);
        cp *= 16;
        if (u >= '0'  &&  u <= '9')
        {
            cp += u - '0';
        }
        else if (u >= 'a'  &&  u <= 'f')
        {
            cp += u - 'a' + 10;
        }
        else if (u >= 'A'  &&  u <= 'F')
        {
            cp += u - 'A' + 10;
        }
        else
        {
            std::ostringstream os;
            os << "Expected hexadecimal digit, found " << u << ".";
            err_handler_.fatal_error("JPE202", os.str(), *this);
        }
        ++index;
    }
    if (index != 4)
    {
        err_handler_.fatal_error("JPE202", "Bad codepoint escape sequence in string: four digits expected.", *this);
    }
    return cp;
}

typedef basic_csv_reader<char> csv_reader;

}

#endif

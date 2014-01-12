// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_READER_HPP
#define JSONCONS_JSON_READER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include "jsoncons/jsoncons_config.hpp"
#include "jsoncons/json_char_traits.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/error_handler.hpp"
#include "jsoncons/json_exception.hpp"

namespace jsoncons {

template<class Char>
class basic_json_reader : private basic_parsing_context<Char>
{
    static default_error_handler default_err_handler;

    enum structure_type {object_t, array_t};
    struct stack_item
    {
        stack_item(structure_type type)
           : type_(type), count_(0), comma_(false)
        {
            received_name_ = false;
        }

        bool is_object() const
        {
            return type_ == object_t;
        }

        bool is_array() const
        {
            return type_ == array_t;
        }

        size_t count_;
        structure_type type_;
        bool comma_;
        bool received_name_;

    };
public:
    // Structural characters
    static const char begin_array = '[';
    static const char begin_object = '{';
    static const char end_array = ']';
    static const char end_object = '}';
    static const char name_separator = ':';
    static const char value_separator = ',';

    static const size_t default_max_buffer_length = 16384;
    //!  Parse an input stream of JSON text into a json object
    /*!
      \param is The input stream to read from
    */
    basic_json_reader(std::basic_istream<Char>& is,
                      basic_json_input_handler<Char>& handler,
                      basic_error_handler<Char>& err_handler)
       : is_(is),
         handler_(handler),
         err_handler_(err_handler),
         buffer_(default_max_buffer_length+1),
         input_buffer_(0),
         buffer_position_(0),
         bof_(true),
         eof_(false),
         buffer_length_(0),
         buffer_capacity_(default_max_buffer_length),
         minimum_structure_capacity_(0)
    {
        if (!is.good())
        {
            JSONCONS_THROW_EXCEPTION("Input stream is invalid");
        }
        input_buffer_ = &buffer_[0];
    }
    basic_json_reader(std::basic_istream<Char>& is,
                      basic_json_input_handler<Char>& handler)

       : is_(is),
         handler_(handler),
         err_handler_(default_err_handler),
         buffer_(default_max_buffer_length+1),
         input_buffer_(0),
         buffer_position_(0),
         bof_(true),
         eof_(false),
         buffer_length_(0),
         buffer_capacity_(default_max_buffer_length),
         minimum_structure_capacity_(0)
    {
        if (!is.good())
        {
            JSONCONS_THROW_EXCEPTION("Input stream is invalid");
        }
        input_buffer_ = &buffer_[0];
    }

    ~basic_json_reader()
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
        buffer_.resize(buffer_capacity+1);
        input_buffer_ = &buffer_[0];
    }

    virtual unsigned long line_number() const
    {
        return line_;
    }

    virtual unsigned long column_number() const
    {
        return column_;
    }

    virtual size_t minimum_structure_capacity() const
    {
        return minimum_structure_capacity_;
    }

    virtual const std::basic_string<Char>& buffer() const
    {
        return string_buffer_;
    }

private:
    basic_json_reader(const basic_json_reader&); // noop
    basic_json_reader& operator = (const basic_json_reader&); // noop

    size_t estimate_minimum_array_capacity() const;
    size_t estimate_minimum_object_capacity() const;
    size_t skip_array(size_t pos) const;
    size_t skip_object(size_t pos) const;
    size_t skip_string(size_t pos) const;
    size_t skip_number(size_t pos) const;
    void skip_separator();
    void parse_number(Char c);
    void parse_string();
    void ignore_single_line_comment();
    void ignore_multi_line_comment();
    bool fast_ignore_single_line_comment();
    bool fast_ignore_multi_line_comment();
    bool read_until_match_fails(char char1, char char2, char char3);
    void fast_skip_white_space();
    bool read_until_match_fails(char char1, char char2, char char3, char char4);
    unsigned int decode_unicode_codepoint();
    unsigned int decode_unicode_escape_sequence();

    void read_data_block()
    {
        buffer_position_ = 0;
        if (!is_.eof())
        {
            if (bof_)
            {
                is_.read(input_buffer_, buffer_capacity_);
                buffer_length_ = static_cast<size_t>(is_.gcount());
                bof_ = false;
                if (buffer_length_ == 0)
                {
                    eof_ = true;
                }
                else if (!is_.eof())
                {
                    --buffer_length_;
                }
            }
            else
            {
                input_buffer_[0] = input_buffer_[buffer_length_];
                is_.read(input_buffer_+1, buffer_capacity_);
                buffer_length_ = static_cast<size_t>(is_.gcount());
                if (is_.eof())
                {
                    ++buffer_length_;
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
            read_data_block();
        }
        Char c = 0;

        if (buffer_position_ < buffer_length_)
        {
            c = input_buffer_[buffer_position_++];
            if (c == '\r' && peek() == '\n')
            {
                ++buffer_position_;
                ++line_;
                column_ = 0;
            }
            else if (c == '\n' || c == '\r')
            {
                ++line_;
                column_ = 0;
            }
            else
            {
                ++column_;
            }
        }
        //else
        //{
        //    buffer_position_++;
        //}

        return c;
    }

    Char read_non_lf_ch()
    {
        if (buffer_position_ >= buffer_length_)
        {
            read_data_block();
        }

        Char c = 0;
        if (buffer_position_ < buffer_length_)
        {
            c = input_buffer_[buffer_position_++];
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

    void unread_last_non_lf_ch()
    {
        if (buffer_position_ > 0)
        {
            --buffer_position_;
            --column_;
        }
    }

    size_t minimum_structure_capacity_;
    unsigned long column_;
    unsigned long line_;
    std::basic_string<Char> string_buffer_;
    std::vector<stack_item> stack_;
    std::basic_istream<Char>& is_;
    Char *input_buffer_;
    std::vector<Char> buffer_;
    size_t buffer_capacity_;
    size_t buffer_position_;
    size_t buffer_length_;
    basic_json_input_handler<Char>& handler_;
    basic_error_handler<Char>& err_handler_;
    bool bof_;
    bool eof_;
};

template<class Char>
default_error_handler basic_json_reader<Char>::default_err_handler;

inline
unsigned long long string_to_ulonglong(const char *s, size_t length, const unsigned long long max_value) throw(std::overflow_error)
{
    unsigned long long n = 0;
    for (size_t i = 0; i < length; ++i)
    {
        char c = s[i];
        if (c >= '0' && c <= '9')
        {
            if (n > max_value / 10)
            {
                throw std::overflow_error("Integer overflow");
            }
            n = n * 10;
            long long k = (c - '0');
            if (n > max_value - k)
            {
                throw std::overflow_error("Integer overflow");
            }

            n += k;
        }
    }
    return n;
}

template<class Char>
void basic_json_reader<Char>::read()
{
    line_ = 1;
    column_ = 0;

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
        case begin_object:
            if (stack_.size() == 0)
            {
                handler_.begin_json();
            }
            stack_.push_back(stack_item(object_t));
            minimum_structure_capacity_ = estimate_minimum_object_capacity();
            handler_.begin_object(*this);
            minimum_structure_capacity_ = 0;
            break;
        case begin_array:
            if (stack_.size() == 0)
            {
                handler_.begin_json();
            }
            {
                stack_.push_back(stack_item(array_t));
                minimum_structure_capacity_ = estimate_minimum_array_capacity();
                handler_.begin_array(*this);
                minimum_structure_capacity_ = 0;
            }
            break;
        default:
            if (stack_.size() > 0)
            {
                switch (c)
                {
                case value_separator:
                    if (stack_.back().count_ == 0)
                    {
                        err_handler_.fatal_error("JPE102", "Unexpected comma", *this);
                    }
                    stack_.back().comma_ = true;
                    break;
                case '\"':
                    if (stack_.back().count_ > 0 && !stack_.back().comma_)
                    {
                        err_handler_.fatal_error("JPE102", "Expected comma", *this);
                    }
                    {
                        parse_string();
                        if (stack_.back().is_object() && !stack_.back().received_name_)
                        {
                            handler_.name(string_buffer_, *this);
                            skip_separator();
                            stack_.back().received_name_ = true;
                        }
                        else
                        {
                            handler_.value(string_buffer_, *this);
                            stack_.back().comma_ = false;
                            stack_.back().received_name_ = false;
                            ++stack_.back().count_;
                        }
                    }
                    break;
                case end_object:
                    {
                        if (!stack_.back().is_object())
                        {
                            err_handler_.fatal_error("JPE103", "Unexpected }", *this);
                        }
                        if (stack_.back().comma_)
                        {
                            err_handler_.fatal_error("JPE102", "Unexpected comma", *this);
                        }
                        if (stack_.back().received_name_)
                        {
                            err_handler_.fatal_error("JPE107", "Value not found", *this);
                        }
                        handler_.end_object(*this);
                        stack_.pop_back();
                    }
                    if (stack_.size() > 0)
                    {
                        stack_.back().received_name_ = false;
                        stack_.back().comma_ = false;
                        ++stack_.back().count_;
                    }
                    else
                    {
                        handler_.end_json();
                        return;
                    }
                    break;
                case end_array:
                    {
                        if (!stack_.back().is_array())
                        {
                            err_handler_.fatal_error("JPE104", "Unexpected ]", *this);
                        }
                        if (stack_.back().comma_)
                        {
                            err_handler_.fatal_error("JPE102", "Unexpected comma", *this);
                        }
                        handler_.end_array(*this);
                        stack_.pop_back();
                    }
                    if (stack_.size() > 0)
                    {
                        stack_.back().received_name_ = false;
                        stack_.back().comma_ = false;
                        ++stack_.back().count_;
                    }
                    else
                    {
                        handler_.end_json();
                        return;
                    }
                    break;
                case 't':
                    if (!read_until_match_fails('r', 'u', 'e'))
                    {
                        err_handler_.fatal_error("JPE105", "Unrecognized value", *this);
                    }
                    handler_.value(true, *this);
                    stack_.back().comma_ = false;
                    stack_.back().received_name_ = false;
                    ++stack_.back().count_;
                    break;
                case 'f':
                    if (!read_until_match_fails('a', 'l', 's', 'e'))
                    {
                        err_handler_.fatal_error("JPE105", "Unrecognized value", *this);
                    }
                    handler_.value(false, *this);
                    stack_.back().comma_ = false;
                    stack_.back().received_name_ = false;
                    ++stack_.back().count_;
                    break;
                case 'n':
                    if (!read_until_match_fails('u', 'l', 'l'))
                    {
                        err_handler_.fatal_error("JPE105", "Unrecognized value", *this);
                    }
                    handler_.null_value(*this);
                    stack_.back().comma_ = false;
                    stack_.back().received_name_ = false;
                    ++stack_.back().count_;
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case '-':
                    parse_number(c);
                    stack_.back().received_name_ = false;
                    stack_.back().comma_ = false;
                    ++stack_.back().count_;
                    break;
                default:
                    err_handler_.fatal_error("JPE105", "Unrecognized value", *this);
                    break;
                }
                break;
            }
        }
    }

    if (stack_.size() > 0)
    {
        err_handler_.fatal_error("JPE101", "End of file", *this);
    }
}

template<class Char>
void basic_json_reader<Char>::skip_separator()
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
                    if (next == '*')
                    {
                        ignore_multi_line_comment();
                    }
                }
            }
            break;
        case name_separator:
            //parse_value(handler_);
            return;
        default:
            err_handler_.fatal_error("JPE106", "Expected :", *this);
        }
    }

    err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
}

template<class Char>
bool basic_json_reader<Char>::read_until_match_fails(char char1, char char2, char char3)
{
    if (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
        }
        if (c == char1)
        {
            Char c = read_ch();
            if (eof())
            {
                err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
            }
            if (c == char2)
            {
                Char c = read_ch();
                if (eof())
                {
                    err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
                }
                if (c = char3)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

template<class Char>
bool basic_json_reader<Char>::read_until_match_fails(char char1, char char2, char char3, char char4)
{
    if (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
        }
        if (c == char1)
        {
            Char c = read_ch();
            if (eof())
            {
                err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
            }
            if (c == char2)
            {
                Char c = read_ch();
                if (eof())
                {
                    err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
                }
                if (c = char3)
                {
                    Char c = read_ch();
                    if (eof())
                    {
                        err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
                    }
                    if (c = char4)
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

template<class Char>
void basic_json_reader<Char>::parse_number(Char c)
{
    string_buffer_.clear();
    bool has_frac_or_exp = false;
    bool has_neg = (c == '-') ? true : false;
    if (!has_neg)
    {
        string_buffer_.push_back(c);
    }

    bool done = false;
    while (!done)
    {
        while (!done && buffer_position_ < buffer_length_)
        {
            Char c = buffer_[buffer_position_++]; // shouldn't be lf
            switch (c)
            {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                string_buffer_.push_back(c);
                break;
            case '-':
            case '+':
            case '.':
            case 'e':
            case 'E':
                has_frac_or_exp = true;
                string_buffer_.push_back(c);
                break;
            default:
                {
                    --buffer_position_;
                    if (has_frac_or_exp)
                    {
                        try
                        {
                            double d = string_to_double(string_buffer_);
                            if (has_neg)
                                d = -d;
                            handler_.value(d,*this);
                        }
                        catch (...)
                        {
                            err_handler_.fatal_error("JPE203", "Invalid double value", *this);
                            handler_.null_value(*this);
                        }
                    }
                    else if (has_neg)
                    {
                        try
                        {
                            long long d = static_cast<long long>(string_to_ulonglong(&string_buffer_[0], string_buffer_.length(), std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXPANSION()));
                            handler_.value(-d, *this);
                        }
                        catch (const std::exception&)
                        {
                            try
                            {
                                double d = string_to_double(string_buffer_);
                                handler_.value(-d,*this);
                            }
                            catch (...)
                            {
                                err_handler_.fatal_error("JPE203", "Invalid integer value", *this);
                                handler_.null_value(*this);
                            }
                        }
                    }
                    else
                    {
                        try
                        {
                            unsigned long long d = string_to_ulonglong(&string_buffer_[0], string_buffer_.length(), std::numeric_limits<unsigned long long>::max JSONCONS_NO_MACRO_EXPANSION());
                            handler_.value(d, *this);
                        }
                        catch (const std::exception&)
                        {
                            try
                            {
                                double d = string_to_double(string_buffer_);
                                handler_.value(d,*this);
                            }
                            catch (...)
                            {
                                err_handler_.fatal_error("JPE203", "Invalid integer value", *this);
                                handler_.null_value(*this);
                            }
                        }
                    }
                    done = true;
                }
                break;
            }
        }
        if (!done)
        {
            read_data_block();
            if (eof())
            {
                err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
            }
        }
    }
}

template<class Char>
void basic_json_reader<Char>::parse_string()
{
    string_buffer_.clear();

    bool done = false;
    while (!done)
    {
        while (!done && buffer_position_ < buffer_length_)
        {
            Char c = buffer_[buffer_position_++];
            switch (c)
            {
            case '\r':
                if (buffer_[buffer_position_] == '\n')
                {
                    ++buffer_position_;
                }
                ++line_;
                column_ = 0;
                break;
            case '\n':
                ++line_;
                column_ = 0;
                break;
            default:
                if (is_control_character(c))
                {
                    err_handler_.error("JPE201", "Illegal control character in string", *this);
                }
                switch (c)
                {
                case '\\':
                    {
                        Char next = buffer_[buffer_position_];
                        switch (next)
                        {
                        case '\"':
                            ++buffer_position_;
                            string_buffer_.push_back('\"');
                            break;
                        case '\\':
                            ++buffer_position_;
                            string_buffer_.push_back('\\');
                            break;
                        case '/':
                            ++buffer_position_;
                            string_buffer_.push_back('/');
                            break;
                        case 'n':
                            ++buffer_position_;
                            string_buffer_.push_back('\n');
                            break;
                        case 'b':
                            ++buffer_position_;
                            string_buffer_.push_back('\b');
                            break;
                        case 'f':
                            ++buffer_position_;
                            string_buffer_.push_back('\f');
                            break;
                        case 'r':
                            ++buffer_position_;
                            string_buffer_.push_back('\r');
                            break;
                        case 't':
                            ++buffer_position_;
                            string_buffer_.push_back('\t');
                            break;
                        case 'u':
                            {
                                ++buffer_position_;
                                unsigned int cp = decode_unicode_codepoint();
                                json_char_traits<Char>::append_codepoint_to_string(cp, string_buffer_);
                            }
                            break;
                        default:
                            err_handler_.fatal_error("JPE201", "Invalid character following \\", *this);
                        }
                    }
                    break;
                case '\"':
                    done = true;
                    break;
                default:
                    string_buffer_.push_back(c);
                    break;
                }
                ++column_;
                break;
            }
        }
        if (!done)
        {
            read_data_block();
            if (eof())
            {
                err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
            }
        }
    }
}

template<class Char>
void basic_json_reader<Char>::ignore_single_line_comment()
{
    bool done = fast_ignore_single_line_comment();
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
        }
    }
}

template<class Char>
bool basic_json_reader<Char>::fast_ignore_single_line_comment()
{
    bool done = false;
    const size_t end = buffer_length_ - 1;
    while (!done && buffer_position_ < end)
    {
        Char c = input_buffer_[buffer_position_];
        if (c == '\r' && input_buffer_[buffer_position_ + 1] == '\n')
        {
            done = true;
            ++line_;
            column_ = 0;
            buffer_position_ += 2;
        }
        else if (c == '\n' || c == '\r')
        {
            done = true;
            ++line_;
            column_ = 0;
            ++buffer_position_;
        }
        else
        {
            ++column_;
            ++buffer_position_;
        }
    }
    return done;
}

template<class Char>
bool basic_json_reader<Char>::fast_ignore_multi_line_comment()
{
    bool done = false;
    const size_t end = buffer_length_ - 1;

    while (!done && buffer_position_ < end)
    {
        Char c = input_buffer_[buffer_position_];
        if (c == '\r' && input_buffer_[buffer_position_ + 1] == '\n')
        {
            ++line_;
            column_ = 0;
            buffer_position_ += 2;
        }
        else if (c == '\n' || c == '\r')
        {
            ++line_;
            column_ = 0;
            ++buffer_position_;
        }
        else if (c == '*' && input_buffer_[buffer_position_ + 1] == '/')
        {
            done = true;
            buffer_position_ += 2;
            column_ += 2;
        }
        else
        {
            ++column_;
            ++buffer_position_;
        }
    }
    return done;
}

template<class Char>
void basic_json_reader<Char>::ignore_multi_line_comment()
{
    bool done = fast_ignore_multi_line_comment();
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
                skip_ch();
            }
        }
    }
}

template<class Char>
void basic_json_reader<Char>::fast_skip_white_space()
{
    bool done = false;
    const size_t end = buffer_length_ - 1;
    while (!done && buffer_position_ < end)
    {
        switch (input_buffer_[buffer_position_])
        {
        case '\r':
            if (input_buffer_[buffer_position_ + 1] == '\n')
            {
                ++line_;
                column_ = 0;
                buffer_position_ += 2;
            }
            else
            {
                ++line_;
                column_ = 0;
                ++buffer_position_;
            }
            break;
        case '\n':
            ++buffer_position_;
            ++line_;
            column_ = 0;
            break;
        case '\t':
        case '\v':
        case '\f':
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
unsigned int basic_json_reader<Char>::decode_unicode_codepoint()
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
size_t basic_json_reader<Char>::estimate_minimum_array_capacity() const
{
    size_t size = 0;
    size_t pos = buffer_position_;
    bool done = false;
    while (!done && pos < buffer_length_)
    {
        switch (buffer_[pos])
        {
        case end_array:
            done = true;
            break;
        case begin_array:
            pos = skip_array(pos + 1);
            ++size;
            break;
        case begin_object:
            pos = skip_object(pos + 1);
            ++size;
            break;
        case '\"':
            pos = skip_string(pos + 1);
            ++size;
            break;
        case 't':
            pos += 4;
            ++size;
            break;
        case 'f':
            pos += 5;
            ++size;
            break;
        case 'n':
            pos += 4;
            ++size;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
            pos = skip_number(pos + 1);
            ++size;
            break;
        default:
            ++pos;
            break;
        }
    }

    return size;
}

template<class Char>
size_t basic_json_reader<Char>::estimate_minimum_object_capacity() const
{
    size_t size = 0;
    size_t pos = buffer_position_;
    bool done = false;
    while (!done && pos < buffer_length_)
    {
        switch (buffer_[pos])
        {
        case end_object:
            done = true;
            break;
        case '\"':
            pos = skip_string(pos + 1);
            break;
        case ':':
            ++size;
            ++pos;
            break;
        case begin_array:
            pos = skip_array(pos + 1);
            break;
        case begin_object:
            pos = skip_object(pos + 1);
            break;
        case 't':
            pos += 4;
            break;
        case 'f':
            pos += 5;
            break;
        case 'n':
            pos += 4;
            break;
        default:
            ++pos;
            break;
        }
    }

    return size;
}

template<class Char>
size_t basic_json_reader<Char>::skip_array(size_t pos) const
{
    bool done = false;
    while (!done && pos < buffer_length_)
    {
        switch (buffer_[pos])
        {
        case begin_array:
            pos = skip_array(pos + 1);
            break;
        case begin_object:
            pos = skip_object(pos + 1);
            break;
        case '\"':
            pos = skip_string(pos + 1);
            break;
        case end_array:
            done = true;
            ++pos;
            break;
        default:
            ++pos;
            break;
        }
    }

    return pos;
}

template<class Char>
size_t basic_json_reader<Char>::skip_string(size_t pos) const
{
    bool done = false;
    while (!done && pos < buffer_length_)
    {
        switch (buffer_[pos])
        {
        case '\\':
            ++pos;
            if (pos < buffer_length_ && buffer_[pos] == 'u')
            {
                pos += 4;
            }
            else
            {
                ++pos;
            }
            break;
        case '\"':
            done = true;
            ++pos;
            break;
        default:
            ++pos;
            break;
        }
    }

    return pos;
}

template<class Char>
size_t basic_json_reader<Char>::skip_number(size_t pos) const
{
    bool done = false;
    while (!done && pos < buffer_length_)
    {
        switch (buffer_[pos])
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
        case '+':
        case '.':
        case 'e':
        case 'E':
            ++pos;
            break;
        default:
            done = true;
            break;
        }
    }

    return pos;
}

template<class Char>
size_t basic_json_reader<Char>::skip_object(size_t pos) const
{
    bool done = false;
    while (!done && pos < buffer_length_)
    {
        switch (buffer_[pos])
        {
        case begin_object:
            pos = skip_object(pos + 1);
            break;
        case begin_array:
            pos = skip_array(pos + 1);
            break;
        case '\"':
            pos = skip_string(pos + 1);
            break;
        case end_object:
            done = true;
            ++pos;
            break;
        default:
            ++pos;
            break;
        }
    }

    return pos;
}

template<class Char>
unsigned int basic_json_reader<Char>::decode_unicode_escape_sequence()
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

typedef basic_json_reader<char> json_reader;

}

#endif

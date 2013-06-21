// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSON_PARSER_HPP
#define JSONCONS_JSON_PARSER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <istream>
#include <cstdlib>
#include "jsoncons/jsoncons_config.hpp"
#include "jsoncons/json_char_traits.hpp"

namespace jsoncons {

class json_parser_exception : public std::exception
{
public:
    json_parser_exception(std::string s, 
                          unsigned long line, 
                          unsigned long column)
    {
        std::ostringstream os;
        os << s << " on line " << line << " at column " << column;
        message_ = os.str();
    }
    const char* what() const
    {
        return message_.c_str();
    }
private:
    std::string message_;
};

#define JSONCONS_THROW_PARSER_EXCEPTION(x,n,m) throw json_parser_exception(x,n,m)

template <class Char>
class json_object;

template <class Char>
class basic_json_parser
{
    enum state_type {};
    struct stack_item
    {
        stack_item()
            : count_(0)
        {
        }

        size_t count_;
        state_type state_;

    };
public:
    // Structural characters
    static const char begin_array = '[';
    static const char begin_object = '{';
    static const char end_array = ']';
    static const char end_object = '}';
    static const char name_separator = ':';
    static const char value_separator = ',';

    static const size_t max_buffer_length = 16384;
    //!  Parse an input stream of JSON text into a json object
    /*!
      \param is The input stream to read from
    */
    basic_json_parser(std::basic_istream<Char>& is)
        : is_(is), data_block_(0), 
          buffer_position_(0), buffer_length_(0)
    {
#ifdef JSONCONS_BUFFER_READ
        data_block_ = new Char[max_buffer_length];
#endif
    }

    ~basic_json_parser()
    {
#ifdef JSONCONS_BUFFER_READ
        delete[] data_block_;
#endif
    }

    template <class StreamListener>
    void parse(StreamListener& handler);
private:
    template <class StreamListener>
    void parse_object(StreamListener& handler);
    template <class StreamListener>
    void parse_separator_value(StreamListener& handler);
    template <class StreamListener>
    void parse_value(StreamListener& handler);
    template <class StreamListener>
    void parse_number(Char c, StreamListener& handler);
    template <class StreamListener>
    void parse_array(StreamListener& handler);
    template <class StreamListener>
    void parse_string(StreamListener& handler);
    void ignore_single_line_comment();
    bool read_until_match_fails(char char1, char char2, char char3);
    bool read_until_match_fails(char char1, char char2, char char3, char char4);
    unsigned int decode_unicode_codepoint();
    unsigned int decode_unicode_escape_sequence();

    bool eof() const
    {
#ifdef JSONCONS_BUFFER_READ
        return buffer_position_ > buffer_length_ && is_.eof();
#else
        return is_.eof();
#endif
    }

    void read_data_block()
    {
#ifdef JSONCONS_BUFFER_READ
        buffer_position_ = 0;
        if (!is_.eof())
        {
            is_.read(data_block_,max_buffer_length);
            buffer_length_ = static_cast<int>(is_.gcount());
        }
        else 
        {
            buffer_length_ = 0;
        }
#endif
    }

    Char read_ch()
    {
#ifdef JSONCONS_BUFFER_READ
        if (buffer_position_ >= buffer_length_)
        {
            read_data_block();
        }
        Char c = 0;

        //std::cout << "buffer_position = " << buffer_position_ << ", buffer_length=" << buffer_length_ << std::endl;
        if (buffer_position_ < buffer_length_)
        {
            c = data_block_[buffer_position_++];
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
#else
        Char c = static_cast<Char>(is_.get());
        if (c == '\n')
        {
            ++line_;
            column_ = 0;
        }
        ++column_;
        return c;
#endif
    }

    Char peek()
    {
#ifdef JSONCONS_BUFFER_READ
        if (buffer_position_ >= buffer_length_)
        {
            read_data_block();
        }
        Char c = 0;
        if (buffer_position_ < buffer_length_)
        {
            c = data_block_[buffer_position_];
        }

        return c;
#else
        Char c = is_.peek();
        return c;
#endif
    }

    void skip_ch()
    {
#ifdef JSONCONS_BUFFER_READ
        read_ch();
#else
        is_.ignore();
        ++column_;
#endif
    }

    void unread_ch(Char ch)
    {
#ifdef JSONCONS_BUFFER_READ
        if (buffer_position_ > 0)
        {
            --buffer_position_;
            --column_;
            if (ch == '\n') {
                --line_;
                column_ = 0;
            }
        }
#else
        is_.putback(ch);
        --column_;
        if (ch == '\n') {
            --line_;
            column_ = 0;
        }
#endif
    }

    unsigned long column_;
    unsigned long line_;
    std::basic_string<Char> buffer_;
    std::vector<stack_item> stack_;
    std::basic_istream<Char>& is_;
    Char* data_block_;
    int buffer_position_;
    int buffer_length_;
};

template <class Char>
unsigned long long string_to_uinteger(const std::basic_string<Char>& s)
{
    unsigned long long i = 0;
    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it)
    {
        if (*it >= '0' && *it <= '9')
        {
            i = i * 10 + (*it - '0');
        }
    }
    return i;
}

template <class Char>
template <class StreamListener>
void basic_json_parser<Char>::parse(StreamListener& handler)
{
    handler.begin_json();
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
            break;
        case '/':
            {
                if (!eof())
                {
                    Char next = peek();
                    if (next == '/')
                    {
                        skip_ch();
                        if (eof())
                        {
                            JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
                        }
                        ignore_single_line_comment();
                    }
                }
            }
            break;
        case begin_object:
            handler.begin_object();
            parse_object(handler);
            handler.end_json();
            return;
        case begin_array:
            handler.begin_array();
            parse_array(handler);
            handler.end_json();
            return;
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("End of file", line_,column_);
}

template <class Char>
template <class StreamListener>
void basic_json_parser<Char>::parse_object(StreamListener& handler)
{
    size_t count = 0;
    bool comma = false;

    while (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
        }
        switch (c)
        {
        case '\n':
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            break;
        case '/':
            {
                if (!eof())
                {
                    Char next = peek();
                    if (next == '/')
                    {
                        ignore_single_line_comment();
                    }
                }
            }
            break;
        case '\"':
            if (count > 0 && !comma)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Expected comma", line_,column_);
            }
            {
                parse_string(handler);
                handler.name(std::move(buffer_));
                parse_separator_value(handler);
                comma = false;
                ++count;
            }
            break;
        case value_separator:
            if (count == 0)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Unexpected comma", line_,column_);
            }
            comma = true;
            break;

        case end_object:
            {
                if (comma)
                {
                    JSONCONS_THROW_PARSER_EXCEPTION("Unexpected comma", line_,column_);
                }
                handler.end_object();
                return;
            }
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("Expected }", line_,column_);
}

template <class Char>
template <class StreamListener>
void basic_json_parser<Char>::parse_separator_value(StreamListener& handler)
{
    while (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
        }
        switch (c)
        {
        case '\n':
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            break;
        case '/':
            {
                if (!eof())
                {
                    Char next = peek();
                    if (next == '/')
                    {
                        ignore_single_line_comment();
                    }
                }
            }
            break;
        case name_separator:
            parse_value(handler);
            return;
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("Expected :", line_,column_);
}

template <class Char>
template <class StreamListener>
void basic_json_parser<Char>::parse_value(StreamListener& handler)
{
    while (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
        }
        switch (c)
        {
        case '\n':
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            break;
        case '/':
            {
                if (!eof())
                {
                    Char next = peek();
                    if (next == '/')
                    {
                        ignore_single_line_comment();
                    }
                }
            }
            break;
        case '\"': // string value
            {
                parse_string(handler);
                handler.value(std::move(buffer_));
                //handler.value(buffer_);
                return;
            }
        case begin_object: // object value
            {
                handler.begin_object();
                parse_object(handler);
                return;
            }
        case begin_array: // array value
            handler.begin_array();
            parse_array(handler);
            return;
        case 't':
            if (!read_until_match_fails('r', 'u', 'e'))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_,column_);
            }
            handler.value(true);
            return;
        case 'f':
            if (!read_until_match_fails('a', 'l', 's', 'e'))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_,column_);
            }
            handler.value(false);
            return;
        case 'n':
            if (!read_until_match_fails('u', 'l', 'l'))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_,column_);
            }
            handler.null();
            return;
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
            parse_number(c, handler);
            return;
        }
    }
}

template <class Char>
bool basic_json_parser<Char>::read_until_match_fails(char char1, char char2, char char3)
{
    if (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
        }
        if (c == char1)
        {
            Char c = read_ch();
            if (eof())
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
            }
            if (c == char2)
            {
                Char c = read_ch();
                if (eof())
                {
                    JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
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

template <class Char>
bool basic_json_parser<Char>::read_until_match_fails(char char1, char char2, char char3, char char4)
{
    if (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
        }
        if (c == char1)
        {
            Char c = read_ch();
            if (eof())
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
            }
            if (c == char2)
            {
                Char c = read_ch();
                if (eof())
                {
                    JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
                }
                if (c = char3)
                {
                    Char c = read_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
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

template <class Char>
template <class StreamListener>
void basic_json_parser<Char>::parse_array(StreamListener& handler)
{
    size_t count = 0;
    bool comma = false;
    while (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
        }
        switch (c)
        {
        case '\n':
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            break;
        case '/':
            {
                if (!eof())
                {
                    Char next = peek();
                    if (next == '/')
                    {
                        ignore_single_line_comment();
                    }
                }
            }
            break;
        case value_separator:
            if (count == 0)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Unxpected comma", line_,column_);
            }
            comma = true;
            break;
        case ']':
            if (comma)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Unxpected comma", line_,column_);
            }
            handler.end_array();
            return;
        default:
            if (count > 0 && !comma)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Expected comma", line_,column_);
            }
            unread_ch(c);
            parse_value(handler);
            ++count;
            comma = false;
            break;
        }

    }
}

template <class Char>
template <class StreamListener>
void basic_json_parser<Char>::parse_number(Char c, StreamListener& handler)
{
    buffer_.clear();
    bool has_frac_or_exp = false;
    bool has_neg = (c == '-') ? true : false;
    if (!has_neg)
    {
        buffer_.push_back(c);
    }

    while (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
        }
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
            buffer_.push_back(c);
            break;
        case '-':
        case '+':
        case '.':
        case 'e':
        case 'E':
            has_frac_or_exp = true;
            buffer_.push_back(c);
            break;
        default:
            {
                unread_ch(c);
                if (has_frac_or_exp)
                {
                    const Char *begin = buffer_.c_str();
                    Char *end;
                    double d = std::strtod(begin, &end);
                    if (end == begin)
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Invalid double value", line_,column_);
                    }
					if (has_neg)
						d = -d;
                    handler.value(d);
                    return;
                }
                else
                {
                    unsigned long long d = string_to_uinteger(buffer_);
					if (has_neg)
                    {
                        handler.value(-static_cast<long long>(d));
						return;
                    }
                    handler.value(d);
                    return;
                }
            }
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("Unexpected eof", line_,column_);
}

template <class Char>
template <class StreamListener>
void basic_json_parser<Char>::parse_string(StreamListener& handler)
{
    buffer_.clear();

    while (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
        }
        switch (c)
        {
        case '\a':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v':
        case '\0':
            JSONCONS_THROW_PARSER_EXCEPTION("Illegal control character in string", line_,column_);
            break;
        case '\\':
            if (!eof())
            {
                Char next = peek();
                switch (next)
                {
                case '\"':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
                    }
                    buffer_.push_back('\"');
                    break;
                case '\\':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
                    }
                    buffer_.push_back('\\');
                    break;
                case '/':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
                    }
                    buffer_.push_back('/');
                    break;
                case 'n':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
                    }
                    buffer_.push_back('\n');
                    break;
                case 'b':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
                    }
                    buffer_.push_back('\n');
                    break;
                case 'f':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
                    }
                    buffer_.push_back('\n');
                    break;
                case 'r':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
                    }
                    buffer_.push_back('\n');
                    break;
                case 't':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
                    }
                    buffer_.push_back('\n');
                    break;
                case 'u':
                    {
                        skip_ch();
                        if (eof())
                        {
                            JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
                        }
                        unsigned int cp = decode_unicode_codepoint();
                        json_char_traits<Char>::append_codepoint_to_string(cp, buffer_);
                    }
                    break;
                default:
                    JSONCONS_THROW_PARSER_EXCEPTION("Invalid character following \\", line_,column_);
                }
            }
            break;
        case '\"':
            return;
        default:
            buffer_.push_back(c);
            break;
        }
    }
    JSONCONS_THROW_PARSER_EXCEPTION("Expected \"", line_,column_);
}

template <class Char>
void basic_json_parser<Char>::ignore_single_line_comment()
{
    buffer_.clear();

    while (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
        }
        if (c == '\n')
        {
            return;
        }
    }
}

template <class Char>
unsigned int basic_json_parser<Char>::decode_unicode_codepoint()
{

    unsigned int cp = decode_unicode_escape_sequence();
    if (cp >= 0xD800 && cp <= 0xDBFF)
    {
        // surrogate pairs
        if (read_ch() == '\\' && read_ch() == 'u')
        {
            if (eof())
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
            }
            unsigned int surrogate_pair = decode_unicode_escape_sequence();
            cp = 0x10000 + ((cp & 0x3FF) << 10) + (surrogate_pair & 0x3FF);
        }
        else
        {
            JSONCONS_THROW_PARSER_EXCEPTION("expecting another \\u token to begin the second half of a cp surrogate pair.", line_,column_);
        }
    }
    return cp;
}

template <class Char>
unsigned int basic_json_parser<Char>::decode_unicode_escape_sequence()
{
    unsigned int cp = 0;
    size_t index = 0;
    while (!eof() && index < 4)
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSER_EXCEPTION("Unexpected EOF",line_,column_);
        }
        const unsigned int u(c >= 0 ? c : 256 + c );
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
            JSONCONS_THROW_PARSER_EXCEPTION(os.str(), line_,column_);
        }
        ++index;
    }
    if (index != 4)
    {
        JSONCONS_THROW_PARSER_EXCEPTION("Bad cp escape sequence in string: four digits expected.", line_,column_);
    }
    return cp;
}

typedef basic_json_parser<char> json_parser;

}

#endif

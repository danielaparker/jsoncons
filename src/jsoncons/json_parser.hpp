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
public:
    // Structural characters
    static const char begin_array = '[';
    static const char begin_object = '{';
    static const char end_array = ']';
    static const char end_object = '}';
    static const char name_separator = ':';
    static const char value_separator = ',';
    //!  Parse an input stream of JSON text into a json object
    /*!
      \param is The input stream to read from
    */
    template <class StreamListener>
    void parse(std::basic_istream<Char>& is, StreamListener& handler);
private:
    template <class StreamListener>
    void parse_object(std::basic_istream<Char>& is, StreamListener& handler);
    template <class StreamListener>
    void parse_separator_value(std::basic_istream<Char>& is, StreamListener& handler);
    template <class StreamListener>
    void parse_value(std::basic_istream<Char>& is, StreamListener& handler);
    template <class StreamListener>
    void parse_number(std::basic_istream<Char>& is, Char c, StreamListener& handler);
    template <class StreamListener>
    void parse_array(std::basic_istream<Char>& is, StreamListener& handler);
    template <class StreamListener>
    void parse_string(std::basic_istream<Char>& is, StreamListener& handler);
    void ignore_single_line_comment(std::basic_istream<Char>& is);
    bool read_until_match_fails(std::basic_istream<Char>& is, char char1, char char2, char char3);
    bool read_until_match_fails(std::basic_istream<Char>& is, char char1, char char2, char char3, char char4);
    unsigned int decode_unicode_codepoint(std::basic_istream<Char>& is);
    unsigned int decode_unicode_escape_sequence(std::basic_istream<Char>& is);

    unsigned long column_;
    unsigned long line_;
    std::basic_string<Char> buffer_;
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
void basic_json_parser<Char>::parse(std::basic_istream<Char>& is, StreamListener& handler)
{
    handler.begin_json();
    line_ = 1;
    column_ = 0;

    while (is)
    {
        Char c = static_cast<Char>(is.get());
        ++column_;
        switch (c)
        {
        case '\n':
            ++line_;
            column_ = 0;
            break;
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            break;
        case '/':
            {
                if (is)
                {
                    Char next = static_cast<Char>(is.peek());
                    if (next == '/')
                    {
                        is.ignore();
                        ++column_;
                        ignore_single_line_comment(is);
                    }
                }
            }
            break;
        case begin_object:
            handler.begin_object();
            parse_object(is,handler);
            handler.end_json();
            return;
        case begin_array:
            handler.begin_array();
            parse_array(is,handler);
            handler.end_json();
            return;
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("End of file", line_,column_);
}

template <class Char>
template <class StreamListener>
void basic_json_parser<Char>::parse_object(std::basic_istream<Char>& is, StreamListener& handler)
{
    size_t count = 0;
    bool comma = false;

    while (is)
    {
        Char c = static_cast<Char>(is.get());
        ++column_;
        switch (c)
        {
        case '\n':
            ++line_;
            column_ = 0;
            break;
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            break;
        case '/':
            {
                if (is)
                {
                    Char next = static_cast<Char>(is.peek());
                    if (next == '/')
                    {
                        ignore_single_line_comment(is);
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
                parse_string(is,handler);
                handler.name(std::move(buffer_));
                parse_separator_value(is,handler);
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
void basic_json_parser<Char>::parse_separator_value(std::basic_istream<Char>& is, StreamListener& handler)
{
    while (is)
    {
        Char c = static_cast<Char>(is.get());
        ++column_;
        switch (c)
        {
        case '\n':
            ++line_;
            column_ = 0;
            break;
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            break;
        case '/':
            {
                if (is)
                {
                    Char next = static_cast<Char>(is.peek());
                    if (next == '/')
                    {
                        ignore_single_line_comment(is);
                    }
                }
            }
            break;
        case name_separator:
            parse_value(is,handler);
            return;
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("Expected :", line_,column_);
}

template <class Char>
template <class StreamListener>
void basic_json_parser<Char>::parse_value(std::basic_istream<Char>& is, StreamListener& handler)
{
    while (is)
    {
        Char c = static_cast<Char>(is.get());
        ++column_;
        switch (c)
        {
        case '\n':
            ++line_;
            column_ = 0;
            break;
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            break;
        case '/':
            {
                if (is)
                {
                    Char next = static_cast<Char>(is.peek());
                    if (next == '/')
                    {
                        ignore_single_line_comment(is);
                    }
                }
            }
            break;
        case '\"': // string value
            {
                parse_string(is,handler);
                handler.value(std::move(buffer_));
                //handler.value(buffer_);
                return;
            }
        case begin_object: // object value
            {
                handler.begin_object();
                parse_object(is,handler);
                return;
            }
        case begin_array: // array value
            handler.begin_array();
            parse_array(is,handler);
            return;
        case 't':
            if (!read_until_match_fails(is, 'r', 'u', 'e'))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_,column_);
            }
            handler.value(true);
            return;
        case 'f':
            if (!read_until_match_fails(is, 'a', 'l', 's', 'e'))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_,column_);
            }
            handler.value(false);
            return;
        case 'n':
            if (!read_until_match_fails(is, 'u', 'l', 'l'))
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
            parse_number(is, c, handler);
            return;
        }
    }
}

template <class Char>
bool basic_json_parser<Char>::read_until_match_fails(std::basic_istream<Char>& is, char char1, char char2, char char3)
{
    if (is)
    {
        Char c = static_cast<Char>(is.get());
        ++column_;
        if (c == char1 && is)
        {
            Char c = static_cast<Char>(is.get());
            ++column_;
            if (c == char2 && is)
            {
                Char c = static_cast<Char>(is.get());
                ++column_;
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
bool basic_json_parser<Char>::read_until_match_fails(std::basic_istream<Char>& is, char char1, char char2, char char3, char char4)
{
    if (is)
    {
        Char c = static_cast<Char>(is.get());
        ++column_;
        if (c == char1 && is)
        {
            Char c = static_cast<Char>(is.get());
            ++column_;
            if (c == char2 && is)
            {
                Char c = static_cast<Char>(is.get());
                ++column_;
                if (c = char3 && is)
                {
                    Char c = static_cast<Char>(is.get());
                    ++column_;
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
void basic_json_parser<Char>::parse_array(std::basic_istream<Char>& is, StreamListener& handler)
{
    size_t count = 0;
    bool comma = false;
    while (is)
    {
        Char c = static_cast<Char>(is.get());
        ++column_;
        switch (c)
        {
        case '\n':
            ++line_;
            column_ = 0;
            break;
        case '\t':
        case '\v':
        case '\f':
        case '\r':
        case ' ':
            break;
        case '/':
            {
                if (is)
                {
                    Char next = static_cast<Char>(is.peek());
                    if (next == '/')
                    {
                        ignore_single_line_comment(is);
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
            is.putback(c);
            --column_;
            parse_value(is,handler);
            ++count;
            comma = false;
            break;
        }

    }
}

template <class Char>
template <class StreamListener>
void basic_json_parser<Char>::parse_number(std::basic_istream<Char>& is, Char c, StreamListener& handler)
{
    buffer_.clear();
    bool has_frac_or_exp = false;
    bool has_neg = (c == '-') ? true : false;
    if (!has_neg)
    {
        buffer_.push_back(c);
    }

    while (is)
    {
        Char c = static_cast<Char>(is.get());
        ++column_;
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
                is.putback(c);
                --column_;
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
void basic_json_parser<Char>::parse_string(std::basic_istream<Char>& is, StreamListener& handler)
{
    buffer_.clear();

    while (is)
    {
        Char c = static_cast<Char>(is.get());
        ++column_;
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
            if (is)
            {
                Char next = is.peek();
                switch (next)
                {
                case '\"':
                    is.ignore();
                    ++column_;
                    buffer_.push_back('\"');
                    break;
                case '\\':
                    is.ignore();
                    ++column_;
                    buffer_.push_back('\\');
                    break;
                case '/':
                    is.ignore();
                    ++column_;
                    buffer_.push_back('/');
                    break;
                case 'n':
                    is.ignore();
                    ++column_;
                    buffer_.push_back('\n');
                    break;
                case 'b':
                    is.ignore();
                    ++column_;
                    buffer_.push_back('\n');
                    break;
                case 'f':
                    is.ignore();
                    ++column_;
                    buffer_.push_back('\n');
                    break;
                case 'r':
                    is.ignore();
                    ++column_;
                    buffer_.push_back('\n');
                    break;
                case 't':
                    is.ignore();
                    ++column_;
                    buffer_.push_back('\n');
                    break;
                case 'u':
                    {
                        is.ignore();
                        ++column_;
                        unsigned int cp = decode_unicode_codepoint(is);
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
void basic_json_parser<Char>::ignore_single_line_comment(std::basic_istream<Char>& is)
{
    buffer_.clear();

    while (is)
    {
        Char c = static_cast<Char>(is.get());
        ++column_;
        if (c == '\n')
        {
            ++line_;
            column_ = 0;
            return;
        }
    }
}

template <class Char>
unsigned int basic_json_parser<Char>::decode_unicode_codepoint(std::basic_istream<Char>& is)
{

    unsigned int cp = decode_unicode_escape_sequence(is);
    if (cp >= 0xD800 && cp <= 0xDBFF)
    {
        // surrogate pairs
        if (static_cast<Char>(is.get()) == '\\' && static_cast<Char>(is.get()) == 'u')
        {
            column_ += 2;
            unsigned int surrogate_pair = decode_unicode_escape_sequence(is);
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
unsigned int basic_json_parser<Char>::decode_unicode_escape_sequence(std::basic_istream<Char>& is)
{
    unsigned int cp = 0;
    size_t index = 0;
    while (is && index < 4)
    {
        Char c = static_cast<Char>(is.get());
        ++column_;
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

// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSONPARSER_HPP
#define JSONCONS_JSONPARSER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <istream>

namespace jsoncons {

template <class Char>
struct json_char_traits
{
};

template <>
struct json_char_traits<char>
{
    // Structural characters
    static const char begin_array = '[';
    static const char begin_object = '{';
    static const char end_array = ']';
    static const char end_object = '}';
    static const char name_separator = ':';
    static const char value_separator = ',';

    static const char forward_slash = '/';

    // Values
    static const char* rue() {return "rue";};
    static const char* alse () {return "alse";};
    static const char* ull() {return "ull";};
 
    static void append_codepoint_to_string(unsigned int cp, std::string& s)
    {
        if (cp <= 0x7f)
        {
            s.push_back(static_cast<char>(cp));
        }
        else if (cp <= 0x7FF)
        {
            s.push_back(static_cast<char>(0xC0 | (0x1f & (cp >> 6))));
            s.push_back(static_cast<char>(0x80 | (0x3f & cp)));
        }
        else if (cp <= 0xFFFF)
        {
            s.push_back(0xE0 | static_cast<char>((0xf & (cp >> 12))));
            s.push_back(0x80 | static_cast<char>((0x3f & (cp >> 6))));
            s.push_back(static_cast<char>(0x80 | (0x3f & cp)));
        }
        else if (cp <= 0x10FFFF)
        {
            s.push_back(static_cast<char>(0xF0 | (0x7 & (cp >> 18))));
            s.push_back(static_cast<char>(0x80 | (0x3f & (cp >> 12))));
            s.push_back(static_cast<char>(0x80 | (0x3f & (cp >> 6))));
            s.push_back(static_cast<char>(0x80 | (0x3f & cp)));
        }
    }

};

class json_parser_exception : public std::exception
{
public:
    json_parser_exception(std::string s, unsigned long line_number)
    {
        std::ostringstream os(s);
        os << " on line " << line_number;
        message_ = os.str();
    }
    const char* what() const
    {
        return message_.c_str();
    }
private:
    std::string message_;
};

#define JSONCONS_THROW_PARSER_EXCEPTION(x,n) throw json_parser_exception(x,n)

template <class Char>
class json_variant;

template <class Char>
class json_object;

template <class Char>
class json_parser
{
public:
    json_object<Char>* parse(std::basic_istream<Char>& is);
    json_object<Char>* parse_object(std::basic_istream<Char>& is);
    json_variant<Char>* parse_separator_value(std::basic_istream<Char>& is);
    json_variant<Char>* parse_value(std::basic_istream<Char>& is);
    json_variant<Char>* parse_number(std::basic_istream<Char>& is, Char c);
    json_variant<Char>* parse_array(std::basic_istream<Char>& is);
    void parse_string(std::basic_istream<Char>& is);
    void ignore_till_end_of_line(std::basic_istream<Char>& is);
    bool read_until_match_fails(std::basic_istream<Char>& is, const Char *s);
    unsigned int decode_unicode_codepoint(std::basic_istream<Char>& is);
    unsigned int decode_unicode_escape_sequence(std::basic_istream<Char>& is);
private:
    unsigned long line_number_;
    std::basic_string<Char> buffer_;
};

template <class Char>
json_object<Char>* json_parser<Char>::parse(std::basic_istream<Char>& is)
{
    line_number_ = 0;

    while (is)
    {
        Char c = static_cast<Char>(is.get());
        switch (c)
        {
        case '\n':
            ++line_number_;
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
                    if (next == json_char_traits<Char>::forward_slash)
                    {
                        ignore_till_end_of_line(is);
                    }
                }
            }
            break;
        case json_char_traits<Char>::begin_object:
            {
                json_object<Char> *value = parse_object(is);
                return value;
            }
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("End of file", line_number_);
    return 0; // noop
}

template <class Char>
json_object<Char>* json_parser<Char>::parse_object(std::basic_istream<Char>& is)
{
    json_object<Char> *object = new json_object<Char>();
    bool comma = false;

    while (is)
    {
        Char c = static_cast<Char>(is.get());
        switch (c)
        {
        case '\n':
            ++line_number_;
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
                    if (next == json_char_traits<Char>::forward_slash)
                    {
                        ignore_till_end_of_line(is);
                    }
                }
            }
            break;
        case '\"':
            if (object->size() > 0 && !comma)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Expected comma", line_number_);
            }
            {
                name_value_pair<Char> pair;
                parse_string(is);
                //pair->name_ = std::move(buffer_);
                pair.name_ = buffer_;
                pair.value_ = basic_json<Char>(parse_separator_value(is));
                object->push_back(pair);
            }
            break;
        case json_char_traits<Char>::value_separator:
            if (object->size() == 0)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Unexpected comma", line_number_);
            }
            comma = true;
            break;

        case json_char_traits<Char>::end_object:
            {
                if (comma)
                {
                    JSONCONS_THROW_PARSER_EXCEPTION("Unexpected comma", line_number_);
                }
                object->sort_members();
                return object;
            }
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("Expected }", line_number_);
    return 0; // noop
}

template <class Char>
json_variant<Char>* json_parser<Char>::parse_separator_value(std::basic_istream<Char>& is)
{
    while (is)
    {
        Char c = static_cast<Char>(is.get());
        switch (c)
        {
        case '\n':
            ++line_number_;
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
                    if (next == json_char_traits<Char>::forward_slash)
                    {
                        ignore_till_end_of_line(is);
                    }
                }
            }
            break;
        case json_char_traits<Char>::name_separator:
            return parse_value(is);
            break;
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("Expected :", line_number_);
    return 0; // noop
}

template <class Char>
json_variant<Char>* json_parser<Char>::parse_value(std::basic_istream<Char>& is)
{
    while (is)
    {
        Char c = static_cast<Char>(is.get());
        switch (c)
        {
        case '\n':
            ++line_number_;
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
                    if (next == json_char_traits<Char>::forward_slash)
                    {
                        ignore_till_end_of_line(is);
                    }
                }
            }
            break;
        case '\"': // string value
            {
                parse_string(is);
                json_string<Char> *value = new json_string<Char>();
                value->value_ = buffer_;
                return value;
            }
        case json_char_traits<Char>::begin_object: // object value
            {
                json_variant<Char> *value = parse_object(is);
                return value;
            }
        case json_char_traits<Char>::begin_array: // array value
            return parse_array(is);
        case 't':
            if (!read_until_match_fails(is, json_char_traits<Char>::rue()))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_number_);
            }
            return new json_bool<Char>(true);
        case 'f':
            if (!read_until_match_fails(is, json_char_traits<Char>::alse()))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_number_);
            }
            return new json_bool<Char>(false);
        case 'n':
            if (!read_until_match_fails(is, json_char_traits<Char>::ull()))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_number_);
            }
            return new json_null<Char>();
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
            return parse_number(is, c);
        }
    }

    return 0;
}

template <class Char>
bool json_parser<Char>::read_until_match_fails(std::basic_istream<Char>& is, const Char *s)
{
    for (const Char* p = s; is && *p; ++p)
    {
        Char c = static_cast<Char>(is.get());
        if (*p != c)
        {
            return false;
        }
    }
    return true;
}

template <class Char>
json_variant<Char>* json_parser<Char>::parse_array(std::basic_istream<Char>& is)
{
    json_array<Char> *arrayValue = new json_array<Char>();
    bool comma = false;
    while (is)
    {
        Char c = static_cast<Char>(is.get());
        switch (c)
        {
        case '\n':
            ++line_number_;
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
                    if (next == json_char_traits<Char>::forward_slash)
                    {
                        ignore_till_end_of_line(is);
                    }
                }
            }
            break;
        case json_char_traits<Char>::value_separator:
            if (arrayValue->size() == 0)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Unxpected comma", line_number_);
            }
            comma = true;
            break;
        case ']':
            if (comma)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Unxpected comma", line_number_);
            }
            return arrayValue;
        default:
            if (arrayValue->size() > 0 && !comma)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Expected comma", line_number_);
            }
            is.putback(c);
            arrayValue->push_back(basic_json<Char>(parse_value(is)));
        }

    }

    return 0;
}

template <class Char>
json_variant<Char>* json_parser<Char>::parse_number(std::basic_istream<Char>& is, Char c)
{
    buffer_.clear();
    buffer_.push_back(c);
    bool has_frac_or_exp = false;
    bool has_neg = (c == '-') ? true : false;

    while (is)
    {
        Char c = static_cast<Char>(is.get());
        switch (c)
        {
        case '-':
            has_neg = true;
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
                const Char *begin = buffer_.c_str();
                Char *end;
                if (has_frac_or_exp)
                {
                    double d = std::strtod(begin, &end);
                    if (end == begin)
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Invalid double value", line_number_);
                    }
                    return new json_double<Char>(d);
                }
                else if (has_neg)
                {
                    long d = std::strtol(begin, &end, 10);
                    if (end == begin)
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Invalid long value", line_number_);
                    }
                    return new json_long<Char> (d);
                }
                else
                {
                    unsigned long d = std::strtoul(begin, &end, 10);
                    if (end == begin)
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Invalid unsigned long value", line_number_);
                    }
                    return new json_ulong<Char>(d);
                }
            }
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("Unexpected eof", line_number_);
    return 0;
}

template <class Char>
void json_parser<Char>::parse_string(std::basic_istream<Char>& is)
{
    buffer_.clear();

    while (is)
    {
        Char c = static_cast<Char>(is.get());
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
            JSONCONS_THROW_PARSER_EXCEPTION("Illegal control character in string", line_number_);
            break;
        case '\\':
            if (is)
            {
                Char next = is.peek();
                switch (next)
                {
                case '\"':
                    is.ignore();
                    buffer_.push_back('\"');
                    break;
                case '\\':
                    is.ignore();
                    buffer_.push_back('\\');
                    break;
                case '/':
                    is.ignore();
                    buffer_.push_back('/');
                    break;
                case 'n':
                    is.ignore();
                    buffer_.push_back('\n');
                    break;
                case 'b':
                    is.ignore();
                    buffer_.push_back('\n');
                    break;
                case 'f':
                    is.ignore();
                    buffer_.push_back('\n');
                    break;
                case 'r':
                    is.ignore();
                    buffer_.push_back('\n');
                    break;
                case 't':
                    is.ignore();
                    buffer_.push_back('\n');
                    break;
                case 'u':
                    {
                        is.ignore();
                        unsigned int cp = decode_unicode_codepoint(is);
                        json_char_traits<Char>::append_codepoint_to_string(cp, buffer_);
                    }
                    break;
                default:
                    JSONCONS_THROW_PARSER_EXCEPTION("Invalid character following \\", line_number_);
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
    JSONCONS_THROW_PARSER_EXCEPTION("Expected \"", line_number_);
}

template <class Char>
void json_parser<Char>::ignore_till_end_of_line(std::basic_istream<Char>& is)
{
    while (is)
    {
        Char c = static_cast<Char>(is.get());
        if (c == '\n')
        {
            ++line_number_;
            return;
        }
    }
}

template <class Char>
unsigned int json_parser<Char>::decode_unicode_codepoint(std::basic_istream<Char>& is)
{

    unsigned int cp = decode_unicode_escape_sequence(is);
    if (cp >= 0xD800 && cp <= 0xDBFF)
    {
        // surrogate pairs
        if (static_cast<Char>(is.get()) == '\\' && static_cast<Char>(is.get()) == 'u')
        {
            unsigned int surrogate_pair = decode_unicode_escape_sequence(is);
            cp = 0x10000 + ((cp & 0x3FF) << 10) + (surrogate_pair & 0x3FF);
        }
        else
        {
            JSONCONS_THROW_PARSER_EXCEPTION("expecting another \\u token to begin the second half of a cp surrogate pair.", line_number_);
        }
    }
    return cp;
}

template <class Char>
unsigned int json_parser<Char>::decode_unicode_escape_sequence(std::basic_istream<Char>& is)
{
    unsigned int cp = 0;
    size_t index = 0;
    while (is && index < 4)
    {
        Char c = static_cast<Char>(is.get());
        cp *= 16;
        if (c >= '0'  &&  c <= '9')
        {
            cp += c - '0';
        }
        else if (c >= 'a'  &&  c <= 'f')
        {
            cp += c - 'a' + 10;
        }
        else if (c >= 'A'  &&  c <= 'F')
        {
            cp += c - 'A' + 10;
        }
        else
        {
            std::ostringstream os;
            os << "Expected hexadecimal digit, found " << c << ".";
            JSONCONS_THROW_PARSER_EXCEPTION(os.str(), line_number_);
        }
        ++index;
    }
    if (index != 4)
    {
        JSONCONS_THROW_PARSER_EXCEPTION("Bad cp escape sequence in string: four digits expected.", line_number_);
    }
    return cp;
}

}

#endif

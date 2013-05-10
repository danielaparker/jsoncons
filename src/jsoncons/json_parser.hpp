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

    // Whitespace
    static const char nl = '\n';
    static const char tab = '\t';
    static const char vtab = '\v';
    static const char ff = '\f';
    static const char cr = '\r';
    static const char space = ' ';
    static const char fslash = '/';
};

void append_codepoint_to_string(unsigned int cp, std::string& s);

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
    typedef Char char_type;

    json_object<Char>* parse(std::basic_istream<Char>& is);
    json_object<Char>* parse_object(std::basic_istream<Char>& is);
    json_variant<Char>* parse_separator_value(std::basic_istream<Char>& is);
    json_variant<Char>* parse_value(std::basic_istream<Char>& is);
    json_variant<Char>* parse_number(std::basic_istream<Char>& is, char c);
    json_variant<Char>* parse_array(std::basic_istream<Char>& is);
    void parse_string(std::basic_istream<Char>& is);
    void ignore_till_end_of_line(std::basic_istream<Char>& is);
    bool read_until_match_fails(std::basic_istream<Char>& is, const char *s);
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
        char c = static_cast<Char>(is.get());
        switch (c)
        {
        case json_char_traits<Char>::nl:
            ++line_number_;
            break;
        case json_char_traits<Char>::tab:
        case json_char_traits<Char>::vtab:
        case json_char_traits<Char>::ff:
        case json_char_traits<Char>::cr:
        case json_char_traits<Char>::space:
            break;
        case json_char_traits<Char>::fslash:
            {
                if (is)
                {
                    char next = static_cast<Char>(is.peek());
                    if (next == json_char_traits<Char>::fslash)
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
        char c = static_cast<Char>(is.get());
        switch (c)
        {
        case json_char_traits<Char>::nl:
            ++line_number_;
            break;
        case json_char_traits<Char>::tab:
        case json_char_traits<Char>::vtab:
        case json_char_traits<Char>::ff:
        case json_char_traits<Char>::cr:
        case json_char_traits<Char>::space:
            break;
        case json_char_traits<Char>::fslash:
            {
                if (is)
                {
                    char next = static_cast<Char>(is.peek());
                    if (next == '/')
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
                pair.value_ = parse_separator_value(is);
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
        char c = static_cast<Char>(is.get());
        switch (c)
        {
        case json_char_traits<Char>::nl:
            ++line_number_;
            break;
        case json_char_traits<Char>::tab:
        case json_char_traits<Char>::vtab:
        case json_char_traits<Char>::ff:
        case json_char_traits<Char>::cr:
        case json_char_traits<Char>::space:
            break;
        case json_char_traits<Char>::fslash:
            {
                if (is)
                {
                    char next = static_cast<Char>(is.peek());
                    if (next == '/')
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
        char c = static_cast<Char>(is.get());
        switch (c)
        {
        case json_char_traits<Char>::nl:
            ++line_number_;
            break;
        case json_char_traits<Char>::tab:
        case json_char_traits<Char>::vtab:
        case json_char_traits<Char>::ff:
        case json_char_traits<Char>::cr:
        case json_char_traits<Char>::space:
            break;
        case json_char_traits<Char>::fslash:
            {
                if (is)
                {
                    char next = static_cast<Char>(is.peek());
                    if (next == '/')
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
            if (!read_until_match_fails(is, "rue"))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_number_);
            }
            return new json_bool<Char>(true);
        case 'f':
            if (!read_until_match_fails(is, "alse"))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_number_);
            }
            return new json_bool<Char>(false);
        case 'n':
            if (!read_until_match_fails(is, "ull"))
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
bool json_parser<Char>::read_until_match_fails(std::basic_istream<Char>& is, const char *s)
{
    for (const char* p = s; is && *p; ++p)
    {
        char c = static_cast<Char>(is.get());
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
        char c = static_cast<Char>(is.get());
        switch (c)
        {
        case json_char_traits<Char>::nl:
            ++line_number_;
            break;
        case json_char_traits<Char>::tab:
        case json_char_traits<Char>::vtab:
        case json_char_traits<Char>::ff:
        case json_char_traits<Char>::cr:
        case json_char_traits<Char>::space:
            break;
        case json_char_traits<Char>::fslash:
            {
                if (is)
                {
                    char next = static_cast<Char>(is.peek());
                    if (next == '/')
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
            arrayValue->push_back(parse_value(is));
        }

    }

    return 0;
}

template <class Char>
json_variant<Char>* json_parser<Char>::parse_number(std::basic_istream<Char>& is, char c)
{
    buffer_.clear();
    buffer_.push_back(c);
    bool has_frac_or_exp = false;
    bool has_neg = (c == '-') ? true : false;

    while (is)
    {
        char c = static_cast<Char>(is.get());
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
                const char *begin = buffer_.c_str();
                char *end;
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
        char c = static_cast<Char>(is.get());
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
                char next = is.peek();
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
                        append_codepoint_to_string(cp, buffer_);
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
        char c = static_cast<Char>(is.get());
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
        char c = static_cast<Char>(is.get());
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

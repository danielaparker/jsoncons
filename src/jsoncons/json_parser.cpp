#include <string>
#include <utility>
#include <vector>
#include <assert.h>
#include <cstdlib>
#include <cerrno>
#include "jsoncons/json_parser.hpp"
#include "jsoncons/json_variant.hpp"

namespace jsoncons {

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

json_object<char>* json_parser::parse(std::istream& is)
{
    line_number_ = 0;

    while (is)
    {
        char c = static_cast<char>(is.get());
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
                    char next = static_cast<char>(is.peek());
                    if (next == '/')
                    {
                        ignore_till_end_of_line(is);
                    }
                }
            }
            break;
        case '{':
            {
                json_object<char> *value = parse_object(is);
                return value;
            }
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("End of file", line_number_);
    return 0; // noop
}

json_object<char>* json_parser::parse_object(std::istream& is)
{
    json_object<char> *object = new json_object<char>();
    bool comma = false;

    while (is)
    {
        char c = static_cast<char>(is.get());
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
                    char next = static_cast<char>(is.peek());
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
                name_value_pair<char> pair;
                parse_string(is);
                //pair->name_ = std::move(buffer_);
                pair.name_ = buffer_;
                pair.value_ = parse_separator_value(is);
                object->push_back(pair);
            }
            break;
        case ',':
            if (object->size() == 0)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Unexpected comma", line_number_);
            }
            comma = true;
            break;

        case '}':
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

json_variant<char>* json_parser::parse_separator_value(std::istream& is)
{
    while (is)
    {
        char c = static_cast<char>(is.get());
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
                    char next = static_cast<char>(is.peek());
                    if (next == '/')
                    {
                        ignore_till_end_of_line(is);
                    }
                }
            }
            break;
        case ':':
            return parse_value(is);
            break;
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("Expected :", line_number_);
    return 0; // noop
}

json_variant<char>* json_parser::parse_value(std::istream& is)
{
    while (is)
    {
        char c = static_cast<char>(is.get());
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
                    char next = static_cast<char>(is.peek());
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
                json_string<char> *value = new json_string<char>();
                value->value_ = buffer_;
                return value;
            }
        case '{': // object value
            {
                json_variant<char> *value = parse_object(is);
                return value;
            }
        case '[]': // array value
            return parse_array(is);
        case 't':
            if (!read_until_match_fails(is, "rue"))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_number_);
            }
            return new json_bool<char>(true);
        case 'f':
            if (!read_until_match_fails(is, "alse"))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_number_);
            }
            return new json_bool<char>(false);
        case 'n':
            if (!read_until_match_fails(is, "ull"))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_number_);
            }
            return new json_null<char>();
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

bool json_parser::read_until_match_fails(std::istream& is, const char *s)
{
    for (const char* p = s; is && *p; ++p)
    {
        char c = static_cast<char>(is.get());
        if (*p != c)
        {
            return false;
        }
    }
    return true;
}

json_variant<char>* json_parser::parse_array(std::istream& is)
{
    json_array<char> *arrayValue = new json_array<char>();
    bool comma = false;
    while (is)
    {
        char c = static_cast<char>(is.get());
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
                    char next = static_cast<char>(is.peek());
                    if (next == '/')
                    {
                        ignore_till_end_of_line(is);
                    }
                }
            }
            break;
        case ',':
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

json_variant<char>* json_parser::parse_number(std::istream& is, char c)
{
    buffer_.clear();
    buffer_.push_back(c);
    bool has_frac_or_exp = false;
    bool has_neg = (c == '-') ? true : false;

    while (is)
    {
        char c = static_cast<char>(is.get());
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
                    return new json_double<char>(d);
                }
                else if (has_neg)
                {
                    long d = std::strtol(begin, &end, 10);
                    if (end == begin)
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Invalid long value", line_number_);
                    }
                    return new json_long<char> (d);
                }
                else
                {
                    unsigned long d = std::strtoul(begin, &end, 10);
                    if (end == begin)
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Invalid unsigned long value", line_number_);
                    }
                    return new json_ulong<char>(d);
                }
            }
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("Unexpected eof", line_number_);
    return 0;
}

void json_parser::parse_string(std::istream& is)
{
    buffer_.clear();

    while (is)
    {
        char c = static_cast<char>(is.get());
        switch (c)
        {
        case'\a':
        case'\b':
        case'\f':
        case'\n':
        case'\r':
        case'\t':
        case'\v':
        case'\0':
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

void json_parser::ignore_till_end_of_line(std::istream& is)
{
    while (is)
    {
        char c = static_cast<char>(is.get());
        if (c == '\n')
        {
            ++line_number_;
            return;
        }
    }
}

unsigned int json_parser::decode_unicode_codepoint(std::istream& is)
{

    unsigned int cp = decode_unicode_escape_sequence(is);
    if (cp >= 0xD800 && cp <= 0xDBFF)
    {
        // surrogate pairs
        if (static_cast<char>(is.get()) == '\\' && static_cast<char>(is.get()) == 'u')
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

unsigned int json_parser::decode_unicode_escape_sequence(std::istream& is)
{
    unsigned int cp = 0;
    size_t index = 0;
    while (is && index < 4)
    {
        char c = static_cast<char>(is.get());
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


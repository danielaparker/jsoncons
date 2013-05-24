// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_JSONPARSER_HPP
#define JSONCONS_JSONPARSER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <assert.h>
#include <istream>
#include <cstdlib>

namespace jsoncons {

typedef long long integer_type;
typedef unsigned long long uinteger_type;

template <class Char>
struct stack_item
{
    stack_item(json_variant<Char>* var)
        : var_(var)
    {
    }
    std::basic_string<Char> name_;
    json_variant<Char>* var_;
};

template <class Char>
class json_content_handler
{
public:
    void begin_document()
    {
    }
    void end_document()
    {
    }
    void begin_object()
    {
        json_object<Char>* var = new json_object<Char>();
        stack_.push_back(stack_item<Char>(var));
    }
    void end_object()
    {
        json_variant<Char>* var = stack_.back().var_;
        var->object_cast()->sort_members();
        stack_.pop_back();
        if (stack_.size() > 0)
        {
            std::basic_string<Char> name = stack_.back().name_;
            if (stack_.back().var_->is_object())
            {
                stack_.back().var_->object_cast()->push_back(name_value_pair<Char>(name,var));
            }
            else
            {
                stack_.back().var_->array_cast()->push_back(basic_json<Char>(var));
            }
        }
        else
        {
            root_ = basic_json<Char>(var);
        }
    }
    void begin_array()
    {
        json_array<Char>* var = new json_array<Char>();
        stack_.push_back(stack_item<Char>(var));
    }
    void end_array()
    {
        json_variant<Char>* var = stack_.back().var_;
        stack_.pop_back();
        if (stack_.size() > 0)
        {
            std::basic_string<Char> name = stack_.back().name_;
            if (stack_.back().var_->is_object())
            {
                stack_.back().var_->object_cast()->push_back(name_value_pair<Char>(name,var));
            }
            else
            {
                stack_.back().var_->array_cast()->push_back(basic_json<Char>(var));
            }
        }
        else
        {
            root_ = basic_json<Char>(var);
        }
    }
    void name(std::basic_string<Char> name)
    {
        stack_.back().name_ = name;
    }
    void stringValue(std::basic_string<Char> value)
    {
        json_string<Char>* var = new json_string<Char>(std::move(value));
        if (stack_.back().var_->is_object())
        {
            stack_.back().var_->object_cast()->push_back(name_value_pair<Char>(stack_.back().name_,var));
            //std::cout << "stringValue " << vars_.back()->type() << " " << vars_.size() << std::endl;
        } 
        else if (stack_.back().var_->is_array())
        {
            stack_.back().var_->array_cast()->push_back(basic_json<Char>(var));
            //std::cout << "stringValue " << vars_.back()->type() << " " << vars_.size() << std::endl;
        }
        else 
        {
        }
    }
    void doubleValue(double value)
    {
        json_double<Char>* var = new json_double<Char>(value);
        if (stack_.back().var_->is_object())
        {
            stack_.back().var_->object_cast()->push_back(name_value_pair<Char>(stack_.back().name_,var));
        } 
        else if (stack_.back().var_->is_array())
        {
            stack_.back().var_->array_cast()->push_back(basic_json<Char>(var));
        }
        else 
        {
            //std::cout << "doubleValue " << vars_.back()->type() << " " << vars_.size() << std::endl;
        }
    }
    void integerValue(integer_type value)
    {
        json_integer<Char>* var = new json_integer<Char>(value);
        if (stack_.back().var_->is_object())
        {
            stack_.back().var_->object_cast()->push_back(name_value_pair<Char>(stack_.back().name_,var));
        } 
        else if (stack_.back().var_->is_array())
        {
            stack_.back().var_->array_cast()->push_back(basic_json<Char>(var));
        }
        else 
        {
            //std::cout << "integerValue " << vars_.back()->type()  << " " << vars_.size()<< std::endl;
        }
    }
    void uintegerValue(integer_type value)
    {
        json_uinteger<Char>* var = new json_uinteger<Char>(value);
        if (stack_.back().var_->is_object())
        {
            stack_.back().var_->object_cast()->push_back(name_value_pair<Char>(stack_.back().name_,var));
        } 
        else if (stack_.back().var_->is_array())
        {
            stack_.back().var_->array_cast()->push_back(basic_json<Char>(var));
        }
        else 
        {
            //std::cout << "uintegerValue " << vars_.back()->type()  << " " << vars_.size()<< std::endl;
        }
    }
    void boolValue(bool value)
    {
        json_bool<Char>* var = new json_bool<Char>(value);
        if (stack_.back().var_->is_object())
        {
            stack_.back().var_->object_cast()->push_back(name_value_pair<Char>(stack_.back().name_,var));
        } 
        else if (stack_.back().var_->is_array())
        {
            stack_.back().var_->array_cast()->push_back(basic_json<Char>(var));
        }
        else 
        {
            //std::cout << "boolValue " << vars_.back()->type()  << " " << vars_.size()<< std::endl;
        }
    }
    void nullValue()
    {
        json_null<Char>* var = new json_null<Char>();
        if (stack_.back().var_->is_object())
        {
            stack_.back().var_->object_cast()->push_back(name_value_pair<Char>(stack_.back().name_,var));
        } 
        else if (stack_.back().var_->is_array())
        {
            stack_.back().var_->array_cast()->push_back(basic_json<Char>(var));
        }
        else 
        {
            //std::cout << "nullValue " << vars_.back()->type() << " " << vars_.size() << std::endl;
        }
    }
	basic_json<Char> root_;
private:
    std::vector<stack_item<Char>> stack_;
};

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

    static uinteger_type string_to_uinteger(const std::string& s)
    {
        uinteger_type i = 0;
        for (std::string::const_iterator it; it != s.end(); ++it)
        {
            if (*it >= '0' && *it <= '9')
            {
                i = i * 10 + *it;
            }
        }
        return i;
    }

};

class json_parser_exception : public std::exception
{
public:
    json_parser_exception(std::string s, unsigned long line_number)
    {
        std::ostringstream os;
        os << s << " on line " << line_number;
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
    //!  Parse an input stream of JSON text into a json object
    /*!
      \param is The input stream to read from
    */
    json_variant<Char>* parse(std::basic_istream<Char>& is);
private:
    void parse_object(std::basic_istream<Char>& is);
    void parse_separator_value(std::basic_istream<Char>& is);
    void parse_value(std::basic_istream<Char>& is);
    void parse_number(std::basic_istream<Char>& is, Char c);
    void parse_array(std::basic_istream<Char>& is);
    void parse_string(std::basic_istream<Char>& is);
    void ignore_till_end_of_line(std::basic_istream<Char>& is);
    bool read_until_match_fails(std::basic_istream<Char>& is, const Char *s);
    unsigned int decode_unicode_codepoint(std::basic_istream<Char>& is);
    unsigned int decode_unicode_escape_sequence(std::basic_istream<Char>& is);

    unsigned long line_number_;
    std::basic_string<Char> buffer_;
    json_content_handler<Char> handler_;
};

template <class Char>
json_variant<Char>* json_parser<Char>::parse(std::basic_istream<Char>& is)
{
    handler_.begin_document();
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
                handler_.begin_object();
                parse_object(is);
                handler_.end_document();
                json_variant<Char>* val = 0;
                std::swap(val,handler_.root_.var_);
                return val;
            }
            break;
        case json_char_traits<Char>::begin_array:
            {
                handler_.begin_array();
                parse_array(is);
                handler_.end_document();
                json_variant<Char>* val = 0;
                std::swap(val,handler_.root_.var_);
                return val;
            }
            break;
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("End of file", line_number_);
    return 0; // noop
}

template <class Char>
void json_parser<Char>::parse_object(std::basic_istream<Char>& is)
{
    size_t count = 0;
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
            if (count > 0 && !comma)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Expected comma", line_number_);
            }
            {
                parse_string(is);
                handler_.name(buffer_);
                parse_separator_value(is);
                comma = false;
                ++count;
            }
            break;
        case json_char_traits<Char>::value_separator:
            if (count == 0)
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
                handler_.end_object();
                return;
            }
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("Expected }", line_number_);
}

template <class Char>
void json_parser<Char>::parse_separator_value(std::basic_istream<Char>& is)
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
            parse_value(is);
            return;
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("Expected :", line_number_);
}

template <class Char>
void json_parser<Char>::parse_value(std::basic_istream<Char>& is)
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
                handler_.stringValue(buffer_);
                return;
            }
        case json_char_traits<Char>::begin_object: // object value
            {
                handler_.begin_object();
                parse_object(is);
                return;
            }
        case json_char_traits<Char>::begin_array: // array value
            handler_.begin_array();
            parse_array(is);
            return;
        case 't':
            if (!read_until_match_fails(is, json_char_traits<Char>::rue()))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_number_);
            }
            handler_.boolValue(true);
            return;
        case 'f':
            if (!read_until_match_fails(is, json_char_traits<Char>::alse()))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_number_);
            }
            handler_.boolValue(false);
            return;
        case 'n':
            if (!read_until_match_fails(is, json_char_traits<Char>::ull()))
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Invalid value", line_number_);
            }
            handler_.nullValue();
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
            parse_number(is, c);
            return;
        }
    }
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
void json_parser<Char>::parse_array(std::basic_istream<Char>& is)
{
    size_t count = 0;
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
            if (count == 0)
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
            handler_.end_array();
            return;
        default:
            if (count > 0 && !comma)
            {
                JSONCONS_THROW_PARSER_EXCEPTION("Expected comma", line_number_);
            }
            is.putback(c);
            parse_value(is);
            ++count;
            comma = false;
            break;
        }

    }
}

template <class Char>
void json_parser<Char>::parse_number(std::basic_istream<Char>& is, Char c)
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
                if (has_frac_or_exp)
                {
                    const Char *begin = buffer_.c_str();
                    Char *end;
                    double d = std::strtod(begin, &end);
                    if (end == begin)
                    {
                        JSONCONS_THROW_PARSER_EXCEPTION("Invalid double value", line_number_);
                    }
					if (has_neg)
						d = -d;
                    handler_.doubleValue(d);
                    return;
                }
                else
                {
                    uinteger_type d = json_char_traits<Char>::string_to_uinteger(buffer_);
					if (has_neg)
                    {
                        handler_.integerValue(-static_cast<integer_type>(d));
						return;
                    }
                    handler_.uintegerValue(d);
                    return;
                }
            }
        }
    }

    JSONCONS_THROW_PARSER_EXCEPTION("Unexpected eof", line_number_);
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

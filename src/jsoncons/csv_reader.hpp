// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifndef JSONCONS_CSV_READER_HPP
#define JSONCONS_CSV_READER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include "jsoncons/jsoncons_config.hpp"
#include "jsoncons/json_char_traits.hpp"
#include "jsoncons/json_listener.hpp"
#include "jsoncons/json_exception.hpp"

namespace jsoncons {

template<class Char>
class basic_csv_reader
{
    enum structure_type {object_t, array_t};
    struct stack_item
    {
        stack_item(structure_type type)
           : type_(type), count_(0), comma_(false)
        {
            read_name_ = is_object() ? false : true;
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
        bool read_name_;

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
    basic_csv_reader(std::basic_istream<Char>& is,
                      basic_json_listener<Char>& handler)
       : is_(is), handler_(handler), input_buffer_(0), 
         buffer_position_(0), buffer_length_(0)
    {
#ifdef JSONCONS_BUFFER_READ
        input_buffer_ = new Char[buffer_capacity_];
#endif
    }

    ~basic_csv_reader()
    {
#ifdef JSONCONS_BUFFER_READ
        delete[] input_buffer_;
#endif
    }

    void read();

    bool eof() const
    {
#ifdef JSONCONS_BUFFER_READ
        return buffer_position_ > buffer_length_ && is_.eof();
#else
        return is_.eof();
#endif
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

    void skip_separator();
    void parse_number(Char c);
    void parse_string();
    void ignore_single_line_comment();
    void ignore_multi_line_comment();
    void fast_ignore_single_line_comment();
    void fast_ignore_multi_line_comment();
    bool read_until_match_fails(char char1, char char2, char char3);
    void fast_skip_white_space();
    bool read_until_match_fails(char char1, char char2, char char3, char char4);
    unsigned int decode_unicode_codepoint();
    unsigned int decode_unicode_escape_sequence();

    void read_data_block()
    {
#ifdef JSONCONS_BUFFER_READ
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
            c = input_buffer_[buffer_position_];
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
            if (ch == '\n')
            {
                --line_;
                column_ = 0;
            }
        }
#else
        is_.putback(ch);
        --column_;
        if (ch == '\n')
        {
            --line_;
            column_ = 0;
        }
#endif
    }

    unsigned long column_;
    unsigned long line_;
    std::basic_string<Char> string_buffer_;
    std::string number_buffer_;
    std::vector<stack_item> stack_;
    std::basic_istream<Char>& is_;
    Char *input_buffer_;
    size_t buffer_capacity_;
    int buffer_position_;
    int buffer_length_;
    basic_json_listener<Char>& handler_;
};

inline
unsigned long long string_to_ulonglong(const char* s, size_t length, const unsigned long long max_value)
{
	unsigned long long n = 0;
    for (size_t i = 0; i < length; ++i)
    {
        char c = s[i];
        if (c >= '0' && c <= '9')
        {
			if (n > max_value/10)
			{
		        JSONCONS_THROW_PARSE_EXCEPTION("Integer overflow", 0, 0);
			}
            n = n * 10;
			long long k = (c - '0'); 
			if (n > max_value - k)
			{
				JSONCONS_THROW_PARSE_EXCEPTION("Integer overflow", 0, 0);
			}

			n += k;
        }
    }
    return n;
}

template<class Char>
void basic_csv_reader<Char>::read()
{
    line_ = 1;
    column_ = 0;

    handler_.begin_array();

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
                    JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                }
                if (next == '/')
                {
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                    }
                    ignore_single_line_comment();
                }
                if (next == '*')
                {
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                    }
                    ignore_multi_line_comment();
                }
            }
            continue;
        case begin_array:
            if (stack_.size() == 0)
            {
                handler_.begin_json();
            }
            stack_.push_back(stack_item(array_t));
            handler_.begin_array();
            break;
        }
        if (stack_.size() > 0)
        {
            switch (c)
            {
            case value_separator:
                if (stack_.back().count_ == 0)
                {
                    JSONCONS_THROW_PARSE_EXCEPTION("Unexpected comma", line_, column_);
                }
                stack_.back().comma_ = true;
                break;
            case '\"':
                if (stack_.back().count_ > 0 && !stack_.back().comma_)
                {
                    JSONCONS_THROW_PARSE_EXCEPTION("Expected comma", line_, column_);
                }
                {
                    parse_string();
                    if (stack_.back().is_object() && !stack_.back().read_name_)
                    {
                        handler_.name(string_buffer_);
                        skip_separator();
                        stack_.back().read_name_ = true;
                    }
                    else
                    {
                        handler_.value(string_buffer_);
                        stack_.back().comma_ = false;
                        stack_.back().read_name_ = false;
                        ++stack_.back().count_;
                    }
                }
                break;
            case '\n':
                {
                    if (!stack_.back().is_object())
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected }", line_, column_);
                    }
                    if (stack_.back().comma_)
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected comma", line_, column_);
                    }
                    handler_.end_object();
                    stack_.pop_back();
                }
                if (stack_.size() > 0)
                {
                    stack_.back().read_name_ = false;
                    stack_.back().comma_ = false;
                    ++stack_.back().count_;
                }
                else
                {
                    handler_.end_json();
                    return;
                }
                break;
            case '\n':
                {
                    if (!stack_.back().is_array())
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected ]", line_, column_);
                    }
                    if (stack_.back().comma_)
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected comma", line_, column_);
                    }
                    handler_.end_array();
                    stack_.pop_back();
                }
                if (stack_.size() > 0)
                {
                    stack_.back().read_name_ = false;
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
                    JSONCONS_THROW_PARSE_EXCEPTION("Invalid value", line_, column_);
                }
                handler_.value(true);
                stack_.back().comma_ = false;
                stack_.back().read_name_ = false;
                ++stack_.back().count_;
                break;
            case 'f':
                if (!read_until_match_fails('a', 'l', 's', 'e'))
                {
                    JSONCONS_THROW_PARSE_EXCEPTION("Invalid value", line_, column_);
                }
                handler_.value(false);
                stack_.back().comma_ = false;
                stack_.back().read_name_ = false;
                ++stack_.back().count_;
                break;
            case 'n':
                if (!read_until_match_fails('u', 'l', 'l'))
                {
                    JSONCONS_THROW_PARSE_EXCEPTION("Invalid value", line_, column_);
                }
                handler_.null_value();
                stack_.back().comma_ = false;
                stack_.back().read_name_ = false;
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
                stack_.back().read_name_ = false;
                stack_.back().comma_ = false;
                ++stack_.back().count_;
                break;
            }
        }
    }

    if (stack_.size() > 0)
    {
        JSONCONS_THROW_PARSE_EXCEPTION("End of file", line_, column_);
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
            JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
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
        case name_separator:
            //parse_value(handler_);
            return;
        }
    }

    JSONCONS_THROW_PARSE_EXCEPTION("Expected :", line_, column_);
}

template<class Char>
bool basic_csv_reader<Char>::read_until_match_fails(char char1, char char2, char char3)
{
    if (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
        }
        if (c == char1)
        {
            Char c = read_ch();
            if (eof())
            {
                JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
            }
            if (c == char2)
            {
                Char c = read_ch();
                if (eof())
                {
                    JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
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
bool basic_csv_reader<Char>::read_until_match_fails(char char1, char char2, char char3, char char4)
{
    if (!eof())
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
        }
        if (c == char1)
        {
            Char c = read_ch();
            if (eof())
            {
                JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
            }
            if (c == char2)
            {
                Char c = read_ch();
                if (eof())
                {
                    JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                }
                if (c = char3)
                {
                    Char c = read_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
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
void basic_csv_reader<Char>::parse_number(Char c)
{
    number_buffer_.clear();
    bool has_frac_or_exp = false;
    bool has_neg = (c == '-') ? true : false;
    if (!has_neg)
    {
        number_buffer_.push_back(c);
    }

    bool done = false;
    while (!done)
    {
        Char c = read_ch();
        if (eof())
        {
            JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
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
            number_buffer_.push_back(c);
            break;
        case '-':
        case '+':
        case '.':
        case 'e':
        case 'E':
            has_frac_or_exp = true;
            number_buffer_.push_back(c);
            break;
        default:
            {
                unread_ch(c);
                if (has_frac_or_exp)
                {
                    const Char *begin = number_buffer_.c_str();
                    Char *end;
                    double d = std::strtod(begin, &end);
                    if (end == begin)
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Invalid double value", line_, column_);
                    }
                    if (has_neg)
                        d = -d;
                    handler_.value(d);
                }
                else if (has_neg)
                {
                    try
                    {
                        long long d = static_cast<long long>(string_to_ulonglong(&number_buffer_[0],number_buffer_.length(),std::numeric_limits<long long>::max()));
                        handler_.value(-d);
                    }
                    catch (const std::exception&)
                    {
                        const Char *begin = number_buffer_.c_str();
                        Char *end;
                        double d = std::strtod(begin, &end);
                        if (end == begin)
                        {
                            JSONCONS_THROW_PARSE_EXCEPTION("Invalid double value", line_, column_);
                        }
                        if (has_neg)
                            d = -d;
                        handler_.value(d);
                    }
                }
                else 
                {
                    try
                    {
                        unsigned long long d = string_to_ulonglong(&number_buffer_[0],number_buffer_.length(),std::numeric_limits<unsigned long long>::max());
                        handler_.value(d);
                    }
                    catch (const std::exception&)
                    {
                        const Char *begin = number_buffer_.c_str();
                        Char *end;
                        double d = std::strtod(begin, &end);
                        if (end == begin)
                        {
                            JSONCONS_THROW_PARSE_EXCEPTION("Invalid double value", line_, column_);
                        }
                        if (has_neg)
                            d = -d;
                        handler_.value(d);
                    }
                }
                done = true;
            }
            break;
        }
    }
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
            JSONCONS_THROW_PARSE_EXCEPTION("EOF, expected \"", line_, column_);
        }
        if (is_control_character(c))
        {
            JSONCONS_THROW_PARSE_EXCEPTION("Illegal control character in string", line_, column_);
        }
        switch (c)
        {
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
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                    }
                    string_buffer_.push_back('\"');
                    break;
                case '\\':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                    }
                    string_buffer_.push_back('\\');
                    break;
                case '/':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                    }
                    string_buffer_.push_back('/');
                    break;
                case 'n':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                    }
                    string_buffer_.push_back('\n');
                    break;
                case 'b':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                    }
                    string_buffer_.push_back('\n');
                    break;
                case 'f':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                    }
                    string_buffer_.push_back('\n');
                    break;
                case 'r':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                    }
                    string_buffer_.push_back('\n');
                    break;
                case 't':
                    skip_ch();
                    if (eof())
                    {
                        JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                    }
                    string_buffer_.push_back('\n');
                    break;
                case 'u':
                    {
                        skip_ch();
                        if (eof())
                        {
                            JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
                        }
                        unsigned int cp = decode_unicode_codepoint();
                        json_char_traits<Char>::append_codepoint_to_string(cp, string_buffer_);
                    }
                    break;
                default:
                    JSONCONS_THROW_PARSE_EXCEPTION("Invalid character following \\", line_, column_);
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
            JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
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
            JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
        }
        if (c == '*')
        {
            Char next = peek();
            if (eof())
            {
                JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
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
                JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
            }
            unsigned int surrogate_pair = decode_unicode_escape_sequence();
            cp = 0x10000 + ((cp & 0x3FF) << 10) + (surrogate_pair & 0x3FF);
        }
        else
        {
            JSONCONS_THROW_PARSE_EXCEPTION("expecting another \\u token to begin the second half of a cp surrogate pair.", line_, column_);
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
            JSONCONS_THROW_PARSE_EXCEPTION("Unexpected EOF", line_, column_);
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
            JSONCONS_THROW_PARSE_EXCEPTION(os.str(), line_, column_);
        }
        ++index;
    }
    if (index != 4)
    {
        JSONCONS_THROW_PARSE_EXCEPTION("Bad cp escape sequence in string: four digits expected.", line_, column_);
    }
    return cp;
}

typedef basic_csv_reader<char> csv_reader;

}

#endif

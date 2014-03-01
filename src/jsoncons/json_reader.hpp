// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSON_READER_HPP
#define JSONCONS_JSON_READER_HPP

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/error_handler.hpp"

namespace jsoncons {

template<typename Char>
class basic_json_reader : private basic_parsing_context<Char>
{
    struct buffered_stream
    {
        buffered_stream(std::basic_istream<Char>& is)
            : is_(is)
        {
        }

        std::basic_istream<Char>& is_;
    };

    static default_basic_error_handler<Char> default_err_handler;

    struct stack_item
    {
        stack_item(bool type)
           :
           value_count_(0),
           is_object_(type),
           comma_(false),
           name_count_(0)
        {
        }

        size_t value_count_;
        bool is_object_;
        bool comma_;
        size_t name_count_;

    };
public:
    // Structural characters
    static const char begin_array = '[';
    static const char begin_object = '{';
    static const char end_array = ']';
    static const char end_object = '}';
    static const char name_separator = ':';
    static const char value_separator = ',';

    static const size_t read_ahead_length = 12;
    static const size_t default_max_buffer_length = 16384;

    //  Parse an input stream of JSON text into a json object
    basic_json_reader(std::basic_istream<Char>& is,
                      basic_json_input_handler<Char>& handler,
                      basic_error_handler<Char>& err_handler)
       :
         minimum_structure_capacity_(0),
         column_(),
         line_(),
         string_buffer_(),
         stack_(),
         buffer_capacity_(default_max_buffer_length),
         buffer_position_(0),
         buffer_length_(0),
         hard_buffer_length_(0),
         estimation_buffer_length_(default_max_buffer_length),
         handler_(handler),
         err_handler_(err_handler),
         bof_(true),
         eof_(false),
         stream_ptr_(new buffered_stream(is))
    {
    }
    basic_json_reader(std::basic_istream<Char>& is,
                      basic_json_input_handler<Char>& handler)

       :
         minimum_structure_capacity_(0),
         column_(),
         line_(),
         string_buffer_(),
         stack_(),
         buffer_capacity_(default_max_buffer_length),
         buffer_position_(0),
         buffer_length_(0),
         hard_buffer_length_(0),
         estimation_buffer_length_(default_max_buffer_length),
         handler_(handler),
         err_handler_(default_err_handler),
         bof_(true),
         eof_(false), 
         stream_ptr_(new buffered_stream(is))
    {
    }

    ~basic_json_reader()
    {
    }

    void read()
    {
        read(stream_ptr_->is_);
    }

    void read(std::basic_istream<Char>& is);

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
    size_t skip_array(size_t pos, const size_t end) const;
    size_t skip_object(size_t pos, const size_t end) const;
    size_t skip_string(size_t pos, const size_t end) const;
    size_t skip_number(size_t pos, const size_t end) const;
    void parse_separator();
    void parse_number(Char c);
    void parse_string();
    void ignore_single_line_comment();
    void ignore_multi_line_comment();
    uint32_t decode_unicode_codepoint();
    uint32_t decode_unicode_escape_sequence();

    void read_some()
    {
        if (buffer_position_ < buffer_length_)
        {
            return; // exhaust buffer first
        }
        JSONCONS_ASSERT(buffer_position_ <= hard_buffer_length_);
        JSONCONS_ASSERT(buffer_position_ - buffer_length_ < read_ahead_length);

        size_t extra = 0;
        if (buffer_position_ > buffer_length_)
        {
            extra = buffer_position_ - buffer_length_;
        }

        buffer_position_ = 0;
        if (!stream_ptr_->is_.eof())
        {
            if (bof_)
            {
                stream_ptr_->is_.read(&buffer_[0], buffer_capacity_);
                buffer_length_ = static_cast<size_t>(stream_ptr_->is_.gcount());
                bof_ = false;
                if (buffer_length_ == 0)
                {
                    hard_buffer_length_ = 0;
                    eof_ = true;
                }
                else if (buffer_length_ == buffer_capacity_)
                {
                    hard_buffer_length_ = buffer_length_;
                    buffer_length_ -= read_ahead_length;
                }
                else
                {
                    hard_buffer_length_ = buffer_length_;
                }
            }
            else // not eof
            {
                size_t unread = read_ahead_length - extra;
                size_t real_buffer_length = buffer_length_ + extra;
                for (size_t i = 0; i < unread; ++i)
                {
                    buffer_[i] = buffer_[real_buffer_length + i];
                }
                stream_ptr_->is_.read(&buffer_[0] + unread, buffer_capacity_);
                buffer_length_ = static_cast<size_t>(stream_ptr_->is_.gcount());
                if (!stream_ptr_->is_.eof())
                {
                    buffer_length_ -= extra;
                    hard_buffer_length_ = buffer_length_ + read_ahead_length;
                }
                else
                {
                    buffer_length_ += unread;
                    for (size_t i = 0; i < read_ahead_length; ++i)
                    {
                        buffer_[buffer_length_ + i] = 0;
                    }
                    hard_buffer_length_ = buffer_length_;
                }
            }
        }
        else
        {
            buffer_length_ = 0;
            hard_buffer_length_ = 0;
            eof_ = true;
        }

    }

    size_t minimum_structure_capacity_;
    unsigned long column_;
    unsigned long line_;
    std::basic_string<Char> string_buffer_;
    std::vector<stack_item> stack_;
    std::vector<Char> buffer_;
    size_t buffer_capacity_;
    size_t buffer_position_;
    size_t buffer_length_;
    size_t hard_buffer_length_;
    size_t estimation_buffer_length_;
    basic_json_input_handler<Char>& handler_;
    basic_error_handler<Char>& err_handler_;
    bool bof_;
    bool eof_;
    std::unique_ptr<buffered_stream> stream_ptr_;
};

template<typename Char>
default_basic_error_handler<Char> basic_json_reader<Char>::default_err_handler;

template <typename Char>
unsigned long long string_to_ulonglong(const Char *s, size_t length, const unsigned long long max_value) throw(std::overflow_error)
{
    unsigned long long n = 0;
    for (size_t i = 0; i < length; ++i)
    {
        Char c = s[i];
        if ((c >= '0') & (c <= '9'))
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

template<typename Char>
void basic_json_reader<Char>::read(std::basic_istream<Char>& is)
{
    if (is.bad())
    {
        JSONCONS_THROW_EXCEPTION("Input stream is invalid");
    }
    stream_ptr_ = std::unique_ptr<buffered_stream>(new buffered_stream(is));
    buffer_.resize(buffer_capacity_ + 2*read_ahead_length);
    buffer_position_ = 0;
    buffer_length_ = 0;
    hard_buffer_length_ = 0;
    bof_ = true;
    eof_ = false;
    line_ = 1;
    column_ = 0;

    if (buffer_position_ >= buffer_length_)
    {
        read_some();
    }
    while (!eof())
    {
        while (buffer_position_ < buffer_length_)
        {
            Char c = buffer_[buffer_position_++];
            ++column_;
            switch (c)
            {
            case '\r':
                ++line_;
                column_ = 0;
                if (buffer_[buffer_position_] == '\n')
                {
                    ++buffer_position_;
                }
                break;
            case '\n':
                ++line_;
                column_ = 0;
                break;
            case '\t':
            case '\v':
            case '\f':
            case ' ':
                {
                    size_t count = 0;
                    if (buffer_[buffer_position_] == ' ')
                    {
                        ++count;
                    }
                    if ((buffer_[buffer_position_] == ' ') & (buffer_[buffer_position_+1] == ' '))
                    {
                        ++count;
                    }
                    if ((buffer_[buffer_position_] == ' ') & (buffer_[buffer_position_+1] == ' ') & (buffer_[buffer_position_+2] == ' '))
                    {
                        ++count;
                    }
                    buffer_position_ += count;
                    column_ += count;
                }
                break;
            case '/':
                {
                    Char next = buffer_[buffer_position_];
                    if (next == '/')
                    {
                        ++buffer_position_;
                        ignore_single_line_comment();
                    }
                    if (next == '*')
                    {
                        ++buffer_position_;
                        ignore_multi_line_comment();
                    }
                }
                continue;
            case begin_object:
                if (stack_.size() == 0)
                {
                    handler_.begin_json();
                }
                stack_.push_back(stack_item(true));
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
                    stack_.push_back(stack_item(false));
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
                        if (stack_.back().value_count_ == 0)
                        {
                            err_handler_.fatal_error("JPE102", "Unexpected comma", *this);
                        }
                        stack_.back().comma_ = true;
                        break;
                    case '\"':
                        if ((stack_.back().value_count_ > 0) & !stack_.back().comma_)
                        {
                            err_handler_.fatal_error("JPE102", "Expected comma", *this);
                        }
                        {
                            parse_string();
                            size_t count1 = 0;
                            if (stack_.back().is_object_ & (stack_.back().name_count_ == stack_.back().value_count_))
                            {
                                handler_.name(string_buffer_, *this);
                                count1 = 0;
                                if (buffer_[buffer_position_] == ':')
                                {
                                    ++count1;
                                }
                                if ((buffer_[buffer_position_] == ' ') & (buffer_[buffer_position_+1] == ':'))
                                {
                                    count1 += 2;
                                }
                                buffer_position_ += count1;
                                column_ += count1;

                                if (count1 == 0)
                                {
                                    parse_separator();
                                }
                                ++stack_.back().name_count_;
                            }
                            else
                            {
                                handler_.string_value(string_buffer_, *this);
                                stack_.back().comma_ = false;
                                ++stack_.back().value_count_;
                            }
                        }
                        break;
                    case end_object:
                        {
                            if (!stack_.back().is_object_)
                            {
                                err_handler_.fatal_error("JPE103", "Unexpected }", *this);
                            }
                            if (stack_.back().comma_)
                            {
                                err_handler_.fatal_error("JPE102", "Unexpected comma", *this);
                            }
                            if (stack_.back().name_count_ != stack_.back().value_count_)
                            {
                                err_handler_.fatal_error("JPE107", "Value not found", *this);
                            }
                            handler_.end_object(*this);
                            stack_.pop_back();
                        }
                        if (stack_.size() > 0)
                        {
                            stack_.back().comma_ = false;
                            ++stack_.back().value_count_;
                        }
                        else
                        {
                            handler_.end_json();
                            return;
                        }
                        break;
                    case end_array:
                        {
                            if (stack_.back().is_object_)
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
                            stack_.back().comma_ = false;
                            ++stack_.back().value_count_;
                        }
                        else
                        {
                            handler_.end_json();
                            return;
                        }
                        break;
                    case 't':
                        if (!((buffer_[buffer_position_] == 'r') & (buffer_[buffer_position_ + 1] == 'u') & (buffer_[buffer_position_ + 2] == 'e')))
                        {
                            err_handler_.fatal_error("JPE105", "Unrecognized value", *this);
                        }
                        buffer_position_ += 3;
                        column_ += 3;
                        handler_.bool_value(true, *this);
                        stack_.back().comma_ = false;
                        ++stack_.back().value_count_;
                        break;
                    case 'f':
                        if (!((buffer_[buffer_position_] == 'a') & (buffer_[buffer_position_ + 1] == 'l') & (buffer_[buffer_position_ + 2] == 's') & (buffer_[buffer_position_ + 3] == 'e')))
                        {
                            err_handler_.fatal_error("JPE105", "Unrecognized value", *this);
                        }
                        buffer_position_ += 4;
                        column_ += 4;
                        handler_.bool_value(false, *this);
                        stack_.back().comma_ = false;
                        ++stack_.back().value_count_;
                        break;
                    case 'n':
                        if (!((buffer_[buffer_position_] == 'u') & (buffer_[buffer_position_ + 1] == 'l') & (buffer_[buffer_position_ + 2] == 'l')))
                        {
                            err_handler_.fatal_error("JPE105", "Unrecognized value", *this);
                        }
                        buffer_position_ += 3;
                        column_ += 3;
                        handler_.null_value(*this);
                        stack_.back().comma_ = false;
                        ++stack_.back().value_count_;
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
                        stack_.back().comma_ = false;
                        ++stack_.back().value_count_;
                        break;
                    default:
                        err_handler_.fatal_error("JPE105", "Unrecognized value", *this);
                        break;
                    }
                    break;
                }
            }
        }
        if (buffer_position_ >= buffer_length_)
        {
            read_some();
        }
    }

    if (stack_.size() > 0)
    {
        err_handler_.fatal_error("JPE101", "End of file", *this);
    }
}

template<typename Char>
void basic_json_reader<Char>::parse_separator()
{
    bool done = false;
    while (!done)
    {
        const size_t end = buffer_length_;
        while (!done & (buffer_position_ < end))
        {
            Char c = buffer_[buffer_position_++];
            ++column_;
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
            case '\t':
            case '\v':
            case '\f':
            case ' ':
                break;
            case '/':
                {
                    Char next = buffer_[buffer_position_];
                    if (next == '/')
                    {
                        ignore_single_line_comment();
                    }
                    if (next == '*')
                    {
                        ignore_multi_line_comment();
                    }
                }
                break;
            case name_separator:
                done = true;
                break;
            default:
                err_handler_.fatal_error("JPE106", "Expected :", *this);
            }
        }
        if (!done)
        {
            read_some();
            if (eof())
            {
                err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
            }
        }
    }
}

template<typename Char>
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
        const size_t end = buffer_length_;
        while (!done & (buffer_position_ < end))
        {
            Char c = buffer_[buffer_position_++]; // shouldn't be lf
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
                            handler_.double_value(d, *this);
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
                            long long d = static_cast<long long>(string_to_ulonglong(&string_buffer_[0], string_buffer_.length(), std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXP()));
                            handler_.longlong_value(-d, *this);
                        }
                        catch (const std::exception&)
                        {
                            try
                            {
                                double d = string_to_double(string_buffer_);
                                handler_.double_value(-d, *this);
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
                            unsigned long long d = string_to_ulonglong(&string_buffer_[0], string_buffer_.length(), std::numeric_limits<unsigned long long>::max JSONCONS_NO_MACRO_EXP());
                            handler_.ulonglong_value(d, *this);
                        }
                        catch (const std::exception&)
                        {
                            try
                            {
                                double d = string_to_double(string_buffer_);
                                handler_.double_value(d, *this);
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
            read_some();
            if (eof())
            {
                err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
            }
        }
    }
}

template<typename Char>
void basic_json_reader<Char>::parse_string()
{
    string_buffer_.clear();

    bool done = false;
    while (!done)
    {
        const size_t end = buffer_length_;
        while (!done & (buffer_position_ < end))
        {
            Char c = buffer_[buffer_position_++];
            ++column_;
            switch (c)
            {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x09:
            case 0x0a:
            case 0x0b:
            case 0x0c:
            case 0x0d:
            case 0x0e:
            case 0x0f:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            case 0x14:
            case 0x15:
            case 0x16:
            case 0x17:
            case 0x18:
            case 0x19:
            case 0x1a:
            case 0x1b:
            case 0x1c:
            case 0x1d:
            case 0x1e:
            case 0x1f:
                err_handler_.error("JPE201", "Illegal control character in string", *this);
                break;
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
                            uint32_t cp = decode_unicode_codepoint();
                            json_char_traits<Char,sizeof(Char)>::append_codepoint_to_string(cp, string_buffer_);
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
        }
        if (!done)
        {
            read_some();
            if (eof())
            {
                err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
            }
        }
    }
}

template<typename Char>
void basic_json_reader<Char>::ignore_single_line_comment()
{
    bool done = false;
    while (!done)
    {
        const size_t end = buffer_length_;
        while (!done & (buffer_position_ < end))
        {
            Char c = buffer_[buffer_position_++];
            ++column_;
            switch (c)
            {
            case '\r':
                if (buffer_[buffer_position_] == '\n')
                {
                    ++buffer_position_;
                }
                ++line_;
                column_ = 0;
                done = true;
                break;
            case '\n':
                ++line_;
                column_ = 0;
                done = true;
                break;
            }
        }
        if (!done)
        {
            read_some();
            if (eof())
            {
                err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
            }
        }
    }
}

template<typename Char>
void basic_json_reader<Char>::ignore_multi_line_comment()
{
    bool done = false;
    while (!done)
    {
        const size_t end = buffer_length_;
        while (!done & (buffer_position_ < end))
        {
            Char c = buffer_[buffer_position_++];
            ++column_;
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
            case '*':
                {
                    Char next = buffer_[buffer_position_];
                    if (next == '/')
                    {
                        done = true;
                        buffer_position_++;
                        column_++;
                    }
                }
                break;
            }
        }
        if (!done)
        {
            read_some();
            if (eof())
            {
                err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
            }
        }
    }
}

template<typename Char>
size_t basic_json_reader<Char>::estimate_minimum_array_capacity() const
{
    size_t size = 0;
    size_t pos = buffer_position_;
    bool done = false;
    const size_t end = std::min JSONCONS_NO_MACRO_EXP(buffer_length_,estimation_buffer_length_);
    while (!done & (pos < end))
    {
        switch (buffer_[pos])
        {
        case end_array:
            done = true;
            break;
        case begin_array:
            pos = skip_array(pos + 1,end);
            ++size;
            break;
        case begin_object:
            pos = skip_object(pos + 1,end);
            ++size;
            break;
        case '\"':
            pos = skip_string(pos + 1,end);
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
            pos = skip_number(pos + 1,end);
            ++size;
            break;
        default:
            ++pos;
            break;
        }
    }

    return size;
}

template<typename Char>
size_t basic_json_reader<Char>::estimate_minimum_object_capacity() const
{
    size_t size = 0;
    size_t pos = buffer_position_;
    bool done = false;
    const size_t end = std::min JSONCONS_NO_MACRO_EXP(buffer_length_,estimation_buffer_length_);
    while (!done & (pos < end))
    {
        switch (buffer_[pos])
        {
        case end_object:
            done = true;
            break;
        case '\"':
            pos = skip_string(pos + 1,end);
            break;
        case ':':
            ++size;
            ++pos;
            break;
        case begin_array:
            pos = skip_array(pos + 1,end);
            break;
        case begin_object:
            pos = skip_object(pos + 1,end);
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

template<typename Char>
size_t basic_json_reader<Char>::skip_array(size_t pos, const size_t end) const
{
    bool done = false;
    while (!done & (pos < end))
    {
        switch (buffer_[pos])
        {
        case begin_array:
            pos = skip_array(pos + 1, end);
            break;
        case begin_object:
            pos = skip_object(pos + 1, end);
            break;
        case '\"':
            pos = skip_string(pos + 1, end);
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

template<typename Char>
size_t basic_json_reader<Char>::skip_string(size_t pos, const size_t end) const
{
    bool done = false;
    while (!done & (pos < end))
    {
        switch (buffer_[pos])
        {
        case '\\':
            ++pos;
            if ((pos < buffer_length_) & (buffer_[pos] == 'u'))
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

template<typename Char>
size_t basic_json_reader<Char>::skip_number(size_t pos, const size_t end) const
{
    bool done = false;
    while (!done & (pos < end))
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

template<typename Char>
size_t basic_json_reader<Char>::skip_object(size_t pos, const size_t end) const
{
    bool done = false;
    while (!done & (pos < end))
    {
        switch (buffer_[pos])
        {
        case begin_object:
            pos = skip_object(pos + 1, end);
            break;
        case begin_array:
            pos = skip_array(pos + 1, end);
            break;
        case '\"':
            pos = skip_string(pos + 1, end);
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

template<typename Char>
uint32_t basic_json_reader<Char>::decode_unicode_codepoint()
{
    uint32_t cp = decode_unicode_escape_sequence();
    if (hard_buffer_length_ - buffer_position_ < 2)
    {
        err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
    }
    if (cp >= min_lead_surrogate && cp <= max_lead_surrogate)
    {
        // surrogate pair
        if (buffer_[buffer_position_++] == '\\' && buffer_[buffer_position_++] == 'u')
        {
            column_ += 2;
            uint32_t surrogate_pair = decode_unicode_escape_sequence();

            cp = 0x10000 + ((cp & 0x3FF) << 10) + (surrogate_pair & 0x3FF);
        }
        else
        {
            err_handler_.fatal_error("JPE202", "expecting another \\u token to begin the second half of a codepoint surrogate pair.", *this);
        }
    }
    return cp;
}

template<typename Char>
uint32_t basic_json_reader<Char>::decode_unicode_escape_sequence()
{
    if (hard_buffer_length_ - buffer_position_ < 4)
    {
        err_handler_.fatal_error("JPE101", "Unexpected EOF", *this);
    }
    uint32_t cp = 0;
    size_t index = 0;
    while (index < 4)
    {
        Char c = buffer_[buffer_position_++];
        ++column_;
        const uint32_t u(c >= 0 ? c : 256 + c);
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
typedef basic_json_reader<wchar_t> wjson_reader;

}

#endif

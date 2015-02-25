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
#include <system_error>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/parse_error_handler.hpp"

namespace jsoncons {

namespace structure_type
{
    enum structure_type_t {json_text,json_object_impl,json_array_impl};
}

namespace parse_state_type
{
    enum parse_state_type_t {initial_s,name_s,value_separator_s,name_separator_s,value_completed_s};
}

template<typename Char>
class basic_json_reader : private basic_parsing_context<Char>
{
    struct stack_item
    {
        stack_item(structure_type::structure_type_t structure, size_t minimum_structure_capacity=0)
           : structure_(structure),
             state_(parse_state_type::initial_s),
             minimum_structure_capacity_(minimum_structure_capacity)
        {
        }

        bool is_object() const {return structure_ == structure_type::json_object_impl;}
        bool is_array() const {return structure_ == structure_type::json_array_impl   ;}
        bool is_top() const {return structure_ == structure_type::json_text;}

        int check_default() const
        {
            int err = 0;
            if (is_object())
            {
                if ((state_ == parse_state_type::initial_s) | (state_ == parse_state_type::value_separator_s))
                {
                    err = json_parser_errc::expected_name;
                }
                else if (state_ == parse_state_type::name_s)
                {
                    err = json_parser_errc::expected_name_separator;
                }
                else if (state_ == parse_state_type::name_separator_s)
                {
                    err = json_parser_errc::expected_value;
                }
                else 
                {
                    err = json_parser_errc::expected_value;
                }
            }
            else if ((state_ == parse_state_type::initial_s) | (state_ == parse_state_type::value_separator_s))
            {
                err = json_parser_errc::expected_value;
            }
            else
            {
                err = json_parser_errc::expected_value;
            }
            return err;
        }

        int check_value_precondition() const
        {
            int err = 0;
            if (state_ == parse_state_type::value_completed_s)
            {
                err = json_parser_errc::expected_value_separator;
            }
            else if ((structure_ == structure_type::json_object_impl) & (state_ != parse_state_type::name_separator_s))
            {
                if (state_ == parse_state_type::name_s)
                {
                    err = json_parser_errc::expected_name_separator;
                }
                else
                {
                    err = json_parser_errc::expected_name;
                }
            }
            return err;
        }

        int check_sub_value_precondition() const
        {
            int err = check_value_precondition();
    if (!err && is_top()) {
err = json_parser_errc::expected_container;
    }
    return err;
        }

        structure_type::structure_type_t structure_;
        parse_state_type::parse_state_type_t state_;
        size_t minimum_structure_capacity_;
    };

    void parse();

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

    //  Parse an input stream of JSON text into a json json_object_impl
    basic_json_reader(std::basic_istream<Char>& is,
                      basic_json_input_handler<Char>& handler,
                      basic_parse_error_handler<Char>& err_handler)
       :
         column_(0),
         line_(0),
         stack_(),
         buffer_capacity_(default_max_buffer_length),
         buffer_position_(0),
         buffer_length_(0),
         hard_buffer_length_(0),
         estimation_buffer_length_(default_max_buffer_length),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(err_handler)),
         bof_(true),
         eof_(false),
         is_(std::addressof(is))
    {
    }
    basic_json_reader(std::basic_istream<Char>& is,
                      basic_json_input_handler<Char>& handler)

       :
         column_(0),
         line_(0),
         stack_(),
         buffer_capacity_(default_max_buffer_length),
         buffer_position_(0),
         buffer_length_(0),
         hard_buffer_length_(0),
         estimation_buffer_length_(default_max_buffer_length),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(default_basic_parse_error_handler<Char>::instance())),
         bof_(true),
         eof_(false),
         is_(std::addressof(is))
    {
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
    void parse_number();
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
        if (!is_->eof())
        {
            if (bof_)
            {
                is_->read(&buffer_[0], buffer_capacity_);
                buffer_length_ = static_cast<size_t>(is_->gcount());
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
                is_->read(&buffer_[0] + unread, buffer_capacity_);
                buffer_length_ = static_cast<size_t>(is_->gcount());
                if (!is_->eof())
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

    unsigned long do_line_number() const override
    {
        return line_;
    }

    unsigned long do_column_number() const override
    {
        return column_;
    }

    //Char do_peek() const override
    //{
       // return buffer_[buffer_position_];
    //}

    bool do_eof() const override
    {
        return eof_;
    }

    Char do_last_char() const override
    {
        return c_;
    }

    size_t do_minimum_structure_capacity() const override
    {
        return stack_.size() > 0 ? stack_.back().minimum_structure_capacity_ : 0;
    }

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
    basic_json_input_handler<Char> *handler_;
    basic_parse_error_handler<Char> *err_handler_;
    bool bof_;
    bool eof_;
    std::basic_istream<Char> *is_;
    Char c_;
};

template<typename Char>
unsigned long long string_to_unsigned(const Char *s, size_t length) throw(std::overflow_error)
{
    const unsigned long long max_value = std::numeric_limits<unsigned long long>::max JSONCONS_NO_MACRO_EXP();
    const unsigned long long max_value_div_10 = max_value/10;
    unsigned long long n = 0;
    for (size_t i = 0; i < length; ++i)
    {
        unsigned long long x = s[i] - '0';
        if (n > max_value_div_10)
        {
            throw std::overflow_error("Unsigned overflow");
        }
        n = n * 10;
        if (n > max_value - x)
        {
            throw std::overflow_error("Unsigned overflow");
        }

        n += x;
    }
    return n;
}

template<typename Char>
long long string_to_integer(bool has_neg, const Char *s, size_t length) throw(std::overflow_error)
{
    const long long max_value = std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXP();
    const long long max_value_div_10 = max_value/10;

    long long n = 0;
    for (size_t i = 0; i < length; ++i)
    {
        long long x = s[i] - '0';
        if (n > max_value_div_10)
        {
            throw std::overflow_error("Integer overflow");
        }
        n = n * 10;
        if (n > max_value - x)
        {
            throw std::overflow_error("Integer overflow");
        }

        n += x;
    }
    return has_neg ? -n : n;
}

template<typename Char>
void basic_json_reader<Char>::read()
{
    if (is_->bad())
    {
        JSONCONS_THROW_EXCEPTION("Input stream is invalid");
    }
    buffer_.resize(buffer_capacity_ + 2 * read_ahead_length);
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

    stack_.push_back(stack_item(structure_type::json_text));
    parse();
    stack_.pop_back();
    if (stack_.size() > 0)
    {
        err_handler_->error(std::error_code(json_parser_errc::unexpected_eof, json_parser_category()), *this);
    }
}

template<typename Char>
void basic_json_reader<Char>::parse()
{
    while (!eof())
    {
        while (buffer_position_ < buffer_length_)
        {
            c_ = buffer_[buffer_position_++];
            ++column_;
            switch (c_)
            {
            // Skip whitespace
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
                continue;
            // Skip comments
            case '/':
                {
                    Char next = buffer_[buffer_position_];
                    if (next == '/')
                    {
                        ++buffer_position_;
                        ignore_single_line_comment();
                    }
                    else if (next == '*')
                    {
                        ++buffer_position_;
                        ignore_multi_line_comment();
                    }
                }
                continue;

            case begin_object:
                {
                    int err = stack_.back().check_value_precondition();
                    if (err != 0)
                    {
                        err_handler_->error(std::error_code(err, json_parser_category()), *this);
                    }
                }
                if (stack_.back().is_top())
                {
                    handler_->begin_json();
                }
                stack_.push_back(stack_item(structure_type::json_object_impl,estimate_minimum_object_capacity()));
                handler_->begin_object(*this);
                break;
            case begin_array:
                {
                    int err = stack_.back().check_value_precondition();
                    if (err != 0)
                    {
                        err_handler_->error(std::error_code(err, json_parser_category()), *this);
                    }
                }
                if (stack_.back().is_top())
                {
                    handler_->begin_json();
                }
                {
                    stack_.push_back(stack_item(structure_type::json_array_impl   ,estimate_minimum_array_capacity()));
                    handler_->begin_array(*this);
                }
                break;
            case value_separator:
                if (stack_.back().state_ != parse_state_type::value_completed_s)
                {
                    err_handler_->error(std::error_code(json_parser_errc::unexpected_value_separator, json_parser_category()), *this);
                }
                stack_.back().state_ = parse_state_type::value_separator_s;
                break;
            case name_separator:
                if (stack_.back().state_ != parse_state_type::name_s)
                {
                    err_handler_->error(std::error_code(json_parser_errc::unexpected_name_separator, json_parser_category()), *this);
                }
                stack_.back().state_ = parse_state_type::name_separator_s;
                break;
            case '\"':
                if (stack_.back().state_ == parse_state_type::value_completed_s)
                {
                    err_handler_->error(std::error_code(json_parser_errc::expected_value_separator, json_parser_category()), *this);
                }
                {
                    parse_string();
                    if (stack_.back().is_object() & ((stack_.back().state_ == parse_state_type::initial_s) | (stack_.back().state_ == parse_state_type::value_separator_s)))
                    {
                        handler_->name(string_buffer_.c_str(), string_buffer_.length(), *this);
                        stack_.back().state_ = parse_state_type::name_s;
                    }
                    else
                    {
                        int err = stack_.back().check_sub_value_precondition();
                        if (err != 0)
                        {
                            err_handler_->error(std::error_code(err, json_parser_category()), *this);
                        }
                        handler_->value(string_buffer_.c_str(), string_buffer_.length(), *this);
                        stack_.back().state_ = parse_state_type::value_completed_s;
                    }
                }
                break;
            case end_object:
                {
                    if (!stack_.back().is_object())
                    {
                        err_handler_->error(std::error_code(json_parser_errc::unexpected_end_of_object, json_parser_category()), *this);
                    }
                    else if (stack_.back().state_ == parse_state_type::value_separator_s) // dap
                    {
                        err_handler_->error(std::error_code(json_parser_errc::unexpected_value_separator, json_parser_category()), *this);
                    }
                    else if (!((stack_.back().state_ == parse_state_type::initial_s) | (stack_.back().state_ == parse_state_type::value_completed_s)))
                    {
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    }
                    handler_->end_object(*this);
                    stack_.pop_back();
                }
                if (!stack_.back().is_top())
                {
                    stack_.back().state_ = parse_state_type::value_completed_s;
                }
                else
                {
                    handler_->end_json();
                    return;
                }
                break;
            case end_array:
                {
                    if (stack_.back().is_object())
                    {
                        err_handler_->error(std::error_code(json_parser_errc::unexpected_end_of_array, json_parser_category()), *this);
                    }
                    else if (stack_.back().state_ == parse_state_type::value_separator_s) // dap
                    {
                        err_handler_->error(std::error_code(json_parser_errc::unexpected_value_separator, json_parser_category()), *this);
                    }
                    else if (!((stack_.back().state_ == parse_state_type::initial_s) | (stack_.back().state_ == parse_state_type::value_completed_s)))
                    {
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    }
                    handler_->end_array(*this);
                    stack_.pop_back();
                }
                if (!stack_.back().is_top())
                {
                    stack_.back().state_ = parse_state_type::value_completed_s;
                }
                else
                {
                    handler_->end_json();
                    return;
                }
                break;
            case 't':
                {
                    int err = stack_.back().check_sub_value_precondition();
                    if (err != 0)
                    {
                        err_handler_->error(std::error_code(err, json_parser_category()), *this);
                    }
                
                    if (!((buffer_[buffer_position_] == 'r') & (buffer_[buffer_position_ + 1] == 'u') & (buffer_[buffer_position_ + 2] == 'e')))
                    {
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                    }
                    buffer_position_ += 3;
                    column_ += 3;
                    handler_->value(true, *this);
                    stack_.back().state_ = parse_state_type::value_completed_s;
                }
                break;
            case 'f':
                {
                    int err = stack_.back().check_sub_value_precondition();
                    if (err != 0)
                    {
                        err_handler_->error(std::error_code(err, json_parser_category()), *this);
                    }
                }
                if (!((buffer_[buffer_position_] == 'a') & (buffer_[buffer_position_ + 1] == 'l') & (buffer_[buffer_position_ + 2] == 's') & (buffer_[buffer_position_ + 3] == 'e')))
                {
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                }
                buffer_position_ += 4;
                column_ += 4;
                handler_->value(false, *this);
                stack_.back().state_ = parse_state_type::value_completed_s;
                break;
            case 'n':
                {
                    int err = stack_.back().check_sub_value_precondition();
                    if (err != 0)
                    {
                        err_handler_->error(std::error_code(err, json_parser_category()), *this);
                    }
                }
                if (!((buffer_[buffer_position_] == 'u') & (buffer_[buffer_position_ + 1] == 'l') & (buffer_[buffer_position_ + 2] == 'l')))
                {
                    err_handler_->error(std::error_code(json_parser_errc::expected_value, json_parser_category()), *this);
                }
                buffer_position_ += 3;
                column_ += 3;
                handler_->value(null_type(), *this);
                stack_.back().state_ = parse_state_type::value_completed_s;
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
                {
                    int err = stack_.back().check_sub_value_precondition();
                    if (err != 0)
                    {
                        err_handler_->error(std::error_code(err, json_parser_category()), *this);
                    }
                    parse_number();
                    stack_.back().state_ = parse_state_type::value_completed_s;
                }
                break;
            default:
                {
                    int err = stack_.back().check_default();
                    if (err != 0)
                    {
                        err_handler_->error(std::error_code(err, json_parser_category()), *this);
                    }
                }
                break;
            }
        }
        if (buffer_position_ >= buffer_length_)
        {
            read_some();
        }
    }
}

template<typename Char>
void basic_json_reader<Char>::parse_number()
{
    string_buffer_.clear();
    bool has_frac_or_exp = false;
    bool has_neg = (c_ == '-') ? true : false;
    if (!has_neg)
    {
        string_buffer_.push_back(c_);
    }

    bool done = false;
    while (!done)
    {
        const size_t end = buffer_length_;
        while (!done & (buffer_position_ < end))
        {
            c_ = buffer_[buffer_position_]; // shouldn't be lf
            switch (c_)
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
                string_buffer_.push_back(c_);
                ++buffer_position_;
                ++column_;
                break;
            case '-':
            case '+':
            case '.':
            case 'e':
            case 'E':
                has_frac_or_exp = true;
                string_buffer_.push_back(c_);
                ++buffer_position_;
                ++column_;
                break;
            default:
                {
                    if (has_frac_or_exp)
                    {
                        try
                        {
                            double d = string_to_float(string_buffer_);
                            if (has_neg)
                                d = -d;
                            handler_->value(d, *this);
                        }
                        catch (...)
                        {
                            err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_parser_category()), *this);
                            handler_->value(null_type(), *this);
                        }
                    }
                    else if (has_neg)
                    {
                        try
                        {
                            long long d = string_to_integer(has_neg,string_buffer_.c_str(), string_buffer_.length());
                            handler_->value(d, *this);
                        }
                        catch (const std::exception&)
                        {
                            try
                            {
                                double d = string_to_float(string_buffer_);
                                handler_->value(-d, *this);
                            }
                            catch (...)
                            {
                                err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_parser_category()), *this);
                                handler_->value(null_type(), *this);
                            }
                        }
                    }
                    else
                    {
                        try
                        {
                            unsigned long long d = string_to_unsigned(string_buffer_.c_str(), string_buffer_.length());
                            handler_->value(d, *this);
                        }
                        catch (const std::exception&)
                        {
                            try
                            {
                                double d = string_to_float(string_buffer_);
                                handler_->value(d, *this);
                            }
                            catch (...)
                            {
                                err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_parser_category()), *this);
                                handler_->value(null_type(), *this);
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
                err_handler_->error(std::error_code(json_parser_errc::eof_reading_numeric_value, json_parser_category()), *this);
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
            c_ = buffer_[buffer_position_++];
            ++column_;
            switch (c_)
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
                err_handler_->error(std::error_code(json_parser_errc::illegal_control_character, json_parser_category()), *this);
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
                            json_char_traits<Char, sizeof(Char)>::append_codepoint_to_string(cp, string_buffer_);
                        }
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::illegal_escaped_character, json_parser_category()), *this);
                    }
                }
                break;
            case '\"':
                done = true;
                break;
            default:
                string_buffer_.push_back(c_);
                break;
            }
        }
        if (!done)
        {
            read_some();
            if (eof())
            {
                err_handler_->error(std::error_code(json_parser_errc::eof_reading_string_value, json_parser_category()), *this);
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
            c_ = buffer_[buffer_position_++];
            ++column_;
            switch (c_)
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
                err_handler_->error(std::error_code(json_parser_errc::unexpected_eof, json_parser_category()), *this);
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
            c_ = buffer_[buffer_position_++];
            ++column_;
            switch (c_)
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
                err_handler_->error(std::error_code(json_parser_errc::unexpected_eof, json_parser_category()), *this);
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
    const size_t end = std::min JSONCONS_NO_MACRO_EXP(buffer_length_, estimation_buffer_length_);
    while (!done & (pos < end))
    {
        switch (buffer_[pos])
        {
        case end_array:
            done = true;
            break;
        case begin_array:
            pos = skip_array(pos + 1, end);
            ++size;
            break;
        case begin_object:
            pos = skip_object(pos + 1, end);
            ++size;
            break;
        case '\"':
            pos = skip_string(pos + 1, end);
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
            pos = skip_number(pos + 1, end);
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
    const size_t end = std::min JSONCONS_NO_MACRO_EXP(buffer_length_, estimation_buffer_length_);
    while (!done & (pos < end))
    {
        switch (buffer_[pos])
        {
        case end_object:
            done = true;
            break;
        case '\"':
            pos = skip_string(pos + 1, end);
            break;
        case ':':
            ++size;
            ++pos;
            break;
        case begin_array:
            pos = skip_array(pos + 1, end);
            break;
        case begin_object:
            pos = skip_object(pos + 1, end);
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
        err_handler_->error(std::error_code(json_parser_errc::unexpected_eof, json_parser_category()), *this);
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
            err_handler_->error(std::error_code(json_parser_errc::invalid_codepoint_surrogate_pair, json_parser_category()), *this);
        }
    }
    return cp;
}

template<typename Char>
uint32_t basic_json_reader<Char>::decode_unicode_escape_sequence()
{
    if (hard_buffer_length_ - buffer_position_ < 4)
    {
        err_handler_->error(std::error_code(json_parser_errc::unexpected_eof, json_parser_category()), *this);
    }
    uint32_t cp = 0;
    size_t index = 0;
    while (index < 4)
    {
        c_ = buffer_[buffer_position_++];
        ++column_;
        const uint32_t u(c_ >= 0 ? c_ : 256 + c_);
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
            err_handler_->error(std::error_code(json_parser_errc::invalid_hex_escape_sequence, json_parser_category()), *this);
        }
        ++index;
    }
    if (index != 4)
    {
        err_handler_->error(std::error_code(json_parser_errc::invalid_unicode_escape_sequence, json_parser_category()), *this);
    }
    return cp;
}

typedef basic_json_reader<char> json_reader;
typedef basic_json_reader<wchar_t> wjson_reader;

}

#endif

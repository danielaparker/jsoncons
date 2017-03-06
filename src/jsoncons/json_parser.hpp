// Copyright 2015 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_PARSER_HPP
#define JSONCONS_JSON_PARSER_HPP

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include <jsoncons/jsoncons.hpp>
#include <jsoncons/json_input_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons/json_error_category.hpp>

namespace jsoncons {

template<class CharT> inline
bool try_string_to_uinteger(const CharT *s, size_t length, uint64_t& result)
{
    static const uint64_t max_value = (std::numeric_limits<uint64_t>::max)();
    static const uint64_t max_value_div_10 = max_value / 10;
    uint64_t n = 0;
    for (size_t i = 0; i < length; ++i)
    {
        uint64_t x = s[i] - '0';
        if (n > max_value_div_10)
        {
            return false;
        }
        n = n * 10;
        if (n > max_value - x)
        {
            return false;
        }

        n += x;
    }
    result = n;
    return true;
}

template<class CharT> inline
bool try_string_to_integer(bool has_neg, const CharT *s, size_t length, int64_t& result)
{
    if (has_neg)
    {
        static const int64_t min_value = (std::numeric_limits<int64_t>::min)();
        static const int64_t min_value_div_10 = min_value / 10;

        int64_t n = 0;
        const CharT* end = s+length; 
        for (const CharT* p = s; p < end; ++p)
        {
            int64_t x = *p - '0';
            if (n < min_value_div_10)
            {
                return false;
            }
            n = n * 10;
            if (n < min_value + x)
            {
                return false;
            }

            n -= x;
        }
        result = n;
        return true;
    }
    else
    {
        static const int64_t max_value = (std::numeric_limits<int64_t>::max)();
        static const int64_t max_value_div_10 = max_value / 10;

        int64_t n = 0;
        const CharT* end = s+length; 
        for (const CharT* p = s; p < end; ++p)
        {
            int64_t x = *p - '0';
            if (n > max_value_div_10)
            {
                return false;
            }
            n = n * 10;
            if (n > max_value - x)
            {
                return false;
            }

            n += x;
        }
        result = n;
        return true;
    }
}

enum class parse_state 
{
    root,
    start, 
    slash,  
    slash_slash, 
    slash_star, 
    slash_star_star,
    expect_comma_or_end,  
    object,
    expect_member_name_or_end, 
    expect_member_name, 
    expect_colon,
    expect_value_or_end,
    expect_value,
    array, 
    string,
    member_name,
    escape, 
    escape_u1, 
    escape_u2, 
    escape_u3, 
    escape_u4, 
    escape_expect_surrogate_pair1, 
    escape_expect_surrogate_pair2, 
    escape_u6, 
    escape_u7, 
    escape_u8, 
    escape_u9, 
    minus, 
    zero,  
    integer,
    fraction1,
    fraction2,
    exp1,
    exp2,
    exp3,
    n,
    t,  
    f,  
    cr,
    lf,
    done
};

template<class CharT>
class basic_json_parser : private basic_parsing_context<CharT>
{
    static const int default_initial_stack_capacity_ = 100;

    basic_default_parse_error_handler<CharT> default_err_handler_;
    std::vector<parse_state> stack_;
    basic_json_input_handler<CharT>& handler_;
    basic_parse_error_handler<CharT>& err_handler_;
    uint32_t cp_;
    uint32_t cp2_;
    std::basic_string<CharT> string_buffer_;
    bool is_negative_;

    size_t line_;
    size_t column_;
    int nesting_depth_;
    int initial_stack_capacity_;

    int max_depth_;
    string_to_double<CharT> str_to_double_;
    const CharT* begin_input_;
    const CharT* end_input_;
    const CharT* p_;
    std::pair<const CharT*,size_t> literal_;
    uint8_t precision_;
    size_t literal_index_;

    // Noncopyable and nonmoveable
    basic_json_parser(const basic_json_parser&) = delete;
    basic_json_parser& operator=(const basic_json_parser&) = delete;

public:

    basic_json_parser(basic_json_input_handler<CharT>& handler)
       : handler_(handler),
         err_handler_(default_err_handler_),
         cp_(0),
         cp2_(0),
         is_negative_(false),
         line_(1),
         column_(1),
         nesting_depth_(0), 
         initial_stack_capacity_(default_initial_stack_capacity_),
         precision_(0), 
         literal_index_(0),
         begin_input_(nullptr),
         end_input_(nullptr),
         p_(nullptr)
    {
        max_depth_ = (std::numeric_limits<int>::max)();

        stack_.reserve(initial_stack_capacity_);
        stack_.push_back(parse_state::root);
        stack_.push_back(parse_state::start);
    }

    basic_json_parser(basic_json_input_handler<CharT>& handler,
                      basic_parse_error_handler<CharT>& err_handler)
       : handler_(handler),
         err_handler_(err_handler),
         cp_(0),
         cp2_(0),
         is_negative_(false),
         line_(1),
         column_(1),
         nesting_depth_(0), 
         initial_stack_capacity_(default_initial_stack_capacity_),
         precision_(0), 
         literal_index_(0),
         begin_input_(nullptr),
         end_input_(nullptr),
         p_(nullptr)
    {
        max_depth_ = (std::numeric_limits<int>::max)();

        stack_.reserve(initial_stack_capacity_);
        stack_.push_back(parse_state::root);
        stack_.push_back(parse_state::start);
    }

    bool source_exhausted() const
    {
        return p_ == end_input_;
    }

    const basic_parsing_context<CharT>& parsing_context() const
    {
        return *this;
    }

    ~basic_json_parser()
    {
    }

    size_t max_nesting_depth() const
    {
        return static_cast<size_t>(max_depth_);
    }

    void max_nesting_depth(size_t max_nesting_depth)
    {
        max_depth_ = static_cast<int>((std::min)(max_nesting_depth,static_cast<size_t>((std::numeric_limits<int>::max)())));
    }

    parse_state parent() const
    {
        return stack_[stack_.size()-2];
    }

    bool done() const
    {
        return stack_.back() == parse_state::done;
    }

    void do_space()
    {
        while ((p_ + 1) < end_input_ && (*(p_ + 1) == ' ' || *(p_ + 1) == '\t')) 
        {                                      
            ++p_;                          
            ++column_;                     
        }                                      
    }

    void do_begin_object()
    {
        if (++nesting_depth_ >= max_depth_)
        {
            err_handler_.error(json_parser_errc::max_depth_exceeded, *this);
        }
        stack_.back() = parse_state::object;
        stack_.push_back(parse_state::expect_member_name_or_end);
        handler_.begin_object(*this);
    }

    void do_end_object()
    {
        --nesting_depth_;
        JSONCONS_ASSERT(!stack_.empty())
        stack_.pop_back();
        if (stack_.back() == parse_state::object)
        {
            handler_.end_object(*this);
        }
        else if (stack_.back() == parse_state::array)
        {
            err_handler_.fatal_error(json_parser_errc::expected_comma_or_right_bracket, *this);
        }
        else
        {
            err_handler_.fatal_error(json_parser_errc::unexpected_right_brace, *this);
        }

        JSONCONS_ASSERT(stack_.size() >= 2);
        if (parent() == parse_state::root)
        {
            stack_.back() = parse_state::done;
            handler_.end_json();
        }
        else
        {
            stack_.back() = parse_state::expect_comma_or_end;
        }
    }

    void do_begin_array()
    {
        if (++nesting_depth_ >= max_depth_)
        {
            err_handler_.error(json_parser_errc::max_depth_exceeded, *this);
        }
        stack_.back() = parse_state::array;
        stack_.push_back(parse_state::expect_value_or_end);
        handler_.begin_array(*this);
    }

    void do_end_array()
    {
        --nesting_depth_;
        JSONCONS_ASSERT(!stack_.empty())
        stack_.pop_back();
        if (stack_.back() == parse_state::array)
        {
            handler_.end_array(*this);
        }
        else if (stack_.back() == parse_state::object)
        {
            err_handler_.fatal_error(json_parser_errc::expected_comma_or_right_brace, *this);
        }
        else
        {
            err_handler_.fatal_error(json_parser_errc::unexpected_right_bracket, *this);
        }
        JSONCONS_ASSERT(stack_.size() >= 2);
        if (parent() == parse_state::root)
        {
            stack_.back() = parse_state::done;
            handler_.end_json();
        }
        else
        {
            stack_.back() = parse_state::expect_comma_or_end;
        }
    }

    void reset()
    {
        stack_.clear();
        stack_.reserve(initial_stack_capacity_);
        stack_.push_back(parse_state::root);
        stack_.push_back(parse_state::start);
        line_ = 1;
        column_ = 1;
        nesting_depth_ = 0;
    }

    void check_done()
    {
        JSONCONS_ASSERT(stack_.size() >= 1);
        if (stack_.back() != parse_state::done)
        {
            err_handler_.error(json_parser_errc::unexpected_eof, *this);
        }
        for (; p_ != end_input_; ++p_)
        {
            CharT curr_char_ = *p_;
            switch (curr_char_)
            {
            case '\n':
            case '\r':
            case '\t':
            case ' ':
                break;
            default:
                err_handler_.error(json_parser_errc::extra_character, *this);
                break;
            }
        }
    }
    void parse_string()
    {
        const CharT* sb = p_;
        bool done = false;
        while (!done && p_ < end_input_)
        {
            switch (*p_)
            {
            case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x08:case 0x0b:
            case 0x0c:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:case 0x15:case 0x16:
            case 0x17:case 0x18:case 0x19:case 0x1a:case 0x1b:case 0x1c:case 0x1d:case 0x1e:case 0x1f:
                string_buffer_.append(sb,p_-sb);
                column_ += (p_ - sb + 1);
                err_handler_.error(json_parser_errc::illegal_control_character, *this);
                // recovery - skip
                done = true;
                ++p_;
                break;
            case '\r':
                {
                    column_ += (p_ - sb + 1);
                    err_handler_.error(json_parser_errc::illegal_character_in_string, *this);
                    // recovery - keep
                    string_buffer_.append(sb, p_ - sb + 1);
                    stack_.push_back(parse_state::cr);
                    done = true;
                    ++p_;
                }
                break;
            case '\n':
                {
                    column_ += (p_ - sb + 1);
                    err_handler_.error(json_parser_errc::illegal_character_in_string, *this);
                    // recovery - keep
                    string_buffer_.append(sb, p_ - sb + 1);
                    stack_.push_back(parse_state::lf);
                    done = true;
                    ++p_;
                }
                break;
            case '\t':
                {
                    column_ += (p_ - sb + 1);
                    err_handler_.error(json_parser_errc::illegal_character_in_string, *this);
                    // recovery - keep
                    string_buffer_.append(sb, p_ - sb + 1);
                    done = true;
                    ++p_;
                }
                break;
            case '\\': 
                string_buffer_.append(sb,p_-sb);
                column_ += (p_ - sb + 1);
                stack_.back() = parse_state::escape;
                done = true;
                ++p_;
                break;
            case '\"':
                if (string_buffer_.length() == 0)
                {
                    auto result = unicons::validate(sb,p_);
                    if (result.first == unicons::conv_errc::ok)
                    {
                        end_string_value(sb,p_-sb);
                    }
                    else
                    {
                        error(result.first);
                    }
                }
                else
                {
                    string_buffer_.append(sb,p_-sb);
                    auto result = unicons::validate(string_buffer_.begin(),string_buffer_.end());
                    if (result.first == unicons::conv_errc::ok)
                    {
                        end_string_value(string_buffer_.data(),string_buffer_.length());
                        string_buffer_.clear();
                    }
                    else
                    {
                        error(result.first);
                    }
                }
                column_ += (p_ - sb + 1);
                done = true;
                ++p_;
                break;
            default:
                ++p_;
                break;
            }
        }
        if (!done)
        {
            string_buffer_.append(sb,p_-sb);
            column_ += (p_ - sb + 1);
        }
    }

    void error(unicons::conv_errc  result)
    {
        switch (result)
        {
        case unicons::conv_errc::ok:
            break;
        case unicons::conv_errc::over_long_utf8_sequence:
            err_handler_.error(json_parser_errc::over_long_utf8_sequence, *this);
            break;
        case unicons::conv_errc::unpaired_high_surrogate:
            err_handler_.error(json_parser_errc::unpaired_high_surrogate, *this);
            break;
        case unicons::conv_errc::expected_continuation_byte:
            err_handler_.error(json_parser_errc::expected_continuation_byte, *this);
            break;
        case unicons::conv_errc::illegal_surrogate_value:
            err_handler_.error(json_parser_errc::illegal_surrogate_value, *this);
            break;
        default:
            err_handler_.error(json_parser_errc::illegal_codepoint, *this);
            break;
        }
    }

    void skip_bom()
    {
        auto result = unicons::skip_bom(p_, end_input_);
        switch (result.first)
        {
        case unicons::encoding_errc::expected_u8_found_u16:
            err_handler_.fatal_error(json_parser_errc::expected_u8_found_u16, *this);
            break;
        case unicons::encoding_errc::expected_u8_found_u32:
            err_handler_.fatal_error(json_parser_errc::expected_u8_found_u32, *this);
            break;
        case unicons::encoding_errc::expected_u16_found_fffe:
            err_handler_.fatal_error(json_parser_errc::expected_u16_found_fffe, *this);
            break;
        case unicons::encoding_errc::expected_u32_found_fffe:
            err_handler_.fatal_error(json_parser_errc::expected_u32_found_fffe, *this);
            break;
        default: // ok
            break;
        }
        begin_input_ = result.second;
        column_ = begin_input_ - p_ + 1;
        p_ = begin_input_;
    }

    void parse()
    {
        while ((p_ < end_input_) && (stack_.back() != parse_state::done))
        {
            switch (*p_)
            {
            case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x08:case 0x0b:
            case 0x0c:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:case 0x15:case 0x16:
            case 0x17:case 0x18:case 0x19:case 0x1a:case 0x1b:case 0x1c:case 0x1d:case 0x1e:case 0x1f:
                err_handler_.error(json_parser_errc::illegal_control_character, *this);
                break;
            default:
                break;
            }

            switch (stack_.back())
            {
            case parse_state::cr:
                ++line_;
                column_ = 1;
                switch (*p_)
                {
                case '\n':
                    JSONCONS_ASSERT(!stack_.empty())
                    stack_.pop_back();
                    ++p_;
                    break;
                default:
                    JSONCONS_ASSERT(!stack_.empty())
                    stack_.pop_back();
                    break;
                }
                break;
            case parse_state::lf:
                ++line_;
                column_ = 1;
                JSONCONS_ASSERT(!stack_.empty())
                stack_.pop_back();
                break;
            case parse_state::start: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(parse_state::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(parse_state::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(parse_state::slash);
                        break;
                    case '{':
                        handler_.begin_json();
                        do_begin_object();
                        break;
                    case '[':
                        handler_.begin_json();
                        do_begin_array();
                        break;
                    case '\"':
                        handler_.begin_json();
                        stack_.back() = parse_state::string;
                        break;
                    case '-':
                        handler_.begin_json();
                        is_negative_ = true;
                        stack_.back() = parse_state::minus;
                        break;
                    case '0': 
                        handler_.begin_json();
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        handler_.begin_json();
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::integer;
                        break;
                    case 'f':
                        handler_.begin_json();
                        stack_.back() = parse_state::f;
                        literal_ = json_literals<CharT>::false_literal();
                        literal_index_ = 1;
                        break;
                    case 'n':
                        handler_.begin_json();
                        stack_.back() = parse_state::n;
                        literal_ = json_literals<CharT>::null_literal();
                        literal_index_ = 1;
                        break;
                    case 't':
                        handler_.begin_json();
                        stack_.back() = parse_state::t;
                        literal_ = json_literals<CharT>::true_literal();
                        literal_index_ = 1;
                        break;
                    case '}':
                        err_handler_.fatal_error(json_parser_errc::unexpected_right_brace, *this);
                        break;
                    case ']':
                        err_handler_.fatal_error(json_parser_errc::unexpected_right_bracket, *this);
                        break;
                    default:
                        err_handler_.fatal_error(json_parser_errc::invalid_json_text, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;

            case parse_state::expect_comma_or_end: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(parse_state::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(parse_state::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(parse_state::slash);
                        break;
                    case '}':
                        do_end_object();
                        break;
                    case ']':
                        do_end_array();
                        break;
                    case ',':
                        begin_member_or_element();
                        break;
                    default:
                        JSONCONS_ASSERT(stack_.size() >= 2);
                        if (parent() == parse_state::array)
                        {
                            err_handler_.error(json_parser_errc::expected_comma_or_right_bracket, *this);
                        }
                        else if (parent() == parse_state::object)
                        {
                            err_handler_.error(json_parser_errc::expected_comma_or_right_brace, *this);
                        }
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::expect_member_name_or_end: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(parse_state::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(parse_state::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(parse_state::slash);
                        break;
                    case '}':
                        do_end_object();
                        break;
                    case '\"':
                        stack_.back() = parse_state::member_name;
                        stack_.push_back(parse_state::string);
                        break;
                    case '\'':
                        err_handler_.error(json_parser_errc::single_quote, *this);
                        break;
                    default:
                        err_handler_.error(json_parser_errc::expected_name, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::expect_member_name: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(parse_state::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(parse_state::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(parse_state::slash);
                        break;
                    case '\"':
                        stack_.back() = parse_state::member_name;
                        stack_.push_back(parse_state::string);
                        break;
                    case '}':
                        err_handler_.error(json_parser_errc::extra_comma, *this);
                        do_end_object();  // Recover
                        break;
                    case '\'':
                        err_handler_.error(json_parser_errc::single_quote, *this);
                        break;
                    default:
                        err_handler_.error(json_parser_errc::expected_name, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::expect_colon: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(parse_state::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(parse_state::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(parse_state::slash);
                        break;
                    case ':':
                        stack_.back() = parse_state::expect_value;
                        break;
                    default:
                        err_handler_.error(json_parser_errc::expected_colon, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::expect_value: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(parse_state::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(parse_state::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(parse_state::slash);
                        break;
                    case '{':
                        do_begin_object();
                        break;
                    case '[':
                        do_begin_array();
                        break;
                    case '\"':
                        stack_.back() = parse_state::string;
                        break;
                    case '-':
                        is_negative_ = true;
                        stack_.back() = parse_state::minus;
                        break;
                    case '0': 
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::integer;
                        break;
                    case 'f':
                        stack_.back() = parse_state::f;
                        literal_ = json_literals<CharT>::false_literal();
                        literal_index_ = 1;
                        break;
                    case 'n':
                        stack_.back() = parse_state::n;
                        literal_ = json_literals<CharT>::null_literal();
                        literal_index_ = 1;
                        break;
                    case 't':
                        stack_.back() = parse_state::t;
                        literal_ = json_literals<CharT>::true_literal();
                        literal_index_ = 1;
                        break;
                    case ']':
                        JSONCONS_ASSERT(stack_.size() >= 2);
                        if (parent() == parse_state::array)
                        {
                            err_handler_.error(json_parser_errc::extra_comma, *this);
                            do_end_array();  // Recover
                        }
                        else
                        {
                            err_handler_.error(json_parser_errc::expected_value, *this);
                        }
                        break;
                    case '\'':
                        err_handler_.error(json_parser_errc::single_quote, *this);
                        break;
                    default:
                        err_handler_.error(json_parser_errc::expected_value, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::expect_value_or_end: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(parse_state::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(parse_state::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(parse_state::slash);
                        break;
                    case '{':
                        do_begin_object();
                        break;
                    case '[':
                        do_begin_array();
                        break;
                    case ']':
                        do_end_array();
                        break;
                    case '\"':
                        stack_.back() = parse_state::string;
                        break;
                    case '-':
                        is_negative_ = true;
                        stack_.back() = parse_state::minus;
                        break;
                    case '0': 
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::integer;
                        break;
                    case 'f':
                        stack_.back() = parse_state::f;
                        literal_ = json_literals<CharT>::false_literal();
                        literal_index_ = 1;
                        break;
                    case 'n':
                        stack_.back() = parse_state::n;
                        literal_ = json_literals<CharT>::null_literal();
                        literal_index_ = 1;
                        break;
                    case 't':
                        stack_.back() = parse_state::t;
                        literal_ = json_literals<CharT>::true_literal();
                        literal_index_ = 1;
                        break;
                    case '\'':
                        err_handler_.error(json_parser_errc::single_quote, *this);
                        break;
                    default:
                        err_handler_.error(json_parser_errc::expected_value, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::string: 
                parse_string();
                break;
            case parse_state::escape: 
                {
                    escape_next_char(*p_);
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u1: 
                {
                    append_codepoint(*p_);
                    stack_.back() = parse_state::escape_u2;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u2: 
                {
                    append_codepoint(*p_);
                    stack_.back() = parse_state::escape_u3;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u3: 
                {
                    append_codepoint(*p_);
                    stack_.back() = parse_state::escape_u4;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u4: 
                {
                    append_codepoint(*p_);
                    if (unicons::is_high_surrogate(cp_))
                    {
                        stack_.back() = parse_state::escape_expect_surrogate_pair1;
                    }
                    else
                    {
                        unicons::convert(&cp_, &cp_ + 1, std::back_inserter(string_buffer_));
                        stack_.back() = parse_state::string;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_expect_surrogate_pair1: 
                {
                    switch (*p_)
                    {
                    case '\\': 
                        cp2_ = 0;
                        stack_.back() = parse_state::escape_expect_surrogate_pair2;
                        break;
                    default:
                        err_handler_.error(json_parser_errc::expected_codepoint_surrogate_pair, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_expect_surrogate_pair2: 
                {
                    switch (*p_)
                    {
                    case 'u':
                        stack_.back() = parse_state::escape_u6;
                        break;
                    default:
                        err_handler_.error(json_parser_errc::expected_codepoint_surrogate_pair, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u6:
                {
                    append_second_codepoint(*p_);
                    stack_.back() = parse_state::escape_u7;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u7: 
                {
                    append_second_codepoint(*p_);
                    stack_.back() = parse_state::escape_u8;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u8: 
                {
                    append_second_codepoint(*p_);
                    stack_.back() = parse_state::escape_u9;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u9: 
                {
                    append_second_codepoint(*p_);
                    uint32_t cp = 0x10000 + ((cp_ & 0x3FF) << 10) + (cp2_ & 0x3FF);
                    unicons::convert(&cp, &cp + 1, std::back_inserter(string_buffer_));
                    stack_.back() = parse_state::string;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::minus:  
                {
                    switch (*p_)
                    {
                    case '0': 
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::integer;
                        break;
                    default:
                        err_handler_.error(json_parser_errc::expected_value, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::zero:  
                {
                    switch (*p_)
                    {
                    case '\r': 
                        end_integer_value();
                        stack_.push_back(parse_state::cr);
                        break; 
                    case '\n': 
                        end_integer_value();
                        stack_.push_back(parse_state::lf); 
                        break;   
                    case ' ':case '\t':
                        end_integer_value();
                        do_space();
                        break;
                    case '/': 
                        end_integer_value();
                        stack_.push_back(parse_state::slash);
                        break;
                    case '}':
                        end_integer_value();
                        do_end_object();
                        break;
                    case ']':
                        end_integer_value();
                        do_end_array();
                        break;
                    case '.':
                        precision_ = static_cast<uint8_t>(string_buffer_.length());
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::fraction1;
                        break;
                    case 'e':case 'E':
                        precision_ = static_cast<uint8_t>(string_buffer_.length());
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::exp1;
                        break;
                    case ',':
                        end_integer_value();
                        begin_member_or_element();
                        break;
                    case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        err_handler_.error(json_parser_errc::leading_zero, *this);
                        break;
                    default:
                        err_handler_.error(json_parser_errc::invalid_number, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::integer: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        end_integer_value();
                        stack_.push_back(parse_state::cr);
                        break; 
                    case '\n': 
                        end_integer_value();
                        stack_.push_back(parse_state::lf); 
                        break;   
                    case ' ':case '\t':
                        end_integer_value();
                        do_space();
                        break;
                    case '/': 
                        end_integer_value();
                        stack_.push_back(parse_state::slash);
                        break;
                    case '}':
                        end_integer_value();
                        do_end_object();
                        break;
                    case ']':
                        end_integer_value();
                        do_end_array();
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::integer;
                        break;
                    case '.':
                        precision_ = static_cast<uint8_t>(string_buffer_.length());
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::fraction1;
                        break;
                    case ',':
                        end_integer_value();
                        begin_member_or_element();
                        break;
                    case 'e':case 'E':
                        precision_ = static_cast<uint8_t>(string_buffer_.length());
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::exp1;
                        break;
                    default:
                        err_handler_.error(json_parser_errc::invalid_number, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::fraction1: 
                {
                    switch (*p_)
                    {
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        ++precision_;
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::fraction2;
                        break;
                    default:
                        err_handler_.error(json_parser_errc::invalid_number, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::fraction2: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        end_fraction_value();
                        stack_.push_back(parse_state::cr);
                        break; 
                    case '\n': 
                        end_fraction_value();
                        stack_.push_back(parse_state::lf); 
                        break;   
                    case ' ':case '\t':
                        end_fraction_value();
                        do_space();
                        break;
                    case '/': 
                        end_fraction_value();
                        stack_.push_back(parse_state::slash);
                        break;
                    case '}':
                        end_fraction_value();
                        do_end_object();
                        break;
                    case ']':
                        end_fraction_value();
                        do_end_array();
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        ++precision_;
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::fraction2;
                        break;
                    case ',':
                        end_fraction_value();
                        begin_member_or_element();
                        break;
                    case 'e':case 'E':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::exp1;
                        break;
                    default:
                        err_handler_.error(json_parser_errc::invalid_number, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::exp1: 
                {
                    switch (*p_)
                    {
                    case '+':
                        stack_.back() = parse_state::exp2;
                        break;
                    case '-':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::exp2;
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::exp3;
                        break;
                    default:
                        err_handler_.error(json_parser_errc::expected_value, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::exp2:  
                {
                    switch (*p_)
                    {
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::exp3;
                        break;
                    default:
                        err_handler_.error(json_parser_errc::expected_value, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::exp3: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        end_fraction_value();
                        stack_.push_back(parse_state::cr);
                        break; 
                    case '\n': 
                        end_fraction_value();
                        stack_.push_back(parse_state::lf); 
                        break;   
                    case ' ':case '\t':
                        end_fraction_value();
                        do_space();
                        break;
                    case '/': 
                        end_fraction_value();
                        stack_.push_back(parse_state::slash);
                        break;
                    case '}':
                        end_fraction_value();
                        do_end_object();
                        break;
                    case ']':
                        end_fraction_value();
                        do_end_array();
                        break;
                    case ',':
                        end_fraction_value();
                        begin_member_or_element();
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = parse_state::exp3;
                        break;
                    default:
                        err_handler_.error(json_parser_errc::invalid_number, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::t: 
                while (p_ < end_input_ && literal_index_ < literal_.second)
                {
                    if (*p_ != literal_.first[literal_index_])
                    {
                        err_handler_.error(json_parser_errc::invalid_value, *this);
                    }
                    ++p_;
                    ++literal_index_;
                    ++column_;
                }
                if (literal_index_ == literal_.second)
                {
                    handler_.value(true, *this);
                    JSONCONS_ASSERT(stack_.size() >= 2);
                    if (parent() == parse_state::root)
                    {
                        stack_.back() = parse_state::done;
                        handler_.end_json();
                    }
                    else
                    {
                        stack_.back() = parse_state::expect_comma_or_end;
                    }
                }
                break;
            case parse_state::f:  
                while (p_ < end_input_ && literal_index_ < literal_.second)
                {
                    if (*p_ != literal_.first[literal_index_])
                    {
                        err_handler_.error(json_parser_errc::invalid_value, *this);
                    }
                    ++p_;
                    ++literal_index_;
                    ++column_;
                }
                if (literal_index_ == literal_.second)
                {
                    handler_.value(false, *this);
                    JSONCONS_ASSERT(stack_.size() >= 2);
                    if (parent() == parse_state::root)
                    {
                        stack_.back() = parse_state::done;
                        handler_.end_json();
                    }
                    else
                    {
                        stack_.back() = parse_state::expect_comma_or_end;
                    }
                }
                break;
            case parse_state::n: 
                while (p_ < end_input_ && literal_index_ < literal_.second)
                {
                    if (*p_ != literal_.first[literal_index_])
                    {
                        err_handler_.error(json_parser_errc::invalid_value, *this);
                    }
                    ++p_;
                    ++literal_index_;
                    ++column_;
                }
                if (literal_index_ == literal_.second)
                {
                    handler_.value(null_type(), *this);
                    JSONCONS_ASSERT(stack_.size() >= 2);
                    if (parent() == parse_state::root)
                    {
                        stack_.back() = parse_state::done;
                        handler_.end_json();
                    }
                    else
                    {
                        stack_.back() = parse_state::expect_comma_or_end;
                    }
                }
                break;
            case parse_state::slash: 
                {
                    switch (*p_)
                    {
                    case '*':
                        stack_.back() = parse_state::slash_star;
                        err_handler_.error(json_parser_errc::illegal_comment, *this);
                        break;
                    case '/':
                        stack_.back() = parse_state::slash_slash;
                        err_handler_.error(json_parser_errc::illegal_comment, *this);
                        break;
                    default:    
                        err_handler_.error(json_parser_errc::invalid_json_text, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::slash_star:  
                {
                    switch (*p_)
                    {
                    case '\r':
                        stack_.push_back(parse_state::cr);
                        break;
                    case '\n':
                        stack_.push_back(parse_state::lf);
                        break;
                    case '*':
                        stack_.back() = parse_state::slash_star_star;
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::slash_slash: 
                {
                    switch (*p_)
                    {
                    case '\r':
                        stack_.pop_back();
                        break;
                    case '\n':
                        stack_.pop_back();
                        break;
                    default:
                        ++p_;
                        ++column_;
                    }
                }
                break;
            case parse_state::slash_star_star: 
                {
                    switch (*p_)
                    {
                    case '/':
                        JSONCONS_ASSERT(!stack_.empty())
                        stack_.pop_back();
                        break;
                    default:    
                        stack_.back() = parse_state::slash_star;
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            default:
                JSONCONS_THROW_EXCEPTION(std::runtime_error,"Bad parser state");
                break;
            }
        }
    }

    void end_parse()
    {
        JSONCONS_ASSERT(stack_.size() >= 2);
        
        if (parent() == parse_state::root)
        {
            switch (stack_.back())
            {
            case parse_state::zero:  
            case parse_state::integer:
                end_integer_value();
                break;
            case parse_state::fraction2:
            case parse_state::exp3:
                end_fraction_value();
                break;
            default:
                break;
            }
        }
        if (stack_.back() == parse_state::lf || stack_.back() == parse_state::cr)
        { 
            stack_.pop_back();
        }
        if (!(stack_.back() == parse_state::done || stack_.back() == parse_state::start))
        {
            err_handler_.error(json_parser_errc::unexpected_eof, *this);
        }
    }

    parse_state state() const
    {
        return stack_.back();
    }

    void set_source(const CharT* input, size_t length)
    {
        begin_input_ = input;
        end_input_ = input + length;
        p_ = begin_input_;
    }
private:
    void end_fraction_value()
    {
        try
        {
            double d = str_to_double_(string_buffer_.data(), precision_);
            if (is_negative_)
                d = -d;
            handler_.value(d, static_cast<uint8_t>(precision_), *this);
        }
        catch (...)
        {
            err_handler_.error(json_parser_errc::invalid_number, *this);
            handler_.value(null_type(), *this); // recovery
        }
        string_buffer_.clear();
        is_negative_ = false;

        JSONCONS_ASSERT(stack_.size() >= 2);
        switch (parent())
        {
        case parse_state::array:
        case parse_state::object:
            stack_.back() = parse_state::expect_comma_or_end;
            break;
        case parse_state::root:
            stack_.back() = parse_state::done;
            handler_.end_json();
            break;
        default:
            err_handler_.error(json_parser_errc::invalid_json_text, *this);
            break;
        }
    }

    void end_integer_value()
    {
        if (is_negative_)
        {
            int64_t d;
            if (try_string_to_integer(is_negative_, string_buffer_.data(), string_buffer_.length(),d))
            {
                handler_.value(d, *this);
            }
            else
            {
                try
                {
                    double d2 = str_to_double_(string_buffer_.data(), string_buffer_.length());
                    handler_.value(-d2, static_cast<uint8_t>(string_buffer_.length()), *this);
                }
                catch (...)
                {
                    err_handler_.error(json_parser_errc::invalid_number, *this);
                    handler_.value(null_type(), *this);
                }
            }
        }
        else
        {
            uint64_t d;
            if (try_string_to_uinteger(string_buffer_.data(), string_buffer_.length(),d))
            {
                handler_.value(d, *this);
            }
            else
            {
                try
                {
                    double d2 = str_to_double_(string_buffer_.data(),string_buffer_.length());
                    handler_.value(d2, static_cast<uint8_t>(string_buffer_.length()), *this);
                }
                catch (...)
                {
                    err_handler_.error(json_parser_errc::invalid_number, *this);
                    handler_.value(null_type(), *this);
                }
            }
        }

        JSONCONS_ASSERT(stack_.size() >= 2);
        switch (parent())
        {
        case parse_state::array:
        case parse_state::object:
            stack_.back() = parse_state::expect_comma_or_end;
            break;
        case parse_state::root:
            stack_.back() = parse_state::done;
            handler_.end_json();
            break;
        default:
            err_handler_.error(json_parser_errc::invalid_json_text, *this);
            break;
        }
        string_buffer_.clear();
        is_negative_ = false;
    }

    void append_codepoint(int c)
    {
        switch (c)
        {
        case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
        case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
        case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
            cp_ = append_to_codepoint(cp_, c);
            break;
        default:
            err_handler_.error(json_parser_errc::expected_value, *this);
            break;
        }
    }

    void append_second_codepoint(int c)
    {
        switch (c)
        {
        case '0': 
        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
        case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
        case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
            cp2_ = append_to_codepoint(cp2_, c);
            break;
        default:
            err_handler_.error(json_parser_errc::expected_value, *this);
            break;
        }
    }

    void escape_next_char(int next_input)
    {
        switch (next_input)
        {
        case '\"':
            string_buffer_.push_back('\"');
            stack_.back() = parse_state::string;
            break;
        case '\\': 
            string_buffer_.push_back('\\');
            stack_.back() = parse_state::string;
            break;
        case '/':
            string_buffer_.push_back('/');
            stack_.back() = parse_state::string;
            break;
        case 'b':
            string_buffer_.push_back('\b');
            stack_.back() = parse_state::string;
            break;
        case 'f':  
            string_buffer_.push_back('\f');
            stack_.back() = parse_state::string;
            break;
        case 'n':
            string_buffer_.push_back('\n');
            stack_.back() = parse_state::string;
            break;
        case 'r':
            string_buffer_.push_back('\r');
            stack_.back() = parse_state::string;
            break;
        case 't':
            string_buffer_.push_back('\t');
            stack_.back() = parse_state::string;
            break;
        case 'u':
            cp_ = 0;
            stack_.back() = parse_state::escape_u1;
            break;
        default:    
            err_handler_.error(json_parser_errc::illegal_escaped_character, *this);
            break;
        }
    }

    void end_string_value(const CharT* s, size_t length) 
    {
        JSONCONS_ASSERT(stack_.size() >= 2);
        switch (parent())
        {
        case parse_state::member_name:
            handler_.name(s, length, *this);
            stack_.pop_back();
            stack_.back() = parse_state::expect_colon;
            break;
        case parse_state::object:
        case parse_state::array:
            handler_.value(s, length, *this);
            stack_.back() = parse_state::expect_comma_or_end;
            break;
        case parse_state::root:
            handler_.value(s, length, *this);
            stack_.back() = parse_state::done;
            handler_.end_json();
            break;
        default:
            err_handler_.error(json_parser_errc::invalid_json_text, *this);
            break;
        }
    }

    void begin_member_or_element() 
    {
        JSONCONS_ASSERT(stack_.size() >= 2);
        switch (parent())
        {
        case parse_state::object:
            stack_.back() = parse_state::expect_member_name;
            break;
        case parse_state::array:
            stack_.back() = parse_state::expect_value;
            break;
        case parse_state::root:
            break;
        default:
            err_handler_.error(json_parser_errc::invalid_json_text, *this);
            break;
        }
    }
 
    uint32_t append_to_codepoint(uint32_t cp, int c)
    {
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
            err_handler_.error(json_parser_errc::invalid_hex_escape_sequence, *this);
        }
        return cp;
    }

    size_t do_line_number() const override
    {
        return line_;
    }

    size_t do_column_number() const override
    {
        return column_;
    }

    CharT do_current_char() const override
    {
        return p_ < end_input_? *p_ : 0;
    }
};

typedef basic_json_parser<char> json_parser;
typedef basic_json_parser<wchar_t> wjson_parser;

}

#endif


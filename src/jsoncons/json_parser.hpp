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
#include <jsoncons/json_text_traits.hpp>
#include <jsoncons/json_input_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons/json_error_category.hpp>

namespace jsoncons {

template<class CharT> inline
bool try_string_to_uinteger(const CharT *s, size_t length, uint64_t& result)
{
    static const uint64_t max_value = std::numeric_limits<uint64_t>::max JSONCONS_NO_MACRO_EXP();
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
    static const int64_t max_value = std::numeric_limits<int64_t>::max JSONCONS_NO_MACRO_EXP();
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
    result = has_neg ? -n : n;
    return true;
}

enum class string_states
{
    u1,
    u2,
    u3,
    u4
};

enum class states 
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

    std::vector<states> stack_;
    basic_json_input_handler<CharT> *handler_;
    basic_parse_error_handler<CharT> *err_handler_;
    size_t column_;
    size_t line_;
    uint32_t cp_;
    uint32_t cp2_;
    std::basic_string<CharT> string_buffer_;
    std::basic_string<char> number_buffer_;
    bool is_negative_;
    size_t index_;
    int initial_stack_capacity_;
    int max_depth_;
    float_reader float_reader_;
    const CharT* begin_input_;
    const CharT* end_input_;
    const CharT* p_;
    std::pair<const CharT*,size_t> literal_;
    int nesting_depth_;
    uint8_t precision_;
    size_t literal_index_;
    size_t continuation_count_;
    string_states string_state_;

public:
    basic_json_parser(basic_json_input_handler<CharT>& handler)
       : handler_(std::addressof(handler)),
         err_handler_(std::addressof(basic_default_parse_error_handler<CharT>::instance())),
         column_(0),
         line_(0),
         cp_(0),
         cp2_(0),
         is_negative_(false),
         index_(0),
         initial_stack_capacity_(default_initial_stack_capacity_),
         nesting_depth_(0), 
         precision_(0), 
         literal_index_(0),
         continuation_count_(0),
         string_state_(string_states::u1 )
    {
        max_depth_ = std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
    }

    basic_json_parser(basic_json_input_handler<CharT>& handler,
                      basic_parse_error_handler<CharT>& err_handler)
       : handler_(std::addressof(handler)),
         err_handler_(std::addressof(err_handler)),
         column_(0),
         line_(0),
         cp_(0),
         cp2_(0),
         is_negative_(false),
         index_(0),
         initial_stack_capacity_(default_initial_stack_capacity_),
         nesting_depth_(0), 
         precision_(0), 
         literal_index_(0),
         continuation_count_(0),
         string_state_(string_states::u1 )
    {
        max_depth_ = std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
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
        max_depth_ = static_cast<int>(std::min(max_nesting_depth,static_cast<size_t>(std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP())));
    }

    states parent() const
    {
        return stack_[stack_.size()-2];
    }

    bool done() const
    {
        return stack_.back() == states::done;
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
            err_handler_->error(json_parser_errc::max_depth_exceeded, *this);
        }
        stack_.back() = states::object;
        stack_.push_back(states::expect_member_name_or_end);
        handler_->begin_object(*this);
    }

    void do_end_object()
    {
        --nesting_depth_;
        JSONCONS_ASSERT(!stack_.empty())
        stack_.pop_back();
        if (stack_.back() == states::object)
        {
            handler_->end_object(*this);
        }
        else if (stack_.back() == states::array)
        {
            err_handler_->fatal_error(json_parser_errc::expected_comma_or_right_bracket, *this);
        }
        else
        {
            err_handler_->fatal_error(json_parser_errc::unexpected_right_brace, *this);
        }

        JSONCONS_ASSERT(stack_.size() >= 2);
        if (parent() == states::root)
        {
            stack_.back() = states::done;
            handler_->end_json();
        }
        else
        {
            stack_.back() = states::expect_comma_or_end;
        }
    }

    void do_begin_array()
    {
        if (++nesting_depth_ >= max_depth_)
        {
            err_handler_->error(json_parser_errc::max_depth_exceeded, *this);
        }
        stack_.back() = states::array;
        stack_.push_back(states::expect_value_or_end);
        handler_->begin_array(*this);
    }

    void do_end_array()
    {
        --nesting_depth_;
        JSONCONS_ASSERT(!stack_.empty())
        stack_.pop_back();
        if (stack_.back() == states::array)
        {
            handler_->end_array(*this);
        }
        else if (stack_.back() == states::object)
        {
            err_handler_->fatal_error(json_parser_errc::expected_comma_or_right_brace, *this);
        }
        else
        {
            err_handler_->fatal_error(json_parser_errc::unexpected_right_bracket, *this);
        }
        JSONCONS_ASSERT(stack_.size() >= 2);
        if (parent() == states::root)
        {
            stack_.back() = states::done;
            handler_->end_json();
        }
        else
        {
            stack_.back() = states::expect_comma_or_end;
        }
    }

    void begin_parse()
    {
        stack_.clear();
        stack_.reserve(initial_stack_capacity_);
        stack_.push_back(states::root);
        stack_.push_back(states::start);
        line_ = 1;
        column_ = 1;
        nesting_depth_ = 0;
    }

    void check_done(const CharT* input, size_t start, size_t length)
    {
        JSONCONS_ASSERT(stack_.size() >= 1);
        if (stack_.back() != states::done)
        {
            err_handler_->error(json_parser_errc::unexpected_eof, *this);
        }
        index_ = start;
        for (; index_ < length; ++index_)
        {
            CharT curr_char_ = input[index_];
            switch (curr_char_)
            {
            case '\n':
            case '\r':
            case '\t':
            case ' ':
                break;
            default:
                err_handler_->error(json_parser_errc::extra_character, *this);
                break;
            }
        }
    }

    template<class T=CharT>
    typename std::enable_if<(sizeof(T) == sizeof(uint8_t))>::type 
    parse_string()
    {
        const CharT* sb = p_;
        bool done = false;
        while (!done && p_ < end_input_)
        {
            switch (string_state_)
            {
            case string_states::u1 :
                {
                    switch (*p_)
                    {
                    case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x08:case 0x0b:
                    case 0x0c:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:case 0x15:case 0x16:
                    case 0x17:case 0x18:case 0x19:case 0x1a:case 0x1b:case 0x1c:case 0x1d:case 0x1e:case 0x1f:
                        string_buffer_.append(sb,p_-sb);
                        column_ += (p_ - sb + 1);
                        err_handler_->error(json_parser_errc::illegal_control_character, *this);
                        // recovery - skip
                        done = true;
                        ++p_;
                        break;
                    case '\r':
                        {
                            column_ += (p_ - sb + 1);
                            err_handler_->error(json_parser_errc::illegal_character_in_string, *this);
                            // recovery - keep
                            string_buffer_.append(sb, p_ - sb + 1);
                            stack_.push_back(states::cr);
                            done = true;
                            ++p_;
                        }
                        break;
                    case '\n':
                        {
                            column_ += (p_ - sb + 1);
                            err_handler_->error(json_parser_errc::illegal_character_in_string, *this);
                            // recovery - keep
                            string_buffer_.append(sb, p_ - sb + 1);
                            stack_.push_back(states::lf);
                            done = true;
                            ++p_;
                        }
                        break;
                    case '\t':
                        {
                            column_ += (p_ - sb + 1);
                            err_handler_->error(json_parser_errc::illegal_character_in_string, *this);
                            // recovery - keep
                            string_buffer_.append(sb, p_ - sb + 1);
                            done = true;
                            ++p_;
                        }
                        break;
                    case '\\': 
                        string_buffer_.append(sb,p_-sb);
                        column_ += (p_ - sb + 1);
                        stack_.back() = states::escape;
                        done = true;
                        ++p_;
                        break;
                    case '\"':
                        if (string_buffer_.length() == 0)
                        {
                            end_string_value(sb,p_-sb);
                        }
                        else
                        {
                            string_buffer_.append(sb,p_-sb);
                            end_string_value(string_buffer_.data(),string_buffer_.length());
                            string_buffer_.clear();
                        }
                        column_ += (p_ - sb + 1);
                        done = true;
                        ++p_;
                        break;
                    default:
                        if (static_cast<unsigned char>(*p_) < 0x80)
                        {
                        }
                        else if (is_continuation_byte(*p_))
                        {
                            err_handler_->error(json_parser_errc::unexpected_continuation_byte, *this);
                        }
                        else if (static_cast<unsigned char>(*p_) < 0xe0)
                        {
                            cp_  = (static_cast<unsigned char>(*p_) & 0x1f) << 6;
                            continuation_count_ = 1;
                            string_state_ = string_states::u2;
                        }
                        else if (static_cast<unsigned char>(*p_) < 0xf0)
                        {
                            continuation_count_ = 2;
                            cp_  = (static_cast<unsigned char>(*p_) & 0x0f) << 12;
                            string_state_ = string_states::u3;
                        }
                        else if (static_cast<unsigned char>(*p_) < 0xf8)
                        {
                            continuation_count_ = 3;
                            cp_  = (static_cast<unsigned char>(*p_) & 0x07) << 18;
                            string_state_ = string_states::u4;
                        }
                        else
                        {
                            err_handler_->error(json_parser_errc::illegal_character_in_string, *this);
                        }

                        ++p_;
                        break;
                    }
                }
                break;
            case string_states::u2:
                if (!is_continuation_byte(*p_))
                {
                    err_handler_->error(json_parser_errc::expected_continuation_byte, *this);
                }
                continuation_count_ = 0;
                string_state_ = string_states::u1;
                cp_ |= (static_cast<unsigned char>(*p_) & 0x3f);
                if (cp_ < 0x80) 
                {
                    err_handler_->error(json_parser_errc::over_long_utf8_sequence, *this);
                }
                ++p_;
                break;
            case string_states::u3:
                if (!is_continuation_byte(*p_))
                {
                    err_handler_->error(json_parser_errc::expected_continuation_byte, *this);
                }
                switch (continuation_count_)
                {
                case 2:
                    cp_ |= (static_cast<unsigned char>(*p_) & 0x3f) << 6;
                    --continuation_count_;
                    break;
                default:
                    cp_ |= (static_cast<unsigned char>(*p_) & 0x3f);
                    if (cp_ < 0x0800) 
                    {
                        err_handler_->error(json_parser_errc::over_long_utf8_sequence, *this);
                    }
                    if (cp_ >= 0xD800 && cp_ <= 0xDFFF) 
                    {
                        err_handler_->error(json_parser_errc::illegal_codepoint, *this);
                    }
                    continuation_count_ = 0;
                    string_state_ = string_states::u1;
                    break;
                }
                ++p_;
                break;
            case string_states::u4:
                if (!is_continuation_byte(*p_))
                {
                    err_handler_->error(json_parser_errc::expected_continuation_byte, *this);
                }
                switch (continuation_count_)
                {
                case 3:
                    cp_ |= (static_cast<unsigned char>(*p_) & 0x3f) << 12;
                    --continuation_count_;
                    break;
                case 2:
                    --continuation_count_;
                    cp_ |= (static_cast<unsigned char>(*p_) & 0x3f) << 6;
                    break;
                default:
                    cp_ |= (static_cast<unsigned char>(*p_) & 0x3f);
                    if (cp_ <= 0x010000) 
                    {
                        err_handler_->error(json_parser_errc::over_long_utf8_sequence, *this);
                    }
                    continuation_count_ = 0;
                    string_state_ = string_states::u1;
                    break;
                }
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

    template<class T=CharT>
    typename std::enable_if<(sizeof(T) == sizeof(uint16_t))>::type 
    parse_string()
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
                err_handler_->error(json_parser_errc::illegal_control_character, *this);
                // recovery - skip
                done = true;
                ++p_;
                break;
            case '\r':
                {
                    column_ += (p_ - sb + 1);
                    err_handler_->error(json_parser_errc::illegal_character_in_string, *this);
                    // recovery - keep
                    string_buffer_.append(sb, p_ - sb + 1);
                    stack_.push_back(states::cr);
                    done = true;
                    ++p_;
                }
                break;
            case '\n':
                {
                    column_ += (p_ - sb + 1);
                    err_handler_->error(json_parser_errc::illegal_character_in_string, *this);
                    // recovery - keep
                    string_buffer_.append(sb, p_ - sb + 1);
                    stack_.push_back(states::lf);
                    done = true;
                    ++p_;
                }
                break;
            case '\t':
                {
                    column_ += (p_ - sb + 1);
                    err_handler_->error(json_parser_errc::illegal_character_in_string, *this);
                    // recovery - keep
                    string_buffer_.append(sb, p_ - sb + 1);
                    done = true;
                    ++p_;
                }
                break;
            case '\\': 
                string_buffer_.append(sb,p_-sb);
                column_ += (p_ - sb + 1);
                stack_.back() = states::escape;
                done = true;
                ++p_;
                break;
            case '\"':
                if (string_buffer_.length() == 0)
                {
                    end_string_value(sb,p_-sb);
                }
                else
                {
                    string_buffer_.append(sb,p_-sb);
                    end_string_value(string_buffer_.data(),string_buffer_.length());
                    string_buffer_.clear();
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

    template<class T=CharT>
    typename std::enable_if<(sizeof(T) == sizeof(uint32_t))>::type 
    parse_string()
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
                err_handler_->error(json_parser_errc::illegal_control_character, *this);
                // recovery - skip
                done = true;
                ++p_;
                break;
            case '\r':
                {
                    column_ += (p_ - sb + 1);
                    err_handler_->error(json_parser_errc::illegal_character_in_string, *this);
                    // recovery - keep
                    string_buffer_.append(sb, p_ - sb + 1);
                    stack_.push_back(states::cr);
                    done = true;
                    ++p_;
                }
                break;
            case '\n':
                {
                    column_ += (p_ - sb + 1);
                    err_handler_->error(json_parser_errc::illegal_character_in_string, *this);
                    // recovery - keep
                    string_buffer_.append(sb, p_ - sb + 1);
                    stack_.push_back(states::lf);
                    done = true;
                    ++p_;
                }
                break;
            case '\t':
                {
                    column_ += (p_ - sb + 1);
                    err_handler_->error(json_parser_errc::illegal_character_in_string, *this);
                    // recovery - keep
                    string_buffer_.append(sb, p_ - sb + 1);
                    done = true;
                    ++p_;
                }
                break;
            case '\\': 
                string_buffer_.append(sb,p_-sb);
                column_ += (p_ - sb + 1);
                stack_.back() = states::escape;
                done = true;
                ++p_;
                break;
            case '\"':
                if (string_buffer_.length() == 0)
                {
                    end_string_value(sb,p_-sb);
                }
                else
                {
                    string_buffer_.append(sb,p_-sb);
                    end_string_value(string_buffer_.data(),string_buffer_.length());
                    string_buffer_.clear();
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

    void parse(const CharT* const input, size_t start, size_t length)
    {
        end_input_ = input + length;

        if (start == 0)
        {
            index_ = json_text_traits<CharT>::detect_bom(input,length);
            column_ = index_+1;
            begin_input_ = input + index_;
        }
        else
        {
            index_ = start;
            begin_input_ = input + start;
        }
        p_ = begin_input_;

        index_ = (start == 0) ? json_text_traits<CharT>::detect_bom(input,length) : start;
        while ((p_ < end_input_) && (stack_.back() != states::done))
        {
            switch (*p_)
            {
            case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x08:case 0x0b:
            case 0x0c:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:case 0x15:case 0x16:
            case 0x17:case 0x18:case 0x19:case 0x1a:case 0x1b:case 0x1c:case 0x1d:case 0x1e:case 0x1f:
                err_handler_->error(json_parser_errc::illegal_control_character, *this);
                break;
            default:
                break;
            }

            switch (stack_.back())
            {
            case states::cr:
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
            case states::lf:
                ++line_;
                column_ = 1;
                JSONCONS_ASSERT(!stack_.empty())
                stack_.pop_back();
                break;
            case states::start: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(states::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(states::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(states::slash);
                        break;
                    case '{':
                        handler_->begin_json();
                        do_begin_object();
                        break;
                    case '[':
                        handler_->begin_json();
                        do_begin_array();
                        break;
                    case '\"':
                        handler_->begin_json();
                        stack_.back() = states::string;
                        break;
                    case '-':
                        handler_->begin_json();
                        is_negative_ = true;
                        stack_.back() = states::minus;
                        break;
                    case '0': 
                        handler_->begin_json();
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        handler_->begin_json();
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::integer;
                        break;
                    case 'f':
                        handler_->begin_json();
                        stack_.back() = states::f;
                        literal_ = json_text_traits<CharT>::false_literal();
                        literal_index_ = 1;
                        break;
                    case 'n':
                        handler_->begin_json();
                        stack_.back() = states::n;
                        literal_ = json_text_traits<CharT>::null_literal();
                        literal_index_ = 1;
                        break;
                    case 't':
                        handler_->begin_json();
                        stack_.back() = states::t;
                        literal_ = json_text_traits<CharT>::true_literal();
                        literal_index_ = 1;
                        break;
                    case '}':
                        err_handler_->fatal_error(json_parser_errc::unexpected_right_brace, *this);
                        break;
                    case ']':
                        err_handler_->fatal_error(json_parser_errc::unexpected_right_bracket, *this);
                        break;
                    default:
                        err_handler_->fatal_error(json_parser_errc::invalid_json_text, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;

            case states::expect_comma_or_end: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(states::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(states::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(states::slash);
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
                        if (parent() == states::array)
                        {
                            err_handler_->error(json_parser_errc::expected_comma_or_right_bracket, *this);
                        }
                        else if (parent() == states::object)
                        {
                            err_handler_->error(json_parser_errc::expected_comma_or_right_brace, *this);
                        }
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::expect_member_name_or_end: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(states::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(states::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(states::slash);
                        break;
                    case '}':
                        do_end_object();
                        break;
                    case '\"':
                        stack_.back() = states::member_name;
                        stack_.push_back(states::string);
                        break;
                    case '\'':
                        err_handler_->error(json_parser_errc::single_quote, *this);
                        break;
                    default:
                        err_handler_->error(json_parser_errc::expected_name, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::expect_member_name: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(states::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(states::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(states::slash);
                        break;
                    case '\"':
                        stack_.back() = states::member_name;
                        stack_.push_back(states::string);
                        break;
                    case '}':
                        err_handler_->error(json_parser_errc::extra_comma, *this);
                        do_end_object();  // Recover
                        break;
                    case '\'':
                        err_handler_->error(json_parser_errc::single_quote, *this);
                        break;
                    default:
                        err_handler_->error(json_parser_errc::expected_name, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::expect_colon: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(states::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(states::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(states::slash);
                        break;
                    case ':':
                        stack_.back() = states::expect_value;
                        break;
                    default:
                        err_handler_->error(json_parser_errc::expected_colon, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::expect_value: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(states::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(states::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(states::slash);
                        break;
                    case '{':
                        do_begin_object();
                        break;
                    case '[':
                        do_begin_array();
                        break;
                    case '\"':
                        stack_.back() = states::string;
                        break;
                    case '-':
                        is_negative_ = true;
                        stack_.back() = states::minus;
                        break;
                    case '0': 
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::integer;
                        break;
                    case 'f':
                        stack_.back() = states::f;
                        literal_ = json_text_traits<CharT>::false_literal();
                        literal_index_ = 1;
                        break;
                    case 'n':
                        stack_.back() = states::n;
                        literal_ = json_text_traits<CharT>::null_literal();
                        literal_index_ = 1;
                        break;
                    case 't':
                        stack_.back() = states::t;
                        literal_ = json_text_traits<CharT>::true_literal();
                        literal_index_ = 1;
                        break;
                    case ']':
                        JSONCONS_ASSERT(stack_.size() >= 2);
                        if (parent() == states::array)
                        {
                            err_handler_->error(json_parser_errc::extra_comma, *this);
                            do_end_array();  // Recover
                        }
                        else
                        {
                            err_handler_->error(json_parser_errc::expected_value, *this);
                        }
                        break;
                    case '\'':
                        err_handler_->error(json_parser_errc::single_quote, *this);
                        break;
                    default:
                        err_handler_->error(json_parser_errc::expected_value, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::expect_value_or_end: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        stack_.push_back(states::cr);
                        break; 
                    case '\n': 
                        stack_.push_back(states::lf); 
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        stack_.push_back(states::slash);
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
                        stack_.back() = states::string;
                        break;
                    case '-':
                        is_negative_ = true;
                        stack_.back() = states::minus;
                        break;
                    case '0': 
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::integer;
                        break;
                    case 'f':
                        stack_.back() = states::f;
                        literal_ = json_text_traits<CharT>::false_literal();
                        literal_index_ = 1;
                        break;
                    case 'n':
                        stack_.back() = states::n;
                        literal_ = json_text_traits<CharT>::null_literal();
                        literal_index_ = 1;
                        break;
                    case 't':
                        stack_.back() = states::t;
                        literal_ = json_text_traits<CharT>::true_literal();
                        literal_index_ = 1;
                        break;
                    case '\'':
                        err_handler_->error(json_parser_errc::single_quote, *this);
                        break;
                    default:
                        err_handler_->error(json_parser_errc::expected_value, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::string: 
                parse_string();
                break;
            case states::escape: 
                {
                    escape_next_char(*p_);
                }
                ++p_;
                ++column_;
                break;
            case states::escape_u1: 
                {
                    append_codepoint(*p_);
                    stack_.back() = states::escape_u2;
                }
                ++p_;
                ++column_;
                break;
            case states::escape_u2: 
                {
                    append_codepoint(*p_);
                    stack_.back() = states::escape_u3;
                }
                ++p_;
                ++column_;
                break;
            case states::escape_u3: 
                {
                    append_codepoint(*p_);
                    stack_.back() = states::escape_u4;
                }
                ++p_;
                ++column_;
                break;
            case states::escape_u4: 
                {
                    append_codepoint(*p_);
                    if (cp_ >= min_lead_surrogate && cp_ <= max_lead_surrogate)
                    {
                        stack_.back() = states::escape_expect_surrogate_pair1;
                    }
                    else
                    {
                        json_text_traits<CharT>::append_codepoint_to_string(cp_, string_buffer_);
                        stack_.back() = states::string;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::escape_expect_surrogate_pair1: 
                {
                    switch (*p_)
                    {
                    case '\\': 
                        cp2_ = 0;
                        stack_.back() = states::escape_expect_surrogate_pair2;
                        break;
                    default:
                        err_handler_->error(json_parser_errc::expected_codepoint_surrogate_pair, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::escape_expect_surrogate_pair2: 
                {
                    switch (*p_)
                    {
                    case 'u':
                        stack_.back() = states::escape_u6;
                        break;
                    default:
                        err_handler_->error(json_parser_errc::expected_codepoint_surrogate_pair, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::escape_u6:
                {
                    append_second_codepoint(*p_);
                    stack_.back() = states::escape_u7;
                }
                ++p_;
                ++column_;
                break;
            case states::escape_u7: 
                {
                    append_second_codepoint(*p_);
                    stack_.back() = states::escape_u8;
                }
                ++p_;
                ++column_;
                break;
            case states::escape_u8: 
                {
                    append_second_codepoint(*p_);
                    stack_.back() = states::escape_u9;
                }
                ++p_;
                ++column_;
                break;
            case states::escape_u9: 
                {
                    append_second_codepoint(*p_);
                    uint32_t cp = 0x10000 + ((cp_ & 0x3FF) << 10) + (cp2_ & 0x3FF);
                    json_text_traits<CharT>::append_codepoint_to_string(cp, string_buffer_);
                    stack_.back() = states::string;
                }
                ++p_;
                ++column_;
                break;
            case states::minus:  
                {
                    switch (*p_)
                    {
                    case '0': 
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::integer;
                        break;
                    default:
                        err_handler_->error(json_parser_errc::expected_value, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::zero:  
                {
                    switch (*p_)
                    {
                    case '\r': 
                        end_integer_value();
                        stack_.push_back(states::cr);
                        break; 
                    case '\n': 
                        end_integer_value();
                        stack_.push_back(states::lf); 
                        break;   
                    case ' ':case '\t':
                        end_integer_value();
                        do_space();
                        break;
                    case '/': 
                        end_integer_value();
                        stack_.push_back(states::slash);
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
                        precision_ = static_cast<uint8_t>(number_buffer_.length());
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::fraction1;
                        break;
                    case 'e':case 'E':
                        precision_ = static_cast<uint8_t>(number_buffer_.length());
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::exp1;
                        break;
                    case ',':
                        end_integer_value();
                        begin_member_or_element();
                        break;
                    case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        err_handler_->error(json_parser_errc::leading_zero, *this);
                        break;
                    default:
                        err_handler_->error(json_parser_errc::invalid_number, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::integer: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        end_integer_value();
                        stack_.push_back(states::cr);
                        break; 
                    case '\n': 
                        end_integer_value();
                        stack_.push_back(states::lf); 
                        break;   
                    case ' ':case '\t':
                        end_integer_value();
                        do_space();
                        break;
                    case '/': 
                        end_integer_value();
                        stack_.push_back(states::slash);
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
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::integer;
                        break;
                    case '.':
                        precision_ = static_cast<uint8_t>(number_buffer_.length());
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::fraction1;
                        break;
                    case ',':
                        end_integer_value();
                        begin_member_or_element();
                        break;
                    case 'e':case 'E':
                        precision_ = static_cast<uint8_t>(number_buffer_.length());
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::exp1;
                        break;
                    default:
                        err_handler_->error(json_parser_errc::invalid_number, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::fraction1: 
                {
                    switch (*p_)
                    {
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        ++precision_;
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::fraction2;
                        break;
                    default:
                        err_handler_->error(json_parser_errc::invalid_number, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::fraction2: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        end_fraction_value();
                        stack_.push_back(states::cr);
                        break; 
                    case '\n': 
                        end_fraction_value();
                        stack_.push_back(states::lf); 
                        break;   
                    case ' ':case '\t':
                        end_fraction_value();
                        do_space();
                        break;
                    case '/': 
                        end_fraction_value();
                        stack_.push_back(states::slash);
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
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::fraction2;
                        break;
                    case ',':
                        end_fraction_value();
                        begin_member_or_element();
                        break;
                    case 'e':case 'E':
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::exp1;
                        break;
                    default:
                        err_handler_->error(json_parser_errc::invalid_number, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::exp1: 
                {
                    switch (*p_)
                    {
                    case '+':
                        stack_.back() = states::exp2;
                        break;
                    case '-':
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::exp2;
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::exp3;
                        break;
                    default:
                        err_handler_->error(json_parser_errc::expected_value, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::exp2:  
                {
                    switch (*p_)
                    {
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::exp3;
                        break;
                    default:
                        err_handler_->error(json_parser_errc::expected_value, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::exp3: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        end_fraction_value();
                        stack_.push_back(states::cr);
                        break; 
                    case '\n': 
                        end_fraction_value();
                        stack_.push_back(states::lf); 
                        break;   
                    case ' ':case '\t':
                        end_fraction_value();
                        do_space();
                        break;
                    case '/': 
                        end_fraction_value();
                        stack_.push_back(states::slash);
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
                        number_buffer_.push_back(static_cast<char>(*p_));
                        stack_.back() = states::exp3;
                        break;
                    default:
                        err_handler_->error(json_parser_errc::invalid_number, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::t: 
                while (p_ < end_input_ && literal_index_ < literal_.second)
                {
                    if (*p_ != literal_.first[literal_index_])
                    {
                        err_handler_->error(json_parser_errc::invalid_value, *this);
                    }
                    ++p_;
                    ++literal_index_;
                    ++column_;
                }
                if (literal_index_ == literal_.second)
                {
                    handler_->value(true, *this);
                    JSONCONS_ASSERT(stack_.size() >= 2);
                    if (parent() == states::root)
                    {
                        stack_.back() = states::done;
                        handler_->end_json();
                    }
                    else
                    {
                        stack_.back() = states::expect_comma_or_end;
                    }
                }
                break;
            case states::f:  
                while (p_ < end_input_ && literal_index_ < literal_.second)
                {
                    if (*p_ != literal_.first[literal_index_])
                    {
                        err_handler_->error(json_parser_errc::invalid_value, *this);
                    }
                    ++p_;
                    ++literal_index_;
                    ++column_;
                }
                if (literal_index_ == literal_.second)
                {
                    handler_->value(false, *this);
                    JSONCONS_ASSERT(stack_.size() >= 2);
                    if (parent() == states::root)
                    {
                        stack_.back() = states::done;
                        handler_->end_json();
                    }
                    else
                    {
                        stack_.back() = states::expect_comma_or_end;
                    }
                }
                break;
            case states::n: 
                while (p_ < end_input_ && literal_index_ < literal_.second)
                {
                    if (*p_ != literal_.first[literal_index_])
                    {
                        err_handler_->error(json_parser_errc::invalid_value, *this);
                    }
                    ++p_;
                    ++literal_index_;
                    ++column_;
                }
                if (literal_index_ == literal_.second)
                {
                    handler_->value(null_type(), *this);
                    JSONCONS_ASSERT(stack_.size() >= 2);
                    if (parent() == states::root)
                    {
                        stack_.back() = states::done;
                        handler_->end_json();
                    }
                    else
                    {
                        stack_.back() = states::expect_comma_or_end;
                    }
                }
                break;
            case states::slash: 
                {
                    switch (*p_)
                    {
                    case '*':
                        stack_.back() = states::slash_star;
                        err_handler_->error(json_parser_errc::illegal_comment, *this);
                        break;
                    case '/':
                        stack_.back() = states::slash_slash;
                        err_handler_->error(json_parser_errc::illegal_comment, *this);
                        break;
                    default:    
                        err_handler_->error(json_parser_errc::invalid_json_text, *this);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::slash_star:  
                {
                    switch (*p_)
                    {
                    case '\r':
                        stack_.push_back(states::cr);
                        break;
                    case '\n':
                        stack_.push_back(states::lf);
                        break;
                    case '*':
                        stack_.back() = states::slash_star_star;
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case states::slash_slash: 
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
            case states::slash_star_star: 
                {
                    switch (*p_)
                    {
                    case '/':
                        JSONCONS_ASSERT(!stack_.empty())
                        stack_.pop_back();
                        break;
                    default:    
                        stack_.back() = states::slash_star;
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
        index_ += (p_-begin_input_);
    }

    void end_parse()
    {
        JSONCONS_ASSERT(stack_.size() >= 2);
        
        if (parent() == states::root)
        {
            switch (stack_.back())
            {
            case states::zero:  
            case states::integer:
                end_integer_value();
                break;
            case states::fraction2:
            case states::exp3:
                end_fraction_value();
                break;
            default:
                break;
            }
        }
        if (stack_.back() == states::lf || stack_.back() == states::cr)
        { 
            stack_.pop_back();
        }
        if (!(stack_.back() == states::done || stack_.back() == states::start))
        {
            err_handler_->error(json_parser_errc::unexpected_eof, *this);
        }
    }

    states state() const
    {
        return stack_.back();
    }

    size_t index() const
    {
        return index_;
    }
private:
    void end_fraction_value()
    {
        try
        {
            double d = float_reader_.read(number_buffer_.data(), precision_);
            if (is_negative_)
                d = -d;
            handler_->value(d, static_cast<uint8_t>(precision_), *this);
        }
        catch (...)
        {
            err_handler_->error(json_parser_errc::invalid_number, *this);
            handler_->value(null_type(), *this); // recovery
        }
        number_buffer_.clear();
        is_negative_ = false;

        JSONCONS_ASSERT(stack_.size() >= 2);
        switch (parent())
        {
        case states::array:
        case states::object:
            stack_.back() = states::expect_comma_or_end;
            break;
        case states::root:
            stack_.back() = states::done;
            handler_->end_json();
            break;
        default:
            err_handler_->error(json_parser_errc::invalid_json_text, *this);
            break;
        }
    }

    void end_integer_value()
    {
        if (is_negative_)
        {
            int64_t d;
            if (try_string_to_integer(is_negative_, number_buffer_.data(), number_buffer_.length(),d))
            {
                handler_->value(d, *this);
            }
            else
            {
                try
                {
                    double d2 = float_reader_.read(number_buffer_.data(), number_buffer_.length());
                    handler_->value(-d2, static_cast<uint8_t>(number_buffer_.length()), *this);
                }
                catch (...)
                {
                    err_handler_->error(json_parser_errc::invalid_number, *this);
                    handler_->value(null_type(), *this);
                }
            }
        }
        else
        {
            uint64_t d;
            if (try_string_to_uinteger(number_buffer_.data(), number_buffer_.length(),d))
            {
                handler_->value(d, *this);
            }
            else
            {
                try
                {
                    double d2 = float_reader_.read(number_buffer_.data(),number_buffer_.length());
                    handler_->value(d2, static_cast<uint8_t>(number_buffer_.length()), *this);
                }
                catch (...)
                {
                    err_handler_->error(json_parser_errc::invalid_number, *this);
                    handler_->value(null_type(), *this);
                }
            }
        }

        JSONCONS_ASSERT(stack_.size() >= 2);
        switch (parent())
        {
        case states::array:
        case states::object:
            stack_.back() = states::expect_comma_or_end;
            break;
        case states::root:
            stack_.back() = states::done;
            handler_->end_json();
            break;
        default:
            err_handler_->error(json_parser_errc::invalid_json_text, *this);
            break;
        }
        number_buffer_.clear();
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
            err_handler_->error(json_parser_errc::expected_value, *this);
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
            err_handler_->error(json_parser_errc::expected_value, *this);
            break;
        }
    }

    void escape_next_char(int next_input)
    {
        switch (next_input)
        {
        case '\"':
            string_buffer_.push_back('\"');
            stack_.back() = states::string;
            break;
        case '\\': 
            string_buffer_.push_back('\\');
            stack_.back() = states::string;
            break;
        case '/':
            string_buffer_.push_back('/');
            stack_.back() = states::string;
            break;
        case 'b':
            string_buffer_.push_back('\b');
            stack_.back() = states::string;
            break;
        case 'f':  
            string_buffer_.push_back('\f');
            stack_.back() = states::string;
            break;
        case 'n':
            string_buffer_.push_back('\n');
            stack_.back() = states::string;
            break;
        case 'r':
            string_buffer_.push_back('\r');
            stack_.back() = states::string;
            break;
        case 't':
            string_buffer_.push_back('\t');
            stack_.back() = states::string;
            break;
        case 'u':
            cp_ = 0;
            stack_.back() = states::escape_u1;
            break;
        default:    
            err_handler_->error(json_parser_errc::illegal_escaped_character, *this);
            break;
        }
    }

    void end_string_value(const CharT* s, size_t length) 
    {
        JSONCONS_ASSERT(stack_.size() >= 2);
        switch (parent())
        {
        case states::member_name:
            handler_->name(s, length, *this);
            stack_.pop_back();
            stack_.back() = states::expect_colon;
            break;
        case states::object:
        case states::array:
            handler_->value(s, length, *this);
            stack_.back() = states::expect_comma_or_end;
            break;
        case states::root:
            handler_->value(s, length, *this);
            stack_.back() = states::done;
            handler_->end_json();
            break;
        default:
            err_handler_->error(json_parser_errc::invalid_json_text, *this);
            break;
        }
    }

    void begin_member_or_element() 
    {
        JSONCONS_ASSERT(stack_.size() >= 2);
        switch (parent())
        {
        case states::object:
            stack_.back() = states::expect_member_name;
            break;
        case states::array:
            stack_.back() = states::expect_value;
            break;
        case states::root:
            break;
        default:
            err_handler_->error(json_parser_errc::invalid_json_text, *this);
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
            err_handler_->error(json_parser_errc::invalid_hex_escape_sequence, *this);
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


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
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_input_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons/json_error_category.hpp>

#define JSONCONS_ILLEGAL_CONTROL_CHARACTER \
        case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x08:case 0x0b: \
        case 0x0c:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:case 0x15:case 0x16: \
        case 0x17:case 0x18:case 0x19:case 0x1a:case 0x1b:case 0x1c:case 0x1d:case 0x1e:case 0x1f 

namespace jsoncons {

// try_string_to_uinteger

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

enum class parse_state : uint8_t 
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
    string_u1,
    string_u2,
    string_u3,
    string_u4,
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

class json_parser : private parsing_context
{
    static const int default_initial_stack_capacity_ = 100;
    typedef json_input_handler::string_view_type string_view_type;

    basic_null_json_input_handler<char> default_input_handler_;
    default_parse_error_handler default_err_handler_;

    basic_json_input_handler<char>& handler_;
    parse_error_handler& err_handler_;
    uint32_t cp_;
    uint32_t cp2_;
    std::basic_string<char> string_buffer_;
    bool is_negative_;

    size_t line_;
    size_t column_;
    int nesting_depth_;
    int initial_stack_capacity_;

    int max_depth_;
    string_to_double<char> str_to_double_;
    uint8_t precision_;
    size_t literal_index_;
    const char* begin_input_;
    const char* end_input_;
    const char* p_;
    std::pair<const char*,size_t> literal_;

    parse_state state_;
    size_t continuation_count_;   
    std::vector<parse_state> state_stack_;

    // Noncopyable and nonmoveable
    json_parser(const json_parser&) = delete;
    json_parser& operator=(const json_parser&) = delete;

public:

    json_parser()
       : handler_(default_input_handler_),
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
         p_(nullptr),
         state_(parse_state::start),
         continuation_count_(0)
    {
        max_depth_ = (std::numeric_limits<int>::max)();

        state_stack_.reserve(initial_stack_capacity_);
        push_state(parse_state::root);
    }

    json_parser(parse_error_handler& err_handler)
       : handler_(default_input_handler_),
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
         p_(nullptr),
         state_(parse_state::start),
         continuation_count_(0)
    {
        max_depth_ = (std::numeric_limits<int>::max)();

        state_stack_.reserve(initial_stack_capacity_);
        push_state(parse_state::root);
    }

    json_parser(basic_json_input_handler<char>& handler)
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
         p_(nullptr),
         state_(parse_state::start),
         continuation_count_(0)
    {
        max_depth_ = (std::numeric_limits<int>::max)();

        state_stack_.reserve(initial_stack_capacity_);
        push_state(parse_state::root);
    }

    json_parser(basic_json_input_handler<char>& handler,
                      parse_error_handler& err_handler)
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
         p_(nullptr),
         state_(parse_state::start),
         continuation_count_(0)
    {
        max_depth_ = (std::numeric_limits<int>::max)();

        state_stack_.reserve(initial_stack_capacity_);
        push_state(parse_state::root);
    }

    size_t line_number() const
    {
        return line_;
    }

    size_t column_number() const
    {
        return column_;
    }

    void set_column_number(size_t column)
    {
        column_ = column;
    }

    bool source_exhausted() const
    {
        return p_ == end_input_;
    }

    const parsing_context& parsing_context() const
    {
        return *this;
    }

    ~json_parser()
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
        JSONCONS_ASSERT(state_stack_.size() >= 1);
        return state_stack_.back();
    }

    bool done() const
    {
        return state_ == parse_state::done;
    }

    void do_space()
    {
        while ((p_ + 1) < end_input_ && (*(p_ + 1) == ' ' || *(p_ + 1) == '\t')) 
        {                                      
            ++p_;                          
            ++column_;                     
        }                                      
    }

    void do_begin_object(std::error_code& ec)
    {
        if (++nesting_depth_ >= max_depth_)
        {
            if (err_handler_.error(json_parser_errc::max_depth_exceeded, *this))
            {
                ec = json_parser_errc::max_depth_exceeded;
                return;
            }
        } 
        push_state(parse_state::object);
        state_ = parse_state::expect_member_name_or_end;
        handler_.begin_object(*this);
    }

    void do_end_object(std::error_code& ec)
    {
        --nesting_depth_;
        state_ = pop_state();
        if (state_ == parse_state::object)
        {
            handler_.end_object(*this);
        }
        else if (state_ == parse_state::array)
        {
            err_handler_.fatal_error(json_parser_errc::expected_comma_or_right_bracket, *this);
            ec = json_parser_errc::expected_comma_or_right_bracket;
            return;
        }
        else
        {
            err_handler_.fatal_error(json_parser_errc::unexpected_right_brace, *this);
            ec = json_parser_errc::unexpected_right_brace;
            return;
        }

        if (parent() == parse_state::root)
        {
            state_ = parse_state::done;
            handler_.end_json();
        }
        else
        {
            state_ = parse_state::expect_comma_or_end;
        }
    }

    void do_begin_array(std::error_code& ec)
    {
        if (++nesting_depth_ >= max_depth_)
        {
            if (err_handler_.error(json_parser_errc::max_depth_exceeded, *this))
            {
                ec = json_parser_errc::max_depth_exceeded;
                return;
            }

        }
        push_state(parse_state::array);
        state_ = parse_state::expect_value_or_end;
        handler_.begin_array(*this);
    }

    void do_end_array(std::error_code& ec)
    {
        --nesting_depth_;
        state_ = pop_state();
        if (state_ == parse_state::array)
        {
            handler_.end_array(*this);
        }
        else if (state_ == parse_state::object)
        {
            err_handler_.fatal_error(json_parser_errc::expected_comma_or_right_brace, *this);
            ec = json_parser_errc::expected_comma_or_right_brace;
            return;
        }
        else
        {
            err_handler_.fatal_error(json_parser_errc::unexpected_right_bracket, *this);
            ec = json_parser_errc::unexpected_right_bracket;
            return;
        }
        if (parent() == parse_state::root)
        {
            state_ = parse_state::done;
            handler_.end_json();
        }
        else
        {
            state_ = parse_state::expect_comma_or_end;
        }
    }

    void reset()
    {
        state_stack_.clear();
        state_stack_.reserve(initial_stack_capacity_);
        push_state(parse_state::root);
        state_ = parse_state::start;
        line_ = 1;
        column_ = 1;
        nesting_depth_ = 0;
    }

    void check_done()
    {
        std::error_code ec;
        check_done(ec);
        if (ec)
        {
            throw parse_error(ec,line_,column_);
        }
    }

    void check_done(std::error_code& ec)
    {
        if (state_ != parse_state::done)
        {
            if (err_handler_.error(json_parser_errc::unexpected_eof, *this))
            {
                ec = json_parser_errc::unexpected_eof;
                return;
            }
        }
        for (; p_ != end_input_; ++p_)
        {
            char curr_char_ = *p_;
            switch (curr_char_)
            {
            case '\n':
            case '\r':
            case '\t':
            case ' ':
                break;
            default:
                if (err_handler_.error(json_parser_errc::extra_character, *this))
                {
                    ec = json_parser_errc::extra_character;
                    return;
                }
                break;
            }
        }
    }

    void parse_string(std::error_code& ec)
    {
        //std::cout << "enter parse_string " << (int)state_ << std::endl;

        const char* local_end_input = end_input_;
        const char* sb = p_;

        switch (state_)
        {
        case parse_state::string_u1:
            goto string_u1;
        case parse_state::string_u2:
            goto string_u2;
        case parse_state::string_u3:
            goto string_u3;
        case parse_state::string_u4:
            goto string_u4;
        default:
            JSONCONS_UNREACHABLE();               
        }
        
string_u1:
        //std::cout << "parse_string string_u1" << std::endl;

        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            string_buffer_.append(sb,p_-sb);
            column_ += (p_ - sb + 1);

            state_ = parse_state::string_u1;
            return;
        }
        switch (*p_)
        {
            JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                string_buffer_.append(sb,p_-sb);
                column_ += (p_ - sb + 1);
                if (err_handler_.error(json_parser_errc::illegal_control_character, *this))
                {
                    ec = json_parser_errc::illegal_control_character;

                    state_ = parse_state::string_u1;
                    return;
                }
                // recovery - skip
                ++p_;
                goto string_u1;
            case '\r':
                {
                    column_ += (p_ - sb + 1);
                    if (err_handler_.error(json_parser_errc::illegal_character_in_string, *this))
                    {
                        ec = json_parser_errc::illegal_character_in_string;

                        state_ = parse_state::string_u1;
                        return;
                    }
                    // recovery - keep
                    string_buffer_.append(sb, p_ - sb + 1);

                    ++p_;
                    push_state(state_);
                    state_ = parse_state::cr;
                    return;
                }
            case '\n':
                {
                    column_ += (p_ - sb + 1);
                    if (err_handler_.error(json_parser_errc::illegal_character_in_string, *this))
                    {
                        ec = json_parser_errc::illegal_character_in_string;
                        return;
                    }
                    // recovery - keep
                    string_buffer_.append(sb, p_ - sb + 1);
                    ++p_;
                    push_state(state_);
                    state_ = parse_state::lf;
                    return;
                }
            case '\t':
                {
                    column_ += (p_ - sb + 1);
                    if (err_handler_.error(json_parser_errc::illegal_character_in_string, *this))
                    {
                        ec = json_parser_errc::illegal_character_in_string;
                        return;
                    }
                    // recovery - keep
                    string_buffer_.append(sb, p_ - sb + 1);
                    ++p_;
                    state_ = parse_state::string_u1;
                    return;
                }
            case '\\': 
                string_buffer_.append(sb,p_-sb);
                column_ += (p_ - sb + 1);
                ++p_;
                state_ = parse_state::escape;
                return;
            case '\"':
                if (string_buffer_.length() == 0)
                {
                     end_string_value(sb,p_-sb, ec);
                }
                else
                {
                    string_buffer_.append(sb,p_-sb);
                    end_string_value(string_buffer_.data(),string_buffer_.length(), ec);
                    string_buffer_.clear();
                }
                column_ += (p_ - sb + 1);
                ++p_;
                return;
            default:
                if (static_cast<unsigned char>(*p_) < 0x80)
                {
                    ++p_;
                    goto string_u1;
                }
                else if (unicons::is_continuation_byte(*p_))
                {
                    err_handler_.error(json_parser_errc::unexpected_continuation_byte, *this);
                    ec = json_parser_errc::unexpected_continuation_byte;
                    ++p_;
                    return;
                }
                else if (static_cast<unsigned char>(*p_) < 0xe0)
                {
                    cp_  = (static_cast<unsigned char>(*p_) & 0x1f) << 6;
                    ++p_;
                    continuation_count_ = 1;
                    goto string_u2;
                }
                else if (static_cast<unsigned char>(*p_) < 0xf0)
                {
                    cp_  = (static_cast<unsigned char>(*p_) & 0x0f) << 12;
                    ++p_;
                    continuation_count_ = 2;
                    goto string_u3;
                }
                else if (static_cast<unsigned char>(*p_) < 0xf8)
                {
                    cp_  = (static_cast<unsigned char>(*p_) & 0x07) << 18;
                    ++p_;
                    continuation_count_ = 3;
                    goto string_u4;
                }
                else
                {
                    err_handler_.error(json_parser_errc::illegal_character_in_string, *this);
                    ec = json_parser_errc::illegal_character_in_string;
                    ++p_;
                    state_ = parse_state::string_u1;
                    return;
                }
            }
        
string_u2:
        //std::cout << "parse_string string_u2" << std::endl;

        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            string_buffer_.append(sb,p_-sb);
            column_ += (p_ - sb + 1);
            state_ = parse_state::string_u2;
            return;
        }
        if (!unicons::is_continuation_byte(*p_))
        {
            err_handler_.error(json_parser_errc::expected_continuation_byte, *this);
            ec = json_parser_errc::expected_continuation_byte;
            return;
        }
        continuation_count_ = 0;
        state_ = parse_state::string_u1;
        cp_ |= (static_cast<unsigned char>(*p_) & 0x3f);
        if (cp_ < 0x80) 
        {
            err_handler_.error(json_parser_errc::over_long_utf8_sequence, *this);
            ec = json_parser_errc::over_long_utf8_sequence;
            return;
        }
        ++p_;
        goto string_u1;

string_u3:
        //std::cout << "parse_string string_u3" << std::endl;

        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            string_buffer_.append(sb,p_-sb);
            column_ += (p_ - sb + 1);
            state_ = parse_state::string_u3;
            return;
        }
        if (!unicons::is_continuation_byte(*p_))
        {
            err_handler_.error(json_parser_errc::expected_continuation_byte, *this);
            ec = json_parser_errc::expected_continuation_byte;
            return;
        }
        switch (continuation_count_)
        {
        case 2:
            cp_ |= (static_cast<unsigned char>(*p_) & 0x3f) << 6;
            ++p_;
            --continuation_count_;
            goto string_u3;
        default:
            cp_ |= (static_cast<unsigned char>(*p_) & 0x3f);
            if (cp_ < 0x0800) 
            {
                err_handler_.error(json_parser_errc::over_long_utf8_sequence, *this);
                ec = json_parser_errc::over_long_utf8_sequence;
                return;
            }
            if (cp_ >= 0xD800 && cp_ <= 0xDFFF) 
            {
                err_handler_.error(json_parser_errc::illegal_codepoint, *this);
                ec = json_parser_errc::illegal_codepoint;
                return;
            }
            continuation_count_ = 0;
            ++p_;
            goto string_u1;
        }

string_u4:
        //std::cout << "parse_string string_u4" << std::endl;

        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            string_buffer_.append(sb,p_-sb);
            column_ += (p_ - sb + 1);
            state_ = parse_state::string_u4;
            return;
        }
        if (!unicons::is_continuation_byte(*p_))
        {
            err_handler_.error(json_parser_errc::expected_continuation_byte, *this);
            ec = json_parser_errc::expected_continuation_byte;
            return;
        }
        switch (continuation_count_)
        {
        case 3:
            cp_ |= (static_cast<unsigned char>(*p_) & 0x3f) << 12;
            --continuation_count_;
            ++p_;
            goto string_u4;
        case 2:
            --continuation_count_;
            cp_ |= (static_cast<unsigned char>(*p_) & 0x3f) << 6;
            ++p_;
            goto string_u4;
        default:
            cp_ |= (static_cast<unsigned char>(*p_) & 0x3f);
            if (cp_ <= 0x010000) 
            {
                err_handler_.error(json_parser_errc::over_long_utf8_sequence, *this);
                ec = json_parser_errc::over_long_utf8_sequence;
                return;
            }
            ++p_;
            continuation_count_ = 0;
            state_ = parse_state::string_u1;
            goto string_u1;
        }
        JSONCONS_UNREACHABLE();               
    }

    void parse()
    {
        std::error_code ec;
        parse(ec);
        if (ec)
        {
            throw parse_error(ec,line_,column_);
        }
    }

    void parse(std::error_code& ec)
    {
        while ((p_ < end_input_) && (state_ != parse_state::done))
        {
            switch (*p_)
            {
            JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                if (err_handler_.error(json_parser_errc::illegal_control_character, *this))
                {
                    ec = json_parser_errc::illegal_control_character;
                    return;
                }
                break;
            default:
                break;
            }

            switch (state_)
            {
            case parse_state::cr:
                ++line_;
                column_ = 1;
                switch (*p_)
                {
                case '\n':
                    state_ = pop_state();
                    ++p_;
                    break;
                default:
                    state_ = pop_state();
                    break;
                }
                break;
            case parse_state::lf:
                ++line_;
                column_ = 1;
                state_ = pop_state();
                break;
            case parse_state::start: 
                {
                    switch (*p_)
                    {
                    case '\r': 
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        push_state(state_);
                        state_ = parse_state::slash;
                        break;
                    case '{':
                        handler_.begin_json();
                        do_begin_object(ec);
                        if (ec) return;
                        break;
                    case '[':
                        handler_.begin_json();
                        do_begin_array(ec);
                        if (ec) return;
                        break;
                    case '\"':
                        handler_.begin_json();
                        state_ = parse_state::string_u1;
                        break;
                    case '-':
                        handler_.begin_json();
                        is_negative_ = true;
                        state_ = parse_state::minus;
                        break;
                    case '0': 
                        handler_.begin_json();
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        handler_.begin_json();
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::integer;
                        break;
                    case 'f':
                        handler_.begin_json();
                        state_ = parse_state::f;
                        literal_ = json_literals<char>::false_literal();
                        literal_index_ = 1;
                        break;
                    case 'n':
                        handler_.begin_json();
                        state_ = parse_state::n;
                        literal_ = json_literals<char>::null_literal();
                        literal_index_ = 1;
                        break;
                    case 't':
                        handler_.begin_json();
                        state_ = parse_state::t;
                        literal_ = json_literals<char>::true_literal();
                        literal_index_ = 1;
                        break;
                    case '}':
                        err_handler_.fatal_error(json_parser_errc::unexpected_right_brace, *this);
                        ec = json_parser_errc::unexpected_right_brace;
                        return;
                    case ']':
                        err_handler_.fatal_error(json_parser_errc::unexpected_right_bracket, *this);
                        ec = json_parser_errc::unexpected_right_bracket;
                        return;
                    default:
                        err_handler_.fatal_error(json_parser_errc::invalid_json_text, *this);
                        ec = json_parser_errc::invalid_json_text;
                        return;
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
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/':
                        push_state(state_); 
                        state_ = parse_state::slash;
                        break;
                    case '}':
                        do_end_object(ec);
                        if (ec) return;
                        break;
                    case ']':
                        do_end_array(ec);
                        if (ec) return;
                        break;
                    case ',':
                        begin_member_or_element(ec);
                        if (ec) return;
                        break;
                    default:
                        if (parent() == parse_state::array)
                        {
                            if (err_handler_.error(json_parser_errc::expected_comma_or_right_bracket, *this))
                            {
                                ec = json_parser_errc::expected_comma_or_right_bracket;
                                return;
                            }
                        }
                        else if (parent() == parse_state::object)
                        {
                            if (err_handler_.error(json_parser_errc::expected_comma_or_right_brace, *this))
                            {
                                ec = json_parser_errc::expected_comma_or_right_brace;
                                return;
                            }
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
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/':
                        push_state(state_); 
                        state_ = parse_state::slash;
                        break;
                    case '}':
                        do_end_object(ec);
                        if (ec) return;
                        break;
                    case '\"':
                        push_state(parse_state::member_name);
                        state_ = parse_state::string_u1;
                        break;
                    case '\'':
                        if (err_handler_.error(json_parser_errc::single_quote, *this))
                        {
                            ec = json_parser_errc::single_quote;
                            return;
                        }
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_name, *this))
                        {
                            ec = json_parser_errc::expected_name;
                            return;
                        }
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
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        push_state(state_);
                        state_ = parse_state::slash;
                        break;
                    case '\"':
                        push_state(parse_state::member_name);
                        state_ = parse_state::string_u1;
                        break;
                    case '}':
                        if (err_handler_.error(json_parser_errc::extra_comma, *this))
                        {
                            ec = json_parser_errc::extra_comma;
                            return;
                        }
                        do_end_object(ec);  // Recover
                        if (ec) return;
                        break;
                    case '\'':
                        if (err_handler_.error(json_parser_errc::single_quote, *this))
                        {
                            ec = json_parser_errc::single_quote;
                            return;
                        }
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_name, *this))
                        {
                            ec = json_parser_errc::expected_name;
                            return;
                        }
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
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        push_state(state_);
                        state_ = parse_state::slash;
                        break;
                    case ':':
                        state_ = parse_state::expect_value;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_colon, *this))
                        {
                            ec = json_parser_errc::expected_colon;
                            return;
                        }
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
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        push_state(state_);
                        state_ = parse_state::slash;
                        break;
                    case '{':
                        do_begin_object(ec);
                        if (ec) return;
                        break;
                    case '[':
                        do_begin_array(ec);
                        if (ec) return;
                        break;
                    case '\"':
                        state_ = parse_state::string_u1;
                        break;
                    case '-':
                        is_negative_ = true;
                        state_ = parse_state::minus;
                        break;
                    case '0': 
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::integer;
                        break;
                    case 'f':
                        state_ = parse_state::f;
                        literal_ = json_literals<char>::false_literal();
                        literal_index_ = 1;
                        break;
                    case 'n':
                        state_ = parse_state::n;
                        literal_ = json_literals<char>::null_literal();
                        literal_index_ = 1;
                        break;
                    case 't':
                        state_ = parse_state::t;
                        literal_ = json_literals<char>::true_literal();
                        literal_index_ = 1;
                        break;
                    case ']':
                        if (parent() == parse_state::array)
                        {
                            if (err_handler_.error(json_parser_errc::extra_comma, *this))
                            {
                                ec = json_parser_errc::extra_comma;
                                return;
                            }
                            do_end_array(ec);  // Recover
                            if (ec) return;
                        }
                        else
                        {
                            if (err_handler_.error(json_parser_errc::expected_value, *this))
                            {
                                ec = json_parser_errc::expected_value;
                                return;
                            }
                        }
                        break;
                    case '\'':
                        if (err_handler_.error(json_parser_errc::single_quote, *this))
                        {
                            ec = json_parser_errc::single_quote;
                            return;
                        }
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_value, *this))
                        {
                            ec = json_parser_errc::expected_value;
                            return;
                        }
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
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        do_space();
                        break;
                    case '/': 
                        push_state(state_);
                        state_ = parse_state::slash;
                        break;
                    case '{':
                        do_begin_object(ec);
                        if (ec) return;
                        break;
                    case '[':
                        do_begin_array(ec);
                        if (ec) return;
                        break;
                    case ']':
                        do_end_array(ec);
                        if (ec) return;
                        break;
                    case '\"':
                        state_ = parse_state::string_u1;
                        break;
                    case '-':
                        is_negative_ = true;
                        state_ = parse_state::minus;
                        break;
                    case '0': 
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::integer;
                        break;
                    case 'f':
                        state_ = parse_state::f;
                        literal_ = json_literals<char>::false_literal();
                        literal_index_ = 1;
                        break;
                    case 'n':
                        state_ = parse_state::n;
                        literal_ = json_literals<char>::null_literal();
                        literal_index_ = 1;
                        break;
                    case 't':
                        state_ = parse_state::t;
                        literal_ = json_literals<char>::true_literal();
                        literal_index_ = 1;
                        break;
                    case '\'':
                        if (err_handler_.error(json_parser_errc::single_quote, *this))
                        {
                            ec = json_parser_errc::single_quote;
                            return;
                        }
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_value, *this))
                        {
                            ec = json_parser_errc::expected_value;
                            return;
                        }
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::string_u1: 
            case parse_state::string_u2: 
            case parse_state::string_u3: 
            case parse_state::string_u4: 
                parse_string(ec);
                if (ec) return;
                break;
            case parse_state::escape: 
                escape_next_char(*p_, ec);
                if (ec) return;
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u1: 
                {
                    append_codepoint(*p_,ec);
                    if (ec) return;
                    state_ = parse_state::escape_u2;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u2: 
                {
                    append_codepoint(*p_, ec);
                    if (ec) return;
                    state_ = parse_state::escape_u3;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u3: 
                {
                    append_codepoint(*p_, ec);
                    if (ec) return;
                    state_ = parse_state::escape_u4;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u4: 
                {
                    append_codepoint(*p_, ec);
                    if (ec) return;
                    if (unicons::is_high_surrogate(cp_))
                    {
                        state_ = parse_state::escape_expect_surrogate_pair1;
                    }
                    else
                    {
                        unicons::convert(&cp_, &cp_ + 1, std::back_inserter(string_buffer_));
                        state_ = parse_state::string_u1;
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
                        state_ = parse_state::escape_expect_surrogate_pair2;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_codepoint_surrogate_pair, *this))
                        {
                            ec = json_parser_errc::expected_codepoint_surrogate_pair;
                            return;
                        }
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
                        state_ = parse_state::escape_u6;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_codepoint_surrogate_pair, *this))
                        {
                            ec = json_parser_errc::expected_codepoint_surrogate_pair;
                            return;
                        }
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u6:
                {
                    append_second_codepoint(*p_, ec);
                    if (ec) return;
                    state_ = parse_state::escape_u7;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u7: 
                {
                    append_second_codepoint(*p_, ec);
                    if (ec) return;
                    state_ = parse_state::escape_u8;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u8: 
                {
                    append_second_codepoint(*p_, ec);
                    if (ec) return;
                    state_ = parse_state::escape_u9;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::escape_u9: 
                {
                    append_second_codepoint(*p_, ec);
                    if (ec) return;
                    uint32_t cp = 0x10000 + ((cp_ & 0x3FF) << 10) + (cp2_ & 0x3FF);
                    unicons::convert(&cp, &cp + 1, std::back_inserter(string_buffer_));
                    state_ = parse_state::string_u1;
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
                        state_ = parse_state::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::integer;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_value, *this))
                        {
                            ec = json_parser_errc::expected_value;
                            return;
                        }
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
                        end_integer_value(ec);
                        if (ec) return;
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        end_integer_value(ec);
                        if (ec) return;
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        end_integer_value(ec);
                        if (ec) return;
                        do_space();
                        break;
                    case '/': 
                        end_integer_value(ec);
                        if (ec) return;
                        push_state(state_);
                        state_ = parse_state::slash;
                        break;
                    case '}':
                        end_integer_value(ec);
                        if (ec) return;
                        do_end_object(ec);
                        if (ec) return;
                        break;
                    case ']':
                        end_integer_value(ec);
                        if (ec) return;
                        do_end_array(ec);
                        if (ec) return;
                        break;
                    case '.':
                        precision_ = static_cast<uint8_t>(string_buffer_.length());
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::fraction1;
                        break;
                    case 'e':case 'E':
                        precision_ = static_cast<uint8_t>(string_buffer_.length());
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::exp1;
                        break;
                    case ',':
                        end_integer_value(ec);
                        if (ec) return;
                        begin_member_or_element(ec);
                        if (ec) return;
                        break;
                    case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        if (err_handler_.error(json_parser_errc::leading_zero, *this))
                        {
                            ec = json_parser_errc::leading_zero;
                            return;
                        }
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::invalid_number, *this))
                        {
                            ec = json_parser_errc::invalid_number;
                            return;
                        }
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
                        end_integer_value(ec);
                        if (ec) return;
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        end_integer_value(ec);
                        if (ec) return;
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        end_integer_value(ec);
                        if (ec) return;
                        do_space();
                        break;
                    case '/': 
                        end_integer_value(ec);
                        if (ec) return;
                        push_state(state_);
                        state_ = parse_state::slash;
                        break;
                    case '}':
                        end_integer_value(ec);
                        if (ec) return;
                        do_end_object(ec);
                        if (ec) return;
                        break;
                    case ']':
                        end_integer_value(ec);
                        if (ec) return;
                        do_end_array(ec);
                        if (ec) return;
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::integer;
                        break;
                    case '.':
                        precision_ = static_cast<uint8_t>(string_buffer_.length());
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::fraction1;
                        break;
                    case ',':
                        end_integer_value(ec);
                        if (ec) return;
                        begin_member_or_element(ec);
                        if (ec) return;
                        break;
                    case 'e':case 'E':
                        precision_ = static_cast<uint8_t>(string_buffer_.length());
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::exp1;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::invalid_number, *this))
                        {
                            ec = json_parser_errc::invalid_number;
                            return;
                        }
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
                        state_ = parse_state::fraction2;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::invalid_number, *this))
                        {
                            ec = json_parser_errc::invalid_number;
                            return;
                        }
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
                        end_fraction_value(ec);
                        if (ec) return;
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        end_fraction_value(ec);
                        if (ec) return;
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        end_fraction_value(ec);
                        if (ec) return;
                        do_space();
                        break;
                    case '/': 
                        end_fraction_value(ec);
                        if (ec) return;
                        push_state(state_);
                        state_ = parse_state::slash;
                        break;
                    case '}':
                        end_fraction_value(ec);
                        if (ec) return;
                        do_end_object(ec);
                        if (ec) return;
                        break;
                    case ']':
                        end_fraction_value(ec);
                        if (ec) return;
                        do_end_array(ec);
                        if (ec) return;
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        ++precision_;
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::fraction2;
                        break;
                    case ',':
                        end_fraction_value(ec);
                        if (ec) return;
                        begin_member_or_element(ec);
                        if (ec) return;
                        break;
                    case 'e':case 'E':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::exp1;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::invalid_number, *this))
                        {
                            ec = json_parser_errc::invalid_number;
                            return;
                        }
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
                        state_ = parse_state::exp2;
                        break;
                    case '-':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::exp2;
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::exp3;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_value, *this))
                        {
                            ec = json_parser_errc::expected_value;
                            return;
                        }
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
                        state_ = parse_state::exp3;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_value, *this))
                        {
                            ec = json_parser_errc::expected_value;
                            return;
                        }
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
                        end_fraction_value(ec);
                        if (ec) return;
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        end_fraction_value(ec);
                        if (ec) return;
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        end_fraction_value(ec);
                        if (ec) return;
                        do_space();
                        break;
                    case '/': 
                        end_fraction_value(ec);
                        if (ec) return;
                        push_state(state_);
                        state_ = parse_state::slash;
                        break;
                    case '}':
                        end_fraction_value(ec);
                        if (ec) return;
                        do_end_object(ec);
                        if (ec) return;
                        break;
                    case ']':
                        end_fraction_value(ec);
                        if (ec) return;
                        do_end_array(ec);
                        if (ec) return;
                        break;
                    case ',':
                        end_fraction_value(ec);
                        if (ec) return;
                        begin_member_or_element(ec);
                        if (ec) return;
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(static_cast<char>(*p_));
                        state_ = parse_state::exp3;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::invalid_number, *this))
                        {
                            ec = json_parser_errc::invalid_number;
                            return;
                        }
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
                        if (err_handler_.error(json_parser_errc::invalid_value, *this))
                        {
                            ec = json_parser_errc::invalid_value;
                            return;
                        }
                    }
                    ++p_;
                    ++literal_index_;
                    ++column_;
                }
                if (literal_index_ == literal_.second)
                {
                    handler_.bool_value(true, *this);
                    if (parent() == parse_state::root)
                    {
                        state_ = parse_state::done;
                        handler_.end_json();
                    }
                    else
                    {
                        state_ = parse_state::expect_comma_or_end;
                    }
                }
                break;
            case parse_state::f:  
                while (p_ < end_input_ && literal_index_ < literal_.second)
                {
                    if (*p_ != literal_.first[literal_index_])
                    {
                        if (err_handler_.error(json_parser_errc::invalid_value, *this))
                        {
                            ec = json_parser_errc::invalid_value;
                            return;
                        }
                    }
                    ++p_;
                    ++literal_index_;
                    ++column_;
                }
                if (literal_index_ == literal_.second)
                {
                    handler_.bool_value(false, *this);
                    if (parent() == parse_state::root)
                    {
                        state_ = parse_state::done;
                        handler_.end_json();
                    }
                    else
                    {
                        state_ = parse_state::expect_comma_or_end;
                    }
                }
                break;
            case parse_state::n: 
                while (p_ < end_input_ && literal_index_ < literal_.second)
                {
                    if (*p_ != literal_.first[literal_index_])
                    {
                        if (err_handler_.error(json_parser_errc::invalid_value, *this))
                        {
                            ec = json_parser_errc::invalid_value;
                            return;
                        }
                    }
                    ++p_;
                    ++literal_index_;
                    ++column_;
                }
                if (literal_index_ == literal_.second)
                {
                    handler_.null_value(*this);
                    if (parent() == parse_state::root)
                    {
                        state_ = parse_state::done;
                        handler_.end_json();
                    }
                    else
                    {
                        state_ = parse_state::expect_comma_or_end;
                    }
                }
                break;
            case parse_state::slash: 
                {
                    switch (*p_)
                    {
                    case '*':
                        state_ = parse_state::slash_star;
                        if (err_handler_.error(json_parser_errc::illegal_comment, *this))
                        {
                            ec = json_parser_errc::illegal_comment;
                            return;
                        }
                        break;
                    case '/':
                        state_ = parse_state::slash_slash;
                        if (err_handler_.error(json_parser_errc::illegal_comment, *this))
                        {
                            ec = json_parser_errc::illegal_comment;
                            return;
                        }
                        break;
                    default:    
                        if (err_handler_.error(json_parser_errc::invalid_json_text, *this))
                        {
                            ec = json_parser_errc::invalid_json_text;
                            return;
                        }
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
                        push_state(state_);
                        state_ = parse_state::cr;
                        break;
                    case '\n':
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;
                    case '*':
                        state_ = parse_state::slash_star_star;
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
                        state_ = pop_state();
                        break;
                    case '\n':
                        state_ = pop_state();
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
                        state_ = pop_state();
                        break;
                    default:    
                        state_ = parse_state::slash_star;
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            default:
                JSONCONS_ASSERT(false);
                break;
            }
        }
    }

    void end_parse()
    {
        std::error_code ec;
        end_parse(ec);
        if (ec)
        {
            throw parse_error(ec,line_,column_);
        }
    }

    void end_parse(std::error_code& ec)
    {
        if (parent() == parse_state::root)
        {
            switch (state_)
            {
            case parse_state::zero:  
            case parse_state::integer:
                end_integer_value(ec);
                if (ec) return;
                break;
            case parse_state::fraction2:
            case parse_state::exp3:
                end_fraction_value(ec);
                if (ec) return;
                break;
            default:
                break;
            }
        }
        if (state_ == parse_state::lf || state_ == parse_state::cr)
        { 
            state_ = pop_state();
        }
        if (!(state_ == parse_state::done || state_ == parse_state::start))
        {
            if (err_handler_.error(json_parser_errc::unexpected_eof, *this))
            {
                ec = json_parser_errc::unexpected_eof;
                return;
            }
        }
    }

    parse_state state() const
    {
        return state_;
    }

    void set_source(const char* input, size_t length)
    {
        begin_input_ = input;
        end_input_ = input + length;
        p_ = begin_input_;
    }
private:
    void end_fraction_value(std::error_code& ec)
    {
        try
        {
            double d = str_to_double_(string_buffer_.data(), precision_);
            if (is_negative_)
                d = -d;
            handler_.double_value(d, static_cast<uint8_t>(precision_), *this);
        }
        catch (...)
        {
            if (err_handler_.error(json_parser_errc::invalid_number, *this))
            {
                ec = json_parser_errc::invalid_number;
                return;
            }
            handler_.null_value(*this); // recovery
        }
        string_buffer_.clear();
        is_negative_ = false;

        switch (parent())
        {
        case parse_state::array:
        case parse_state::object:
            state_ = parse_state::expect_comma_or_end;
            break;
        case parse_state::root:
            state_ = parse_state::done;
            handler_.end_json();
            break;
        default:
            if (err_handler_.error(json_parser_errc::invalid_json_text, *this))
            {
                ec = json_parser_errc::invalid_json_text;
                return;
            }
            break;
        }
    }

    void end_integer_value(std::error_code& ec)
    {
        if (is_negative_)
        {
            int64_t d;
            if (try_string_to_integer(is_negative_, string_buffer_.data(), string_buffer_.length(),d))
            {
                handler_.integer_value(d, *this);
            }
            else
            {
                try
                {
                    double d2 = str_to_double_(string_buffer_.data(), string_buffer_.length());
                    handler_.double_value(-d2, static_cast<uint8_t>(string_buffer_.length()), *this);
                }
                catch (...)
                {
                    if (err_handler_.error(json_parser_errc::invalid_number, *this))
                    {
                        ec = json_parser_errc::invalid_number;
                        return;
                    }
                    handler_.null_value(*this);
                }
            }
        }
        else
        {
            uint64_t d;
            if (try_string_to_uinteger(string_buffer_.data(), string_buffer_.length(),d))
            {
                handler_.uinteger_value(d, *this);
            }
            else
            {
                try
                {
                    double d2 = str_to_double_(string_buffer_.data(),string_buffer_.length());
                    handler_.double_value(d2, static_cast<uint8_t>(string_buffer_.length()), *this);
                }
                catch (...)
                {
                    if (err_handler_.error(json_parser_errc::invalid_number, *this))
                    {
                        ec = json_parser_errc::invalid_number;
                        return;
                    }
                    handler_.null_value(*this);
                }
            }
        }

        switch (parent())
        {
        case parse_state::array:
        case parse_state::object:
            state_ = parse_state::expect_comma_or_end;
            break;
        case parse_state::root:
            state_ = parse_state::done;
            handler_.end_json();
            break;
        default:
            if (err_handler_.error(json_parser_errc::invalid_json_text, *this))
            {
                ec = json_parser_errc::invalid_json_text;
                return;
            }
            break;
        }
        string_buffer_.clear();
        is_negative_ = false;
    }

    void append_codepoint(int c, std::error_code& ec)
    {
        switch (c)
        {
        case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
        case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
        case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
            cp_ = append_to_codepoint(cp_, c, ec);
            if (ec) return;
            break;
        default:
            if (err_handler_.error(json_parser_errc::expected_value, *this))
            {
                ec = json_parser_errc::expected_value;
                return;
            }
            break;
        }
    }

    void append_second_codepoint(int c, std::error_code& ec)
    {
        switch (c)
        {
        case '0': 
        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
        case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
        case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
            cp2_ = append_to_codepoint(cp2_, c, ec);
            if (ec) return;
            break;
        default:
            if (err_handler_.error(json_parser_errc::expected_value, *this))
            {
                ec = json_parser_errc::expected_value;
                return;
            }
            break;
        }
    }

    void escape_next_char(int next_input, std::error_code& ec)
    {
        switch (next_input)
        {
        case '\"':
            string_buffer_.push_back('\"');
            state_ = parse_state::string_u1;
            break;
        case '\\': 
            string_buffer_.push_back('\\');
            state_ = parse_state::string_u1;
            break;
        case '/':
            string_buffer_.push_back('/');
            state_ = parse_state::string_u1;
            break;
        case 'b':
            string_buffer_.push_back('\b');
            state_ = parse_state::string_u1;
            break;
        case 'f':  
            string_buffer_.push_back('\f');
            state_ = parse_state::string_u1;
            break;
        case 'n':
            string_buffer_.push_back('\n');
            state_ = parse_state::string_u1;
            break;
        case 'r':
            string_buffer_.push_back('\r');
            state_ = parse_state::string_u1;
            break;
        case 't':
            string_buffer_.push_back('\t');
            state_ = parse_state::string_u1;
            break;
        case 'u':
            cp_ = 0;
            state_ = parse_state::escape_u1;
            break;
        default:    
            if (err_handler_.error(json_parser_errc::illegal_escaped_character, *this))
            {
                ec = json_parser_errc::illegal_escaped_character;
                return;
            }
            break;
        }
    }

    void end_string_value(const char* s, size_t length, std::error_code& ec) 
    {
        switch (parent())
        {
        case parse_state::member_name:
            handler_.name(string_view_type(s, length), *this);
            state_ = pop_state();
            state_ = parse_state::expect_colon;
            break;
        case parse_state::object:
        case parse_state::array:
            handler_.string_value(string_view_type(s, length), *this);
            state_ = parse_state::expect_comma_or_end;
            break;
        case parse_state::root:
            handler_.string_value(string_view_type(s, length), *this);
            state_ = parse_state::done;
            handler_.end_json();
            break;
        default:
            if (err_handler_.error(json_parser_errc::invalid_json_text, *this))
            {
                ec = json_parser_errc::invalid_json_text;
                return;
            }
            break;
        }
    }

    void begin_member_or_element(std::error_code& ec) 
    {
        switch (parent())
        {
        case parse_state::object:
            state_ = parse_state::expect_member_name;
            break;
        case parse_state::array:
            state_ = parse_state::expect_value;
            break;
        case parse_state::root:
            break;
        default:
            if (err_handler_.error(json_parser_errc::invalid_json_text, *this))
            {
                ec = json_parser_errc::invalid_json_text;
                return;
            }
            break;
        }
    }

    void push_state(parse_state state)
    {
        state_stack_.push_back(state);
    }

    parse_state pop_state()
    {
        JSONCONS_ASSERT(!state_stack_.empty())
        parse_state state = state_stack_.back();
        state_stack_.pop_back();
        return state;
    }
 
    uint32_t append_to_codepoint(uint32_t cp, int c, std::error_code& ec)
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
            if (err_handler_.error(json_parser_errc::invalid_hex_escape_sequence, *this))
            {
                ec = json_parser_errc::invalid_hex_escape_sequence;
                return cp;
            }
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
};

}

#endif


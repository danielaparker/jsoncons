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
        uint64_t digit = s[i] - '0';
        if (n > max_value_div_10)
        {
            return false;
        }
        n = n * 10;
        if (n > max_value - digit)
        {
            return false;
        }

        n += digit;
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
            int64_t digit = *p - '0';
            if (n < min_value_div_10)
            {
                return false;
            }
            n = n * 10;
            if (n < min_value + digit)
            {
                return false;
            }

            n -= digit;
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
            int64_t digit = *p - '0';
            if (n > max_value_div_10)
            {
                return false;
            }
            n = n * 10;
            if (n > max_value - digit)
            {
                return false;
            }

            n += digit;
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
    positive_zero,  
    negative_zero,  
    negative_integer,
    positive_integer,
    try_fraction,
    fraction1,
    fraction2,
    exp1,
    exp2,
    exp3,
    n,
    nu,
    nul,
    t,  
    tr,  
    tru,  
    f,  
    fa,  
    fal,  
    fals,  
    cr,
    lf,
    done
};

template <class CharT>
class basic_json_parser : private parsing_context
{
    static const int default_initial_stack_capacity_ = 100;
    typedef typename basic_json_input_handler<CharT>::string_view_type string_view_type;

    basic_null_json_input_handler<CharT> default_input_handler_;
    default_parse_error_handler default_err_handler_;

    basic_json_input_handler<CharT>& handler_;
    parse_error_handler& err_handler_;
    uint32_t cp_;
    uint32_t cp2_;
    std::basic_string<CharT> string_buffer_;

    bool is_negative_;
    uint8_t precision_;
    int64_t negative_val_;
    uint64_t positive_val_;
    double double_val_;
    int exponent_;
    int exp_;
    int sign_;

    size_t line_;
    size_t column_;
    int nesting_depth_;
    int initial_stack_capacity_;

    int max_depth_;
    string_to_double<CharT> str_to_double_;
    const CharT* begin_input_;
    const CharT* end_input_;
    const CharT* p_;

    parse_state state_;
    std::vector<parse_state> state_stack_;

    // Noncopyable and nonmoveable
    basic_json_parser(const basic_json_parser&) = delete;
    basic_json_parser& operator=(const basic_json_parser&) = delete;

public:

    basic_json_parser()
       : handler_(default_input_handler_),
         err_handler_(default_err_handler_),
         cp_(0),
         cp2_(0),
         is_negative_(false),
         precision_(0), 
         negative_val_(0),
         positive_val_(0),
         double_val_(0.0),
         exponent_(0),
         exp_(0),
         sign_(0),
         line_(1),
         column_(1),
         nesting_depth_(0), 
         initial_stack_capacity_(default_initial_stack_capacity_),
         begin_input_(nullptr),
         end_input_(nullptr),
         p_(nullptr),
         state_(parse_state::start)
    {
        max_depth_ = (std::numeric_limits<int>::max)();

        state_stack_.reserve(initial_stack_capacity_);
        push_state(parse_state::root);
    }

    basic_json_parser(parse_error_handler& err_handler)
       : handler_(default_input_handler_),
         err_handler_(err_handler),
         cp_(0),
         cp2_(0),
         is_negative_(false),
         precision_(0), 
         negative_val_(0),
         positive_val_(0),
         double_val_(0.0),
         exponent_(0),
         exp_(0),
         sign_(0),
         line_(1),
         column_(1),
         nesting_depth_(0), 
         initial_stack_capacity_(default_initial_stack_capacity_),
         begin_input_(nullptr),
         end_input_(nullptr),
         p_(nullptr),
         state_(parse_state::start)
    {
        max_depth_ = (std::numeric_limits<int>::max)();

        state_stack_.reserve(initial_stack_capacity_);
        push_state(parse_state::root);
    }

    basic_json_parser(basic_json_input_handler<CharT>& handler)
       : handler_(handler),
         err_handler_(default_err_handler_),
         cp_(0),
         cp2_(0),
         is_negative_(false),
         precision_(0), 
         negative_val_(0),
         positive_val_(0),
         double_val_(0.0),
         exponent_(0),
         exp_(0),
         sign_(0),
         line_(1),
         column_(1),
         nesting_depth_(0), 
         initial_stack_capacity_(default_initial_stack_capacity_),
         begin_input_(nullptr),
         end_input_(nullptr),
         p_(nullptr),
         state_(parse_state::start)
    {
        max_depth_ = (std::numeric_limits<int>::max)();

        state_stack_.reserve(initial_stack_capacity_);
        push_state(parse_state::root);
    }

    basic_json_parser(basic_json_input_handler<CharT>& handler,
                      parse_error_handler& err_handler)
       : handler_(handler),
         err_handler_(err_handler),
         cp_(0),
         cp2_(0),
         is_negative_(false),
         precision_(0), 
         negative_val_(0),
         positive_val_(0),
         double_val_(0.0),
         exponent_(0),
         exp_(0),
         sign_(0),
         line_(1),
         column_(1),
         nesting_depth_(0), 
         initial_stack_capacity_(default_initial_stack_capacity_),
         begin_input_(nullptr),
         end_input_(nullptr),
         p_(nullptr),
         state_(parse_state::start)
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
        JSONCONS_ASSERT(state_stack_.size() >= 1);
        return state_stack_.back();
    }

    bool done() const
    {
        return state_ == parse_state::done;
    }

    void skip_whitespace()
    {
        const CharT* local_end_input = end_input_;
        for (;;) 
        {
            if (JSONCONS_UNLIKELY(p_ == local_end_input)) 
            {
                return;
            } 
            else if (*p_ == ' ' || *p_ == '\t') 
            {
                ++p_;
                ++column_;                     
            } 
            else 
            {
                return;
            }
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
            CharT curr_char_ = *p_;
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

    void parse(std::error_code& ec)
    {
        const CharT* local_end_input = end_input_;

        while ((p_ < local_end_input) && (state_ != parse_state::done))
        {
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
                    handler_.begin_json();
                    switch (*p_)
                    {
                    JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                        if (err_handler_.error(json_parser_errc::illegal_control_character, *this))
                        {
                            ec = json_parser_errc::illegal_control_character;
                            return;
                        }
                        break;
                    case '\r': 
                        push_state(state_);
                        ++p_;
                        ++column_;
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        ++p_;
                        ++column_;
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        skip_whitespace();
                        break;
                    case '/': 
                        ++p_;
                        ++column_;
                        push_state(state_);
                        state_ = parse_state::slash;
                        break;
                    case '{':
                        do_begin_object(ec);
                        if (ec) return;
                        ++p_;
                        ++column_;
                        break;
                    case '[':
                        do_begin_array(ec);
                        if (ec) return;
                        ++p_;
                        ++column_;
                        break;
                    case '\"':
                        state_ = parse_state::string_u1;
                        ++p_;
                        ++column_;
                        break;
                    case '-':
                        is_negative_ = true;
                        sign_ = -1;
                        negative_val_ = 0;
                        precision_ = 0;
                        ++p_;
                        ++column_;
                        state_ = parse_state::minus;
                        parse_number(ec);
                        if (ec) {return;}
                        break;
                    case '0': 
                        sign_ = 1;
                        positive_val_ = 0;
                        precision_ = 1;
                        string_buffer_.push_back(*p_);
                        state_ = parse_state::positive_zero;
                        ++p_;
                        ++column_;
                        parse_number(ec);
                        if (ec) {return;}
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        sign_ = 1;
                        positive_val_ = (*p_ - '0');
                        precision_ = 1;
                        string_buffer_.push_back(*p_);
                        state_ = parse_state::positive_integer;
                        ++p_;
                        ++column_;
                        parse_number(ec);
                        if (ec) {return;}
                        break;
                    case 'n':
                        parse_null(ec);
                        if (ec) {return;}
                        break;
                    case 't':
                        parse_true(ec);
                        if (ec) {return;}
                        break;
                    case 'f':
                        parse_false(ec);
                        if (ec) {return;}
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
                break;

            case parse_state::expect_comma_or_end: 
                {
                    switch (*p_)
                    {
                    JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                        if (err_handler_.error(json_parser_errc::illegal_control_character, *this))
                        {
                            ec = json_parser_errc::illegal_control_character;
                            return;
                        }
                        ++p_;
                        ++column_;
                        break;
                    case '\r': 
                        ++p_;
                        ++column_;
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        ++p_;
                        ++column_;
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        skip_whitespace();
                        break;
                    case '/':
                        ++p_;
                        ++column_;
                        push_state(state_); 
                        state_ = parse_state::slash;
                        break;
                    case '}':
                        do_end_object(ec);
                        if (ec) return;
                        ++p_;
                        ++column_;
                        break;
                    case ']':
                        do_end_array(ec);
                        if (ec) return;
                        ++p_;
                        ++column_;
                        break;
                    case ',':
                        begin_member_or_element(ec);
                        if (ec) return;
                        ++p_;
                        ++column_;
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
                        ++p_;
                        ++column_;
                        break;
                    }
                }
                break;
            case parse_state::expect_member_name_or_end: 
                {
                    switch (*p_)
                    {
                    JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                        if (err_handler_.error(json_parser_errc::illegal_control_character, *this))
                        {
                            ec = json_parser_errc::illegal_control_character;
                            return;
                        }
                        ++p_;
                        ++column_;
                        break;
                    case '\r': 
                        ++p_;
                        ++column_;
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        ++p_;
                        ++column_;
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        skip_whitespace();
                        break;
                    case '/':
                        ++p_;
                        ++column_;
                        push_state(state_); 
                        state_ = parse_state::slash;
                        break;
                    case '}':
                        do_end_object(ec);
                        if (ec) return;
                        ++p_;
                        ++column_;
                        break;
                    case '\"':
                        ++p_;
                        ++column_;
                        push_state(parse_state::member_name);
                        state_ = parse_state::string_u1;
                        break;
                    case '\'':
                        if (err_handler_.error(json_parser_errc::single_quote, *this))
                        {
                            ec = json_parser_errc::single_quote;
                            return;
                        }
                        ++p_;
                        ++column_;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_name, *this))
                        {
                            ec = json_parser_errc::expected_name;
                            return;
                        }
                        ++p_;
                        ++column_;
                        break;
                    }
                }
                break;
            case parse_state::expect_member_name: 
                {
                    switch (*p_)
                    {
                    JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                        if (err_handler_.error(json_parser_errc::illegal_control_character, *this))
                        {
                            ec = json_parser_errc::illegal_control_character;
                            return;
                        }
                        ++p_;
                        ++column_;
                        break;
                    case '\r': 
                        ++p_;
                        ++column_;
                        push_state(state_);
                        state_ = parse_state::cr;
                        break; 
                    case '\n': 
                        ++p_;
                        ++column_;
                        push_state(state_);
                        state_ = parse_state::lf;
                        break;   
                    case ' ':case '\t':
                        skip_whitespace();
                        break;
                    case '/': 
                        ++p_;
                        ++column_;
                        push_state(state_);
                        state_ = parse_state::slash;
                        break;
                    case '\"':
                        ++p_;
                        ++column_;
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
                        ++p_;
                        ++column_;
                        break;
                    case '\'':
                        if (err_handler_.error(json_parser_errc::single_quote, *this))
                        {
                            ec = json_parser_errc::single_quote;
                            return;
                        }
                        ++p_;
                        ++column_;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_name, *this))
                        {
                            ec = json_parser_errc::expected_name;
                            return;
                        }
                        ++p_;
                        ++column_;
                        break;
                    }
                }
                break;
            case parse_state::expect_colon: 
                {
                    switch (*p_)
                    {
                    JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                        if (err_handler_.error(json_parser_errc::illegal_control_character, *this))
                        {
                            ec = json_parser_errc::illegal_control_character;
                            return;
                        }
                        ++p_;
                        ++column_;
                        break;
                    case '\r': 
                        push_state(state_);
                        state_ = parse_state::cr;
                        ++p_;
                        ++column_;
                        break; 
                    case '\n': 
                        push_state(state_);
                        state_ = parse_state::lf;
                        ++p_;
                        ++column_;
                        break;   
                    case ' ':case '\t':
                        skip_whitespace();
                        break;
                    case '/': 
                        push_state(state_);
                        state_ = parse_state::slash;
                        ++p_;
                        ++column_;
                        break;
                    case ':':
                        state_ = parse_state::expect_value;
                        ++p_;
                        ++column_;
                        break;
                    default:
                        if (err_handler_.error(json_parser_errc::expected_colon, *this))
                        {
                            ec = json_parser_errc::expected_colon;
                            return;
                        }
                        ++p_;
                        ++column_;
                        break;
                    }
                }
                break;

                case parse_state::expect_value: 
                {
                    switch (*p_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            if (err_handler_.error(json_parser_errc::illegal_control_character, *this))
                            {
                                ec = json_parser_errc::illegal_control_character;
                                return;
                            }
                            ++p_;
                            ++column_;
                            break;
                        case '\r': 
                            push_state(state_);
                            ++p_;
                            ++column_;
                            state_ = parse_state::cr;
                            break; 
                        case '\n': 
                            push_state(state_);
                            ++p_;
                            ++column_;
                            state_ = parse_state::lf;
                            break;   
                        case ' ':case '\t':
                            skip_whitespace();
                            break;
                        case '/': 
                            push_state(state_);
                            ++p_;
                            ++column_;
                            state_ = parse_state::slash;
                            break;
                        case '{':
                            do_begin_object(ec);
                            if (ec) return;
                            ++p_;
                            ++column_;
                            break;
                        case '[':
                            do_begin_array(ec);
                            if (ec) return;
                            ++p_;
                            ++column_;
                            break;
                        case '\"':
                            ++p_;
                            ++column_;
                            state_ = parse_state::string_u1;
                            break;
                        case '-':
                            is_negative_ = true;
                            sign_ = -1;
                            negative_val_ = 0;
                            precision_ = 0;
                            ++p_;
                            ++column_;
                            state_ = parse_state::minus;
                            parse_number(ec);
                            if (ec) {return;}
                            break;
                        case '0': 
                            sign_ = 1;
                            positive_val_ = 0;
                            precision_ = 1;
                            string_buffer_.push_back(*p_);
                            ++p_;
                            ++column_;
                            state_ = parse_state::positive_zero;
                            parse_number(ec);
                            if (ec) {return;}
                            break;
                        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                            sign_ = 1;
                            positive_val_ = (*p_ - '0');
                            precision_ = 1;
                            string_buffer_.push_back(*p_);
                            ++p_;
                            ++column_;
                            state_ = parse_state::positive_integer;
                            parse_number(ec);
                            if (ec) {return;}
                            break;
                        case 'n':
                            parse_null(ec);
                            if (ec) {return;}
                            break;
                        case 't':
                            parse_true(ec);
                            if (ec) {return;}
                            break;
                        case 'f':
                            parse_false(ec);
                            if (ec) {return;}
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
                            ++p_;
                            ++column_;
                            break;
                        case '\'':
                            if (err_handler_.error(json_parser_errc::single_quote, *this))
                            {
                                ec = json_parser_errc::single_quote;
                                return;
                            }
                            ++p_;
                            ++column_;
                            break;
                        default:
                            if (err_handler_.error(json_parser_errc::expected_value, *this))
                            {
                                ec = json_parser_errc::expected_value;
                                return;
                            }
                            ++p_;
                            ++column_;
                            break;
                    }
                }
                break;
                case parse_state::expect_value_or_end: 
                {
                    switch (*p_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            if (err_handler_.error(json_parser_errc::illegal_control_character, *this))
                            {
                                ec = json_parser_errc::illegal_control_character;
                                return;
                            }
                            ++p_;
                            ++column_;
                            break;
                        case '\r': 
                            ++p_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::cr;
                            break; 
                        case '\n': 
                            ++p_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::lf;
                            break;   
                        case ' ':case '\t':
                            skip_whitespace();
                            break;
                        case '/': 
                            ++p_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::slash;
                            break;
                        case '{':
                            do_begin_object(ec);
                            if (ec) return;
                            ++p_;
                            ++column_;
                            break;
                        case '[':
                            do_begin_array(ec);
                            if (ec) return;
                            ++p_;
                            ++column_;
                            break;
                        case ']':
                            do_end_array(ec);
                            if (ec) return;
                            ++p_;
                            ++column_;
                            break;
                        case '\"':
                            ++p_;
                            ++column_;
                            state_ = parse_state::string_u1;
                            break;
                        case '-':
                            is_negative_ = true;
                            sign_ = -1;
                            negative_val_ = 0;
                            precision_ = 0;
                            ++p_;
                            ++column_;
                            state_ = parse_state::minus;
                            parse_number(ec);
                            if (ec) {return;}
                            break;
                        case '0': 
                            sign_ = 1;
                            positive_val_ = 0;
                            precision_ = 1;
                            string_buffer_.push_back(*p_);
                            ++p_;
                            ++column_;
                            state_ = parse_state::positive_zero;
                            parse_number(ec);
                            if (ec) {return;}
                            break;
                        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                            sign_ = 1;
                            positive_val_ = (*p_ - '0');
                            precision_ = 1;
                            string_buffer_.push_back(*p_);
                            ++p_;
                            ++column_;
                            state_ = parse_state::positive_integer;
                            parse_number(ec);
                            if (ec) {return;}
                            break;
                        case 'n':
                            parse_null(ec);
                            if (ec) {return;}
                            break;
                        case 't':
                            parse_true(ec);
                            if (ec) {return;}
                            break;
                        case 'f':
                            parse_false(ec);
                            if (ec) {return;}
                            break;
                        case '\'':
                            if (err_handler_.error(json_parser_errc::single_quote, *this))
                            {
                                ec = json_parser_errc::single_quote;
                                return;
                            }
                            ++p_;
                            ++column_;
                            break;
                        default:
                            if (err_handler_.error(json_parser_errc::expected_value, *this))
                            {
                                ec = json_parser_errc::expected_value;
                                return;
                            }
                            ++p_;
                            ++column_;
                            break;
                        }
                    }
                break;
            case parse_state::string_u1: 
            case parse_state::escape: 
            case parse_state::escape_u1: 
            case parse_state::escape_u2: 
            case parse_state::escape_u3: 
            case parse_state::escape_u4: 
            case parse_state::escape_expect_surrogate_pair1: 
            case parse_state::escape_expect_surrogate_pair2: 
            case parse_state::escape_u6: 
            case parse_state::escape_u7: 
            case parse_state::escape_u8: 
            case parse_state::escape_u9: 
                parse_string(ec);
                if (ec) return;
                break;
            case parse_state::minus:
            case parse_state::positive_zero:  
            case parse_state::negative_zero:  
            case parse_state::positive_integer: 
            case parse_state::negative_integer: 
            case parse_state::try_fraction: 
            case parse_state::fraction1: 
            case parse_state::fraction2: 
            case parse_state::exp1: 
            case parse_state::exp2:  
            case parse_state::exp3: 
                parse_number(ec);  
                if (ec) return;
                break;
            case parse_state::t: 
                switch (*p_)
                {
                case 'r':
                    ++p_;
                    ++column_;
                    state_ = parse_state::tr;
                    break;
                default:
                    err_handler_.error(json_parser_errc::invalid_value, *this);
                    ec = json_parser_errc::invalid_value;
                    return;
                }
                break;
            case parse_state::tr: 
                switch (*p_)
                {
                case 'u':
                    state_ = parse_state::tru;
                    break;
                default:
                    err_handler_.error(json_parser_errc::invalid_value, *this);
                    ec = json_parser_errc::invalid_value;
                    return;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::tru: 
                switch (*p_)
                {
                case 'e':
                    handler_.bool_value(true,*this);
                    if (parent() == parse_state::root)
                    {
                        state_ = parse_state::done;
                        handler_.end_json();
                    }
                    else
                    {
                        state_ = parse_state::expect_comma_or_end;
                    }
                    break;
                default:
                    err_handler_.error(json_parser_errc::invalid_value, *this);
                    ec = json_parser_errc::invalid_value;
                    return;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::f: 
                switch (*p_)
                {
                case 'a':
                    ++p_;
                    ++column_;
                    state_ = parse_state::fa;
                    break;
                default:
                    err_handler_.error(json_parser_errc::invalid_value, *this);
                    ec = json_parser_errc::invalid_value;
                    return;
                }
                break;
            case parse_state::fa: 
                switch (*p_)
                {
                case 'l':
                    state_ = parse_state::fal;
                    break;
                default:
                    err_handler_.error(json_parser_errc::invalid_value, *this);
                    ec = json_parser_errc::invalid_value;
                    return;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::fal: 
                switch (*p_)
                {
                case 's':
                    state_ = parse_state::fals;
                    break;
                default:
                    err_handler_.error(json_parser_errc::invalid_value, *this);
                    ec = json_parser_errc::invalid_value;
                    return;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::fals: 
                switch (*p_)
                {
                case 'e':
                    handler_.bool_value(false,*this);
                    if (parent() == parse_state::root)
                    {
                        state_ = parse_state::done;
                        handler_.end_json();
                    }
                    else
                    {
                        state_ = parse_state::expect_comma_or_end;
                    }
                    break;
                default:
                    err_handler_.error(json_parser_errc::invalid_value, *this);
                    ec = json_parser_errc::invalid_value;
                    return;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::n: 
                switch (*p_)
                {
                case 'u':
                    ++p_;
                    ++column_;
                    state_ = parse_state::nu;
                    break;
                default:
                    err_handler_.error(json_parser_errc::invalid_value, *this);
                    ec = json_parser_errc::invalid_value;
                    return;
                }
                break;
            case parse_state::nu: 
                switch (*p_)
                {
                case 'l':
                    state_ = parse_state::nul;
                    break;
                default:
                    err_handler_.error(json_parser_errc::invalid_value, *this);
                    ec = json_parser_errc::invalid_value;
                    return;
                }
                ++p_;
                ++column_;
                break;
            case parse_state::nul: 
                switch (*p_)
                {
                case 'l':
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
                    break;
                default:
                    err_handler_.error(json_parser_errc::invalid_value, *this);
                    ec = json_parser_errc::invalid_value;
                    return;
                }
                ++p_;
                ++column_;
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

    void parse_true(std::error_code& ec)
    {
        if (JSONCONS_LIKELY(end_input_ - p_ >= 4))
        {
            if (*(p_+1) == 'r' && *(p_+2) == 'u' && *(p_+3) == 'e')
            {
                handler_.bool_value(true,*this);
                p_ += 4;
                column_ += 4;
                if (parent() == parse_state::root)
                {
                    handler_.end_json();
                    state_ = parse_state::done;
                }
                else
                {
                    state_ = parse_state::expect_comma_or_end;
                }
            }
            else
            {
                err_handler_.error(json_parser_errc::invalid_value, *this);
                ec = json_parser_errc::invalid_value;
                return;
            }
        }
        else
        {
            ++p_;
            ++column_;
            state_ = parse_state::t;
        }
    }

    void parse_null(std::error_code& ec)
    {
        if (JSONCONS_LIKELY(end_input_ - p_ >= 4))
        {
            if (*(p_+1) == 'u' && *(p_+2) == 'l' && *(p_+3) == 'l')
            {
                handler_.null_value(*this);
                p_ += 4;
                column_ += 4;
                if (parent() == parse_state::root)
                {
                    handler_.end_json();
                    state_ = parse_state::done;
                }
                else
                {
                    state_ = parse_state::expect_comma_or_end;
                }
            }
            else
            {
                err_handler_.error(json_parser_errc::invalid_value, *this);
                ec = json_parser_errc::invalid_value;
                return;
            }
        }
        else
        {
            ++p_;
            ++column_;
            state_ = parse_state::n;
        }
    }

    void parse_false(std::error_code& ec)
    {
        if (JSONCONS_LIKELY(end_input_ - p_ >= 5))
        {
            if (*(p_+1) == 'a' && *(p_+2) == 'l' && *(p_+3) == 's' && *(p_+4) == 'e')
            {
                handler_.bool_value(false,*this);
                p_ += 5;
                column_ += 5;
                if (parent() == parse_state::root)
                {
                    handler_.end_json();
                    state_ = parse_state::done;
                }
                else
                {
                    state_ = parse_state::expect_comma_or_end;
                }
            }
            else
            {
                err_handler_.error(json_parser_errc::invalid_value, *this);
                ec = json_parser_errc::invalid_value;
                return;
            }
        }
        else
        {
            ++p_;
            ++column_;
            state_ = parse_state::f;
        }
    }

    bool try_add_to_positive_val(uint8_t digit)
    {
        static const uint64_t max_value = (std::numeric_limits<uint64_t>::max)();
        static const uint64_t max_value_div_10 = max_value / 10;

        if (JSONCONS_UNLIKELY(positive_val_ > max_value_div_10))
        {
            return false;
        }
        positive_val_ *= 10;
        if (JSONCONS_UNLIKELY(positive_val_ > max_value - digit))
        {
            return false;
        }

        positive_val_ += digit;
        return true;
    }

    bool try_sub_from_negative_val(uint8_t digit)
    {
        static const int64_t min_value = (std::numeric_limits<int64_t>::min)();
        static const int64_t min_value_div_10 = min_value / 10;

        if (JSONCONS_UNLIKELY(negative_val_ < min_value_div_10))
        {
            return false;
        }
        negative_val_ *= 10;
        if (JSONCONS_UNLIKELY(negative_val_ < min_value + digit))
        {
            return false;
        }

        negative_val_ -= digit;
        return true;
    }

    void parse_number(std::error_code& ec)
    {
        const CharT* local_end_input = end_input_;
        const CharT* sb = p_;

        switch (state_)
        {
            case parse_state::minus:
                goto minus;
            case parse_state::positive_zero:
                goto positive_zero;
            case parse_state::negative_zero:
                goto negative_zero;
            case parse_state::positive_integer:
                goto positive_integer;
            case parse_state::negative_integer:
                goto negative_integer;
            case parse_state::try_fraction:
                goto try_fraction;
            case parse_state::fraction1:
                goto fraction1;
            case parse_state::fraction2:
                goto fraction2;
            case parse_state::exp1:
                goto exp1;
            case parse_state::exp2:
                goto exp2;
            case parse_state::exp3:
                goto exp3;
            default:
                JSONCONS_UNREACHABLE();               
        }
minus:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::minus;
            return;
        }
        switch (*p_)
        {
            case '0': 
                string_buffer_.push_back(*p_);
                ++precision_;
                ++p_;
                ++column_;
                goto negative_zero;
            case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                try_sub_from_negative_val(*p_ - '0');
                string_buffer_.push_back(*p_);
                ++precision_;
                ++p_;
                ++column_;
                goto negative_integer;
            default:
                err_handler_.error(json_parser_errc::expected_value, *this);
                ec = json_parser_errc::expected_value;
                return;
        }
negative_zero:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::negative_zero;
            return;
        }
        switch (*p_)
        {
            case '\r': 
                end_integer_value(ec);
                if (ec) return;
                ++p_;
                ++column_;
                push_state(state_);
                state_ = parse_state::cr;
                return; 
            case '\n': 
                end_integer_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::lf;
                return;   
            case ' ':case '\t':
                end_integer_value(ec);
                if (ec) return;
                skip_whitespace();
                return;
            case '/': 
                end_integer_value(ec);
                if (ec) return;
                ++p_;
                ++column_;
                push_state(state_);
                state_ = parse_state::slash;
                return;
            case '}':
                end_integer_value(ec);
                if (ec) return;
                do_end_object(ec);
                ++p_;
                ++column_;
                if (ec) return;
                return;
            case ']':
                end_integer_value(ec);
                if (ec) return;
                do_end_array(ec);
                ++p_;
                ++column_;
                if (ec) return;
                return;
            case '.':
                JSONCONS_ASSERT(precision_ == string_buffer_.length());
                string_buffer_.push_back(*p_);
                double_val_ = static_cast<double>(negative_val_);
                exponent_ = 0;
                exp_ = 0;
                ++p_;
                ++column_;
                goto fraction1;
            case 'e':case 'E':
                JSONCONS_ASSERT(precision_ == string_buffer_.length());
                string_buffer_.push_back(*p_);
                double_val_ = static_cast<double>(negative_val_);
                exponent_ = 0;
                exp_ = 0,
                ++p_;
                ++column_;
                goto exp1;
            case ',':
                end_integer_value(ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                err_handler_.error(json_parser_errc::leading_zero, *this);
                ec = json_parser_errc::leading_zero;
                state_ = parse_state::negative_zero;
                return;
            default:
                err_handler_.error(json_parser_errc::invalid_number, *this);
                ec = json_parser_errc::invalid_number;
                state_ = parse_state::negative_zero;
                return;
        }
negative_integer:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::negative_integer;
            return;
        }
        switch (*p_)
        {
            case '\r': 
                end_integer_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::cr;
                return; 
            case '\n': 
                end_integer_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::lf;
                return;   
            case ' ':case '\t':
                end_integer_value(ec);
                if (ec) return;
                skip_whitespace();
                return;
            case '/': 
                end_integer_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::slash;
                return;
            case '}':
                end_integer_value(ec);
                if (ec) return;
                do_end_object(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case ']':
                end_integer_value(ec);
                if (ec) return;
                do_end_array(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                if (try_sub_from_negative_val(*p_ - '0'))
                {
                    string_buffer_.push_back(*p_);
                    ++precision_;
                    ++p_;
                    ++column_;
                    goto negative_integer;
                }
                else
                {
                    double_val_ = static_cast<double>(negative_val_);
                    goto try_fraction;
                }
            case ',':
                end_integer_value(ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case '.':
                JSONCONS_ASSERT(precision_ == string_buffer_.length());
                string_buffer_.push_back(*p_);
                double_val_ = static_cast<double>(negative_val_);
                exponent_ = 0;
                exp_ = 0;
                ++p_;
                ++column_;
                goto fraction1;
            case 'e':case 'E':
                JSONCONS_ASSERT(precision_ == string_buffer_.length());
                string_buffer_.push_back(*p_);
                double_val_ = static_cast<double>(negative_val_);
                exponent_ = 0;
                exp_ = 0,
                ++p_;
                ++column_;
                goto exp1;
            default:
                err_handler_.error(json_parser_errc::invalid_number, *this);
                ec = json_parser_errc::invalid_number;
                state_ = parse_state::negative_integer;
                return;
        }
positive_zero:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::positive_zero;
            return;
        }
        switch (*p_)
        {
            case '\r': 
                end_uinteger_value(ec);
                if (ec) return;
                ++p_;
                ++column_;
                push_state(state_);
                state_ = parse_state::cr;
                return; 
            case '\n': 
                end_uinteger_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::lf;
                return;   
            case ' ':case '\t':
                end_uinteger_value(ec);
                if (ec) return;
                skip_whitespace();
                return;
            case '/': 
                end_uinteger_value(ec);
                if (ec) return;
                ++p_;
                ++column_;
                push_state(state_);
                state_ = parse_state::slash;
                return;
            case '}':
                end_uinteger_value(ec);
                if (ec) return;
                do_end_object(ec);
                ++p_;
                ++column_;
                if (ec) return;
                return;
            case ']':
                end_uinteger_value(ec);
                if (ec) return;
                do_end_array(ec);
                ++p_;
                ++column_;
                if (ec) return;
                return;
            case '.':
                JSONCONS_ASSERT(precision_ == string_buffer_.length());
                string_buffer_.push_back(*p_);
                double_val_ = static_cast<double>(positive_val_);
                exponent_ = 0;
                exp_ = 0;
                ++p_;
                ++column_;
                goto fraction1;
            case 'e':case 'E':
                JSONCONS_ASSERT(precision_ == string_buffer_.length());
                string_buffer_.push_back(*p_);
                double_val_ = static_cast<double>(positive_val_);
                exponent_ = 0;
                exp_ = 0,
                ++p_;
                ++column_;
                goto exp1;
            case ',':
                end_uinteger_value(ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                err_handler_.error(json_parser_errc::leading_zero, *this);
                ec = json_parser_errc::leading_zero;
                state_ = parse_state::positive_zero;
                return;
            default:
                err_handler_.error(json_parser_errc::invalid_number, *this);
                ec = json_parser_errc::invalid_number;
                state_ = parse_state::positive_zero;
                return;
        }
positive_integer:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::positive_integer;
            return;
        }
        switch (*p_)
        {
            case '\r': 
                end_uinteger_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::cr;
                return; 
            case '\n': 
                end_uinteger_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::lf;
                return;   
            case ' ':case '\t':
                end_uinteger_value(ec);
                if (ec) return;
                skip_whitespace();
                return;
            case '/': 
                end_uinteger_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::slash;
                return;
            case '}':
                end_uinteger_value(ec);
                if (ec) return;
                do_end_object(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case ']':
                end_uinteger_value(ec);
                if (ec) return;
                do_end_array(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                if (try_add_to_positive_val(*p_ - '0'))
                {
                    string_buffer_.push_back(*p_);
                    ++precision_;
                    ++p_;
                    ++column_;
                    goto positive_integer;
                }
                else
                {
                    double_val_ = static_cast<double>(positive_val_);
                    goto try_fraction;
                }
            case '.':
                JSONCONS_ASSERT(precision_ == string_buffer_.length());
                string_buffer_.push_back(*p_);
                double_val_ = static_cast<double>(positive_val_);
                exponent_ = 0;
                exp_ = 0;
                ++p_;
                ++column_;
                goto fraction1;
            case 'e':case 'E':
                JSONCONS_ASSERT(precision_ == string_buffer_.length());
                string_buffer_.push_back(*p_);
                double_val_ = static_cast<double>(positive_val_);
                exponent_ = 0;
                exp_ = 0,
                ++p_;
                ++column_;
                goto exp1;
            case ',':
                end_uinteger_value(ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            default:
                err_handler_.error(json_parser_errc::invalid_number, *this);
                ec = json_parser_errc::invalid_number;
                state_ = parse_state::positive_integer;
                return;
        }
try_fraction:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::positive_integer;
            return;
        }
        switch (*p_)
        {
            case '\r': 
                end_fraction_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::cr;
                return; 
            case '\n': 
                end_fraction_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::lf;
                return;   
            case ' ':case '\t':
                end_fraction_value(ec);
                if (ec) return;
                skip_whitespace();
                return;
            case '/': 
                end_fraction_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::slash;
                return;
            case '}':
                end_fraction_value(ec);
                if (ec) return;
                do_end_object(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case ']':
                end_fraction_value(ec);
                if (ec) return;
                do_end_array(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case ',':
                end_fraction_value(ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                string_buffer_.push_back(*p_);
                double_val_ = double_val_*10 + sign_*(*p_ - '0');
                ++precision_;
                ++p_;
                ++column_;
                goto try_fraction;
            case '.':
                JSONCONS_ASSERT(precision_ == string_buffer_.length());
                string_buffer_.push_back(*p_);
                exponent_ = 0;
                exp_ = 0;
                ++p_;
                ++column_;
                goto fraction1;
            case 'e':case 'E':
                JSONCONS_ASSERT(precision_ == string_buffer_.length());
                string_buffer_.push_back(*p_);
                exponent_ = 0;
                exp_ = 0,
                ++p_;
                ++column_;
                goto exp1;
            default:
                err_handler_.error(json_parser_errc::invalid_number, *this);
                ec = json_parser_errc::invalid_number;
                state_ = parse_state::positive_integer;
                return;
        }
fraction1:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::fraction1;
            return;
        }
        switch (*p_)
        {
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                double_val_ = double_val_*10 + sign_*(*p_ - '0');
                ++precision_;
                --exponent_;
                string_buffer_.push_back(*p_);
                ++p_;
                ++column_;
                goto fraction2;
            default:
                err_handler_.error(json_parser_errc::invalid_number, *this);
                ec = json_parser_errc::invalid_number;
                state_ = parse_state::fraction1;
                return;
        }
fraction2:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::fraction2;
            return;
        }
        switch (*p_)
        {
            case '\r': 
                end_fraction_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::cr;
                return; 
            case '\n': 
                end_fraction_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::lf;
                return;   
            case ' ':case '\t':
                end_fraction_value(ec);
                if (ec) return;
                skip_whitespace();
                return;
            case '/': 
                end_fraction_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::slash;
                return;
            case '}':
                end_fraction_value(ec);
                if (ec) return;
                do_end_object(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case ']':
                end_fraction_value(ec);
                if (ec) return;
                do_end_array(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case ',':
                end_fraction_value(ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                ++precision_;
                double_val_ = double_val_*10 + sign_*(*p_ - '0');
                --exponent_;
                string_buffer_.push_back(*p_);
                ++p_;
                ++column_;
                goto fraction2;
            case 'e':case 'E':
                string_buffer_.push_back(*p_);
                exp_ = 0;
                ++p_;
                ++column_;
                goto exp1;
            default:
                err_handler_.error(json_parser_errc::invalid_number, *this);
                ec = json_parser_errc::invalid_number;
                state_ = parse_state::fraction2;
                return;
        }
exp1:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::exp1;
            return;
        }
        switch (*p_)
        {
            case '+':
                sign_ = 1;
                ++p_;
                ++column_;
                goto exp2;
            case '-':
                sign_ = -1;
                string_buffer_.push_back(*p_);
                ++p_;
                ++column_;
                goto exp2;
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                sign_ = 1;
                exp_ = 10*exp_ + (*p_ - '0');
                string_buffer_.push_back(*p_);
                ++p_;
                ++column_;
                goto exp3;
            default:
                err_handler_.error(json_parser_errc::expected_value, *this);
                ec = json_parser_errc::expected_value;
                state_ = parse_state::exp1;
                return;
        }
exp2:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::exp2;
            return;
        }
        switch (*p_)
        {
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                exp_ = 10*exp_ + (*p_ - '0');
                string_buffer_.push_back(*p_);
                ++p_;
                ++column_;
                goto exp3;
            default:
                err_handler_.error(json_parser_errc::expected_value, *this);
                ec = json_parser_errc::expected_value;
                state_ = parse_state::exp2;
                return;
        }
        
exp3:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::exp3;
            return;
        }
        switch (*p_)
        {
            case '\r': 
                end_fraction_value(ec);
                if (ec) return;
                ++p_;
                ++column_;
                push_state(state_);
                state_ = parse_state::cr;
                return; 
            case '\n': 
                end_fraction_value(ec);
                if (ec) return;
                ++p_;
                ++column_;
                push_state(state_);
                state_ = parse_state::lf;
                return;   
            case ' ':case '\t':
                end_fraction_value(ec);
                if (ec) return;
                skip_whitespace();
                return;
            case '/': 
                end_fraction_value(ec);
                if (ec) return;
                push_state(state_);
                ++p_;
                ++column_;
                state_ = parse_state::slash;
                return;
            case '}':
                end_fraction_value(ec);
                if (ec) return;
                do_end_object(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case ']':
                end_fraction_value(ec);
                if (ec) return;
                do_end_array(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case ',':
                end_fraction_value(ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++p_;
                ++column_;
                return;
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                exp_ = 10*exp_ + (*p_ - '0');
                string_buffer_.push_back(*p_);
                ++p_;
                ++column_;
                goto exp3;
            default:
                err_handler_.error(json_parser_errc::invalid_number, *this);
                ec = json_parser_errc::invalid_number;
                state_ = parse_state::exp3;
                return;
        }

        JSONCONS_UNREACHABLE();               
    }

    void parse_string(std::error_code& ec)
    {
        const CharT* local_end_input = end_input_;
        const CharT* sb = p_;

        switch (state_)
        {
            case parse_state::string_u1:
                goto string_u1;
            case parse_state::escape:
                goto escape;
            case parse_state::escape_u1:
                goto escape_u1;
            case parse_state::escape_u2:
                goto escape_u2;
            case parse_state::escape_u3:
                goto escape_u3;
            case parse_state::escape_u4:
                goto escape_u4;
            case parse_state::escape_expect_surrogate_pair1:
                goto escape_expect_surrogate_pair1;
            case parse_state::escape_expect_surrogate_pair2:
                goto escape_expect_surrogate_pair2;
            case parse_state::escape_u6:
                goto escape_u6;
            case parse_state::escape_u7:
                goto escape_u7;
            case parse_state::escape_u8:
                goto escape_u8;
            case parse_state::escape_u9:
                goto escape_u9;
            default:
                JSONCONS_UNREACHABLE();               
        }

string_u1:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            auto result = unicons::validate(sb,p_);
            if (result.ec != unicons::conv_errc())
            {
                translate_conv_errc(result.ec,ec);
                column_ += (result.it - sb);
                return;
            }
            string_buffer_.append(sb,p_-sb);
            column_ += (p_ - sb + 1);
            state_ = parse_state::string_u1;
            return;
        }
        switch (*p_)
        {
            JSONCONS_ILLEGAL_CONTROL_CHARACTER:
            {
                column_ += (p_ - sb + 1);
                if (err_handler_.error(json_parser_errc::illegal_control_character, *this))
                {
                    ec = json_parser_errc::illegal_control_character;
                    state_ = parse_state::string_u1;
                    return;
                }
                // recovery - skip
                auto result = unicons::validate(sb,p_);
                if (result.ec != unicons::conv_errc())
                {
                    translate_conv_errc(result.ec,ec);
                    column_ += (result.it - sb);
                    return;
                }
                string_buffer_.append(sb,p_-sb);
                ++p_;
                state_ = parse_state::string_u1;
                return;
            }
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
                auto result = unicons::validate(sb,p_);
                if (result.ec != unicons::conv_errc())
                {
                    translate_conv_errc(result.ec,ec);
                    column_ += (result.it - sb);
                    return;
                }
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
                    state_ = parse_state::string_u1;
                    return;
                }
                // recovery - keep
                auto result = unicons::validate(sb,p_);
                if (result.ec != unicons::conv_errc())
                {
                    translate_conv_errc(result.ec,ec);
                    column_ += (result.it - sb);
                    return;
                }
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
                    state_ = parse_state::string_u1;
                    return;
                }
                // recovery - keep
                auto result = unicons::validate(sb,p_);
                if (result.ec != unicons::conv_errc())
                {
                    translate_conv_errc(result.ec,ec);
                    column_ += (result.it - sb);
                    return;
                }
                string_buffer_.append(sb, p_ - sb + 1);
                ++p_;
                state_ = parse_state::string_u1;
                return;
            }
            case '\\': 
            {
                auto result = unicons::validate(sb,p_);
                if (result.ec != unicons::conv_errc())
                {
                    translate_conv_errc(result.ec,ec);
                    column_ += (result.it - sb);
                    return;
                }
                string_buffer_.append(sb,p_-sb);
                column_ += (p_ - sb + 1);
                ++p_;
                goto escape;
            }
            case '\"':
            {
                auto result = unicons::validate(sb,p_);
                if (result.ec != unicons::conv_errc())
                {
                    translate_conv_errc(result.ec,ec);
                    column_ += (result.it - sb);
                    return;
                }
                if (string_buffer_.length() == 0)
                {
                    end_string_value(sb,p_-sb, ec);
                    if (ec) {return;}
                }
                else
                {
                    string_buffer_.append(sb,p_-sb);
                    end_string_value(string_buffer_.data(),string_buffer_.length(), ec);
                    string_buffer_.clear();
                    if (ec) {return;}
                }
                column_ += (p_ - sb + 1);
                ++p_;
                return;
            }
            default:
                ++p_;
                goto string_u1;
            }

escape:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::escape;
            return;
        }
        switch (*p_)
        {
        case '\"':
            string_buffer_.push_back('\"');
            sb = ++p_;
            ++column_;
            goto string_u1;
        case '\\': 
            string_buffer_.push_back('\\');
            sb = ++p_;
            ++column_;
            goto string_u1;
        case '/':
            string_buffer_.push_back('/');
            sb = ++p_;
            ++column_;
            goto string_u1;
        case 'b':
            string_buffer_.push_back('\b');
            sb = ++p_;
            ++column_;
            goto string_u1;
        case 'f':
            string_buffer_.push_back('\f');
            sb = ++p_;
            ++column_;
            goto string_u1;
        case 'n':
            string_buffer_.push_back('\n');
            sb = ++p_;
            ++column_;
            goto string_u1;
        case 'r':
            string_buffer_.push_back('\r');
            sb = ++p_;
            ++column_;
            goto string_u1;
        case 't':
            string_buffer_.push_back('\t');
            sb = ++p_;
            ++column_;
            goto string_u1;
        case 'u':
            cp_ = 0;
            ++p_;
            ++column_;
            goto escape_u1;
        default:    
            err_handler_.error(json_parser_errc::illegal_escaped_character, *this);
            ec = json_parser_errc::illegal_escaped_character;
            state_ = parse_state::escape;
            return;
        }

escape_u1:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u1;
            return;
        }
        {
            append_codepoint(*p_,ec);
            if (ec)
            {
                state_ = parse_state::escape_u1;
                return;
            }
            ++p_;
            ++column_;
            goto escape_u2;
        }

escape_u2:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u2;
            return;
        }
        {
            append_codepoint(*p_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u2;
                return;
            }
            ++p_;
            ++column_;
            goto escape_u3;
        }

escape_u3:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u3;
            return;
        }
        {
            append_codepoint(*p_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u3;
                return;
            }
            ++p_;
            ++column_;
            goto escape_u4;
        }

escape_u4:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u4;
            return;
        }
        {
            append_codepoint(*p_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u4;
                return;
            }
            if (unicons::is_high_surrogate(cp_))
            {
                ++p_;
                ++column_;
                goto escape_expect_surrogate_pair1;
            }
            else
            {
                unicons::convert(&cp_, &cp_ + 1, std::back_inserter(string_buffer_));
                sb = ++p_;
                ++column_;
                state_ = parse_state::string_u1;
                return;
            }
        }

escape_expect_surrogate_pair1:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::escape_expect_surrogate_pair1;
            return;
        }
        {
            switch (*p_)
            {
            case '\\': 
                cp2_ = 0;
                ++p_;
                ++column_;
                goto escape_expect_surrogate_pair2;
            default:
                err_handler_.error(json_parser_errc::expected_codepoint_surrogate_pair, *this);
                ec = json_parser_errc::expected_codepoint_surrogate_pair;
                state_ = parse_state::escape_expect_surrogate_pair1;
                return;
            }
        }

escape_expect_surrogate_pair2:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::escape_expect_surrogate_pair2;
            return;
        }
        {
            switch (*p_)
            {
            case 'u':
                ++p_;
                ++column_;
                goto escape_u6;
            default:
                err_handler_.error(json_parser_errc::expected_codepoint_surrogate_pair, *this);
                ec = json_parser_errc::expected_codepoint_surrogate_pair;
                state_ = parse_state::escape_expect_surrogate_pair2;
                return;
            }
        }

escape_u6:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u6;
            return;
        }
        {
            append_second_codepoint(*p_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u6;
                return;
            }
        }
        ++p_;
        ++column_;
        goto escape_u7;

escape_u7:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u7;
            return;
        }
        {
            append_second_codepoint(*p_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u7;
                return;
            }
            ++p_;
            ++column_;
            goto escape_u8;
        }

escape_u8:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u8;
            return;
        }
        {
            append_second_codepoint(*p_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u8;
                return;
            }
            ++p_;
            ++column_;
            goto escape_u9;
        }

escape_u9:
        if (JSONCONS_UNLIKELY(p_ >= local_end_input)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u9;
            return;
        }
        {
            append_second_codepoint(*p_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u9;
                return;
            }
            uint32_t cp = 0x10000 + ((cp_ & 0x3FF) << 10) + (cp2_ & 0x3FF);
            unicons::convert(&cp, &cp + 1, std::back_inserter(string_buffer_));
            sb = ++p_;
            ++column_;
            goto string_u1;
        }

        JSONCONS_UNREACHABLE();               
    }

    void translate_conv_errc(unicons::conv_errc result, std::error_code& ec)
    {
        switch (result)
        {
        case unicons::conv_errc():
            break;
        case unicons::conv_errc::over_long_utf8_sequence:
            if (err_handler_.error(json_parser_errc::over_long_utf8_sequence, *this))
            {
                ec = json_parser_errc::over_long_utf8_sequence;
                return;
            }
            break;
        case unicons::conv_errc::unpaired_high_surrogate:
            if (err_handler_.error(json_parser_errc::unpaired_high_surrogate, *this))
            {
                ec = json_parser_errc::unpaired_high_surrogate;
                return;
            }
            break;
        case unicons::conv_errc::expected_continuation_byte:
            if (err_handler_.error(json_parser_errc::expected_continuation_byte, *this))
            {
                ec = json_parser_errc::expected_continuation_byte;
                return;
            }
            break;
        case unicons::conv_errc::illegal_surrogate_value:
            if (err_handler_.error(json_parser_errc::illegal_surrogate_value, *this))
            {
                ec = json_parser_errc::illegal_surrogate_value;
                return;
            }
            break;
        default:
            if (err_handler_.error(json_parser_errc::illegal_codepoint, *this))
            {
                ec = json_parser_errc::illegal_codepoint;
                return;
            }
            break;
        }
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
                case parse_state::positive_zero:  
                case parse_state::positive_integer:
                    end_uinteger_value(ec);
                    if (ec) return;
                    break;
                case parse_state::negative_zero:  
                case parse_state::negative_integer:
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

    void set_source(const CharT* input, size_t length)
    {
        begin_input_ = input;
        end_input_ = input + length;
        p_ = begin_input_;
    }
private:
    static double pow10(int exponent) 
    {
        if (exponent > 308) {
            return std::numeric_limits<double>::infinity();
        } else if (exponent < -323) {
            return 0.0;
        }
        static const double constants[] = {
            1e-323,1e-322,1e-321,1e-320,1e-319,1e-318,1e-317,1e-316,1e-315,1e-314,
            1e-313,1e-312,1e-311,1e-310,1e-309,1e-308,1e-307,1e-306,1e-305,1e-304,
            1e-303,1e-302,1e-301,1e-300,1e-299,1e-298,1e-297,1e-296,1e-295,1e-294,
            1e-293,1e-292,1e-291,1e-290,1e-289,1e-288,1e-287,1e-286,1e-285,1e-284,
            1e-283,1e-282,1e-281,1e-280,1e-279,1e-278,1e-277,1e-276,1e-275,1e-274,
            1e-273,1e-272,1e-271,1e-270,1e-269,1e-268,1e-267,1e-266,1e-265,1e-264,
            1e-263,1e-262,1e-261,1e-260,1e-259,1e-258,1e-257,1e-256,1e-255,1e-254,
            1e-253,1e-252,1e-251,1e-250,1e-249,1e-248,1e-247,1e-246,1e-245,1e-244,
            1e-243,1e-242,1e-241,1e-240,1e-239,1e-238,1e-237,1e-236,1e-235,1e-234,
            1e-233,1e-232,1e-231,1e-230,1e-229,1e-228,1e-227,1e-226,1e-225,1e-224,
            1e-223,1e-222,1e-221,1e-220,1e-219,1e-218,1e-217,1e-216,1e-215,1e-214,
            1e-213,1e-212,1e-211,1e-210,1e-209,1e-208,1e-207,1e-206,1e-205,1e-204,
            1e-203,1e-202,1e-201,1e-200,1e-199,1e-198,1e-197,1e-196,1e-195,1e-194,
            1e-193,1e-192,1e-191,1e-190,1e-189,1e-188,1e-187,1e-186,1e-185,1e-184,
            1e-183,1e-182,1e-181,1e-180,1e-179,1e-178,1e-177,1e-176,1e-175,1e-174,
            1e-173,1e-172,1e-171,1e-170,1e-169,1e-168,1e-167,1e-166,1e-165,1e-164,
            1e-163,1e-162,1e-161,1e-160,1e-159,1e-158,1e-157,1e-156,1e-155,1e-154,
            1e-153,1e-152,1e-151,1e-150,1e-149,1e-148,1e-147,1e-146,1e-145,1e-144,
            1e-143,1e-142,1e-141,1e-140,1e-139,1e-138,1e-137,1e-136,1e-135,1e-134,
            1e-133,1e-132,1e-131,1e-130,1e-129,1e-128,1e-127,1e-126,1e-125,1e-124,
            1e-123,1e-122,1e-121,1e-120,1e-119,1e-118,1e-117,1e-116,1e-115,1e-114,
            1e-113,1e-112,1e-111,1e-110,1e-109,1e-108,1e-107,1e-106,1e-105,1e-104,
            1e-103,1e-102,1e-101,1e-100,1e-99,1e-98,1e-97,1e-96,1e-95,1e-94,1e-93,
            1e-92,1e-91,1e-90,1e-89,1e-88,1e-87,1e-86,1e-85,1e-84,1e-83,1e-82,1e-81,
            1e-80,1e-79,1e-78,1e-77,1e-76,1e-75,1e-74,1e-73,1e-72,1e-71,1e-70,1e-69,
            1e-68,1e-67,1e-66,1e-65,1e-64,1e-63,1e-62,1e-61,1e-60,1e-59,1e-58,1e-57,
            1e-56,1e-55,1e-54,1e-53,1e-52,1e-51,1e-50,1e-49,1e-48,1e-47,1e-46,1e-45,
            1e-44,1e-43,1e-42,1e-41,1e-40,1e-39,1e-38,1e-37,1e-36,1e-35,1e-34,1e-33,
            1e-32,1e-31,1e-30,1e-29,1e-28,1e-27,1e-26,1e-25,1e-24,1e-23,1e-22,1e-21,
            1e-20,1e-19,1e-18,1e-17,1e-16,1e-15,1e-14,1e-13,1e-12,1e-11,1e-10,1e-9,
            1e-8,1e-7,1e-6,1e-5,1e-4,1e-3,1e-2,1e-1,1e0,1e1,1e2,1e3,1e4,1e5,1e6,1e7,
            1e8,1e9,1e10,1e11,1e12,1e13,1e14,1e15,1e16,1e17,1e18,1e19,1e20,1e21,
            1e22,1e23,1e24,1e25,1e26,1e27,1e28,1e29,1e30,1e31,1e32,1e33,1e34,1e35,
            1e36,1e37,1e38,1e39,1e40,1e41,1e42,1e43,1e44,1e45,1e46,1e47,1e48,1e49,
            1e50,1e51,1e52,1e53,1e54,1e55,1e56,1e57,1e58,1e59,1e60,1e61,1e62,1e63,
            1e64,1e65,1e66,1e67,1e68,1e69,1e70,1e71,1e72,1e73,1e74,1e75,1e76,1e77,
            1e78,1e79,1e80,1e81,1e82,1e83,1e84,1e85,1e86,1e87,1e88,1e89,1e90,1e91,
            1e92,1e93,1e94,1e95,1e96,1e97,1e98,1e99,1e100,1e101,1e102,1e103,1e104,
            1e105,1e106,1e107,1e108,1e109,1e110,1e111,1e112,1e113,1e114,1e115,1e116,
            1e117,1e118,1e119,1e120,1e121,1e122,1e123,1e124,1e125,1e126,1e127,1e128,
            1e129,1e130,1e131,1e132,1e133,1e134,1e135,1e136,1e137,1e138,1e139,1e140,
            1e141,1e142,1e143,1e144,1e145,1e146,1e147,1e148,1e149,1e150,1e151,1e152,
            1e153,1e154,1e155,1e156,1e157,1e158,1e159,1e160,1e161,1e162,1e163,1e164,
            1e165,1e166,1e167,1e168,1e169,1e170,1e171,1e172,1e173,1e174,1e175,1e176,
            1e177,1e178,1e179,1e180,1e181,1e182,1e183,1e184,1e185,1e186,1e187,1e188,
            1e189,1e190,1e191,1e192,1e193,1e194,1e195,1e196,1e197,1e198,1e199,1e200,
            1e201,1e202,1e203,1e204,1e205,1e206,1e207,1e208,1e209,1e210,1e211,1e212,
            1e213,1e214,1e215,1e216,1e217,1e218,1e219,1e220,1e221,1e222,1e223,1e224,
            1e225,1e226,1e227,1e228,1e229,1e230,1e231,1e232,1e233,1e234,1e235,1e236,
            1e237,1e238,1e239,1e240,1e241,1e242,1e243,1e244,1e245,1e246,1e247,1e248,
            1e249,1e250,1e251,1e252,1e253,1e254,1e255,1e256,1e257,1e258,1e259,1e260,
            1e261,1e262,1e263,1e264,1e265,1e266,1e267,1e268,1e269,1e270,1e271,1e272,
            1e273,1e274,1e275,1e276,1e277,1e278,1e279,1e280,1e281,1e282,1e283,1e284,
            1e285,1e286,1e287,1e288,1e289,1e290,1e291,1e292,1e293,1e294,1e295,1e296,
            1e297,1e298,1e299,1e300,1e301,1e302,1e303,1e304,1e305,1e306,1e307,1e308
        };
        return constants[exponent + 323];
    }

    double pow10_(int exponent)
    {
        if (exponent > 308) {
            return std::numeric_limits<double>::infinity();
        } else if (exponent < -323) {
            return 0.0;
        }
        long double base = 10;
        unsigned int exp = exponent;
        if (exponent < 0)
        {
            base = 0.1;
            exp = -exponent;
        }
        long double power = 1;
        for (; exp; exp >>= 1, base *= base)
            if (exp & 1)
                power *= base;
        return power;
    }

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
        handler_.integer_value(negative_val_, *this);

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

    void end_uinteger_value(std::error_code& ec)
    {
        handler_.uinteger_value(positive_val_, *this);

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

    void end_string_value(const CharT* s, size_t length, std::error_code& ec) 
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

typedef basic_json_parser<char> json_parser;
typedef basic_json_parser<wchar_t> wjson_parser;

}

#endif


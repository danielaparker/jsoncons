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
#include <jsoncons/json_filter.hpp>
#include <jsoncons/json_serializing_options.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/parse_error_handler.hpp>
#include <jsoncons/json_error_category.hpp>
#include <jsoncons/detail/parse_number.hpp>

#define JSONCONS_ILLEGAL_CONTROL_CHARACTER \
        case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x08:case 0x0b: \
        case 0x0c:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:case 0x15:case 0x16: \
        case 0x17:case 0x18:case 0x19:case 0x1a:case 0x1b:case 0x1c:case 0x1d:case 0x1e:case 0x1f 

namespace jsoncons {

namespace detail {

template <class CharT>
class replacement_filter : public basic_json_filter<CharT>
{
    typedef typename basic_json_content_handler<CharT>::string_view_type string_view_type;
    typedef typename basic_json_serializing_options<CharT>::string_type string_type;

    basic_null_json_content_handler<CharT> default_content_handler_;

    bool can_read_nan_replacement_;
    bool can_read_pos_inf_replacement_;
    bool can_read_neg_inf_replacement_;
    string_type nan_replacement_;
    string_type pos_inf_replacement_;
    string_type neg_inf_replacement_;

public:
    replacement_filter() = delete;

    replacement_filter(basic_json_content_handler<CharT>& handler, const basic_json_read_options<CharT>& options)
        : basic_json_filter<CharT>(handler), 
          can_read_nan_replacement_(options.can_read_nan_replacement()),
          can_read_pos_inf_replacement_(options.can_read_pos_inf_replacement()),
          can_read_neg_inf_replacement_(options.can_read_neg_inf_replacement()),
          nan_replacement_(options.nan_replacement()),
          pos_inf_replacement_(options.pos_inf_replacement()),
          neg_inf_replacement_(options.neg_inf_replacement())
    {
    }

    void do_string_value(const string_view_type& s, const serializing_context& context) override
    {
        if (can_read_nan_replacement_ && s == nan_replacement_.substr(1,nan_replacement_.length()-2))
        {
            this->downstream_handler().double_value(std::nan(""), context);
        }
        else if (can_read_pos_inf_replacement_ && s == pos_inf_replacement_.substr(1,pos_inf_replacement_.length()-2))
        {
            this->downstream_handler().double_value(std::numeric_limits<double>::infinity(), context);
        }
        else if (can_read_neg_inf_replacement_ && s == neg_inf_replacement_.substr(1,neg_inf_replacement_.length()-2))
        {
            this->downstream_handler().double_value(-std::numeric_limits<double>::infinity(), context);
        }
        else
        {
            this->downstream_handler().string_value(s, context);
        }
    }
};

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
    zero,  
    integer,
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

template <class CharT, class Allocator = std::allocator<char>>
class basic_json_parser : private serializing_context
{
    static const size_t initial_string_buffer_capacity_ = 1024;
    static const size_t initial_number_buffer_capacity_ = 64;
    static const int default_initial_stack_capacity_ = 100;
    typedef typename basic_json_content_handler<CharT>::string_view_type string_view_type;

    detail::replacement_filter<CharT> replacement_filter_;
    basic_null_json_content_handler<CharT> default_content_handler_;
    default_parse_error_handler default_err_handler_;

    basic_json_content_handler<CharT>& handler_;
    parse_error_handler& err_handler_;
    uint32_t cp_;
    uint32_t cp2_;

    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT> char_allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<char> numeral_allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<uint8_t> byte_allocator_type;

    std::basic_string<CharT,std::char_traits<CharT>,char_allocator_type> string_buffer_;
    std::basic_string<char,std::char_traits<char>,numeral_allocator_type> number_buffer_;
    std::vector<uint8_t,byte_allocator_type> byte_buffer_;

    uint8_t precision_;
    uint8_t decimal_places_;

    size_t line_;
    size_t column_;
    size_t nesting_depth_;
    int initial_stack_capacity_;

    size_t max_nesting_depth_;
    detail::string_to_double to_double_;
    const CharT* begin_input_;
    const CharT* input_end_;
    const CharT* input_ptr_;

    parse_state state_;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<parse_state> parse_state_allocator_type;
    std::vector<parse_state,parse_state_allocator_type> state_stack_;

    // Noncopyable and nonmoveable
    basic_json_parser(const basic_json_parser&) = delete;
    basic_json_parser& operator=(const basic_json_parser&) = delete;

public:
    basic_json_parser()
        : basic_json_parser(default_content_handler_, basic_json_serializing_options<CharT>(), default_err_handler_)
    {
    }

    basic_json_parser(parse_error_handler& err_handler)
        : basic_json_parser(default_content_handler_, basic_json_serializing_options<CharT>(), err_handler)
    {
    }

    basic_json_parser(basic_json_content_handler<CharT>& handler)
        : basic_json_parser(handler, basic_json_serializing_options<CharT>(), default_err_handler_)
    {
    }

    basic_json_parser(basic_json_content_handler<CharT>& handler,
                      parse_error_handler& err_handler)
        : basic_json_parser(handler, basic_json_serializing_options<CharT>(), err_handler)
    {
    }

    basic_json_parser(const basic_json_read_options<CharT>& options)
        : basic_json_parser(default_content_handler_, options, default_err_handler_)
    {
    }

    basic_json_parser(const basic_json_read_options<CharT>& options, 
                      parse_error_handler& err_handler)
        : basic_json_parser(default_content_handler_, options, err_handler)
    {
    }

    basic_json_parser(basic_json_content_handler<CharT>& handler,
                      const basic_json_read_options<CharT>& options)
        : basic_json_parser(handler, options, default_err_handler_)
    {
    }

    basic_json_parser(basic_json_content_handler<CharT>& handler, 
                      const basic_json_read_options<CharT>& options,
                      parse_error_handler& err_handler)
       : replacement_filter_(handler,options),
         handler_((options.can_read_nan_replacement() || options.can_read_pos_inf_replacement() || options.can_read_neg_inf_replacement()) ? replacement_filter_ : handler),
         err_handler_(err_handler),
         max_nesting_depth_(options.max_nesting_depth()),
         cp_(0),
         cp2_(0),
         precision_(0), 
         decimal_places_(0), 
         line_(1),
         column_(1),
         nesting_depth_(0), 
         initial_stack_capacity_(default_initial_stack_capacity_),
         begin_input_(nullptr),
         input_end_(nullptr),
         input_ptr_(nullptr),
         state_(parse_state::start)
    {
        string_buffer_.reserve(initial_string_buffer_capacity_);
        number_buffer_.reserve(initial_number_buffer_capacity_);

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
        return input_ptr_ == input_end_;
    }

    ~basic_json_parser()
    {
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    size_t max_nesting_depth() const
    {
        return max_nesting_depth_;
    }

    void max_nesting_depth(size_t value)
    {
        max_nesting_depth_ = value;
    }
#endif
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
        const CharT* local_input_end = input_end_;
        for (;;) 
        {
            if (JSONCONS_UNLIKELY(input_ptr_ == local_input_end)) 
            {
                return;
            } 
            else if (*input_ptr_ == ' ' || *input_ptr_ == '\t') 
            {
                ++input_ptr_;
                ++column_;                     
            } 
            else 
            {
                return;
            }
        }
    }

    void begin_object(std::error_code& ec)
    {
        if (++nesting_depth_ > max_nesting_depth_)
        {
            if (err_handler_.error(json_parse_errc::max_depth_exceeded, *this))
            {
                ec = json_parse_errc::max_depth_exceeded;
                return;
            }
        } 
        push_state(parse_state::object);
        state_ = parse_state::expect_member_name_or_end;
        handler_.begin_object(*this);
    }

    void end_object(std::error_code& ec)
    {
        JSONCONS_ASSERT(nesting_depth_ >= 1);
        --nesting_depth_;
        state_ = pop_state();
        if (state_ == parse_state::object)
        {
            handler_.end_object(*this);
        }
        else if (state_ == parse_state::array)
        {
            err_handler_.fatal_error(json_parse_errc::expected_comma_or_right_bracket, *this);
            ec = json_parse_errc::expected_comma_or_right_bracket;
            return;
        }
        else
        {
            err_handler_.fatal_error(json_parse_errc::unexpected_right_brace, *this);
            ec = json_parse_errc::unexpected_right_brace;
            return;
        }

        if (parent() == parse_state::root)
        {
            state_ = parse_state::done;
            handler_.end_document();
        }
        else
        {
            state_ = parse_state::expect_comma_or_end;
        }
    }

    void begin_array(std::error_code& ec)
    {
        if (++nesting_depth_ > max_nesting_depth_)
        {
            if (err_handler_.error(json_parse_errc::max_depth_exceeded, *this))
            {
                ec = json_parse_errc::max_depth_exceeded;
                return;
            }

        }
        push_state(parse_state::array);
        state_ = parse_state::expect_value_or_end;
        handler_.begin_array(*this);
    }

    void end_array(std::error_code& ec)
    {
        JSONCONS_ASSERT(nesting_depth_ >= 1);
        --nesting_depth_;
        state_ = pop_state();
        if (state_ == parse_state::array)
        {
            handler_.end_array(*this);
        }
        else if (state_ == parse_state::object)
        {
            err_handler_.fatal_error(json_parse_errc::expected_comma_or_right_brace, *this);
            ec = json_parse_errc::expected_comma_or_right_brace;
            return;
        }
        else
        {
            err_handler_.fatal_error(json_parse_errc::unexpected_right_bracket, *this);
            ec = json_parse_errc::unexpected_right_bracket;
            return;
        }
        if (parent() == parse_state::root)
        {
            state_ = parse_state::done;
            handler_.end_document();
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
            if (err_handler_.error(json_parse_errc::unexpected_eof, *this))
            {
                ec = json_parse_errc::unexpected_eof;
                return;
            }
        }
        for (; input_ptr_ != input_end_; ++input_ptr_)
        {
            CharT curr_char_ = *input_ptr_;
            switch (curr_char_)
            {
            case '\n':
            case '\r':
            case '\t':
            case ' ':
                break;
            default:
                if (err_handler_.error(json_parse_errc::extra_character, *this))
                {
                    ec = json_parse_errc::extra_character;
                    return;
                }
                break;
            }
        }
    }

    void parse_some(std::error_code& ec)
    {
        const CharT* local_input_end = input_end_;

        while ((input_ptr_ < local_input_end) && (state_ != parse_state::done))
        {
            switch (state_)
            {
            case parse_state::cr:
                ++line_;
                column_ = 1;
                switch (*input_ptr_)
                {
                case '\n':
                    state_ = pop_state();
                    ++input_ptr_;
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
                    handler_.begin_document();
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            if (err_handler_.error(json_parse_errc::illegal_control_character, *this))
                            {
                                ec = json_parse_errc::illegal_control_character;
                                return;
                            }
                            break;
                        case '\r': 
                            push_state(state_);
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::cr;
                            break; 
                        case '\n': 
                            ++input_ptr_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::lf;
                            break;   
                        case ' ':case '\t':
                            skip_whitespace();
                            break;
                        case '/': 
                            ++input_ptr_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::slash;
                            break;
                        case '{':
                            begin_object(ec);
                            if (ec) return;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '[':
                            begin_array(ec);
                            if (ec) return;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '\"':
                            state_ = parse_state::string_u1;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '-':
                            number_buffer_.clear();
                            number_buffer_.push_back('-');
                            precision_ = 0;
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::minus;
                            parse_number(ec);
                            if (ec) {return;}
                            break;
                        case '0': 
                            number_buffer_.clear();
                            precision_ = 1;
                            number_buffer_.push_back(static_cast<char>(*input_ptr_));
                            state_ = parse_state::zero;
                            ++input_ptr_;
                            ++column_;
                            parse_number(ec);
                            if (ec) {return;}
                            break;
                        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                            number_buffer_.clear();
                            precision_ = 1;
                            number_buffer_.push_back(static_cast<char>(*input_ptr_));
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::integer;
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
                            err_handler_.fatal_error(json_parse_errc::unexpected_right_brace, *this);
                            ec = json_parse_errc::unexpected_right_brace;
                            return;
                        case ']':
                            err_handler_.fatal_error(json_parse_errc::unexpected_right_bracket, *this);
                            ec = json_parse_errc::unexpected_right_bracket;
                            return;
                        default:
                            err_handler_.fatal_error(json_parse_errc::invalid_json_text, *this);
                            ec = json_parse_errc::invalid_json_text;
                            return;
                    }
                }
                break;

            case parse_state::expect_comma_or_end: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            if (err_handler_.error(json_parse_errc::illegal_control_character, *this))
                            {
                                ec = json_parse_errc::illegal_control_character;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '\r': 
                            ++input_ptr_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::cr;
                            break; 
                        case '\n': 
                            ++input_ptr_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::lf;
                            break;   
                        case ' ':case '\t':
                            skip_whitespace();
                            break;
                        case '/':
                            ++input_ptr_;
                            ++column_;
                            push_state(state_); 
                            state_ = parse_state::slash;
                            break;
                        case '}':
                            end_object(ec);
                            if (ec) return;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case ']':
                            end_array(ec);
                            if (ec) return;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case ',':
                            begin_member_or_element(ec);
                            if (ec) return;
                            ++input_ptr_;
                            ++column_;
                            break;
                        default:
                            if (parent() == parse_state::array)
                            {
                                if (err_handler_.error(json_parse_errc::expected_comma_or_right_bracket, *this))
                                {
                                    ec = json_parse_errc::expected_comma_or_right_bracket;
                                    return;
                                }
                            }
                            else if (parent() == parse_state::object)
                            {
                                if (err_handler_.error(json_parse_errc::expected_comma_or_right_brace, *this))
                                {
                                    ec = json_parse_errc::expected_comma_or_right_brace;
                                    return;
                                }
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                    }
                }
                break;
            case parse_state::expect_member_name_or_end: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            if (err_handler_.error(json_parse_errc::illegal_control_character, *this))
                            {
                                ec = json_parse_errc::illegal_control_character;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '\r': 
                            ++input_ptr_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::cr;
                            break; 
                        case '\n': 
                            ++input_ptr_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::lf;
                            break;   
                        case ' ':case '\t':
                            skip_whitespace();
                            break;
                        case '/':
                            ++input_ptr_;
                            ++column_;
                            push_state(state_); 
                            state_ = parse_state::slash;
                            break;
                        case '}':
                            end_object(ec);
                            if (ec) return;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '\"':
                            ++input_ptr_;
                            ++column_;
                            push_state(parse_state::member_name);
                            state_ = parse_state::string_u1;
                            break;
                        case '\'':
                            if (err_handler_.error(json_parse_errc::single_quote, *this))
                            {
                                ec = json_parse_errc::single_quote;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                        default:
                            if (err_handler_.error(json_parse_errc::expected_name, *this))
                            {
                                ec = json_parse_errc::expected_name;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                    }
                }
                break;
            case parse_state::expect_member_name: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            if (err_handler_.error(json_parse_errc::illegal_control_character, *this))
                            {
                                ec = json_parse_errc::illegal_control_character;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '\r': 
                            ++input_ptr_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::cr;
                            break; 
                        case '\n': 
                            ++input_ptr_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::lf;
                            break;   
                        case ' ':case '\t':
                            skip_whitespace();
                            break;
                        case '/': 
                            ++input_ptr_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::slash;
                            break;
                        case '\"':
                            ++input_ptr_;
                            ++column_;
                            push_state(parse_state::member_name);
                            state_ = parse_state::string_u1;
                            break;
                        case '}':
                            if (err_handler_.error(json_parse_errc::extra_comma, *this))
                            {
                                ec = json_parse_errc::extra_comma;
                                return;
                            }
                            end_object(ec);  // Recover
                            if (ec) return;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '\'':
                            if (err_handler_.error(json_parse_errc::single_quote, *this))
                            {
                                ec = json_parse_errc::single_quote;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                        default:
                            if (err_handler_.error(json_parse_errc::expected_name, *this))
                            {
                                ec = json_parse_errc::expected_name;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                    }
                }
                break;
            case parse_state::expect_colon: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            if (err_handler_.error(json_parse_errc::illegal_control_character, *this))
                            {
                                ec = json_parse_errc::illegal_control_character;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '\r': 
                            push_state(state_);
                            state_ = parse_state::cr;
                            ++input_ptr_;
                            ++column_;
                            break; 
                        case '\n': 
                            push_state(state_);
                            state_ = parse_state::lf;
                            ++input_ptr_;
                            ++column_;
                            break;   
                        case ' ':case '\t':
                            skip_whitespace();
                            break;
                        case '/': 
                            push_state(state_);
                            state_ = parse_state::slash;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case ':':
                            state_ = parse_state::expect_value;
                            ++input_ptr_;
                            ++column_;
                            break;
                        default:
                            if (err_handler_.error(json_parse_errc::expected_colon, *this))
                            {
                                ec = json_parse_errc::expected_colon;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                    }
                }
                break;

                case parse_state::expect_value: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            if (err_handler_.error(json_parse_errc::illegal_control_character, *this))
                            {
                                ec = json_parse_errc::illegal_control_character;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '\r': 
                            push_state(state_);
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::cr;
                            break; 
                        case '\n': 
                            push_state(state_);
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::lf;
                            break;   
                        case ' ':case '\t':
                            skip_whitespace();
                            break;
                        case '/': 
                            push_state(state_);
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::slash;
                            break;
                        case '{':
                            begin_object(ec);
                            if (ec) return;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '[':
                            begin_array(ec);
                            if (ec) return;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '\"':
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::string_u1;
                            break;
                        case '-':
                            number_buffer_.clear();
                            number_buffer_.push_back('-');
                            precision_ = 0;
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::minus;
                            parse_number(ec);
                            if (ec) {return;}
                            break;
                        case '0': 
                            number_buffer_.clear();
                            precision_ = 1;
                            number_buffer_.push_back(static_cast<char>(*input_ptr_));
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::zero;
                            parse_number(ec);
                            if (ec) {return;}
                            break;
                        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                            number_buffer_.clear();
                            precision_ = 1;
                            number_buffer_.push_back(static_cast<char>(*input_ptr_));
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::integer;
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
                                if (err_handler_.error(json_parse_errc::extra_comma, *this))
                                {
                                    ec = json_parse_errc::extra_comma;
                                    return;
                                }
                                end_array(ec);  // Recover
                                if (ec) return;
                            }
                            else
                            {
                                if (err_handler_.error(json_parse_errc::expected_value, *this))
                                {
                                    ec = json_parse_errc::expected_value;
                                    return;
                                }
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '\'':
                            if (err_handler_.error(json_parse_errc::single_quote, *this))
                            {
                                ec = json_parse_errc::single_quote;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                        default:
                            if (err_handler_.error(json_parse_errc::expected_value, *this))
                            {
                                ec = json_parse_errc::expected_value;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                    }
                }
                break;
                case parse_state::expect_value_or_end: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            if (err_handler_.error(json_parse_errc::illegal_control_character, *this))
                            {
                                ec = json_parse_errc::illegal_control_character;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '\r': 
                            ++input_ptr_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::cr;
                            break; 
                        case '\n': 
                            ++input_ptr_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::lf;
                            break;   
                        case ' ':case '\t':
                            skip_whitespace();
                            break;
                        case '/': 
                            ++input_ptr_;
                            ++column_;
                            push_state(state_);
                            state_ = parse_state::slash;
                            break;
                        case '{':
                            begin_object(ec);
                            if (ec) return;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '[':
                            begin_array(ec);
                            if (ec) return;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case ']':
                            end_array(ec);
                            if (ec) return;
                            ++input_ptr_;
                            ++column_;
                            break;
                        case '\"':
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::string_u1;
                            break;
                        case '-':
                            number_buffer_.clear();
                            number_buffer_.push_back('-');
                            precision_ = 0;
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::minus;
                            parse_number(ec);
                            if (ec) {return;}
                            break;
                        case '0': 
                            number_buffer_.clear();
                            precision_ = 1;
                            number_buffer_.push_back(static_cast<char>(*input_ptr_));
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::zero;
                            parse_number(ec);
                            if (ec) {return;}
                            break;
                        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                            number_buffer_.clear();
                            precision_ = 1;
                            number_buffer_.push_back(static_cast<char>(*input_ptr_));
                            ++input_ptr_;
                            ++column_;
                            state_ = parse_state::integer;
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
                            if (err_handler_.error(json_parse_errc::single_quote, *this))
                            {
                                ec = json_parse_errc::single_quote;
                                return;
                            }
                            ++input_ptr_;
                            ++column_;
                            break;
                        default:
                            if (err_handler_.error(json_parse_errc::expected_value, *this))
                            {
                                ec = json_parse_errc::expected_value;
                                return;
                            }
                            ++input_ptr_;
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
            case parse_state::zero:  
            case parse_state::integer: 
            case parse_state::fraction1: 
            case parse_state::fraction2: 
            case parse_state::exp1: 
            case parse_state::exp2:  
            case parse_state::exp3: 
                parse_number(ec);  
                if (ec) return;
                break;
            case parse_state::t: 
                switch (*input_ptr_)
                {
                case 'r':
                    ++input_ptr_;
                    ++column_;
                    state_ = parse_state::tr;
                    break;
                default:
                    err_handler_.error(json_parse_errc::invalid_value, *this);
                    ec = json_parse_errc::invalid_value;
                    return;
                }
                break;
            case parse_state::tr: 
                switch (*input_ptr_)
                {
                case 'u':
                    state_ = parse_state::tru;
                    break;
                default:
                    err_handler_.error(json_parse_errc::invalid_value, *this);
                    ec = json_parse_errc::invalid_value;
                    return;
                }
                ++input_ptr_;
                ++column_;
                break;
            case parse_state::tru: 
                switch (*input_ptr_)
                {
                case 'e':
                    handler_.bool_value(true,*this);
                    if (parent() == parse_state::root)
                    {
                        state_ = parse_state::done;
                        handler_.end_document();
                    }
                    else
                    {
                        state_ = parse_state::expect_comma_or_end;
                    }
                    break;
                default:
                    err_handler_.error(json_parse_errc::invalid_value, *this);
                    ec = json_parse_errc::invalid_value;
                    return;
                }
                ++input_ptr_;
                ++column_;
                break;
            case parse_state::f: 
                switch (*input_ptr_)
                {
                case 'a':
                    ++input_ptr_;
                    ++column_;
                    state_ = parse_state::fa;
                    break;
                default:
                    err_handler_.error(json_parse_errc::invalid_value, *this);
                    ec = json_parse_errc::invalid_value;
                    return;
                }
                break;
            case parse_state::fa: 
                switch (*input_ptr_)
                {
                case 'l':
                    state_ = parse_state::fal;
                    break;
                default:
                    err_handler_.error(json_parse_errc::invalid_value, *this);
                    ec = json_parse_errc::invalid_value;
                    return;
                }
                ++input_ptr_;
                ++column_;
                break;
            case parse_state::fal: 
                switch (*input_ptr_)
                {
                case 's':
                    state_ = parse_state::fals;
                    break;
                default:
                    err_handler_.error(json_parse_errc::invalid_value, *this);
                    ec = json_parse_errc::invalid_value;
                    return;
                }
                ++input_ptr_;
                ++column_;
                break;
            case parse_state::fals: 
                switch (*input_ptr_)
                {
                case 'e':
                    handler_.bool_value(false,*this);
                    if (parent() == parse_state::root)
                    {
                        state_ = parse_state::done;
                        handler_.end_document();
                    }
                    else
                    {
                        state_ = parse_state::expect_comma_or_end;
                    }
                    break;
                default:
                    err_handler_.error(json_parse_errc::invalid_value, *this);
                    ec = json_parse_errc::invalid_value;
                    return;
                }
                ++input_ptr_;
                ++column_;
                break;
            case parse_state::n: 
                switch (*input_ptr_)
                {
                case 'u':
                    ++input_ptr_;
                    ++column_;
                    state_ = parse_state::nu;
                    break;
                default:
                    err_handler_.error(json_parse_errc::invalid_value, *this);
                    ec = json_parse_errc::invalid_value;
                    return;
                }
                break;
            case parse_state::nu: 
                switch (*input_ptr_)
                {
                case 'l':
                    state_ = parse_state::nul;
                    break;
                default:
                    err_handler_.error(json_parse_errc::invalid_value, *this);
                    ec = json_parse_errc::invalid_value;
                    return;
                }
                ++input_ptr_;
                ++column_;
                break;
            case parse_state::nul: 
                switch (*input_ptr_)
                {
                case 'l':
                    handler_.null_value(*this);
                    if (parent() == parse_state::root)
                    {
                        state_ = parse_state::done;
                        handler_.end_document();
                    }
                    else
                    {
                        state_ = parse_state::expect_comma_or_end;
                    }
                    break;
                default:
                    err_handler_.error(json_parse_errc::invalid_value, *this);
                    ec = json_parse_errc::invalid_value;
                    return;
                }
                ++input_ptr_;
                ++column_;
                break;
            case parse_state::slash: 
                {
                    switch (*input_ptr_)
                    {
                    case '*':
                        state_ = parse_state::slash_star;
                        if (err_handler_.error(json_parse_errc::illegal_comment, *this))
                        {
                            ec = json_parse_errc::illegal_comment;
                            return;
                        }
                        break;
                    case '/':
                        state_ = parse_state::slash_slash;
                        if (err_handler_.error(json_parse_errc::illegal_comment, *this))
                        {
                            ec = json_parse_errc::illegal_comment;
                            return;
                        }
                        break;
                    default:    
                        if (err_handler_.error(json_parse_errc::invalid_json_text, *this))
                        {
                            ec = json_parse_errc::invalid_json_text;
                            return;
                        }
                        break;
                    }
                }
                ++input_ptr_;
                ++column_;
                break;
            case parse_state::slash_star:  
                {
                    switch (*input_ptr_)
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
                ++input_ptr_;
                ++column_;
                break;
            case parse_state::slash_slash: 
                {
                    switch (*input_ptr_)
                    {
                    case '\r':
                        state_ = pop_state();
                        break;
                    case '\n':
                        state_ = pop_state();
                        break;
                    default:
                        ++input_ptr_;
                        ++column_;
                    }
                }
                break;
            case parse_state::slash_star_star: 
                {
                    switch (*input_ptr_)
                    {
                    case '/':
                        state_ = pop_state();
                        break;
                    default:    
                        state_ = parse_state::slash_star;
                        break;
                    }
                }
                ++input_ptr_;
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
        if (JSONCONS_LIKELY(input_end_ - input_ptr_ >= 4))
        {
            if (*(input_ptr_+1) == 'r' && *(input_ptr_+2) == 'u' && *(input_ptr_+3) == 'e')
            {
                handler_.bool_value(true,*this);
                input_ptr_ += 4;
                column_ += 4;
                if (parent() == parse_state::root)
                {
                    handler_.end_document();
                    state_ = parse_state::done;
                }
                else
                {
                    state_ = parse_state::expect_comma_or_end;
                }
            }
            else
            {
                err_handler_.error(json_parse_errc::invalid_value, *this);
                ec = json_parse_errc::invalid_value;
                return;
            }
        }
        else
        {
            ++input_ptr_;
            ++column_;
            state_ = parse_state::t;
        }
    }

    void parse_null(std::error_code& ec)
    {
        if (JSONCONS_LIKELY(input_end_ - input_ptr_ >= 4))
        {
            if (*(input_ptr_+1) == 'u' && *(input_ptr_+2) == 'l' && *(input_ptr_+3) == 'l')
            {
                handler_.null_value(*this);
                input_ptr_ += 4;
                column_ += 4;
                if (parent() == parse_state::root)
                {
                    handler_.end_document();
                    state_ = parse_state::done;
                }
                else
                {
                    state_ = parse_state::expect_comma_or_end;
                }
            }
            else
            {
                err_handler_.error(json_parse_errc::invalid_value, *this);
                ec = json_parse_errc::invalid_value;
                return;
            }
        }
        else
        {
            ++input_ptr_;
            ++column_;
            state_ = parse_state::n;
        }
    }

    void parse_false(std::error_code& ec)
    {
        if (JSONCONS_LIKELY(input_end_ - input_ptr_ >= 5))
        {
            if (*(input_ptr_+1) == 'a' && *(input_ptr_+2) == 'l' && *(input_ptr_+3) == 's' && *(input_ptr_+4) == 'e')
            {
                handler_.bool_value(false,*this);
                input_ptr_ += 5;
                column_ += 5;
                if (parent() == parse_state::root)
                {
                    handler_.end_document();
                    state_ = parse_state::done;
                }
                else
                {
                    state_ = parse_state::expect_comma_or_end;
                }
            }
            else
            {
                err_handler_.error(json_parse_errc::invalid_value, *this);
                ec = json_parse_errc::invalid_value;
                return;
            }
        }
        else
        {
            ++input_ptr_;
            ++column_;
            state_ = parse_state::f;
        }
    }

    void parse_number(std::error_code& ec)
    {
        const CharT* local_input_end = input_end_;

        switch (state_)
        {
            case parse_state::minus:
                goto minus_sign;
            case parse_state::zero:
                goto zero;
            case parse_state::integer:
                goto integer;
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
minus_sign:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::minus;
            return;
        }
        switch (*input_ptr_)
        {
            case '0': 
                number_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++precision_;
                ++input_ptr_;
                ++column_;
                goto zero;
            case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                number_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++precision_;
                ++input_ptr_;
                ++column_;
                goto integer;
            default:
                err_handler_.error(json_parse_errc::expected_value, *this);
                ec = json_parse_errc::expected_value;
                return;
        }
zero:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::zero;
            return;
        }
        switch (*input_ptr_)
        {
            case '\r': 
                end_integer_value(ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                push_state(state_);
                state_ = parse_state::cr;
                return; 
            case '\n': 
                end_integer_value(ec);
                if (ec) return;
                push_state(state_);
                ++input_ptr_;
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
                ++input_ptr_;
                ++column_;
                push_state(state_);
                state_ = parse_state::slash;
                return;
            case '}':
                end_integer_value(ec);
                if (ec) return;
                end_object(ec);
                ++input_ptr_;
                ++column_;
                if (ec) return;
                return;
            case ']':
                end_integer_value(ec);
                if (ec) return;
                end_array(ec);
                ++input_ptr_;
                ++column_;
                if (ec) return;
                return;
            case '.':
                decimal_places_ = 0; 
                number_buffer_.push_back(to_double_.get_decimal_point());
                ++input_ptr_;
                ++column_;
                goto fraction1;
            case 'e':case 'E':
                number_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++column_;
                goto exp1;
            case ',':
                end_integer_value(ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                return;
            case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                err_handler_.error(json_parse_errc::leading_zero, *this);
                ec = json_parse_errc::leading_zero;
                state_ = parse_state::zero;
                return;
            default:
                err_handler_.error(json_parse_errc::invalid_number, *this);
                ec = json_parse_errc::invalid_number;
                state_ = parse_state::zero;
                return;
        }
integer:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::integer;
            return;
        }
        switch (*input_ptr_)
        {
            case '\r': 
                end_integer_value(ec);
                if (ec) return;
                push_state(state_);
                ++input_ptr_;
                ++column_;
                state_ = parse_state::cr;
                return; 
            case '\n': 
                end_integer_value(ec);
                if (ec) return;
                push_state(state_);
                ++input_ptr_;
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
                ++input_ptr_;
                ++column_;
                state_ = parse_state::slash;
                return;
            case '}':
                end_integer_value(ec);
                if (ec) return;
                end_object(ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                return;
            case ']':
                end_integer_value(ec);
                if (ec) return;
                end_array(ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                return;
            case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                number_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++precision_;
                ++input_ptr_;
                ++column_;
                goto integer;
            case '.':
                decimal_places_ = 0; 
                number_buffer_.push_back(to_double_.get_decimal_point());
                ++input_ptr_;
                ++column_;
                goto fraction1;
            case 'e':case 'E':
                number_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++column_;
                goto exp1;
            case ',':
                end_integer_value(ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                return;
            default:
                err_handler_.error(json_parse_errc::invalid_number, *this);
                ec = json_parse_errc::invalid_number;
                state_ = parse_state::integer;
                return;
        }
fraction1:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::fraction1;
            return;
        }
        switch (*input_ptr_)
        {
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                ++precision_;
                ++decimal_places_;
                number_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++column_;
                goto fraction2;
            default:
                err_handler_.error(json_parse_errc::invalid_number, *this);
                ec = json_parse_errc::invalid_number;
                state_ = parse_state::fraction1;
                return;
        }
fraction2:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::fraction2;
            return;
        }
        switch (*input_ptr_)
        {
            case '\r': 
                end_fraction_value(chars_format::fixed,ec);
                if (ec) return;
                push_state(state_);
                ++input_ptr_;
                ++column_;
                state_ = parse_state::cr;
                return; 
            case '\n': 
                end_fraction_value(chars_format::fixed,ec);
                if (ec) return;
                push_state(state_);
                ++input_ptr_;
                ++column_;
                state_ = parse_state::lf;
                return;   
            case ' ':case '\t':
                end_fraction_value(chars_format::fixed,ec);
                if (ec) return;
                skip_whitespace();
                return;
            case '/': 
                end_fraction_value(chars_format::fixed,ec);
                if (ec) return;
                push_state(state_);
                ++input_ptr_;
                ++column_;
                state_ = parse_state::slash;
                return;
            case '}':
                end_fraction_value(chars_format::fixed,ec);
                if (ec) return;
                end_object(ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                return;
            case ']':
                end_fraction_value(chars_format::fixed,ec);
                if (ec) return;
                end_array(ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                return;
            case ',':
                end_fraction_value(chars_format::fixed,ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                return;
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                ++precision_;
                ++decimal_places_;
                number_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++column_;
                goto fraction2;
            case 'e':case 'E':
                number_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++column_;
                goto exp1;
            default:
                err_handler_.error(json_parse_errc::invalid_number, *this);
                ec = json_parse_errc::invalid_number;
                state_ = parse_state::fraction2;
                return;
        }
exp1:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::exp1;
            return;
        }
        switch (*input_ptr_)
        {
            case '+':
                ++input_ptr_;
                ++column_;
                goto exp2;
            case '-':
                number_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++column_;
                goto exp2;
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                number_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++column_;
                goto exp3;
            default:
                err_handler_.error(json_parse_errc::expected_value, *this);
                ec = json_parse_errc::expected_value;
                state_ = parse_state::exp1;
                return;
        }
exp2:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::exp2;
            return;
        }
        switch (*input_ptr_)
        {
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                number_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++column_;
                goto exp3;
            default:
                err_handler_.error(json_parse_errc::expected_value, *this);
                ec = json_parse_errc::expected_value;
                state_ = parse_state::exp2;
                return;
        }
        
exp3:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::exp3;
            return;
        }
        switch (*input_ptr_)
        {
            case '\r': 
                end_fraction_value(chars_format::scientific,ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                push_state(state_);
                state_ = parse_state::cr;
                return; 
            case '\n': 
                end_fraction_value(chars_format::scientific,ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                push_state(state_);
                state_ = parse_state::lf;
                return;   
            case ' ':case '\t':
                end_fraction_value(chars_format::scientific,ec);
                if (ec) return;
                skip_whitespace();
                return;
            case '/': 
                end_fraction_value(chars_format::scientific,ec);
                if (ec) return;
                push_state(state_);
                ++input_ptr_;
                ++column_;
                state_ = parse_state::slash;
                return;
            case '}':
                end_fraction_value(chars_format::scientific,ec);
                if (ec) return;
                end_object(ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                return;
            case ']':
                end_fraction_value(chars_format::scientific,ec);
                if (ec) return;
                end_array(ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                return;
            case ',':
                end_fraction_value(chars_format::scientific,ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++input_ptr_;
                ++column_;
                return;
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                number_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++column_;
                goto exp3;
            default:
                err_handler_.error(json_parse_errc::invalid_number, *this);
                ec = json_parse_errc::invalid_number;
                state_ = parse_state::exp3;
                return;
        }

        JSONCONS_UNREACHABLE();               
    }

    void parse_string(std::error_code& ec)
    {
        const CharT* local_input_end = input_end_;
        const CharT* sb = input_ptr_;

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
        while (input_ptr_ < local_input_end)
        {
            switch (*input_ptr_)
            {
                JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                {
                    column_ += (input_ptr_ - sb + 1);
                    if (err_handler_.error(json_parse_errc::illegal_control_character, *this))
                    {
                        ec = json_parse_errc::illegal_control_character;
                        state_ = parse_state::string_u1;
                        return;
                    }
                    // recovery - skip
                    auto result = unicons::validate(sb,input_ptr_);
                    if (result.ec != unicons::conv_errc())
                    {
                        translate_conv_errc(result.ec,ec);
                        column_ += (result.it - sb);
                        return;
                    }
                    string_buffer_.append(sb,input_ptr_-sb);
                    ++input_ptr_;
                    state_ = parse_state::string_u1;
                    return;
                }
                case '\r':
                {
                    column_ += (input_ptr_ - sb + 1);
                    if (err_handler_.error(json_parse_errc::illegal_character_in_string, *this))
                    {
                        ec = json_parse_errc::illegal_character_in_string;
                        state_ = parse_state::string_u1;
                        return;
                    }
                    // recovery - keep
                    auto result = unicons::validate(sb,input_ptr_);
                    if (result.ec != unicons::conv_errc())
                    {
                        translate_conv_errc(result.ec,ec);
                        column_ += (result.it - sb);
                        return;
                    }
                    string_buffer_.append(sb, input_ptr_ - sb + 1);
                    ++input_ptr_;
                    push_state(state_);
                    state_ = parse_state::cr;
                    return;
                }
                case '\n':
                {
                    column_ += (input_ptr_ - sb + 1);
                    if (err_handler_.error(json_parse_errc::illegal_character_in_string, *this))
                    {
                        ec = json_parse_errc::illegal_character_in_string;
                        state_ = parse_state::string_u1;
                        return;
                    }
                    // recovery - keep
                    auto result = unicons::validate(sb,input_ptr_);
                    if (result.ec != unicons::conv_errc())
                    {
                        translate_conv_errc(result.ec,ec);
                        column_ += (result.it - sb);
                        return;
                    }
                    string_buffer_.append(sb, input_ptr_ - sb + 1);
                    ++input_ptr_;
                    push_state(state_);
                    state_ = parse_state::lf;
                    return;
                }
                case '\t':
                {
                    column_ += (input_ptr_ - sb + 1);
                    if (err_handler_.error(json_parse_errc::illegal_character_in_string, *this))
                    {
                        ec = json_parse_errc::illegal_character_in_string;
                        state_ = parse_state::string_u1;
                        return;
                    }
                    // recovery - keep
                    auto result = unicons::validate(sb,input_ptr_);
                    if (result.ec != unicons::conv_errc())
                    {
                        translate_conv_errc(result.ec,ec);
                        column_ += (result.it - sb);
                        return;
                    }
                    string_buffer_.append(sb, input_ptr_ - sb + 1);
                    ++input_ptr_;
                    state_ = parse_state::string_u1;
                    return;
                }
                case '\\': 
                {
                    auto result = unicons::validate(sb,input_ptr_);
                    if (result.ec != unicons::conv_errc())
                    {
                        translate_conv_errc(result.ec,ec);
                        column_ += (result.it - sb);
                        return;
                    }
                    string_buffer_.append(sb,input_ptr_-sb);
                    column_ += (input_ptr_ - sb + 1);
                    ++input_ptr_;
                    goto escape;
                }
                case '\"':
                {
                    auto result = unicons::validate(sb,input_ptr_);
                    if (result.ec != unicons::conv_errc())
                    {
                        translate_conv_errc(result.ec,ec);
                        column_ += (result.it - sb);
                        return;
                    }
                    if (string_buffer_.length() == 0)
                    {
                        end_string_value(sb,input_ptr_-sb, ec);
                        if (ec) {return;}
                    }
                    else
                    {
                        string_buffer_.append(sb,input_ptr_-sb);
                        end_string_value(string_buffer_.data(),string_buffer_.length(), ec);
                        string_buffer_.clear();
                        if (ec) {return;}
                    }
                    column_ += (input_ptr_ - sb + 1);
                    ++input_ptr_;
                    return;
                }
            default:
                break;
            }
            ++input_ptr_;
        }

        // Buffer exhausted               
        {
            auto result = unicons::validate(sb,input_ptr_);
            if (result.ec != unicons::conv_errc())
            {
                translate_conv_errc(result.ec,ec);
                column_ += (result.it - sb);
                return;
            }
            string_buffer_.append(sb,input_ptr_-sb);
            column_ += (input_ptr_ - sb + 1);
            state_ = parse_state::string_u1;
            return;
        }

escape:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::escape;
            return;
        }
        switch (*input_ptr_)
        {
        case '\"':
            string_buffer_.push_back('\"');
            sb = ++input_ptr_;
            ++column_;
            goto string_u1;
        case '\\': 
            string_buffer_.push_back('\\');
            sb = ++input_ptr_;
            ++column_;
            goto string_u1;
        case '/':
            string_buffer_.push_back('/');
            sb = ++input_ptr_;
            ++column_;
            goto string_u1;
        case 'b':
            string_buffer_.push_back('\b');
            sb = ++input_ptr_;
            ++column_;
            goto string_u1;
        case 'f':
            string_buffer_.push_back('\f');
            sb = ++input_ptr_;
            ++column_;
            goto string_u1;
        case 'n':
            string_buffer_.push_back('\n');
            sb = ++input_ptr_;
            ++column_;
            goto string_u1;
        case 'r':
            string_buffer_.push_back('\r');
            sb = ++input_ptr_;
            ++column_;
            goto string_u1;
        case 't':
            string_buffer_.push_back('\t');
            sb = ++input_ptr_;
            ++column_;
            goto string_u1;
        case 'u':
            cp_ = 0;
            ++input_ptr_;
            ++column_;
            goto escape_u1;
        default:    
            err_handler_.error(json_parse_errc::illegal_escaped_character, *this);
            ec = json_parse_errc::illegal_escaped_character;
            state_ = parse_state::escape;
            return;
        }

escape_u1:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u1;
            return;
        }
        {
            append_codepoint(*input_ptr_,ec);
            if (ec)
            {
                state_ = parse_state::escape_u1;
                return;
            }
            ++input_ptr_;
            ++column_;
            goto escape_u2;
        }

escape_u2:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u2;
            return;
        }
        {
            append_codepoint(*input_ptr_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u2;
                return;
            }
            ++input_ptr_;
            ++column_;
            goto escape_u3;
        }

escape_u3:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u3;
            return;
        }
        {
            append_codepoint(*input_ptr_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u3;
                return;
            }
            ++input_ptr_;
            ++column_;
            goto escape_u4;
        }

escape_u4:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u4;
            return;
        }
        {
            append_codepoint(*input_ptr_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u4;
                return;
            }
            if (unicons::is_high_surrogate(cp_))
            {
                ++input_ptr_;
                ++column_;
                goto escape_expect_surrogate_pair1;
            }
            else
            {
                unicons::convert(&cp_, &cp_ + 1, std::back_inserter(string_buffer_));
                sb = ++input_ptr_;
                ++column_;
                state_ = parse_state::string_u1;
                return;
            }
        }

escape_expect_surrogate_pair1:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::escape_expect_surrogate_pair1;
            return;
        }
        {
            switch (*input_ptr_)
            {
            case '\\': 
                cp2_ = 0;
                ++input_ptr_;
                ++column_;
                goto escape_expect_surrogate_pair2;
            default:
                err_handler_.error(json_parse_errc::expected_codepoint_surrogate_pair, *this);
                ec = json_parse_errc::expected_codepoint_surrogate_pair;
                state_ = parse_state::escape_expect_surrogate_pair1;
                return;
            }
        }

escape_expect_surrogate_pair2:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::escape_expect_surrogate_pair2;
            return;
        }
        {
            switch (*input_ptr_)
            {
            case 'u':
                ++input_ptr_;
                ++column_;
                goto escape_u6;
            default:
                err_handler_.error(json_parse_errc::expected_codepoint_surrogate_pair, *this);
                ec = json_parse_errc::expected_codepoint_surrogate_pair;
                state_ = parse_state::escape_expect_surrogate_pair2;
                return;
            }
        }

escape_u6:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u6;
            return;
        }
        {
            append_second_codepoint(*input_ptr_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u6;
                return;
            }
        }
        ++input_ptr_;
        ++column_;
        goto escape_u7;

escape_u7:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u7;
            return;
        }
        {
            append_second_codepoint(*input_ptr_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u7;
                return;
            }
            ++input_ptr_;
            ++column_;
            goto escape_u8;
        }

escape_u8:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u8;
            return;
        }
        {
            append_second_codepoint(*input_ptr_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u8;
                return;
            }
            ++input_ptr_;
            ++column_;
            goto escape_u9;
        }

escape_u9:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            state_ = parse_state::escape_u9;
            return;
        }
        {
            append_second_codepoint(*input_ptr_, ec);
            if (ec)
            {
                state_ = parse_state::escape_u9;
                return;
            }
            uint32_t cp = 0x10000 + ((cp_ & 0x3FF) << 10) + (cp2_ & 0x3FF);
            unicons::convert(&cp, &cp + 1, std::back_inserter(string_buffer_));
            sb = ++input_ptr_;
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
            if (err_handler_.error(json_parse_errc::over_long_utf8_sequence, *this))
            {
                ec = json_parse_errc::over_long_utf8_sequence;
                return;
            }
            break;
        case unicons::conv_errc::unpaired_high_surrogate:
            if (err_handler_.error(json_parse_errc::unpaired_high_surrogate, *this))
            {
                ec = json_parse_errc::unpaired_high_surrogate;
                return;
            }
            break;
        case unicons::conv_errc::expected_continuation_byte:
            if (err_handler_.error(json_parse_errc::expected_continuation_byte, *this))
            {
                ec = json_parse_errc::expected_continuation_byte;
                return;
            }
            break;
        case unicons::conv_errc::illegal_surrogate_value:
            if (err_handler_.error(json_parse_errc::illegal_surrogate_value, *this))
            {
                ec = json_parse_errc::illegal_surrogate_value;
                return;
            }
            break;
        default:
            if (err_handler_.error(json_parse_errc::illegal_codepoint, *this))
            {
                ec = json_parse_errc::illegal_codepoint;
                return;
            }
            break;
        }
    }

    void parse_some()
    {
        std::error_code ec;
        parse_some(ec);
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
            case parse_state::zero:  
            case parse_state::integer:
                end_integer_value(ec);
                if (ec) return;
                break;
            case parse_state::fraction2:
                end_fraction_value(chars_format::fixed,ec);
                if (ec) return;
                break;
            case parse_state::exp3:
                end_fraction_value(chars_format::scientific,ec);
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
            if (err_handler_.error(json_parse_errc::unexpected_eof, *this))
            {
                ec = json_parse_errc::unexpected_eof;
                return;
            }
        }
    }

    parse_state state() const
    {
        return state_;
    }

    void update(const string_view_type sv)
    {
        update(sv.data(),sv.length());
    }

    void update(const CharT* data, size_t length)
    {
        begin_input_ = data;
        input_end_ = data + length;
        input_ptr_ = begin_input_;
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    void set_source(const CharT* data, size_t length)
    {
        begin_input_ = data;
        input_end_ = data + length;
        input_ptr_ = begin_input_;
    }
#endif
private:

    void end_integer_value(std::error_code& ec)
    {
        if (number_buffer_[0] == '-')
        {
            end_negative_value(ec);
        }
        else
        {
            end_positive_value(ec);
        }
    }

    void end_negative_value(std::error_code& ec)
    {
        jsoncons::detail::to_integer_result result = jsoncons::detail::to_integer(number_buffer_.data(), number_buffer_.length());
        if (!result.overflow)
        {
            handler_.integer_value(result.value, *this);
            after_value(ec);
        }
        else
        {
            byte_buffer_.clear();

            int signum = 0;
            bignum n(number_buffer_.c_str());
            n.dump(signum, byte_buffer_);
            handler_.bignum_value(signum, byte_buffer_.data(), byte_buffer_.size(), *this);
            after_value(ec);
        }
    }

    void end_positive_value(std::error_code& ec)
    {
        jsoncons::detail::to_uinteger_result result = jsoncons::detail::to_uinteger(number_buffer_.data(), number_buffer_.length());
        if (!result.overflow)
        {
            handler_.uinteger_value(result.value, *this);
            after_value(ec);
        }
        else
        {
            byte_buffer_.clear();

            int signum = 0;
            bignum n(number_buffer_.c_str());
            n.dump(signum, byte_buffer_);
            handler_.bignum_value(1, byte_buffer_.data(), byte_buffer_.size(), *this);
            after_value(ec);
        }
    }

    void end_fraction_value(chars_format format, std::error_code& ec)
    {
        try
        {
            double d = to_double_(number_buffer_.c_str(), number_buffer_.length());

            if (precision_ > std::numeric_limits<double>::max_digits10)
            {
                handler_.double_value(d, floating_point_options(format,std::numeric_limits<double>::max_digits10, decimal_places_), *this);
            }
            else
            {
                handler_.double_value(d, floating_point_options(format,static_cast<uint8_t>(precision_), decimal_places_), *this);
            }
        }
        catch (...)
        {
            if (err_handler_.error(json_parse_errc::invalid_number, *this))
            {
                ec = json_parse_errc::invalid_number;
                return;
            }
            handler_.null_value(*this); // recovery
        }

        after_value(ec);
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
            if (err_handler_.error(json_parse_errc::expected_value, *this))
            {
                ec = json_parse_errc::expected_value;
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
            if (err_handler_.error(json_parse_errc::expected_value, *this))
            {
                ec = json_parse_errc::expected_value;
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
            handler_.end_document();
            break;
        default:
            if (err_handler_.error(json_parse_errc::invalid_json_text, *this))
            {
                ec = json_parse_errc::invalid_json_text;
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
            if (err_handler_.error(json_parse_errc::invalid_json_text, *this))
            {
                ec = json_parse_errc::invalid_json_text;
                return;
            }
            break;
        }
    }

    void after_value(std::error_code& ec) 
    {
        switch (parent())
        {
        case parse_state::array:
        case parse_state::object:
            state_ = parse_state::expect_comma_or_end;
            break;
        case parse_state::root:
            state_ = parse_state::done;
            handler_.end_document();
            break;
        default:
            if (err_handler_.error(json_parse_errc::invalid_json_text, *this))
            {
                ec = json_parse_errc::invalid_json_text;
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
            if (err_handler_.error(json_parse_errc::invalid_hex_escape_sequence, *this))
            {
                ec = json_parse_errc::invalid_hex_escape_sequence;
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


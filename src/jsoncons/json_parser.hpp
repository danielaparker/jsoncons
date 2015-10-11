// Copyright 2015 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

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
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/parse_error_handler.hpp"
#include "jsoncons/json_text_error_category.hpp"

namespace jsoncons {

namespace modes {
    enum modes_t {
        array_element,
        object_member_name,
        object_member_value,
        done
    };
};

namespace states {
    enum states_t {
        start, 
        slash,  
        slash_slash, 
        slash_star, 
        slash_star_star,
        expect_comma_or_end,  
        object,
        expect_member_name, 
        expect_colon,
        expect_value,
        array, 
        string,
        escape, 
        u1, 
        u2, 
        u3, 
        u4, 
        expect_surrogate_pair1, 
        expect_surrogate_pair2, 
        u6, 
        u7, 
        u8, 
        u9, 
        minus, 
        zero,  
        integer,
        fraction,
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
        done
    };
};

template<typename Char>
class basic_json_parser : private basic_parsing_context<Char>
{
    static const int default_depth = 100;

public:
    basic_json_parser(basic_json_input_handler<Char>& handler)
       : top_(-1),
         stack_(default_depth),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(default_basic_parse_error_handler<Char>::instance())),
         is_negative_(false),
         cp_(0),
         index_(0)

    {
        depth_ = default_depth;
        state_ = states::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        prev_char_ = 0;
        max_depth_ = std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
    }

    basic_parsing_context<Char> const & parsing_context() const
    {
        return *this;
    }

    basic_json_parser(basic_json_input_handler<Char>& handler,
                      basic_parse_error_handler<Char>& err_handler)
       : top_(-1),
         stack_(default_depth),
         handler_(std::addressof(handler)),
         err_handler_(std::addressof(err_handler)),
         is_negative_(false),
         cp_(0),
         index_(0)

    {
        depth_ = default_depth;
        state_ = states::start;
        top_ = -1;
        line_ = 1;
        column_ = 0;
        prev_char_ = 0;
        max_depth_ = std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP();
    }

    ~basic_json_parser()
    {
    }

    size_t max_depth() const
    {
        return static_cast<size_t>(max_depth_);
    }

    void max_depth(size_t max_depth)
    {
        max_depth_ = static_cast<int>(std::min(max_depth,static_cast<size_t>(std::numeric_limits<int>::max JSONCONS_NO_MACRO_EXP())));
        if (depth_ > max_depth_)
        {
            depth_ = max_depth_;
            stack_.resize(depth_);
        }
    }

    bool done() const
    {
        return state_ == states::done;
    }

    void begin_parse()
    {
        if (!push(modes::done))
        {
            err_handler_->error(std::error_code(json_parser_errc::max_depth_exceeded, json_text_error_category()), *this);
        }
        state_ = states::start;
        prev_char_ = 0;
        line_ = 1;
        column_ = 1;
    }

    void check_done(Char const* p, size_t start, size_t length)
    {
        index_ = start;
        for (; index_ < length; ++index_)
        {
            int curr_char = p[index_];
            switch (curr_char)
            {
            case '\n':
            case '\r':
            case '\t':
            case ' ':
                break;
            default:
                err_handler_->error(std::error_code(json_parser_errc::extra_character, json_text_error_category()), *this);
                break;
            }
        }
    }

    void parse(Char const* p, size_t start, size_t length)
    {
        index_ = start;
        for (; (index_ < length) & (state_ != states::done); ++index_)
        {
            int curr_char = p[index_];
            switch (curr_char)
            {
            case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x08:case 0x0b:
            case 0x0c:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:case 0x15:case 0x16:
            case 0x17:case 0x18:case 0x19:case 0x1a:case 0x1b:case 0x1c:case 0x1d:case 0x1e:case 0x1f:
                err_handler_->error(std::error_code(json_parser_errc::illegal_control_character, json_text_error_category()), *this);
                break;
            }

            switch (state_)
            {
            case states::start: 
                {
                    switch (curr_char)
                    {
                    case ' ':case '\n':case '\r':case '\t':
                        break; 
                    case '{':
                        handler_->begin_json();
                        if (!push(modes::object_member_name))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::max_depth_exceeded, json_text_error_category()), *this);
                        }
                        state_ = states::object;
                        handler_->begin_object(*this);
                        break;
                    case '[':
                        handler_->begin_json();
                        if (!push(modes::array_element))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::max_depth_exceeded, json_text_error_category()), *this);
                        }
                        state_ = states::array;
                        handler_->begin_array(*this);
                        break;
                    case '/':
                        saved_state_ = state_;
                        state_ = states::slash;
                        break;
                    case '}':
                        err_handler_->error(std::error_code(json_parser_errc::unexpected_end_of_object, json_text_error_category()), *this);
                        break;
                    case ']':
                        err_handler_->error(std::error_code(json_parser_errc::unexpected_end_of_array, json_text_error_category()), *this);
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
                        break;
                    }
                }
                break;

            case states::expect_comma_or_end: 
                {
                    switch (curr_char)
                    {
                    case ' ':case '\n':case '\r':case '\t':
                        break; 
                    case '}':
                        if (!pop(modes::object_member_value))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::mismatched_parentheses_or_brackets, json_text_error_category()), *this);
                        }
                        handler_->end_object(*this);
                        if (peek() == modes::done)
                        {
                            state_ = states::done;
                            handler_->end_json();
                        }
                        else
                        {
                            state_ = states::expect_comma_or_end;
                        }
                        break;
                    case ']':
                        if (!pop(modes::array_element))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::mismatched_parentheses_or_brackets, json_text_error_category()), *this);
                        }
                        handler_->end_array(*this);
                        if (peek() == modes::done)
                        {
                            state_ = states::done;
                            handler_->end_json();
                        }
                        else
                        {
                            state_ = states::expect_comma_or_end;
                        }
                        break;
                    case ',':
                        begin_member_or_element();
                        break;
                    case '/':
                        saved_state_ = state_;
                        state_ = states::slash;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_comma_or_end, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::object: 
                {
                    switch (curr_char)
                    {
                    case ' ':case '\n':case '\r':case '\t':
                        break;
                    case '}':
                        if (!pop(modes::object_member_name))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
                        }
                        handler_->end_object(*this);
                        if (peek() == modes::done)
                        {
                            state_ = states::done;
                            handler_->end_json();
                        }
                        else
                        {
                            state_ = states::expect_comma_or_end;
                        }
                        break;
                    case '\"':
                        state_ = states::string;
                        break;
                    case '/':
                        saved_state_ = state_;
                        state_ = states::slash;
                        break;
                    case '\'':
                        err_handler_->error(std::error_code(json_parser_errc::single_quote, json_text_error_category()), *this);
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_name, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::expect_member_name: 
                {
                    switch (curr_char)
                    {
                    case ' ':case '\n':case '\r':case '\t':
                        break;
                    case '\"':
                        state_ = states::string;
                        break;
                    case '/':
                        saved_state_ = state_;
                        state_ = states::slash;
                        break;
                    case '}':
                        err_handler_->error(std::error_code(json_parser_errc::extra_comma, json_text_error_category()), *this);
                        break;
                    case '\'':
                        err_handler_->error(std::error_code(json_parser_errc::single_quote, json_text_error_category()), *this);
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_name, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::expect_colon: 
                {
                    switch (curr_char)
                    {
                    case ' ':case '\n':case '\r':case '\t':
                        break;
                    case ':':
                        begin_member_value();
                        state_ = states::expect_value;
                        break;
                    case '/':
                        saved_state_ = state_;
                        state_ = states::slash;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_colon, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::expect_value: 
                {
                    switch (curr_char)
                    {
                    case ' ':case '\n':case '\r':case '\t':
                        break;
                    case '{':
                        if (!push(modes::object_member_name))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::max_depth_exceeded, json_text_error_category()), *this);
                        }
                        state_ = states::object;
                        handler_->begin_object(*this);
                        break;
                    case '[':
                        if (!push(modes::array_element))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::max_depth_exceeded, json_text_error_category()), *this);
                        }
                        state_ = states::array;
                        handler_->begin_array(*this);
                        break;
                    case '\"':
                        state_ = states::string;
                        break;
                    case '/':
                        saved_state_ = state_;
                        state_ = states::slash;
                        break;
                    case '-':
                        is_negative_ = true;
                        state_ = states::minus;
                        break;
                    case '0': 
                        string_buffer_.push_back(curr_char);
                        state_ = states::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(curr_char);
                        state_ = states::integer;
                        break;
                    case 'f':
                        state_ = states::f;
                        if ((index_+4) < length)
                        {
                            if ((p[index_+1] == 'a') & (p[index_+2] == 'l') & (p[index_+3] == 's') & (p[index_+4] == 'e'))
                            {
                                index_ += 4;
                                column_ += 4;
                                handler_->value(false, *this);
                                state_ = states::expect_comma_or_end;
                            }
                        }
                        break;
                    case 'n':
                        state_ = states::n;
                        if ((index_+3) < length)
                        {
                            if ((p[index_+1] == 'u') & (p[index_+2] == 'l') & (p[index_+3] == 'l'))
                            {
                                index_ += 3;
                                column_ += 3;
                                handler_->value(null_type(), *this);
                                state_ = states::expect_comma_or_end;
                            }
                        }
                        break;
                    case 't':
                        state_ = states::t;
                        if ((index_+3) < length)
                        {
                            if ((p[index_+1] == 'r') & (p[index_+2] == 'u') & (p[index_+3] == 'e'))
                            {
                                index_ += 3;
                                column_ += 3;
                                handler_->value(true, *this);
                                state_ = states::expect_comma_or_end;
                            }
                        }
                        break;
                    case ']':
                        if (peek() == modes::array_element)
                        {
                            err_handler_->error(std::error_code(json_parser_errc::extra_comma, json_text_error_category()), *this);
                        }
                        else
                        {
                            err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        }
                        break;
                    case '\'':
                        err_handler_->error(std::error_code(json_parser_errc::single_quote, json_text_error_category()), *this);
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::array: 
                {
                    switch (curr_char)
                    {
                    case ' ':case '\n':case '\r':case '\t':
                        break;
                    case '{':
                        if (!push(modes::object_member_name))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::max_depth_exceeded, json_text_error_category()), *this);
                        }
                        state_ = states::object;
                        handler_->begin_object(*this);
                        break;
                    case '[':
                        if (!push(modes::array_element))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::max_depth_exceeded, json_text_error_category()), *this);
                        }
                        state_ = states::array;
                        handler_->begin_array(*this);
                        break;
                    case ']':
                        if (!pop(modes::array_element))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
                        }
                        handler_->end_array(*this);
                        if (peek() == modes::done)
                        {
                            state_ = states::done;
                            handler_->end_json();
                        }
                        else
                        {
                            state_ = states::expect_comma_or_end;
                        }
                        break;
                    case '\"':
                        state_ = states::string;
                        break;
                    case '/':
                        saved_state_ = state_;
                        state_ = states::slash;
                        break;
                    case '-':
                        is_negative_ = true;
                        state_ = states::minus;
                        break;
                    case '0': 
                        string_buffer_.push_back(curr_char);
                        state_ = states::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(curr_char);
                        state_ = states::integer;
                        break;
                    case 'f':
                        state_ = states::f;
                        if ((index_+4) < length)
                        {
                            if ((p[index_+1] == 'a') & (p[index_+2] == 'l') & (p[index_+3] == 's') & (p[index_+4] == 'e'))
                            {
                                index_ += 4;
                                column_ += 4;
                                handler_->value(false, *this);
                                state_ = states::expect_comma_or_end;
                            }
                        }
                        break;
                    case 'n':
                        state_ = states::n;
                        if ((index_+3) < length)
                        {
                            if ((p[index_+1] == 'u') & (p[index_+2] == 'l') & (p[index_+3] == 'l'))
                            {
                                index_ += 3;
                                column_ += 3;
                                handler_->value(null_type(), *this);
                                state_ = states::expect_comma_or_end;
                            }
                        }
                        break;
                    case 't':
                        state_ = states::t;
                        if ((index_+3) < length)
                        {
                            if ((p[index_+1] == 'r') & (p[index_+2] == 'u') & (p[index_+3] == 'e'))
                            {
                                index_ += 3;
                                column_ += 3;
                                handler_->value(true, *this);
                                state_ = states::expect_comma_or_end;
                            }
                        }
                        break;
                    case '\'':
                        err_handler_->error(std::error_code(json_parser_errc::single_quote, json_text_error_category()), *this);
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::string: 
                {
                    switch (curr_char)
                    {
                    case '\n':
                    case '\r':
                    case '\t':
                        err_handler_->error(std::error_code(json_parser_errc::illegal_character_in_string, json_text_error_category()), *this);
                        break;
                    case '\\': 
                        state_ = states::escape;
                        break;
                    case '\"':
                        end_string_value();
                        break;
                    default:
                        string_buffer_.push_back(curr_char);
                        break;
                    }
                }
                break;
            case states::escape: 
                {
                    escape_next_char(curr_char);
                }
                break;
            case states::u1: 
                {
                    append_codepoint(curr_char);
                    state_ = states::u2;
                }
                break;
            case states::u2: 
                {
                    append_codepoint(curr_char);
                    state_ = states::u3;
                }
                break;
            case states::u3: 
                {
                    append_codepoint(curr_char);
                    state_ = states::u4;
                }
                break;
            case states::u4: 
                {
                    append_codepoint(curr_char);
                    if (cp_ >= min_lead_surrogate && cp_ <= max_lead_surrogate)
                    {
                        state_ = states::expect_surrogate_pair1;
                    }
                    else
                    {
                        json_char_traits<Char, sizeof(Char)>::append_codepoint_to_string(cp_, string_buffer_);
                        state_ = states::string;
                    }
                }
                break;
            case states::expect_surrogate_pair1: 
                {
                    switch (curr_char)
                    {
                    case '\\': 
                        cp2_ = 0;
                        state_ = states::expect_surrogate_pair2;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_codepoint_surrogate_pair, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::expect_surrogate_pair2: 
                {
                    switch (curr_char)
                    {
                    case 'u':
                        state_ = states::u6;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_codepoint_surrogate_pair, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::u6:
                {
                    append_second_codepoint(curr_char);
                    state_ = states::u7;
                }
                break;
            case states::u7: 
                {
                    append_second_codepoint(curr_char);
                    state_ = states::u8;
                }
                break;
            case states::u8: 
                {
                    append_second_codepoint(curr_char);
                    state_ = states::u9;
                }
                break;
            case states::u9: 
				{
                    append_second_codepoint(curr_char);
                    uint32_t cp = 0x10000 + ((cp_ & 0x3FF) << 10) + (cp2_ & 0x3FF);
                    json_char_traits<Char, sizeof(Char)>::append_codepoint_to_string(cp, string_buffer_);
                    state_ = states::string;
				}
                break;
            case states::minus:  
                {
                    switch (curr_char)
                    {
                    case '0': 
                        string_buffer_.push_back(curr_char);
                        state_ = states::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(curr_char);
                        state_ = states::integer;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::zero:  
                {
                    switch (curr_char)
                    {
                    case ' ':case '\n':case '\r':case '\t':
                        end_integer_value();
                        state_ = states::expect_comma_or_end;
                        break; // No change
                    case '}':
                        end_integer_value();
                        if (!pop(modes::object_member_value))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
                        }
                        handler_->end_object(*this);
                        if (peek() == modes::done)
                        {
                            state_ = states::done;
                            handler_->end_json();
                        }
                        else
                        {
                            state_ = states::expect_comma_or_end;
                        }
                        break;
                    case ']':
                        end_integer_value();
                        if (!pop(modes::array_element))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
                        }
                        handler_->end_array(*this);
                        if (peek() == modes::done)
                        {
                            state_ = states::done;
                            handler_->end_json();
                        }
                        else
                        {
                            state_ = states::expect_comma_or_end;
                        }
                        break;
                    case '.':
                        string_buffer_.push_back(curr_char);
                        state_ = states::fraction;
                        break;
                    case ',':
                        end_integer_value();
                        begin_member_or_element();
                        break;
                    case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        err_handler_->error(std::error_code(json_parser_errc::leading_zero, json_text_error_category()), *this);
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::integer: 
                {
                    switch (curr_char)
                    {
                    case ' ':case '\n':case '\r':case '\t':
                        end_integer_value();
                        state_ = states::expect_comma_or_end;
                        break; 
                    case '}':
                        end_integer_value();
                        if (!pop(modes::object_member_value))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
                        }
                        handler_->end_object(*this);
                        if (peek() == modes::done)
                        {
                            state_ = states::done;
                            handler_->end_json();
                        }
                        else
                        {
                            state_ = states::expect_comma_or_end;
                        }
                        break;
                    case ']':
                        end_integer_value();
                        if (!pop(modes::array_element))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
                        }
                        handler_->end_array(*this);
                        if (peek() == modes::done)
                        {
                            state_ = states::done;
                            handler_->end_json();
                        }
                        else
                        {
                            state_ = states::expect_comma_or_end;
                        }
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(curr_char);
                        state_ = states::integer;
                        break;
                    case '.':
                        string_buffer_.push_back(curr_char);
                        state_ = states::fraction;
                        break;
                    case ',':
                        end_integer_value();
                        begin_member_or_element();
                        break;
                    case 'e':case 'E':
                        string_buffer_.push_back(curr_char);
                        state_ = states::exp1;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::fraction: 
                {
                    switch (curr_char)
                    {
                    case ' ':case '\n':case '\r':case '\t':
                        end_fraction_value();
                        state_ = states::expect_comma_or_end;
                        break; 
                    case '}':
                        end_fraction_value();
                        if (!pop(modes::object_member_value))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
                        }
                        handler_->end_object(*this);
                        if (peek() == modes::done)
                        {
                            state_ = states::done;
                            handler_->end_json();
                        }
                        else
                        {
                            state_ = states::expect_comma_or_end;
                        }
                        break;
                    case ']':
                        end_fraction_value();
                        if (!pop(modes::array_element))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
                        }
                        handler_->end_array(*this);
                        if (peek() == modes::done)
                        {
                            state_ = states::done;
                            handler_->end_json();
                        }
                        else
                        {
                            state_ = states::expect_comma_or_end;
                        }
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(curr_char);
                        state_ = states::fraction;
                        break;
                    case ',':
                        end_fraction_value();
                        begin_member_or_element();
                        break;
                    case 'e':case 'E':
                        string_buffer_.push_back(curr_char);
                        state_ = states::exp1;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::exp1: 
                {
                    switch (curr_char)
                    {
                    case '+':
                        state_ = states::exp2;
                        break;
                    case '-':
                        string_buffer_.push_back(curr_char);
                        state_ = states::exp2;
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(curr_char);
                        state_ = states::exp3;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::exp2:  
                {
                    switch (curr_char)
                    {
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(curr_char);
                        state_ = states::exp3;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::exp3: 
                {
                    switch (curr_char)
                    {
                    case ' ':case '\n':case '\r':case '\t':
                        end_fraction_value();
                        state_ = states::expect_comma_or_end;
                        break; 
                    case '}':
                        end_fraction_value();
                        if (!pop(modes::object_member_value))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
                        }
                        handler_->end_object(*this);
                        if (peek() == modes::done)
                        {
                            state_ = states::done;
                            handler_->end_json();
                        }
                        else
                        {
                            state_ = states::expect_comma_or_end;
                        }
                        break;
                    case ']':
                        end_fraction_value();
                        if (!pop(modes::array_element))
                        {
                            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
                        }
                        handler_->end_array(*this);
                        if (peek() == modes::done)
                        {
                            state_ = states::done;
                            handler_->end_json();
                        }
                        else
                        {
                            state_ = states::expect_comma_or_end;
                        }
                        break;
                    case ',':
                        end_fraction_value();
                        begin_member_or_element();
                        break;
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        string_buffer_.push_back(curr_char);
                        state_ = states::exp3;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::t: 
                {
                    switch (curr_char)
                    {
                    case 'r':
                        state_ = states::tr;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::tr: 
                {
                    switch (curr_char)
                    {
                    case 'u':
                        state_ = states::tru;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::tru:  
                {
                    switch (curr_char)
                    {
                    case 'e': 
                        handler_->value(true, *this);
                        state_ = states::expect_comma_or_end;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::f:  
                {
                    switch (curr_char)
                    {
                    case 'a':
                        state_ = states::fa;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::fa: 
                {
                    switch (curr_char)
                    {
                    case 'l': 
                        state_ = states::fal;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::fal:  
                {
                    switch (curr_char)
                    {
                    case 's':
                        state_ = states::fals;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::fals:  
                {
                    switch (curr_char)
                    {
                    case 'e':
                        handler_->value(false, *this);
                        state_ = states::expect_comma_or_end;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::n: 
                {
                    switch (curr_char)
                    {
                    case 'u':
                        state_ = states::nu;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::nu:  
                {
                    switch (curr_char)
                    {
                    case 'l': 
                        state_ = states::nul;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::nul:  
                {
                    switch (curr_char)
                    {
                    case 'l': 
                        handler_->value(null_type(), *this);
                        state_ = states::expect_comma_or_end;
                        break;
                    default:
                        err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::slash: 
                {
                    switch (curr_char)
                    {
                    case '*':
                        state_ = states::slash_star;
                        break;
                    case '/':
                        state_ = states::slash_slash;
                        break;
                    default:    
                        err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
                        break;
                    }
                }
                break;
            case states::slash_star:  
                {
                    switch (curr_char)
                    {
                    case '*':
                        state_ = states::slash_star_star;
                        break;
                    }
                }
                break;
            case states::slash_slash: 
                {
                    switch (curr_char)
                    {
                    case '\n':
                    case '\r':
                        state_ = saved_state_;
                        break;
                    }
                }
                break;
            case states::slash_star_star: 
                {
                    switch (curr_char)
                    {
                    case '/':
                        state_ = saved_state_;
                        break;
                    default:    
                        state_ = states::slash_star;
                        break;
                    }
                }
                break;
            }
            switch (curr_char)
            {
            case '\r':
                ++line_;
                column_ = 1;
                break;
            case '\n':
                if (prev_char_ != '\r')
                {
                    ++line_;
                }
                column_ = 1;
                break;
            default:
                ++column_;
                break;
            }
            prev_char_ = curr_char;
        }
    }

    void end_parse()
    {
        if (!pop(modes::done))
        {
            err_handler_->error(std::error_code(json_parser_errc::unexpected_eof, json_text_error_category()), *this);
        }
    }

    states::states_t state() const
    {
        return state_;
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
            double d = string_to_float(string_buffer_);
            if (is_negative_)
                d = -d;
            handler_->value(d, *this);
        }
        catch (...)
        {
            err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_text_error_category()), *this);
            handler_->value(null_type(), *this); // recovery
        }
        string_buffer_.clear();
        is_negative_ = false;
        state_ = states::expect_comma_or_end;
    }

    void end_integer_value()
    {
        if (is_negative_)
        {
            try
            {
                long long d = string_to_integer(is_negative_, string_buffer_.c_str(), string_buffer_.length());
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
                    err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_text_error_category()), *this);
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
                    err_handler_->error(std::error_code(json_parser_errc::invalid_number, json_text_error_category()), *this);
                    handler_->value(null_type(), *this);
                }
            }
        }
        string_buffer_.clear();
        is_negative_ = false;
        state_ = states::expect_comma_or_end;
    }

    void append_codepoint(int curr_char)
    {
        switch (curr_char)
        {
        case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
        case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
        case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
            cp_ = append_to_codepoint(cp_, curr_char);
            break;
        default:
            err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
            break;
        }
    }

    void append_second_codepoint(int curr_char)
    {
        switch (curr_char)
        {
        case '0': 
        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
        case 'a':case 'b':case 'c':case 'd':case 'e':case 'f':
        case 'A':case 'B':case 'C':case 'D':case 'E':case 'F':
            cp2_ = append_to_codepoint(cp2_, curr_char);
            break;
        default:
            err_handler_->error(std::error_code(json_parser_errc::expected_value, json_text_error_category()), *this);
            break;
        }
    }

    void escape_next_char(int next_input)
    {
        switch (next_input)
        {
        case '\"':
            string_buffer_.push_back('\"');
            state_ = states::string;
            break;
        case '\\': 
            string_buffer_.push_back('\\');
            state_ = states::string;
            break;
        case '/':
            string_buffer_.push_back('/');
            state_ = states::string;
            break;
        case 'b':
            string_buffer_.push_back('\b');
            state_ = states::string;
            break;
        case 'f':  
            string_buffer_.push_back('\f');
            state_ = states::string;
            break;
        case 'n':
            string_buffer_.push_back('\n');
            state_ = states::string;
            break;
        case 'r':
            string_buffer_.push_back('\r');
            state_ = states::string;
            break;
        case 't':
            string_buffer_.push_back('\t');
            state_ = states::string;
            break;
        case 'u':
            cp_ = 0;
            state_ = states::u1;
            break;
        default:    
            err_handler_->error(std::error_code(json_parser_errc::illegal_escaped_character, json_text_error_category()), *this);
            break;
        }
    }

    void end_string_value() 
    {
        switch (stack_[top_])
        {
        case modes::object_member_name:
            handler_->name(string_buffer_.c_str(), string_buffer_.length(), *this);
            state_ = states::expect_colon;
            break;
        case modes::array_element:
        case modes::object_member_value:
            handler_->value(string_buffer_.c_str(), string_buffer_.length(), *this);
            state_ = states::expect_comma_or_end;
            break;
        default:
            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
            break;
        }
        string_buffer_.clear();
    }

    void begin_member_or_element() 
    {
        switch (stack_[top_])
        {
        case modes::object_member_value:
            // A comma causes a flip from object_member_value modes to object_member_name modes.
            if (!flip(modes::object_member_value, modes::object_member_name))
            {
                err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
            }
            state_ = states::expect_member_name;
            break;
        case modes::array_element:
            state_ = states::expect_value;
            break;
        default:
            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
            break;
        }
    }

    void begin_member_value()
    {
        if (!flip(modes::object_member_name, modes::object_member_value))
        {
            err_handler_->error(std::error_code(json_parser_errc::invalid_json_text, json_text_error_category()), *this);
        }
        state_ = states::expect_value;
    }
 
    uint32_t append_to_codepoint(uint32_t cp, int curr_char)
    {
        cp *= 16;
        if (curr_char >= '0'  &&  curr_char <= '9')
        {
            cp += curr_char - '0';
        }
        else if (curr_char >= 'a'  &&  curr_char <= 'f')
        {
            cp += curr_char - 'a' + 10;
        }
        else if (curr_char >= 'A'  &&  curr_char <= 'F')
        {
            cp += curr_char - 'A' + 10;
        }
        else
        {
            err_handler_->error(std::error_code(json_parser_errc::invalid_hex_escape_sequence, json_text_error_category()), *this);
        }
        return cp;
    }

    unsigned long do_line_number() const override
    {
        return line_;
    }

    unsigned long do_column_number() const override
    {
        return column_;
    }

    Char do_last_char() const override
    {
        return c_;
    }

    bool push(modes::modes_t modes)
    {
        ++top_;
        if (top_ >= depth_)
        {
            if (top_ >= max_depth_)
            {
                return false;
            }
            depth_ *= 2;
            stack_.resize(depth_);
        }
        stack_[top_] = modes;
        return true;
    }

    int peek()
    {
        return stack_[top_];
    }

    bool peek(modes::modes_t modes)
    {
        return stack_[top_] == modes;
    }

    bool flip(modes::modes_t mode1, modes::modes_t mode2)
    {
        if (top_ < 0 || stack_[top_] != mode1)
        {
            return false;
        }
        stack_[top_] = mode2;
        return true;
    }

    bool pop(modes::modes_t modes)
    {
        if (top_ < 0 || stack_[top_] != modes)
        {
            return false;
        }
        --top_;
        return true;
    }

    template<typename CharType>
    unsigned long long string_to_unsigned(const CharType *s, size_t length) throw(std::overflow_error)
    {
        const unsigned long long max_value = std::numeric_limits<unsigned long long>::max JSONCONS_NO_MACRO_EXP();
        const unsigned long long max_value_div_10 = max_value / 10;
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

    template<typename CharType>
    long long string_to_integer(bool has_neg, const CharType *s, size_t length) throw(std::overflow_error)
    {
        const long long max_value = std::numeric_limits<long long>::max JSONCONS_NO_MACRO_EXP();
        const long long max_value_div_10 = max_value / 10;

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

    states::states_t state_;
    int top_;
    std::vector<modes::modes_t> stack_;
    basic_json_input_handler<Char> *handler_;
    basic_parse_error_handler<Char> *err_handler_;
    unsigned long column_;
    unsigned long line_;
    Char c_;
    uint32_t cp_;
    uint32_t cp2_;
    std::basic_string<Char> string_buffer_;
    bool is_negative_;
    states::states_t saved_state_;
    int prev_char_;
    size_t index_;
    int depth_;
    int max_depth_;
};

typedef basic_json_parser<char> json_parser;
typedef basic_json_parser<wchar_t> wjson_parser;

}

#endif


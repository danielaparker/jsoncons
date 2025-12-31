// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TOKENIZER_HPP
#define JSONCONS_JSON_TOKENIZER_HPP

#include <cstddef>
#include <cstdint>
#include <functional> // std::function
#include <limits> // std::numeric_limits
#include <memory> // std::allocator
#include <string>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <vector>

#include <jsoncons/generic_tokenizer.hpp>
#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/utility/read_number.hpp>
#include <jsoncons/json_error.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/ser_util.hpp>
#include <jsoncons/utility/unicode_traits.hpp>

#define JSONCONS_ILLEGAL_CONTROL_CHARACTER \
        case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x08:case 0x0b: \
        case 0x0c:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:case 0x15:case 0x16: \
        case 0x17:case 0x18:case 0x19:case 0x1a:case 0x1b:case 0x1c:case 0x1d:case 0x1e:case 0x1f 

namespace jsoncons {

enum class parse_state : uint8_t 
{
    initial, 
    root,
    accept, 
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
    number,
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
    done
};

enum class parse_string_state : uint8_t 
{
    text = 0,
    escape, 
    escape_u1, 
    escape_u2, 
    escape_u3, 
    escape_u4, 
    escape_expect_surrogate_pair1, 
    escape_expect_surrogate_pair2, 
    escape_u5, 
    escape_u6, 
    escape_u7, 
    escape_u8
};

enum class parse_number_state : uint8_t 
{
    minus, 
    zero,  
    integer,
    fraction1,
    fraction2,
    exp1,
    exp2,
    exp3
};

template <typename CharT,typename TempAlloc  = std::allocator<char>>
class basic_json_tokenizer : private ser_context
{
public:
    using char_type = CharT;
    using string_view_type = jsoncons::basic_string_view<char_type>;
private:
    struct string_maps_to_double
    {
        string_view_type s;

        bool operator()(const std::pair<string_view_type,double>& val) const
        {
            return val.first == s;
        }
    };

    using temp_allocator_type = TempAlloc;
    using char_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<CharT>;
    using parse_state_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<parse_state>;

    static constexpr std::size_t initial_buffer_capacity = 256;
    static constexpr int default_initial_stack_capacity = 66;

    generic_token_kind token_kind_{};
    semantic_tag tag_{};
    union
    {
        bool bool_value_;
        int64_t int64_value_;
        uint64_t uint64_value_;
        double double_value_;
        const CharT* string_data_;
    } value_;
    std::size_t length_{0};

    int max_nesting_depth_;
    bool allow_trailing_comma_;
    bool allow_comments_;    
    bool lossless_number_;    
    bool lossless_bignum_;    

    int level_{0};
    uint32_t cp_{0};
    uint32_t cp2_{0};
    std::size_t line_{1};
    std::size_t position_{0};
    std::size_t mark_position_{0};
    std::size_t begin_position_{0};
    const char_type* input_end_{nullptr};
    const char_type* input_ptr_{nullptr};
    parse_state state_{parse_state::initial};
    parse_string_state string_state_{};
    parse_number_state number_state_{};
    bool more_{true};
    int mark_level_{0};
    bool is_key_{false};
    
    semantic_tag escape_tag_;
    std::basic_string<char_type,std::char_traits<char_type>,char_allocator_type> buffer_;

    std::vector<parse_state,parse_state_allocator_type> state_stack_;
    std::vector<std::pair<std::basic_string<char_type>,double>> string_double_map_;

    // Noncopyable and nonmoveable
    basic_json_tokenizer(const basic_json_tokenizer&) = delete;
    basic_json_tokenizer& operator=(const basic_json_tokenizer&) = delete;

public:

    basic_json_tokenizer()
        : basic_json_tokenizer(basic_json_decode_options<char_type>())
    {
    }

    explicit basic_json_tokenizer(const TempAlloc& temp_alloc)
        : basic_json_tokenizer(basic_json_decode_options<char_type>(), temp_alloc)
    {
    }

    basic_json_tokenizer(const basic_json_decode_options<char_type>& options,
        const TempAlloc& temp_alloc = TempAlloc())
       : max_nesting_depth_(options.max_nesting_depth()),
         allow_trailing_comma_(options.allow_trailing_comma()),
         allow_comments_(options.allow_comments()),
         lossless_number_(options.lossless_number()),
         lossless_bignum_(options.lossless_bignum()),
         buffer_(temp_alloc),
         state_stack_(temp_alloc)
    {
        buffer_.reserve(initial_buffer_capacity);

        std::size_t initial_stack_capacity = options.max_nesting_depth() <= (default_initial_stack_capacity-2) ? (options.max_nesting_depth()+2) : default_initial_stack_capacity;
        state_stack_.reserve(initial_stack_capacity );
        push_state(parse_state::root);

        if (options.enable_str_to_nan())
        {
            string_double_map_.emplace_back(options.nan_to_str(),std::nan(""));
        }
        if (options.enable_str_to_inf())
        {
            string_double_map_.emplace_back(options.inf_to_str(),std::numeric_limits<double>::infinity());
        }
        if (options.enable_str_to_neginf())
        {
            string_double_map_.emplace_back(options.neginf_to_str(),-std::numeric_limits<double>::infinity());
        }
    }

    generic_token_kind token_kind() const
    {
        return token_kind_;
    }

    semantic_tag tag() const
    {
        return tag_;
    }

    bool is_key() const
    {
        return is_key_;
    }

    bool get_bool() const
    {
        return value_.bool_value_;
    }

    int64_t get_int64() const
    {
        return value_.int64_value_;
    }

    uint64_t get_uint64() const
    {
        return value_.uint64_value_;
    }

    double get_double() const
    {
        return value_.double_value_;
    }

    string_view_type get_string_view() const
    {
        return string_view_type(value_.string_data_, length_);
    }

    int level() const
    {
        return level_;
    }

    int mark_level() const 
    {
        return mark_level_;
    }

    void mark_level(int value)
    {
        mark_level_ = value;
    }

    bool source_exhausted() const
    {
        return input_ptr_ == input_end_;
    }

    const char_type* current() const
    {
        return input_ptr_;
    }

    ~basic_json_tokenizer() noexcept
    {
    }

    parse_state parent() const
    {
        JSONCONS_ASSERT(state_stack_.size() >= 1);
        return state_stack_.back();
    }

    bool started() const
    {
        return state_ != parse_state::initial;
    }

    bool done() const
    {
        return state_ == parse_state::done;
    }

    from_json_result try_next_token()
    {
        if (JSONCONS_UNLIKELY(done()))
        {
            return from_json_result{};
        }
        if (state_ == parse_state::accept)
        {
            token_kind_ = generic_token_kind{};
            state_ = parse_state::done;
            more_ = false;
            return from_json_result{};
        }
        else
        {
            more_ = true;
        }
        token_kind_ = generic_token_kind{};
        const char_type* local_input_end = input_end_;
        std::error_code ec{};

        if (input_ptr_ == local_input_end && more_)
        {
            switch (state_)
            {
                case parse_state::number:  
                    if (number_state_ == parse_number_state::zero || number_state_ == parse_number_state::integer)
                    {
                        end_integer_value(ec);
                        if (JSONCONS_UNLIKELY(ec)) return from_json_result{(json_errc)ec.value()};
                    }
                    else if (number_state_ == parse_number_state::fraction2 || number_state_ == parse_number_state::exp3)
                    {
                        auto r = end_fraction_value();
                        if (JSONCONS_UNLIKELY(!r)) return r;
                    }
                    else
                    {
                        more_ = false;
                        return from_json_result{json_errc::unexpected_eof};
                    }
                    break;
                case parse_state::accept:
                    token_kind_ = generic_token_kind{};
                    state_ = parse_state::done;
                    more_ = false;
                    break;
                case parse_state::initial:
                    more_ = false;
                    break;                
                case parse_state::done:
                    more_ = false;
                    break;
                case parse_state::cr:
                    state_ = pop_state();
                    break;
                default:
                    more_ = false;
                    return from_json_result{json_errc::unexpected_eof};
            }
            
            return from_json_result{};
        }
        else
        {
            return try_parse_next();
        }
    }

    bool parsing_started() const
    {
        return state_ == parse_state::initial;
    }

    bool accept() const
    {
        return state_ == parse_state::accept || done();
    }

    bool stopped() const
    {
        return !more_;
    }

    parse_state state() const
    {
        return state_;
    }

    bool finished() const
    {
        return !more_ && state_ != parse_state::accept;
    }

    void check_done(std::error_code& ec)
    {
        for (; input_ptr_ != input_end_; ++input_ptr_)
        {
            char_type curr_char_ = *input_ptr_;
            switch (curr_char_)
            {
                case '\n':
                case '\r':
                case '\t':
                case ' ':
                    break;
                default:
                    ec = json_errc::extra_character;
                    break;
            }
        }
    }

    void skip_whitespace()
    {
        const char_type* local_input_end = input_end_;

        while (input_ptr_ != local_input_end) 
        {
            switch (state_)
            {
                case parse_state::cr:
                    ++line_;
                    //++position_;
                    switch (*input_ptr_)
                    {
                        case '\n':
                            ++input_ptr_;
                            ++position_;
                            mark_position_ = position_;
                            state_ = pop_state();
                            break;
                        default:
                            mark_position_ = position_;
                            state_ = pop_state();
                            break;
                    }
                    break;

                default:
                    switch (*input_ptr_)
                    {
                        case ' ':case '\t':case '\n':case '\r':
                            skip_space(&input_ptr_);
                            break;
                        default:
                            return;
                    }
                    break;
            }
        }
    }

    void begin_map(std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(++level_ > max_nesting_depth_))
        {
            ec = json_errc::max_nesting_depth_exceeded;
            more_ = false;
            return;
        } 

        push_state(parse_state::object);
        state_ = parse_state::expect_member_name_or_end;
        token_kind_ = generic_token_kind::begin_map;
        tag_ = semantic_tag{};
        more_ = false;
    }

    void end_map(std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(level_ < 1))
        {
            ec = json_errc::unexpected_rbrace;
            more_ = false;
            return;
        }
        state_ = pop_state();
        if (state_ == parse_state::object)
        {
            token_kind_ = generic_token_kind::end_map;
            tag_ = semantic_tag{};
        }
        else if (state_ == parse_state::array)
        {
            ec = json_errc::expected_comma_or_rbracket;
            more_ = false;
            return;
        }
        else
        {
            ec = json_errc::unexpected_rbrace;
            more_ = false;
            return;
        }

        more_ = false;
        if (level_ == mark_level_)
        {
            more_ = false;
        }
        --level_;
        if (level_ == 0)
        {
            state_ = parse_state::accept;
        }
        else
        {
            state_ = parse_state::expect_comma_or_end;
        }
    }

    void begin_array(std::error_code& ec)
    {
        if (++level_ > max_nesting_depth_)
        {
            more_ = false;
            ec = json_errc::max_nesting_depth_exceeded;
            return;
        }

        push_state(parse_state::array);
        state_ = parse_state::expect_value_or_end;
        token_kind_ = generic_token_kind::begin_array;
        tag_ = semantic_tag{};

        more_ = false;
    }

    void end_array(std::error_code& ec)
    {
        if (level_ < 1)
        {
            ec = json_errc::unexpected_rbracket;
            more_ = false;
            return;
        }
        state_ = pop_state();
        if (state_ == parse_state::array)
        {
            token_kind_ = generic_token_kind::end_array;
            tag_ = semantic_tag{};
        }
        else if (state_ == parse_state::object)
        {
            ec = json_errc::expected_comma_or_rbrace;
            more_ = false;
            return;
        }
        else
        {
            ec = json_errc::unexpected_rbracket;
            more_ = false;
            return;
        }

        more_ = false;
        if (level_ == mark_level_)
        {
            more_ = false;
        }
        --level_;

        if (level_ == 0)
        {
            state_ = parse_state::accept;
        }
        else
        {
            state_ = parse_state::expect_comma_or_end;
        }
    }

    void reinitialize()
    {
        reset();
        cp_ = 0;
        cp2_ = 0;
        begin_position_ = 0;
        input_end_ = nullptr;
        input_ptr_ = nullptr;
        buffer_.clear();
    }

    void reset()
    {
        state_stack_.clear();
        push_state(parse_state::root);
        state_ = parse_state::initial;
        more_ = true;
        line_ = 1;
        position_ = 0;
        mark_position_ = 0;
        level_ = 0;
    }

    void restart()
    {
        more_ = true;
    }

    void update(string_view_type sv)
    {
        input_ptr_ = sv.data();
        input_end_ = input_ptr_ + sv.size();
    }

    void update(const char_type* data, std::size_t length)
    {
        input_ptr_ = data;
        input_end_ = data + length;
    }

    void parse_next()
    {
        std::error_code ec;
        parse_next(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            JSONCONS_THROW(ser_error(ec,line_,column()));
        }
    }

    from_json_result try_parse_next()
    {
        std::error_code ec;
        parse_next(ec);
        return from_json_result{(json_errc)ec.value()};
    }

    void parse_next(std::error_code& ec)
    {
        more_ = true;
        token_kind_ = generic_token_kind{};
        if (state_ == parse_state::accept)
        {
            token_kind_ = generic_token_kind{};
            state_ = parse_state::done;
            more_ = false;
            return;
        }
        const char_type* local_input_end = input_end_;
 
        while ((input_ptr_ < local_input_end) && more_)
        {
            switch (state_)
            {
                case parse_state::accept:
                    token_kind_ = generic_token_kind{};
                    state_ = parse_state::done;
                    more_ = false;
                    break;
                case parse_state::cr:
                    ++line_;
                    switch (*input_ptr_)
                    {
                        case '\n':
                            ++input_ptr_;
                            ++position_;
                            state_ = pop_state();
                            break;
                        default:
                            state_ = pop_state();
                            break;
                    }
                    mark_position_ = position_;
                    break;
                case parse_state::initial: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            more_ = false;
                            ec = json_errc::illegal_control_character;
                            return;
                        case ' ':case '\t':case '\n':case '\r':
                            skip_space(&input_ptr_);
                            break;
                        case '/': 
                            ++input_ptr_;
                            ++position_;
                            push_state(state_);
                            state_ = parse_state::slash;
                            break;
                        case '{':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            begin_map(ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case '[':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            begin_array(ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case '\"':
                            is_key_ = false;
                            state_ = parse_state::string;
                            string_state_ = parse_string_state{};
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            escape_tag_ = semantic_tag::noesc;
                            buffer_.clear();
                            input_ptr_ = parse_string(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case '-':
                            buffer_.clear();
                            buffer_.push_back('-');
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::number;
                            number_state_ = parse_number_state::minus;
                            input_ptr_ = parse_number(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case '0': 
                            buffer_.clear();
                            buffer_.push_back(static_cast<char>(*input_ptr_));
                            state_ = parse_state::number;
                            number_state_ = parse_number_state::zero;
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            input_ptr_ = parse_number(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                            buffer_.clear();
                            buffer_.push_back(static_cast<char>(*input_ptr_));
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::number;
                            number_state_ = parse_number_state::integer;
                            input_ptr_ = parse_number(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case 'n':
                            parse_null(ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case 't':
                            input_ptr_ = parse_true(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case 'f':
                            input_ptr_ = parse_false(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case '}':
                            ec = json_errc::unexpected_rbrace;
                            more_ = false;
                            return;
                        case ']':
                            ec = json_errc::unexpected_rbracket;
                            more_ = false;
                            return;
                        default:
                            ec = json_errc::syntax_error;
                            more_ = false;
                            return;
                    }
                    break;
                }
                case parse_state::expect_comma_or_end: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            more_ = false;
                            ec = json_errc::illegal_control_character;
                            return;
                        case ' ':case '\t':case '\n':case '\r':
                            skip_space(&input_ptr_);
                            break;
                        case '/':
                            ++input_ptr_;
                            ++position_;
                            push_state(state_); 
                            state_ = parse_state::slash;
                            break;
                        case '}':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            end_map(ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case ']':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            end_array(ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case ',':
                        {
                            auto r = try_begin_member_or_element();
                            if (JSONCONS_UNLIKELY(!r)) 
                            {
                                more_ = false;
                                ec = r.ec;
                                return;
                            }
                            ++input_ptr_;
                            ++position_;
                            break;
                        }
                        default: // REVISIT with options
                            if (parent() == parse_state::array)
                            {
                                more_ = false;
                                ec = json_errc::expected_comma_or_rbracket;
                                return;
                            }
                            else if (parent() == parse_state::object)
                            {
                                more_ = false;
                                ec = json_errc::expected_comma_or_rbrace;
                                return;
                            }
                            else
                            {
                                more_ = false;
                                ec = json_errc::unexpected_character;
                                return;
                            }
                            //++input_ptr_;
                            //++position_;
                            //break;
                    }
                    break;
                }
                case parse_state::expect_member_name_or_end: 
                {
                    if (input_ptr_ >= local_input_end)
                    {
                        return;
                    }
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            more_ = false;
                            ec = json_errc::illegal_control_character;
                            return;
                        case ' ':case '\t':case '\n':case '\r':
                            skip_space(&input_ptr_);
                            break;
                        case '/':
                            ++input_ptr_;
                            ++position_;
                            push_state(state_); 
                            state_ = parse_state::slash;
                            break;
                        case '}':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            end_map(ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case '\"':
                            is_key_ = true;
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            push_state(parse_state::member_name);
                            state_ = parse_state::string;
                            string_state_ = parse_string_state{};
                            escape_tag_ = semantic_tag::noesc;
                            buffer_.clear();
                            input_ptr_ = parse_string(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            is_key_ = true;
                            break;
                        case '\'':
                            more_ = false;
                            ec = json_errc::single_quote;
                            return;
                        default:
                            more_ = false;
                            ec = json_errc::expected_key;
                            return;
                    }
                    break;
                }
                case parse_state::expect_member_name: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            more_ = false;
                            ec = json_errc::illegal_control_character;
                            return;
                        case ' ':case '\t':case '\n':case '\r':
                            skip_space(&input_ptr_);
                            break;
                        case '/': 
                            ++input_ptr_;
                            ++position_;
                            push_state(state_);
                            state_ = parse_state::slash;
                            break;
                        case '\"':
                            is_key_ = true;
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            push_state(parse_state::member_name);
                            state_ = parse_state::string;
                            string_state_ = parse_string_state{};
                            escape_tag_ = semantic_tag::noesc;
                            buffer_.clear();
                            input_ptr_ = parse_string(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            is_key_ = true;
                            break;
                        case '}':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            if (!allow_trailing_comma_)
                            {
                                more_ = false;
                                ec = json_errc::extra_comma;
                                return;
                            }
                            end_map(ec);  // Recover
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case '\'':
                            more_ = false;
                            ec = json_errc::single_quote;
                            return;
                        default:
                            more_ = false;
                            ec = json_errc::expected_key;
                            return;
                    }
                    break;
                }
                case parse_state::expect_colon: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            more_ = false;
                            ec = json_errc::illegal_control_character;
                            return;
                        case ' ':case '\t':case '\n':case '\r':
                            skip_space(&input_ptr_);
                            break;
                        case '/': 
                            push_state(state_);
                            state_ = parse_state::slash;
                            ++input_ptr_;
                            ++position_;
                            break;
                        case ':':
                            state_ = parse_state::expect_value;
                            is_key_ = false;
                            ++input_ptr_;
                            ++position_;
                            break;
                        default:
                            more_ = false;
                            ec = json_errc::expected_colon;
                            return;
                    }
                    break;
                }
                case parse_state::expect_value: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            more_ = false;
                            ec = json_errc::illegal_control_character;
                            return;
                        case ' ':case '\t':case '\n':case '\r':
                            skip_space(&input_ptr_);
                            break;
                        case '/': 
                            push_state(state_);
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::slash;
                            break;
                        case '{':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            begin_map(ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case '[':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            begin_array(ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case '\"':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::string;
                            string_state_ = parse_string_state{};
                            escape_tag_ = semantic_tag::noesc;
                            buffer_.clear();
                            input_ptr_ = parse_string(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case '-':
                            buffer_.clear();
                            buffer_.push_back('-');
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::number;
                            number_state_ = parse_number_state::minus;
                            input_ptr_ = parse_number(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case '0': 
                            buffer_.clear();
                            buffer_.push_back(static_cast<char>(*input_ptr_));
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::number;
                            number_state_ = parse_number_state::zero;
                            input_ptr_ = parse_number(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                            buffer_.clear();
                            buffer_.push_back(static_cast<char>(*input_ptr_));
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::number;
                            number_state_ = parse_number_state::integer;
                            input_ptr_ = parse_number(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case 'n':
                            parse_null(ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case 't':
                            input_ptr_ = parse_true(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case 'f':
                            input_ptr_ = parse_false(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case ']':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            if (parent() == parse_state::array)
                            {
                                if (!allow_trailing_comma_)
                                {
                                    more_ = false;
                                    ec = json_errc::extra_comma;
                                    return;
                                }
                                end_array(ec);  // Recover
                                if (JSONCONS_UNLIKELY(ec)) return;
                            }
                            else
                            {
                                more_ = false;
                                ec = json_errc::expected_value;
                                return;
                            }
                            
                            break;
                        case '\'':
                            more_ = false;
                            ec = json_errc::single_quote;
                            return;
                        default:
                            more_ = false;
                            ec = json_errc::expected_value;
                            return;
                    }
                    break;
                }
                case parse_state::expect_value_or_end: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            more_ = false;
                            ec = json_errc::illegal_control_character;
                            return;
                        case ' ':case '\t':case '\n':case '\r':
                            skip_space(&input_ptr_);
                            break;
                        case '/': 
                            ++input_ptr_;
                            ++position_;
                            push_state(state_);
                            state_ = parse_state::slash;
                            break;
                        case '{':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            begin_map(ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case '[':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            begin_array(ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case ']':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            end_array(ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case '\"':
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::string;
                            string_state_ = parse_string_state{};
                            escape_tag_ = semantic_tag::noesc;
                            buffer_.clear();
                            input_ptr_ = parse_string(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) return;
                            break;
                        case '-':
                            buffer_.clear();
                            buffer_.push_back('-');
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::number;
                            number_state_ = parse_number_state::minus;
                            input_ptr_ = parse_number(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case '0': 
                            buffer_.clear();
                            buffer_.push_back(static_cast<char>(*input_ptr_));
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::number;
                            number_state_ = parse_number_state::zero;
                            input_ptr_ = parse_number(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                            buffer_.clear();
                            buffer_.push_back(static_cast<char>(*input_ptr_));
                            begin_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::number;
                            number_state_ = parse_number_state::integer;
                            input_ptr_ = parse_number(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case 'n':
                            parse_null(ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case 't':
                            input_ptr_ = parse_true(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case 'f':
                            input_ptr_ = parse_false(input_ptr_, ec);
                            if (JSONCONS_UNLIKELY(ec)) {return;}
                            break;
                        case '\'':
                            more_ = false;
                            ec = json_errc::single_quote;
                            return;
                        default:
                            more_ = false;
                            ec = json_errc::expected_value;
                            return;
                        }
                    }
                    break;
                case parse_state::string: 
                    input_ptr_ = parse_string(input_ptr_, ec);
                    if (JSONCONS_UNLIKELY(ec)) return;
                    break;
                case parse_state::number:
                    input_ptr_ = parse_number(input_ptr_, ec);  
                    if (JSONCONS_UNLIKELY(ec)) return;
                    break;
                case parse_state::t: 
                    switch (*input_ptr_)
                    {
                        case 'r':
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::tr;
                            break;
                        default:
                            ec = json_errc::invalid_value;
                            more_ = false;
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
                            ec = json_errc::invalid_value;
                            more_ = false;
                            return;
                    }
                    ++input_ptr_;
                    ++position_;
                    break;
                case parse_state::tru: 
                    switch (*input_ptr_)
                    {
                        case 'e':
                            ++input_ptr_;
                            ++position_;
                            token_kind_ = generic_token_kind::bool_value;
                            value_.bool_value_ = true;
                            tag_ = semantic_tag{};
                            if (level_ == 0)
                            {
                                state_ = parse_state::accept;
                            }
                            else
                            {
                                state_ = parse_state::expect_comma_or_end;
                            }
                            more_ = false;
                            break;
                        default:
                            ec = json_errc::invalid_value;
                            more_ = false;
                            return;
                    }
                    break;
                case parse_state::f: 
                    switch (*input_ptr_)
                    {
                        case 'a':
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::fa;
                            break;
                        default:
                            ec = json_errc::invalid_value;
                            more_ = false;
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
                            ec = json_errc::invalid_value;
                            more_ = false;
                            return;
                    }
                    ++input_ptr_;
                    ++position_;
                    break;
                case parse_state::fal: 
                    switch (*input_ptr_)
                    {
                        case 's':
                            state_ = parse_state::fals;
                            break;
                        default:
                            ec = json_errc::invalid_value;
                            more_ = false;
                            return;
                    }
                    ++input_ptr_;
                    ++position_;
                    break;
                case parse_state::fals: 
                    switch (*input_ptr_)
                    {
                        case 'e':
                            ++input_ptr_;
                            ++position_;
                            token_kind_ = generic_token_kind::bool_value;
                            value_.bool_value_ = false;
                            tag_ = semantic_tag{};
                            if (level_ == 0)
                            {
                                state_ = parse_state::accept;
                            }
                            else
                            {
                                state_ = parse_state::expect_comma_or_end;
                            }
                            more_ = false;
                            break;
                        default:
                            ec = json_errc::invalid_value;
                            more_ = false;
                            return;
                    }
                    break;
                case parse_state::n: 
                    switch (*input_ptr_)
                    {
                        case 'u':
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::nu;
                            break;
                        default:
                            ec = json_errc::invalid_value;
                            more_ = false;
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
                            ec = json_errc::invalid_value;
                            more_ = false;
                            return;
                    }
                    ++input_ptr_;
                    ++position_;
                    break;
                case parse_state::nul: 
                    ++position_;
                    switch (*input_ptr_)
                    {
                        case 'l':
                            token_kind_ = generic_token_kind::null_value;
                            tag_ = semantic_tag{};
                            if (level_ == 0)
                            {
                                state_ = parse_state::accept;
                            }
                            else
                            {
                                state_ = parse_state::expect_comma_or_end;
                            }
                            more_ = false;
                            break;
                        default:
                            ec = json_errc::invalid_value;
                            more_ = false;
                            return;
                    }
                    ++input_ptr_;
                    break;
                case parse_state::slash: 
                {
                    switch (*input_ptr_)
                    {
                        case '*':
                            if (!allow_comments_)
                            {
                                ec = json_errc::illegal_comment;
                                more_ = false;
                                return;
                            }
                            state_ = parse_state::slash_star;
                            break;
                        case '/':
                            if (!allow_comments_)
                            {
                                more_ = false;
                                ec = json_errc::illegal_comment;
                                return;
                            }
                            state_ = parse_state::slash_slash;
                            break;
                        default:    
                            more_ = false;
                            ec = json_errc::syntax_error;
                            return;
                    }
                    ++input_ptr_;
                    ++position_;
                    break;
                }
                case parse_state::slash_star:  
                {
                    switch (*input_ptr_)
                    {
                        case '\r':
                            push_state(state_);
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::cr;
                            break;
                        case '\n':
                            ++input_ptr_;
                            ++line_;
                            ++position_;
                            mark_position_ = position_;
                            break;
                        case '*':
                            ++input_ptr_;
                            ++position_;
                            state_ = parse_state::slash_star_star;
                            break;
                        default:
                            ++input_ptr_;
                            ++position_;
                            break;
                    }
                    break;
                }
                case parse_state::slash_slash: 
                {
                    switch (*input_ptr_)
                    {
                    case '\r':
                    case '\n':
                        state_ = pop_state();
                        break;
                    default:
                        ++input_ptr_;
                        ++position_;
                    }
                    break;
                }
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
                    ++input_ptr_;
                    ++position_;
                    break;
                }
                default:
                    JSONCONS_ASSERT(false); // RECHECK
            }
        }
    }

    const char_type* parse_true(const char_type* cur, std::error_code& ec)
    {
        begin_position_ = position_;
        if (JSONCONS_LIKELY(input_end_ - cur >= 4))
        {
            if (*(cur+1) == 'r' && *(cur+2) == 'u' && *(cur+3) == 'e')
            {
                cur += 4;
                position_ += 4;
                token_kind_ = generic_token_kind::bool_value;
                value_.bool_value_ = true;
                tag_ = semantic_tag::none;
                if (level_ == 0)
                {
                    state_ = parse_state::accept;
                }
                else
                {
                    state_ = parse_state::expect_comma_or_end;
                }
                more_ = false;
            }
            else
            {
                ec = json_errc::invalid_value;
                more_ = false;
                return cur;
            }
        }
        else
        {
            ++cur;
            ++position_;
            state_ = parse_state::t;
        }
        return cur;
    }

    void parse_null(std::error_code& ec)
    {
        begin_position_ = position_;
        if (JSONCONS_LIKELY(input_end_ - input_ptr_ >= 4))
        {
            if (*(input_ptr_+1) == 'u' && *(input_ptr_+2) == 'l' && *(input_ptr_+3) == 'l')
            {
                input_ptr_ += 4;
                position_ += 4;
                token_kind_ = generic_token_kind::null_value;
                tag_ = semantic_tag::none;
                more_ = false;
                if (level_ == 0)
                {
                    state_ = parse_state::accept;
                }
                else
                {
                    state_ = parse_state::expect_comma_or_end;
                }
            }
            else
            {
                ec = json_errc::invalid_value;
                more_ = false;
                return;
            }
        }
        else
        {
            ++input_ptr_;
            ++position_;
            state_ = parse_state::n;
        }
    }

    const char_type* parse_false(const char_type* cur, std::error_code& ec)
    {
        begin_position_ = position_;
        if (JSONCONS_LIKELY(input_end_ - cur >= 5))
        {
            if (*(cur+1) == 'a' && *(cur+2) == 'l' && *(cur+3) == 's' && *(cur+4) == 'e')
            {
                cur += 5;
                position_ += 5;
                token_kind_ = generic_token_kind::bool_value;
                value_.bool_value_ = false;
                tag_ = semantic_tag{};
                more_ = false;
                if (level_ == 0)
                {
                    state_ = parse_state::accept;
                }
                else
                {
                    state_ = parse_state::expect_comma_or_end;
                }
            }
            else
            {
                ec = json_errc::invalid_value;
                more_ = false;
                return cur;
            }
        }
        else
        {
            ++cur;
            ++position_;
            state_ = parse_state::f;
        }
        return cur;
    }

    const char_type* parse_number(const char_type* hdr, std::error_code& ec)
    {
        const char_type* cur = hdr;
        const char_type* local_input_end = input_end_;

        switch (number_state_)
        {
            case parse_number_state::minus:
                goto minus_sign;
            case parse_number_state::zero:
                goto zero;
            case parse_number_state::integer:
                goto integer;
            case parse_number_state::fraction1:
                goto fraction1;
            case parse_number_state::fraction2:
                goto fraction2;
            case parse_number_state::exp1:
                goto exp1;
            case parse_number_state::exp2:
                goto exp2;
            case parse_number_state::exp3:
                goto exp3;
            default:
                JSONCONS_UNREACHABLE();               
        }
minus_sign:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            number_state_ = parse_number_state::minus;
            buffer_.append(hdr, cur);
            position_ += (cur - hdr);
            return cur;
        }
        if (jsoncons::is_nonzero_digit(*cur))
        {
            ++cur;
            goto integer;
        }
        if (*cur == '0')
        {
            ++cur;
            goto zero;
        }
        ec = json_errc::invalid_number;
        more_ = false;
        position_ += (cur - hdr);
        return cur;
zero:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            number_state_ = parse_number_state::integer;
            buffer_.append(hdr, cur);
            position_ += (cur - hdr);
            return cur;
        }
        if (*cur == '.')
        {
            ++cur;
            goto fraction1;
        }
        if (jsoncons::is_exp(*cur))
        {
            ++cur;
            goto exp1;
        }
        if (jsoncons::is_digit(*cur))
        {
            ec = json_errc::leading_zero;
            more_ = false;
            number_state_ = parse_number_state::zero;

            position_ += (cur - hdr);
            return cur;
        }
        buffer_.append(hdr, cur);
        position_ += (cur - hdr);
        end_integer_value(ec);
        return cur;
integer:
        while (true)
        {
            if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
            {
                number_state_ = parse_number_state::integer;
                buffer_.append(hdr, cur);
                position_ += (cur - hdr);
                return cur;
            }
            if (JSONCONS_UNLIKELY(!jsoncons::is_digit(*cur)))
            {
                break;
            }
            ++cur;
        }
        if (*cur == '.')
        {
            ++cur;
            goto fraction1;
        }
        if (jsoncons::is_exp(*cur))
        {
            ++cur;
            goto exp1;
        }
        buffer_.append(hdr, cur);
        position_ += (cur - hdr);
        end_integer_value(ec);
        return cur;
fraction1:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            number_state_ = parse_number_state::fraction1;
            buffer_.append(hdr, cur);
            position_ += (cur - hdr);
            return cur;
        }
        if (jsoncons::is_digit(*cur))
        {
            ++cur;
            goto fraction2;
        }
        ec = json_errc::invalid_number;
        more_ = false;
        number_state_ = parse_number_state::fraction1;
        position_ += (cur - hdr);
        return cur;
fraction2:
        while (true)
        {
            if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
            {
                number_state_ = parse_number_state::fraction2;
                buffer_.append(hdr, cur);
                position_ += (cur - hdr);
                return cur;
            }
            if (JSONCONS_UNLIKELY(!jsoncons::is_digit(*cur)))
            {
                break;
            }
            ++cur;
        }
        if (jsoncons::is_exp(*cur))
        {
            ++cur;
            goto exp1;
        }
        buffer_.append(hdr, cur);
        position_ += (cur - hdr);
        ec = end_fraction_value().ec;
        return cur;
exp1:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            number_state_ = parse_number_state::exp1;
            buffer_.append(hdr, cur);
            position_ += (cur - hdr);
            return cur;
        }
        if (*cur == '-')
        {
            ++cur;
            goto exp2;
        }
        if (jsoncons::is_digit(*cur))
        {
            ++cur;
            goto exp3;
        }
        if (*cur == '+')
        {
            ++cur;
            goto exp2;
        }
        ec = json_errc::invalid_number;
        more_ = false;
        position_ += (cur - hdr);
        return cur;
exp2:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            number_state_ = parse_number_state::exp2;
            buffer_.append(hdr, cur);
            position_ += (cur - hdr);
            return cur;
        }
        if (jsoncons::is_digit(*cur))
        {
            ++cur;
            goto exp3;
        }
        ec = json_errc::invalid_number;
        more_ = false;
        position_ += (cur - hdr);
        return cur;
        
exp3:
        while (true)
        {
            if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
            {
                number_state_ = parse_number_state::exp3;
                buffer_.append(hdr, cur);
                position_ += (cur - hdr);
                return cur;
            }
            if (JSONCONS_UNLIKELY(!jsoncons::is_digit(*cur)))
            {
                break;
            }
            ++cur;
        }
        buffer_.append(hdr, cur);
        position_ += (cur - hdr);
        ec = end_fraction_value().ec;
        return cur;
    }

    const char_type* parse_string(const char_type* cur, std::error_code& ec)
    {
        const char_type* local_input_end = input_end_;
        const char_type* sb = cur;

        switch (string_state_)
        {
            case parse_string_state::text:
                goto text;
            case parse_string_state::escape:
                goto escape;
            case parse_string_state::escape_u1:
                goto escape_u1;
            case parse_string_state::escape_u2:
                goto escape_u2;
            case parse_string_state::escape_u3:
                goto escape_u3;
            case parse_string_state::escape_u4:
                goto escape_u4;
            case parse_string_state::escape_expect_surrogate_pair1:
                goto escape_expect_surrogate_pair1;
            case parse_string_state::escape_expect_surrogate_pair2:
                goto escape_expect_surrogate_pair2;
            case parse_string_state::escape_u5:
                goto escape_u5;
            case parse_string_state::escape_u6:
                goto escape_u6;
            case parse_string_state::escape_u7:
                goto escape_u7;
            case parse_string_state::escape_u8:
                goto escape_u8;
            default:
                JSONCONS_UNREACHABLE();               
        }

text:
        while (cur < local_input_end)
        {
            switch (*cur)
            {
                JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                {
                    position_ += (cur - sb + 1);
                    more_ = false;
                    ec = json_errc::illegal_control_character;
                    string_state_ = parse_string_state{};
                    return cur;
                }
                case '\n':
                case '\r':
                case '\t':
                {
                    position_ += (cur - sb + 1);
                    more_ = false;
                    ec = json_errc::illegal_character_in_string;
                    return cur;
                }
                case '\\': 
                {
                    buffer_.append(sb,cur-sb);
                    position_ += (cur - sb + 1);
                    ++cur;
                    escape_tag_ = semantic_tag{};
                    goto escape;
                }
                case '\"':
                {
                    position_ += (cur - sb + 1);
                    if (buffer_.empty())
                    {
                        ec = end_string_value(sb,cur-sb).ec;
                        if (JSONCONS_UNLIKELY(ec)) {return cur;}
                    }
                    else
                    {
                        buffer_.append(sb,cur-sb);
                        ec = end_string_value(buffer_.data(), buffer_.length()).ec;
                        if (JSONCONS_UNLIKELY(ec)) {return cur;}
                    }
                    ++cur;
                    return cur;
                }
            default:
                break;
            }
            ++cur;
        }

        // Buffer exhausted               
        {
            buffer_.append(sb,cur-sb);
            position_ += (cur - sb);
            string_state_ = parse_string_state{};
            return cur;
        }

escape:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            string_state_ = parse_string_state::escape;
            return cur;
        }
        switch (*cur)
        {
        case '\"':
            buffer_.push_back('\"');
            sb = ++cur;
            ++position_;
            goto text;
        case '\\': 
            buffer_.push_back('\\');
            sb = ++cur;
            ++position_;
            goto text;
        case '/':
            buffer_.push_back('/');
            sb = ++cur;
            ++position_;
            goto text;
        case 'b':
            buffer_.push_back('\b');
            sb = ++cur;
            ++position_;
            goto text;
        case 'f':
            buffer_.push_back('\f');
            sb = ++cur;
            ++position_;
            goto text;
        case 'n':
            buffer_.push_back('\n');
            sb = ++cur;
            ++position_;
            goto text;
        case 'r':
            buffer_.push_back('\r');
            sb = ++cur;
            ++position_;
            goto text;
        case 't':
            buffer_.push_back('\t');
            sb = ++cur;
            ++position_;
            goto text;
        case 'u':
             cp_ = 0;
             ++cur;
             ++position_;
             goto escape_u1;
        default:    
            ec = json_errc::illegal_escaped_character;
            more_ = false;
            string_state_ = parse_string_state::escape;
            return cur;
        }

escape_u1:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            string_state_ = parse_string_state::escape_u1;
            return cur;
        }
        {
            cp_ = 0;
            auto r = try_append_to_codepoint(cp_, *cur);
            if (JSONCONS_UNLIKELY(!r))
            {
                more_ = false;
                ec = r.ec;
                string_state_ = parse_string_state::escape_u1;
                return cur;
            }
            ++cur;
            ++position_;
            goto escape_u2;
        }

escape_u2:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            string_state_ = parse_string_state::escape_u2;
            return cur;
        }
        {
            auto r = try_append_to_codepoint(cp_, *cur);
            if (JSONCONS_UNLIKELY(!r))
            {
                more_ = false;
                ec = r.ec;
                string_state_ = parse_string_state::escape_u2;
                return cur;
            }
            ++cur;
            ++position_;
            goto escape_u3;
        }

escape_u3:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            string_state_ = parse_string_state::escape_u3;
            return cur;
        }
        {
            auto r = try_append_to_codepoint(cp_, *cur);
            if (JSONCONS_UNLIKELY(!r))
            {
                more_ = false;
                ec = r.ec;
                string_state_ = parse_string_state::escape_u3;
                return cur;
            }
            ++cur;
            ++position_;
            goto escape_u4;
        }

escape_u4:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            string_state_ = parse_string_state::escape_u4;
            return cur;
        }
        {
            auto r = try_append_to_codepoint(cp_, *cur);
            if (JSONCONS_UNLIKELY(!r))
            {
                more_ = false;
                ec = r.ec;
                string_state_ = parse_string_state::escape_u4;
                return cur;
            }
            if (unicode_traits::is_high_surrogate(cp_))
            {
                ++cur;
                ++position_;
                goto escape_expect_surrogate_pair1;
            }
            else
            {
                unicode_traits::convert(&cp_, 1, buffer_);
                sb = ++cur;
                ++position_;
                string_state_ = parse_string_state{};
                return cur;
            }
        }

escape_expect_surrogate_pair1:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            string_state_ = parse_string_state::escape_expect_surrogate_pair1;
            return cur;
        }
        {
            switch (*cur)
            {
            case '\\': 
                cp2_ = 0;
                ++cur;
                ++position_;
                goto escape_expect_surrogate_pair2;
            default:
                ec = json_errc::expected_codepoint_surrogate_pair;
                more_ = false;
                string_state_ = parse_string_state::escape_expect_surrogate_pair1;
                return cur;
            }
        }

escape_expect_surrogate_pair2:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            string_state_ = parse_string_state::escape_expect_surrogate_pair2;
            return cur;
        }
        {
            switch (*cur)
            {
            case 'u':
                ++cur;
                ++position_;
                goto escape_u5;
            default:
                ec = json_errc::expected_codepoint_surrogate_pair;
                more_ = false;
                string_state_ = parse_string_state::escape_expect_surrogate_pair2;
                return cur;
            }
        }

escape_u5:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            string_state_ = parse_string_state::escape_u5;
            return cur;
        }
        {
            cp2_ = 0;
            auto r = try_append_to_codepoint(cp2_, *cur);
            if (JSONCONS_UNLIKELY(!r))
            {
                more_ = false;
                ec = r.ec;
                string_state_ = parse_string_state::escape_u5;
                return cur;
            }
        }
        ++cur;
        ++position_;
        goto escape_u6;

escape_u6:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            string_state_ = parse_string_state::escape_u6;
            return cur;
        }
        {
            auto r = try_append_to_codepoint(cp2_, *cur);
            if (JSONCONS_UNLIKELY(!r))
            {
                more_ = false;
                ec = r.ec;
                string_state_ = parse_string_state::escape_u6;
                return cur;
            }
            ++cur;
            ++position_;
            goto escape_u7;
        }

escape_u7:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            string_state_ = parse_string_state::escape_u7;
            return cur;
        }
        {
            auto r = try_append_to_codepoint(cp2_, *cur);
            if (JSONCONS_UNLIKELY(!r))
            {
                more_ = false;
                ec = r.ec;
                string_state_ = parse_string_state::escape_u7;
                return cur;
            }
            ++cur;
            ++position_;
            goto escape_u8;
        }

escape_u8:
        if (JSONCONS_UNLIKELY(cur >= local_input_end)) // Buffer exhausted               
        {
            string_state_ = parse_string_state::escape_u8;
            return cur;
        }
        {
            auto r = try_append_to_codepoint(cp2_, *cur);
            if (JSONCONS_UNLIKELY(!r))
            {
                more_ = false;
                ec = r.ec;
                string_state_ = parse_string_state::escape_u8;
                return cur;
            }
            uint32_t cp = 0x10000 + ((cp_ & 0x3FF) << 10) + (cp2_ & 0x3FF);
            unicode_traits::convert(&cp, 1, buffer_);
            sb = ++cur;
            ++position_;
            goto text;
        }

        JSONCONS_UNREACHABLE();               
    }

    json_errc translate_conv_errc(unicode_traits::conv_errc result)
    {
        switch (result)
        {
            case unicode_traits::conv_errc():
                return json_errc{};
            case unicode_traits::conv_errc::over_long_utf8_sequence:
                return json_errc::over_long_utf8_sequence;
            case unicode_traits::conv_errc::unpaired_high_surrogate:
                return json_errc::unpaired_high_surrogate;
            case unicode_traits::conv_errc::expected_continuation_byte:
                return json_errc::expected_continuation_byte;
            case unicode_traits::conv_errc::illegal_surrogate_value:
                return json_errc::illegal_surrogate_value;
            default:
                return json_errc::illegal_codepoint;
        }
    }

    std::size_t line() const final
    {
        return line_;
    }

    std::size_t column() const final
    {
        return (position_ - mark_position_) + 1;
    }

    const ser_context& get_context() const
    {
        return *this;
    }

    std::size_t begin_position() const
    {
        return begin_position_;
    }

    std::size_t position() const 
    {
        return begin_position_;
    }

    std::size_t end_position() const
    {
        return position_;
    }

private:

    void skip_space(char_type const ** ptr)
    {
        const char_type* local_input_end = input_end_;
        const char_type* cur = *ptr;

        while (cur < local_input_end) 
        {
            if (*cur == ' ' || *cur == '\t')
            {
                ++cur;
                ++position_;
                continue;
            }
            if (*cur == '\n')
            {
                ++cur;
                ++line_;
                ++position_;
                mark_position_ = position_;
                continue;
            }
            if (*cur == '\r')
            {
                ++cur;
                ++position_;
                if (cur < local_input_end)
                {
                    ++line_;
                    if (*cur == '\n')
                    {
                        ++cur;
                        ++position_;
                    }
                    mark_position_ = position_;
                }
                else
                {
                    push_state(state_);
                    state_ = parse_state::cr;
                    *ptr = cur;
                    return; 
                }
                continue;
            }
            break;
        }
        *ptr = cur;
    }

    void end_integer_value(std::error_code& ec)
    {
        if (buffer_[0] == '-')
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
        int64_t val;
        auto result = jsoncons::dec_to_integer(buffer_.data(), buffer_.length(), val);
        if (result)
        {
            token_kind_ = generic_token_kind::int64_value;
            value_.int64_value_ = val;
            tag_ = semantic_tag::none;
        }
        else // Must be overflow
        {
            if (lossless_bignum_)
            {
                token_kind_ = generic_token_kind::string_value;
                value_.string_data_ = buffer_.data();
                length_ = buffer_.size();
                tag_ = semantic_tag::bigint;
            }
            else
            {
                double d{0};
                result = jsoncons::decstr_to_double(&buffer_[0], buffer_.length(), d);
                if (JSONCONS_LIKELY(result))
                {
                    token_kind_ = generic_token_kind::double_value;
                    value_.double_value_ = d;
                    tag_ = semantic_tag{};
                }
                else if (result.ec == std::errc::result_out_of_range)
                {
                    token_kind_ = generic_token_kind::double_value;
                    value_.double_value_ = d;
                    tag_ = semantic_tag{};
                }
                else
                {
                    ec = json_errc::invalid_number;
                    more_ = false;
                    return;
                }
            }

        }
        more_ = false;
        auto res = try_after_value();
        if (!res)
        {
            ec = res.ec;
        }
    }

    void end_positive_value(std::error_code& ec)
    {
        uint64_t val;
        auto result = jsoncons::dec_to_integer(buffer_.data(), buffer_.length(), val);
        if (result)
        {
            token_kind_ = generic_token_kind::uint64_value;
            value_.uint64_value_ = val;
            tag_ = semantic_tag::none;
        }
        else // Must be overflow
        {
            if (lossless_bignum_)
            {
                token_kind_ = generic_token_kind::string_value;
                value_.string_data_ = buffer_.data();
                length_ = buffer_.size();
                tag_ = semantic_tag::bigint;
            }
            else
            {
                double d{0};
                result = jsoncons::decstr_to_double(&buffer_[0], buffer_.length(), d);
                if (JSONCONS_LIKELY(result))
                {
                    token_kind_ = generic_token_kind::double_value;
                    value_.double_value_ = d;
                    tag_ = semantic_tag{};
                }
                else if (result.ec == std::errc::result_out_of_range)
                {
                    token_kind_ = generic_token_kind::double_value;
                    value_.double_value_ = d;
                    tag_ = semantic_tag{};
                }
                else
                {
                    ec = json_errc::invalid_number;
                    more_ = false;
                    return;
                }
            }
        }
        more_ = false;
        auto res = try_after_value();
        if (!res)
        {
            ec = res.ec;
        }
    }

    from_json_result end_fraction_value()
    {
        if (lossless_number_)
        {
            token_kind_ = generic_token_kind::string_value;
            value_.string_data_ = buffer_.data();
            length_ = buffer_.size();
            tag_ = semantic_tag::bigdec;
        }
        else
        {
            double d{0};
            auto result = jsoncons::decstr_to_double(&buffer_[0], buffer_.length(), d);
            if (JSONCONS_LIKELY(result))
            {
                token_kind_ = generic_token_kind::double_value;
                value_.double_value_ = d;
                tag_ = semantic_tag{};
            }
            else if (result.ec == std::errc::result_out_of_range)
            {
                if (lossless_bignum_)
                {
                    token_kind_ = generic_token_kind::string_value;
                    value_.string_data_ = buffer_.data();
                    length_ = buffer_.size();
                    tag_ = semantic_tag::bigdec;
                }
                else
                {
                    token_kind_ = generic_token_kind::double_value;
                    value_.double_value_ = d;
                    tag_ = semantic_tag{};
                }
            }
            else
            {
                more_ = false;
                return from_json_result{json_errc::invalid_number};
            }
        }

        more_ = false;
        return try_after_value();
    }

    from_json_result end_string_value(const char_type* s, std::size_t length) 
    {
        string_view_type sv(s, length);
        auto result = unicode_traits::validate(s, length);
        if (result.ec != unicode_traits::conv_errc())
        {
            more_ = false;
            position_ += (result.ptr - s);
            return from_json_result{translate_conv_errc(result.ec)};
        }
        switch (parent())
        {
            case parse_state::member_name:
                token_kind_ = generic_token_kind::string_value;
                value_.string_data_ = s;
                length_ = length;
                tag_ = semantic_tag{};
                more_ = false;
                pop_state();
                state_ = parse_state::expect_colon;
                return from_json_result{};
            case parse_state::object:
            case parse_state::array:
            {
                auto it = std::find_if(string_double_map_.begin(), string_double_map_.end(), string_maps_to_double{ sv });
                if (it != string_double_map_.end())
                {
                    token_kind_ = generic_token_kind::double_value;
                    value_.double_value_ = (*it).second;
                    tag_ = semantic_tag{};
                    more_ = false;
                }
                else
                {
                    token_kind_ = generic_token_kind::string_value;
                    value_.string_data_ = sv.data();
                    length_ = sv.size();
                    tag_ = escape_tag_;
                    more_ = false;
                }
                state_ = parse_state::expect_comma_or_end;
                return from_json_result{};
            }
            case parse_state::root:
            {
                auto it = std::find_if(string_double_map_.begin(),string_double_map_.end(),string_maps_to_double{sv});
                if (it != string_double_map_.end())
                {
                    token_kind_ = generic_token_kind::double_value;
                    value_.double_value_ = (*it).second;
                    tag_ = semantic_tag{};
                    more_ = false;
                }
                else
                {
                    token_kind_ = generic_token_kind::string_value;
                    value_.string_data_ = sv.data();
                    length_ = sv.size();
                    tag_ = escape_tag_;
                    more_ = false;
                }
                state_ = parse_state::accept;
                return from_json_result{};
            }
            default:
                more_ = false;
                return from_json_result{json_errc::syntax_error};
        }
    }

    from_json_result try_begin_member_or_element() 
    {
        switch (parent())
        {
            case parse_state::object:
                state_ = parse_state::expect_member_name;
                return from_json_result{};
            case parse_state::array:
                state_ = parse_state::expect_value;
                return from_json_result{};
            case parse_state::root:
                return from_json_result{};
            default:
                return from_json_result{json_errc::syntax_error};
        }
    }

    from_json_result try_after_value() 
    {
        switch (parent())
        {
            case parse_state::array:
            case parse_state::object:
                state_ = parse_state::expect_comma_or_end;
                return from_json_result{};
            case parse_state::root:
                state_ = parse_state::accept;
                return from_json_result{};
            default:
                return from_json_result{json_errc::syntax_error};
        }
    }

    void push_state(parse_state state)
    {
        state_stack_.push_back(state);
        //std::cout << "max_nesting_depth: " << max_nesting_depth_ << ", capacity: " << state_stack_.capacity() << ", nesting_depth: " << level_ << ", stack size: " << state_stack_.size() << "\n";
    }

    parse_state pop_state()
    {
        JSONCONS_ASSERT(!state_stack_.empty())
        parse_state state = state_stack_.back();
        state_stack_.pop_back();
        return state;
    }
 
    static from_json_result try_append_to_codepoint(uint32_t& cp, int c)
    {
        cp *= 16;
        if (c >= '0'  &&  c <= '9')
        {
            cp += c - '0';
            return from_json_result{};
        }
        if (c >= 'a'  &&  c <= 'f')
        {
            cp += c - 'a' + 10;
            return from_json_result{};
        }
        if (c >= 'A'  &&  c <= 'F')
        {
            cp += c - 'A' + 10;
            return from_json_result{};
        }
        return from_json_result{json_errc::invalid_unicode_escape_sequence};
    }
};

using json_tokenizer = basic_json_tokenizer<char>;
using wjson_tokenizer = basic_json_tokenizer<wchar_t>;

} // namespace jsoncons

#endif


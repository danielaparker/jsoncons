// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_PARSER_HPP
#define JSONCONS_JSON_PARSER_HPP

#include <memory> // std::allocator
#include <string>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <unordered_map>
#include <limits> // std::numeric_limits
#include <functional> // std::function
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/json_options.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/json_error.hpp>
#include <jsoncons/chunk_reader.hpp>
#include <jsoncons/detail/parse_number.hpp>

#define JSONCONS_ILLEGAL_CONTROL_CHARACTER \
        case 0x00:case 0x01:case 0x02:case 0x03:case 0x04:case 0x05:case 0x06:case 0x07:case 0x08:case 0x0b: \
        case 0x0c:case 0x0e:case 0x0f:case 0x10:case 0x11:case 0x12:case 0x13:case 0x14:case 0x15:case 0x16: \
        case 0x17:case 0x18:case 0x19:case 0x1a:case 0x1b:case 0x1c:case 0x1d:case 0x1e:case 0x1f 

namespace jsoncons {

enum class json_parse_state : uint8_t 
{
    root,
    start, 
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
    member_name,
    cr,
    done
};

template <typename CharT,typename TempAllocator  = std::allocator<char>>
class basic_json_parser : public ser_context, public virtual basic_parser_input<CharT> 
{
public:
    using char_type = CharT;
    using string_view_type = typename basic_json_visitor<CharT>::string_view_type;
    using chunk_reader_type = std::function<bool(basic_parser_input<CharT>& input, std::error_code& ec)>;
private:

    struct string_maps_to_double
    {
        string_view_type s;

        bool operator()(const std::pair<string_view_type,double>& val) const
        {
            return val.first == s;
        }
    };

    using temp_allocator_type = TempAllocator;
    using char_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<CharT>;
    using parse_state_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<json_parse_state>;

    static constexpr std::size_t initial_string_buffer_capacity = 256;
    static constexpr int default_initial_stack_capacity = 66;

    basic_json_decode_options<char_type> options_;

    std::function<bool(json_errc,const ser_context&)> err_handler_;
    int nesting_depth_;
    uint32_t cp_;
    uint32_t cp2_;
    std::size_t line_;
    std::size_t position_;
    std::size_t mark_position_;
    std::size_t saved_position_;
    const char_type* begin_input_;
    const char_type* input_end_;
    const char_type* input_ptr_;
    json_parse_state state_;
    bool more_;
    bool done_;

    std::basic_string<char_type,std::char_traits<char_type>,char_allocator_type> string_buffer_;
    jsoncons::detail::chars_to to_double_;

    std::vector<json_parse_state,parse_state_allocator_type> state_stack_;
    std::vector<std::pair<std::basic_string<char_type>,double>> string_double_map_;

    chunk_reader_adaptor<char_type> chk_rdr_;
    chunk_reader<char_type>* chunk_rdr_;
    
    // Noncopyable and nonmoveable
    basic_json_parser(const basic_json_parser&) = delete;
    basic_json_parser& operator=(const basic_json_parser&) = delete;

public:
    basic_json_parser(const TempAllocator& temp_alloc = TempAllocator())
        : basic_json_parser(basic_json_decode_options<char_type>(), default_json_parsing(), temp_alloc)
    {
    }

    basic_json_parser(chunk_reader_type chunk_rdr, const TempAllocator& temp_alloc = TempAllocator())
        : basic_json_parser(chunk_rdr, basic_json_decode_options<char_type>(), default_json_parsing(), 
            temp_alloc)
    {
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    basic_json_parser(std::function<bool(json_errc,const ser_context&)> err_handler, 
        const TempAllocator& temp_alloc = TempAllocator())
        : basic_json_parser(basic_json_decode_options<char_type>(), err_handler, temp_alloc)
    {
    }
#endif
    basic_json_parser(const basic_json_decode_options<CharT>& options, 
        const TempAllocator& temp_alloc = TempAllocator())
        : basic_json_parser(options, options.err_handler(), temp_alloc)
    {
    }

    basic_json_parser(chunk_reader_type chunk_rdr, const basic_json_decode_options<CharT>& options, 
        const TempAllocator& temp_alloc = TempAllocator())
        : basic_json_parser(chunk_rdr, options, options.err_handler(), temp_alloc)
    {
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    basic_json_parser(const basic_json_decode_options<char_type>& options,
        std::function<bool(json_errc,const ser_context&)> err_handler, 
        const TempAllocator& temp_alloc = TempAllocator())
       : basic_json_parser(&chk_rdr_, options, err_handler, temp_alloc)
    {
    }
#endif
    basic_json_parser(chunk_reader<char_type>* chunk_rdr,
        const basic_json_decode_options<char_type>& options,
        std::function<bool(json_errc,const ser_context&)> err_handler, 
        const TempAllocator& temp_alloc = TempAllocator())
       : options_(options),
         err_handler_(err_handler),
         nesting_depth_(0), 
         cp_(0),
         cp2_(0),
         line_(1),
         position_(0),
         mark_position_(0),
         saved_position_(0),
         begin_input_(nullptr),
         input_end_(nullptr),
         input_ptr_(nullptr),
         state_(json_parse_state::start),
         more_(true),
         done_(false),
         string_buffer_(temp_alloc),
         state_stack_(temp_alloc),
         chk_rdr_{},
         chunk_rdr_(chunk_rdr)
    {
        string_buffer_.reserve(initial_string_buffer_capacity);

        std::size_t initial_stack_capacity = options.max_nesting_depth() <= (default_initial_stack_capacity-2) ? (options.max_nesting_depth()+2) : default_initial_stack_capacity;
        state_stack_.reserve(initial_stack_capacity );
        push_state(json_parse_state::root);

        if (options_.enable_str_to_nan())
        {
            string_double_map_.emplace_back(options_.nan_to_str(),std::nan(""));
        }
        if (options_.enable_str_to_inf())
        {
            string_double_map_.emplace_back(options_.inf_to_str(),std::numeric_limits<double>::infinity());
        }
        if (options_.enable_str_to_neginf())
        {
            string_double_map_.emplace_back(options_.neginf_to_str(),-std::numeric_limits<double>::infinity());
        }
    }

    basic_json_parser(chunk_reader_type chunk_rdr,
        const basic_json_decode_options<char_type>& options,
        std::function<bool(json_errc,const ser_context&)> err_handler, 
        const TempAllocator& temp_alloc = TempAllocator())
       : options_(options),
         err_handler_(err_handler),
         nesting_depth_(0), 
         cp_(0),
         cp2_(0),
         line_(1),
         position_(0),
         mark_position_(0),
         saved_position_(0),
         begin_input_(nullptr),
         input_end_(nullptr),
         input_ptr_(nullptr),
         state_(json_parse_state::start),
         more_(true),
         done_(false),
         string_buffer_(temp_alloc),
         state_stack_(temp_alloc),
         chk_rdr_(chunk_rdr),
         chunk_rdr_(&chk_rdr_)
    {
        string_buffer_.reserve(initial_string_buffer_capacity);

        std::size_t initial_stack_capacity = options.max_nesting_depth() <= (default_initial_stack_capacity-2) ? (options.max_nesting_depth()+2) : default_initial_stack_capacity;
        state_stack_.reserve(initial_stack_capacity );
        push_state(json_parse_state::root);

        if (options_.enable_str_to_nan())
        {
            string_double_map_.emplace_back(options_.nan_to_str(),std::nan(""));
        }
        if (options_.enable_str_to_inf())
        {
            string_double_map_.emplace_back(options_.inf_to_str(),std::numeric_limits<double>::infinity());
        }
        if (options_.enable_str_to_neginf())
        {
            string_double_map_.emplace_back(options_.neginf_to_str(),-std::numeric_limits<double>::infinity());
        }
    }

    bool source_exhausted() const
    {
        return input_ptr_ == input_end_;
    }

    ~basic_json_parser() noexcept
    {
    }

    json_parse_state parent() const
    {
        JSONCONS_ASSERT(state_stack_.size() >= 1);
        return state_stack_.back();
    }

    bool done() const
    {
        return done_;
    }

    bool enter() const
    {
        return state_ == json_parse_state::start;
    }

    bool accept() const
    {
        return state_ == json_parse_state::accept || done_;
    }

    bool stopped() const
    {
        return !more_;
    }

    json_parse_state state() const
    {
        return state_;
    }

    bool finished() const
    {
        return !more_ && state_ != json_parse_state::accept;
    }

    const char_type* first() const
    {
        return begin_input_;
    }

    const char_type* current() const
    {
        return input_ptr_;
    }

    const char_type* last() const
    {
        return input_end_;
    }

    void skip_space(std::error_code& ec)
    {
        const char_type* local_input_end = input_end_;
        while (true) 
        {
            if (input_ptr_ == local_input_end)
            {
                if (!chunk_rdr_->read_chunk(*this, ec))
                {
                    break;
                }
                local_input_end = input_end_;
            }
            switch (*input_ptr_)
            {
                case ' ':
                case '\t':
                    ++input_ptr_;
                    ++position_;
                    break;
                case '\r': 
                    push_state(state_);
                    ++input_ptr_;
                    ++position_;
                    state_ = json_parse_state::cr;
                    return; 
                case '\n': 
                    ++input_ptr_;
                    ++line_;
                    ++position_;
                    mark_position_ = position_;
                    return;   
                default:
                    return;
            }
        }
    }

    void skip_whitespace(std::error_code& ec)
    {
        const char_type* local_input_end = input_end_;

        while (true) 
        {
            if (input_ptr_ == local_input_end)
            {
                if (!chunk_rdr_->read_chunk(*this, ec))
                {
                    break;
                }
                local_input_end = input_end_;
            }
            switch (state_)
            {
                case json_parse_state::cr:
                    ++line_;
                    ++position_;
                    mark_position_ = position_;
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
                    break;

                default:
                    switch (*input_ptr_)
                    {
                        case ' ':
                        case '\t':
                        case '\n':
                        case '\r':
                            skip_space(ec);
                            break;
                        default:
                            return;
                    }
                    break;
            }
        }
    }

    void begin_object(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(++nesting_depth_ > options_.max_nesting_depth()))
        {
            more_ = err_handler_(json_errc::max_nesting_depth_exceeded, *this);
            if (!more_)
            {
                ec = json_errc::max_nesting_depth_exceeded;
                return;
            }
        } 

        push_state(json_parse_state::object);
        state_ = json_parse_state::expect_member_name_or_end;
        more_ = visitor.begin_object(semantic_tag::none, *this, ec);
    }

    void end_object(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(nesting_depth_ < 1))
        {
            err_handler_(json_errc::unexpected_rbrace, *this);
            ec = json_errc::unexpected_rbrace;
            more_ = false;
            return;
        }
        --nesting_depth_;
        state_ = pop_state();
        if (state_ == json_parse_state::object)
        {
            more_ = visitor.end_object(*this, ec);
        }
        else if (state_ == json_parse_state::array)
        {
            err_handler_(json_errc::expected_comma_or_rbracket, *this);
            ec = json_errc::expected_comma_or_rbracket;
            more_ = false;
            return;
        }
        else
        {
            err_handler_(json_errc::unexpected_rbrace, *this);
            ec = json_errc::unexpected_rbrace;
            more_ = false;
            return;
        }

        if (parent() == json_parse_state::root)
        {
            state_ = json_parse_state::accept;
        }
        else
        {
            state_ = json_parse_state::expect_comma_or_end;
        }
    }

    void begin_array(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        if (++nesting_depth_ > options_.max_nesting_depth())
        {
            more_ = err_handler_(json_errc::max_nesting_depth_exceeded, *this);
            if (!more_)
            {
                ec = json_errc::max_nesting_depth_exceeded;
                return;
            }
        }

        push_state(json_parse_state::array);
        state_ = json_parse_state::expect_value_or_end;
        more_ = visitor.begin_array(semantic_tag::none, *this, ec);
    }

    void end_array(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        if (nesting_depth_ < 1)
        {
            err_handler_(json_errc::unexpected_rbracket, *this);
            ec = json_errc::unexpected_rbracket;
            more_ = false;
            return;
        }
        --nesting_depth_;
        state_ = pop_state();
        if (state_ == json_parse_state::array)
        {
            more_ = visitor.end_array(*this, ec);
        }
        else if (state_ == json_parse_state::object)
        {
            err_handler_(json_errc::expected_comma_or_rbrace, *this);
            ec = json_errc::expected_comma_or_rbrace;
            more_ = false;
            return;
        }
        else
        {
            err_handler_(json_errc::unexpected_rbracket, *this);
            ec = json_errc::unexpected_rbracket;
            more_ = false;
            return;
        }
        if (parent() == json_parse_state::root)
        {
            state_ = json_parse_state::accept;
        }
        else
        {
            state_ = json_parse_state::expect_comma_or_end;
        }
    }

    void reinitialize()
    {
        reset();
        cp_ = 0;
        cp2_ = 0;
        saved_position_ = 0;
        begin_input_ = nullptr;
        input_end_ = nullptr;
        input_ptr_ = nullptr;
        string_buffer_.clear();
    }

    void reset()
    {
        state_stack_.clear();
        push_state(json_parse_state::root);
        state_ = json_parse_state::start;
        more_ = true;
        done_ = false;
        line_ = 1;
        position_ = 0;
        mark_position_ = 0;
        nesting_depth_ = 0;
    }

    void restart()
    {
        more_ = true;
    }

    void check_done()
    {
        std::error_code ec;
        check_done(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,line_,column()));
        }
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
                    more_ = err_handler_(json_errc::extra_character, *this);
                    if (!more_)
                    {
                        ec = json_errc::extra_character;
                        return;
                    }
                    break;
            }
        }
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    JSONCONS_DEPRECATED_MSG("Instead, use set_buffer once or provide a chunk reader")
    void update(string_view_type sv)
    {
        update(sv.data(),sv.length());
    }

    JSONCONS_DEPRECATED_MSG("Instead, use set_buffer once or provide a chunk reader")
    void update(const char_type* data, std::size_t length)
    {
        begin_input_ = data;
        input_end_ = data + length;
        input_ptr_ = begin_input_;
    }
#endif

    void set_buffer(const char_type* data, std::size_t length) final
    {
        begin_input_ = data;
        input_end_ = data + length;
        input_ptr_ = begin_input_;
    }

    void parse_some(basic_json_visitor<char_type>& visitor)
    {
        std::error_code ec;
        parse_some(visitor, ec); 
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,line_,column()));
        }
    }

    void parse_some(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        parse_some_(visitor, ec);
    }

    void finish_parse(basic_json_visitor<char_type>& visitor)
    {
        std::error_code ec;
        finish_parse(visitor, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec,line_,column()));
        }
    }

    void finish_parse(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        while (!finished())
        {
            parse_some(visitor, ec);
        }
    }

    void parse_some_(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        if (state_ == json_parse_state::accept)
        {
            visitor.flush();
            done_ = true;
            state_ = json_parse_state::done;
            more_ = false;
            return;
        }

        if (input_ptr_ == input_end_ && more_)
        {
            if (input_ptr_ == input_end_)
            {
                chunk_rdr_->read_chunk(*this, ec);
                if (ec)
                {
                    return;
                }
            }
            if (input_ptr_ == input_end_)
            {
                switch (state_)
                {
                    case json_parse_state::accept:
                        visitor.flush();
                        done_ = true;
                        state_ = json_parse_state::done;
                        more_ = false;
                        break;
                    case json_parse_state::start:
                    case json_parse_state::done:
                        more_ = false;
                        break;
                    case json_parse_state::cr:
                        state_ = pop_state();
                        break;
                    default:
                        err_handler_(json_errc::unexpected_eof, *this);
                        ec = json_errc::unexpected_eof;
                        more_ = false;
                        return;
                }
            }
        }

        while (more_)
        {
            if (input_ptr_ == input_end_)
            {
                if (!chunk_rdr_->read_chunk(*this, ec))
                {
                    break;
                }
            }
            
            switch (state_)
            {
                case json_parse_state::accept:
                    visitor.flush();
                    done_ = true;
                    state_ = json_parse_state::done;
                    more_ = false;
                    break;
                case json_parse_state::cr:
                    ++line_;
                    mark_position_ = position_;
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
                    break;
                case json_parse_state::start: 
                {
                    switch (*input_ptr_)
                    {
                        JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                            more_ = err_handler_(json_errc::illegal_control_character, *this);
                            if (!more_)
                            {
                                ec = json_errc::illegal_control_character;
                                return;
                            }
                            break;
                        case '\r': 
                            push_state(state_);
                            ++input_ptr_;
                            ++position_;
                            state_ = json_parse_state::cr;
                            break; 
                        case '\n': 
                            ++input_ptr_;
                            ++line_;
                            ++position_;
                            mark_position_ = position_;
                            break;   
                        case ' ':case '\t':
                            skip_space(ec);
                            break;
                        case '/': 
                            ++input_ptr_;
                            ++position_;
                            push_state(state_);
                            state_ = json_parse_state::slash;
                            break;
                        case '{':
                            saved_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            begin_object(visitor, ec);
                            if (ec) return;
                            break;
                        case '[':
                            saved_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            begin_array(visitor, ec);
                            if (ec) return;
                            break;
                        case '\"':
                            saved_position_ = position_;
                            ++input_ptr_;
                            ++position_;
                            string_buffer_.clear();
                            parse_string(visitor, ec);
                            if (ec) return;
                            break;
                        case '-':
                            string_buffer_.clear();
                            string_buffer_.push_back('-');
                            saved_position_ = position_;
                            parse_number(visitor, ec);
                            if (ec) {return;}
                            break;
                        case '0': 
                            string_buffer_.clear();
                            string_buffer_.push_back(static_cast<char>(*input_ptr_));
                            saved_position_ = position_;
                            parse_number(visitor, ec);
                            if (ec) {return;}
                            break;
                        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                            string_buffer_.clear();
                            string_buffer_.push_back(static_cast<char>(*input_ptr_));
                            saved_position_ = position_;
                            parse_number(visitor, ec);
                            if (ec) {return;}
                            break;
                        case 'n':
                            parse_null(visitor, ec);
                            if (ec) {return;}
                            break;
                        case 't':
                            parse_true(visitor, ec);
                            if (ec) {return;}
                            break;
                        case 'f':
                            parse_false(visitor, ec);
                            if (ec) {return;}
                            break;
                        case '}':
                            err_handler_(json_errc::unexpected_rbrace, *this);
                            ec = json_errc::unexpected_rbrace;
                            more_ = false;
                            return;
                        case ']':
                            err_handler_(json_errc::unexpected_rbracket, *this);
                            ec = json_errc::unexpected_rbracket;
                            more_ = false;
                            return;
                        default:
                            err_handler_(json_errc::syntax_error, *this);
                            ec = json_errc::syntax_error;
                            more_ = false;
                            return;
                    }
                    break;
                }

                case json_parse_state::expect_comma_or_end: 
                    {
                        switch (*input_ptr_)
                        {
                            JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                                more_ = err_handler_(json_errc::illegal_control_character, *this);
                                if (!more_)
                                {
                                    ec = json_errc::illegal_control_character;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                            case '\r': 
                                ++input_ptr_;
                                ++position_;
                                push_state(state_);
                                state_ = json_parse_state::cr;
                                break; 
                            case '\n': 
                                ++input_ptr_;
                                ++line_;
                                ++position_;
                                mark_position_ = position_;
                                break;   
                            case ' ':case '\t':
                                skip_space(ec);
                                break;
                            case '/':
                                ++input_ptr_;
                                ++position_;
                                push_state(state_); 
                                state_ = json_parse_state::slash;
                                break;
                            case '}':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                end_object(visitor, ec);
                                if (ec) return;
                                break;
                            case ']':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                end_array(visitor, ec);
                                if (ec) return;
                                break;
                            case ',':
                                begin_member_or_element(ec);
                                if (ec) return;
                                ++input_ptr_;
                                ++position_;
                                break;
                            default:
                                if (parent() == json_parse_state::array)
                                {
                                    more_ = err_handler_(json_errc::expected_comma_or_rbracket, *this);
                                    if (!more_)
                                    {
                                        ec = json_errc::expected_comma_or_rbracket;
                                        return;
                                    }
                                }
                                else if (parent() == json_parse_state::object)
                                {
                                    more_ = err_handler_(json_errc::expected_comma_or_rbrace, *this);
                                    if (!more_)
                                    {
                                        ec = json_errc::expected_comma_or_rbrace;
                                        return;
                                    }
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                        }
                    }
                    break;
                case json_parse_state::expect_member_name_or_end: 
                    {
                        switch (*input_ptr_)
                        {
                            JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                                more_ = err_handler_(json_errc::illegal_control_character, *this);
                                if (!more_)
                                {
                                    ec = json_errc::illegal_control_character;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                            case '\r': 
                                ++input_ptr_;
                                ++position_;
                                push_state(state_);
                                state_ = json_parse_state::cr;
                                break; 
                            case '\n': 
                                ++input_ptr_;
                                ++line_;
                                ++position_;
                                mark_position_ = position_;
                                break;   
                            case ' ':case '\t':
                                skip_space(ec);
                                break;
                            case '/':
                                ++input_ptr_;
                                ++position_;
                                push_state(state_); 
                                state_ = json_parse_state::slash;
                                break;
                            case '}':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                end_object(visitor, ec);
                                if (ec) return;
                                break;
                            case '\"':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                push_state(json_parse_state::member_name);
                                string_buffer_.clear();
                                parse_string(visitor, ec);
                                if (ec) return;
                                break;
                            case '\'':
                                more_ = err_handler_(json_errc::single_quote, *this);
                                if (!more_)
                                {
                                    ec = json_errc::single_quote;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                            default:
                                more_ = err_handler_(json_errc::expected_key, *this);
                                if (!more_)
                                {
                                    ec = json_errc::expected_key;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                        }
                    }
                    break;
                case json_parse_state::expect_member_name: 
                    {
                        switch (*input_ptr_)
                        {
                            JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                                more_ = err_handler_(json_errc::illegal_control_character, *this);
                                if (!more_)
                                {
                                    ec = json_errc::illegal_control_character;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                            case '\r': 
                                ++input_ptr_;
                                ++position_;
                                push_state(state_);
                                state_ = json_parse_state::cr;
                                break; 
                            case '\n': 
                                ++input_ptr_;
                                ++line_;
                                ++position_;
                                mark_position_ = position_;
                                break;   
                            case ' ':case '\t':
                                skip_space(ec);
                                break;
                            case '/': 
                                ++input_ptr_;
                                ++position_;
                                push_state(state_);
                                state_ = json_parse_state::slash;
                                break;
                            case '\"':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                push_state(json_parse_state::member_name);
                                string_buffer_.clear();
                                parse_string(visitor, ec);
                                if (ec) return;
                                break;
                            case '}':
                                more_ = err_handler_(json_errc::extra_comma, *this);
                                if (!more_)
                                {
                                    ec = json_errc::extra_comma;
                                    return;
                                }
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                end_object(visitor, ec);  // Recover
                                if (ec) return;
                                break;
                            case '\'':
                                more_ = err_handler_(json_errc::single_quote, *this);
                                if (!more_)
                                {
                                    ec = json_errc::single_quote;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                            default:
                                more_ = err_handler_(json_errc::expected_key, *this);
                                if (!more_)
                                {
                                    ec = json_errc::expected_key;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                        }
                    }
                    break;
                case json_parse_state::expect_colon: 
                    {
                        switch (*input_ptr_)
                        {
                            JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                                more_ = err_handler_(json_errc::illegal_control_character, *this);
                                if (!more_)
                                {
                                    ec = json_errc::illegal_control_character;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                            case '\r': 
                                push_state(state_);
                                state_ = json_parse_state::cr;
                                ++input_ptr_;
                                ++position_;
                                break; 
                            case '\n': 
                                ++input_ptr_;
                                ++line_;
                                ++position_;
                                mark_position_ = position_;
                                break;   
                            case ' ':case '\t':
                                skip_space(ec);
                                break;
                            case '/': 
                                push_state(state_);
                                state_ = json_parse_state::slash;
                                ++input_ptr_;
                                ++position_;
                                break;
                            case ':':
                                state_ = json_parse_state::expect_value;
                                ++input_ptr_;
                                ++position_;
                                break;
                            default:
                                more_ = err_handler_(json_errc::expected_colon, *this);
                                if (!more_)
                                {
                                    ec = json_errc::expected_colon;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                        }
                    }
                    break;

                    case json_parse_state::expect_value: 
                    {
                        switch (*input_ptr_)
                        {
                            JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                                more_ = err_handler_(json_errc::illegal_control_character, *this);
                                if (!more_)
                                {
                                    ec = json_errc::illegal_control_character;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                            case '\r': 
                                push_state(state_);
                                ++input_ptr_;
                                ++position_;
                                state_ = json_parse_state::cr;
                                break; 
                            case '\n': 
                                ++input_ptr_;
                                ++line_;
                                ++position_;
                                mark_position_ = position_;
                                break;   
                            case ' ':case '\t':
                                skip_space(ec);
                                break;
                            case '/': 
                                push_state(state_);
                                ++input_ptr_;
                                ++position_;
                                state_ = json_parse_state::slash;
                                break;
                            case '{':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                begin_object(visitor, ec);
                                if (ec) return;
                                break;
                            case '[':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                begin_array(visitor, ec);
                                if (ec) return;
                                break;
                            case '\"':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                string_buffer_.clear();
                                parse_string(visitor, ec);
                                if (ec) return;
                                break;
                            case '-':
                                string_buffer_.clear();
                                string_buffer_.push_back('-');
                                saved_position_ = position_;
                                parse_number(visitor, ec);
                                if (ec) {return;}
                                break;
                            case '0': 
                                string_buffer_.clear();
                                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                                saved_position_ = position_;
                                parse_number(visitor, ec);
                                if (ec) {return;}
                                break;
                            case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                                string_buffer_.clear();
                                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                                saved_position_ = position_;
                                parse_number(visitor, ec);
                                if (ec) {return;}
                                break;
                            case 'n':
                                parse_null(visitor, ec);
                                if (ec) {return;}
                                break;
                            case 't':
                                parse_true(visitor, ec);
                                if (ec) {return;}
                                break;
                            case 'f':
                                parse_false(visitor, ec);
                                if (ec) {return;}
                                break;
                            case ']':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                if (parent() == json_parse_state::array)
                                {
                                    more_ = err_handler_(json_errc::extra_comma, *this);
                                    if (!more_)
                                    {
                                        ec = json_errc::extra_comma;
                                        return;
                                    }
                                    end_array(visitor, ec);  // Recover
                                    if (ec) return;
                                }
                                else
                                {
                                    more_ = err_handler_(json_errc::expected_value, *this);
                                    if (!more_)
                                    {
                                        ec = json_errc::expected_value;
                                        return;
                                    }
                                }
                                break;
                            case '\'':
                                more_ = err_handler_(json_errc::single_quote, *this);
                                if (!more_)
                                {
                                    ec = json_errc::single_quote;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                            default:
                                more_ = err_handler_(json_errc::expected_value, *this);
                                if (!more_)
                                {
                                    ec = json_errc::expected_value;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                        }
                    }
                    break;
                    case json_parse_state::expect_value_or_end: 
                    {
                        switch (*input_ptr_)
                        {
                            JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                                more_ = err_handler_(json_errc::illegal_control_character, *this);
                                if (!more_)
                                {
                                    ec = json_errc::illegal_control_character;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                            case '\r': 
                                ++input_ptr_;
                                ++position_;
                                push_state(state_);
                                state_ = json_parse_state::cr;
                                break; 
                            case '\n': 
                                ++input_ptr_;
                                ++line_;
                                ++position_;
                                mark_position_ = position_;
                                break;   
                            case ' ':case '\t':
                                skip_space(ec);
                                break;
                            case '/': 
                                ++input_ptr_;
                                ++position_;
                                push_state(state_);
                                state_ = json_parse_state::slash;
                                break;
                            case '{':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                begin_object(visitor, ec);
                                if (ec) return;
                                break;
                            case '[':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                begin_array(visitor, ec);
                                if (ec) return;
                                break;
                            case ']':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                end_array(visitor, ec);
                                if (ec) return;
                                break;
                            case '\"':
                                saved_position_ = position_;
                                ++input_ptr_;
                                ++position_;
                                string_buffer_.clear();
                                parse_string(visitor, ec);
                                if (ec) return;
                                break;
                            case '-':
                                string_buffer_.clear();
                                string_buffer_.push_back('-');
                                saved_position_ = position_;
                                parse_number(visitor, ec);
                                if (ec) {return;}
                                break;
                            case '0': 
                                string_buffer_.clear();
                                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                                saved_position_ = position_;
                                parse_number(visitor, ec);
                                if (ec) {return;}
                                break;
                            case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                                string_buffer_.clear();
                                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                                saved_position_ = position_;
                                parse_number(visitor, ec);
                                if (ec) {return;}
                                break;
                            case 'n':
                                parse_null(visitor, ec);
                                if (ec) {return;}
                                break;
                            case 't':
                                parse_true(visitor, ec);
                                if (ec) {return;}
                                break;
                            case 'f':
                                parse_false(visitor, ec);
                                if (ec) {return;}
                                break;
                            case '\'':
                                more_ = err_handler_(json_errc::single_quote, *this);
                                if (!more_)
                                {
                                    ec = json_errc::single_quote;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                            default:
                                more_ = err_handler_(json_errc::expected_value, *this);
                                if (!more_)
                                {
                                    ec = json_errc::expected_value;
                                    return;
                                }
                                ++input_ptr_;
                                ++position_;
                                break;
                            }
                        }
                    break;
                case json_parse_state::slash: 
                {
                    switch (*input_ptr_)
                    {
                    case '*':
                        state_ = json_parse_state::slash_star;
                        more_ = err_handler_(json_errc::illegal_comment, *this);
                        if (!more_)
                        {
                            ec = json_errc::illegal_comment;
                            return;
                        }
                        break;
                    case '/':
                        state_ = json_parse_state::slash_slash;
                        more_ = err_handler_(json_errc::illegal_comment, *this);
                        if (!more_)
                        {
                            ec = json_errc::illegal_comment;
                            return;
                        }
                        break;
                    default:    
                        more_ = err_handler_(json_errc::syntax_error, *this);
                        if (!more_)
                        {
                            ec = json_errc::syntax_error;
                            return;
                        }
                        break;
                    }
                    ++input_ptr_;
                    ++position_;
                    break;
                }
                case json_parse_state::slash_star:  
                {
                    switch (*input_ptr_)
                    {
                        case '\r':
                            push_state(state_);
                            ++input_ptr_;
                            ++position_;
                            state_ = json_parse_state::cr;
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
                            state_ = json_parse_state::slash_star_star;
                            break;
                        default:
                            ++input_ptr_;
                            ++position_;
                            break;
                    }
                    break;
                }
                case json_parse_state::slash_slash: 
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
                case json_parse_state::slash_star_star: 
                {
                    switch (*input_ptr_)
                    {
                    case '/':
                        state_ = pop_state();
                        break;
                    default:    
                        state_ = json_parse_state::slash_star;
                        break;
                    }
                    ++input_ptr_;
                    ++position_;
                    break;
                }
                default:
                    JSONCONS_ASSERT(false);
                    break;
            }
        }
    }

    void parse_null(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        static const char_type pat[] = {'n','u','l','l'};

        saved_position_ = position_;

        std::ptrdiff_t n = input_end_ - input_ptr_;
        if (JSONCONS_UNLIKELY(n < 4))
        {
            std::ptrdiff_t m = 4 - n;
            int matches = std::memcmp(input_ptr_+1, pat+1, (n-1)*sizeof(char_type));
            input_ptr_ += n;
            position_ += n;
            if (matches != 0)
            {
                ec = json_errc::invalid_value;
                more_ = false;
            }
            else
            {
                if (!chunk_rdr_->read_chunk(*this, ec) || (input_end_ - input_ptr_) < m)
                {
                    ec = json_errc::invalid_value;
                    more_ = false;
                }
                else
                {
                    matches = std::memcmp(input_ptr_, pat+n, m*sizeof(char_type));
                    input_ptr_ += m;
                    position_ += m;
                    if (matches != 0)
                    {
                        ec = json_errc::invalid_value;
                        more_ = false;
                    }
                    more_ = visitor.null_value(semantic_tag::none, *this, ec);
                    if (parent() == json_parse_state::root)
                    {
                        state_ = json_parse_state::accept;
                    }
                    else
                    {
                        state_ = json_parse_state::expect_comma_or_end;
                    }
                }
            }
        }
        else
        {
            if (*(input_ptr_+1) == 'u' && *(input_ptr_+2) == 'l' && *(input_ptr_+3) == 'l')
            {
                input_ptr_ += 4;
                position_ += 4;
                more_ = visitor.null_value(semantic_tag::none, *this, ec);
                if (parent() == json_parse_state::root)
                {
                    state_ = json_parse_state::accept;
                }
                else
                {
                    state_ = json_parse_state::expect_comma_or_end;
                }
            }
            else
            {
                ec = json_errc::invalid_value;
                more_ = false;
            }
        }
    }

    void parse_true(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        static const char_type pat[] = {'t','r','u','e'};

        saved_position_ = position_;

        std::ptrdiff_t n = input_end_ - input_ptr_;
        if (JSONCONS_UNLIKELY(n < 4))
        {
            std::ptrdiff_t m = 4 - n;
            int matches = std::memcmp(input_ptr_+1, pat+1, (n-1)*sizeof(char_type));
            input_ptr_ += n;
            position_ += n;
            if (matches != 0)
            {
                ec = json_errc::invalid_value;
                more_ = false;
            }
            else
            {
                if (!chunk_rdr_->read_chunk(*this, ec) || (input_end_ - input_ptr_) < m)
                {
                    ec = json_errc::invalid_value;
                    more_ = false;
                }
                else
                {
                    matches = std::memcmp(input_ptr_, pat+n, m*sizeof(char_type));
                    input_ptr_ += m;
                    position_ += m;
                    if (matches != 0)
                    {
                        ec = json_errc::invalid_value;
                        more_ = false;
                    }
                    more_ = visitor.bool_value(true, semantic_tag::none, *this, ec);
                    if (parent() == json_parse_state::root)
                    {
                        state_ = json_parse_state::accept;
                    }
                    else
                    {
                        state_ = json_parse_state::expect_comma_or_end;
                    }
                }
            }
        }
        else
        {
            if (*(input_ptr_+1) == 'r' && *(input_ptr_+2) == 'u' && *(input_ptr_+3) == 'e')
            {
                input_ptr_ += 4;
                position_ += 4;
                more_ = visitor.bool_value(true, semantic_tag::none, *this, ec);
                if (parent() == json_parse_state::root)
                {
                    state_ = json_parse_state::accept;
                }
                else
                {
                    state_ = json_parse_state::expect_comma_or_end;
                }
            }
            else
            {
                ec = json_errc::invalid_value;
                more_ = false;
            }
        }
    }

    void parse_false(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        static const char_type pat[] = {'f','a','l','s','e'};
            
        saved_position_ = position_;
        
        std::ptrdiff_t n = input_end_ - input_ptr_;
        if (JSONCONS_UNLIKELY(n < 5))
        {
            std::ptrdiff_t m = 5 - n;
            int matches = std::memcmp(input_ptr_+1, pat+1, (n-1)*sizeof(char_type));
            input_ptr_ += n;
            position_ += n;
            if (matches != 0)
            {
                ec = json_errc::invalid_value;
                more_ = false;
            }
            else
            {
                if (!chunk_rdr_->read_chunk(*this, ec) || (input_end_ - input_ptr_) < m)
                {
                    ec = json_errc::invalid_value;
                    more_ = false;
                }
                else
                {
                    matches = std::memcmp(input_ptr_, pat+n, m*sizeof(char_type));
                    input_ptr_ += m;
                    position_ += m;
                    if (matches != 0)
                    {
                        ec = json_errc::invalid_value;
                        more_ = false;
                    }
                    more_ = visitor.bool_value(false, semantic_tag::none, *this, ec);
                    if (parent() == json_parse_state::root)
                    {
                        state_ = json_parse_state::accept;
                    }
                    else
                    {
                        state_ = json_parse_state::expect_comma_or_end;
                    }
                }
            }
        }
        else
        {
            if (*(input_ptr_+1) == 'a' && *(input_ptr_+2) == 'l' && *(input_ptr_+3) == 's' && *(input_ptr_+4) == 'e')
            {
                input_ptr_ += 5;
                position_ += 5;
                more_ = visitor.bool_value(false, semantic_tag::none, *this, ec);
                if (parent() == json_parse_state::root)
                {
                    state_ = json_parse_state::accept;
                }
                else
                {
                    state_ = json_parse_state::expect_comma_or_end;
                }
            }
            else
            {
                ec = json_errc::invalid_value;
                more_ = false;
            }
        }
    }

    void parse_number(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        const char_type* local_input_end = input_end_;
        
        if (*input_ptr_ == '-')
        {
            ++input_ptr_;
            ++position_;
            goto minus_sign;
        }
        else if (*input_ptr_ == '0')
        {
            ++input_ptr_;
            ++position_;
            goto zero;
        }
        else
        {
            ++input_ptr_;
            ++position_;
            goto integer;
        }
minus_sign:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
        }
        switch (*input_ptr_)
        {
            case '0': 
                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++position_;
                goto zero;
            case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++position_;
                goto integer;
            default:
                err_handler_(json_errc::invalid_number, *this);
                ec = json_errc::expected_value;
                more_ = false;
                return;
        }
zero:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                end_integer_value(visitor, ec);
                return;
            }
            local_input_end = input_end_;
        }
        switch (*input_ptr_)
        {
            case '\r': 
                end_integer_value(visitor, ec);
                if (ec) return;
                ++input_ptr_;
                ++position_;
                push_state(state_);
                state_ = json_parse_state::cr;
                return; 
            case '\n': 
                end_integer_value(visitor, ec);
                if (ec) return;
                ++input_ptr_;
                ++line_;
                ++position_;
                mark_position_ = position_;
                return;   
            case ' ':case '\t':
                end_integer_value(visitor, ec);
                if (ec) return;
                skip_space(ec);
                return;
            case '/': 
                end_integer_value(visitor, ec);
                if (ec) return;
                ++input_ptr_;
                ++position_;
                push_state(state_);
                state_ = json_parse_state::slash;
                return;
            case '}':
            case ']':
                end_integer_value(visitor, ec);
                if (ec) return;
                state_ = json_parse_state::expect_comma_or_end;
                return;
            case '.':
                string_buffer_.push_back(to_double_.get_decimal_point());
                ++input_ptr_;
                ++position_;
                goto fraction1;
            case 'e':case 'E':
                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++position_;
                goto exp1;
            case ',':
                end_integer_value(visitor, ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++input_ptr_;
                ++position_;
                return;
            case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                err_handler_(json_errc::leading_zero, *this);
                ec = json_errc::leading_zero;
                more_ = false;
                return;
            default:
                err_handler_(json_errc::invalid_number, *this);
                ec = json_errc::invalid_number;
                more_ = false;
                return;
        }
integer:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                end_integer_value(visitor, ec);
                return;
            }
            local_input_end = input_end_;
        }
        switch (*input_ptr_)
        {
            case '\r': 
                end_integer_value(visitor, ec);
                if (ec) return;
                push_state(state_);
                ++input_ptr_;
                ++position_;
                state_ = json_parse_state::cr;
                return; 
            case '\n': 
                end_integer_value(visitor, ec);
                if (ec) return;
                ++input_ptr_;
                ++line_;
                ++position_;
                mark_position_ = position_;
                return;   
            case ' ':case '\t':
                end_integer_value(visitor, ec);
                if (ec) return;
                skip_space(ec);
                return;
            case '/': 
                end_integer_value(visitor, ec);
                if (ec) return;
                push_state(state_);
                ++input_ptr_;
                ++position_;
                state_ = json_parse_state::slash;
                return;
            case '}':
            case ']':
                end_integer_value(visitor, ec);
                if (ec) return;
                state_ = json_parse_state::expect_comma_or_end;
                return;
            case '0': case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++position_;
                goto integer;
            case '.':
                string_buffer_.push_back(to_double_.get_decimal_point());
                ++input_ptr_;
                ++position_;
                goto fraction1;
            case 'e':case 'E':
                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++position_;
                goto exp1;
            case ',':
                end_integer_value(visitor, ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++input_ptr_;
                ++position_;
                return;
            default:
                err_handler_(json_errc::invalid_number, *this);
                ec = json_errc::invalid_number;
                more_ = false;
                return;
        }
fraction1:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
        }
        switch (*input_ptr_)
        {
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++position_;
                goto fraction2;
            default:
                err_handler_(json_errc::invalid_number, *this);
                ec = json_errc::invalid_number;
                more_ = false;
                return;
        }
fraction2:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                end_fraction_value(visitor, ec);
                return;
            }
            local_input_end = input_end_;
        }
        switch (*input_ptr_)
        {
            case '\r': 
                end_fraction_value(visitor, ec);
                if (ec) return;
                push_state(state_);
                ++input_ptr_;
                ++position_;
                state_ = json_parse_state::cr;
                return; 
            case '\n': 
                end_fraction_value(visitor, ec);
                if (ec) return;
                ++input_ptr_;
                ++line_;
                ++position_;
                mark_position_ = position_;
                return;   
            case ' ':case '\t':
                end_fraction_value(visitor, ec);
                if (ec) return;
                skip_space(ec);
                return;
            case '/': 
                end_fraction_value(visitor, ec);
                if (ec) return;
                push_state(state_);
                ++input_ptr_;
                ++position_;
                state_ = json_parse_state::slash;
                return;
            case '}':
                end_fraction_value(visitor, ec);
                if (ec) return;
                state_ = json_parse_state::expect_comma_or_end;
                return;
            case ']':
                end_fraction_value(visitor, ec);
                if (ec) return;
                state_ = json_parse_state::expect_comma_or_end;
                return;
            case ',':
                end_fraction_value(visitor, ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++input_ptr_;
                ++position_;
                return;
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++position_;
                goto fraction2;
            case 'e':case 'E':
                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++position_;
                goto exp1;
            default:
                err_handler_(json_errc::invalid_number, *this);
                ec = json_errc::invalid_number;
                more_ = false;
                return;
        }
exp1:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
        }
        switch (*input_ptr_)
        {
            case '+':
                ++input_ptr_;
                ++position_;
                goto exp2;
            case '-':
                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++position_;
                goto exp2;
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++position_;
                goto exp3;
            default:
                err_handler_(json_errc::invalid_number, *this);
                ec = json_errc::expected_value;
                more_ = false;
                return;
        }
exp2:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
        }
        switch (*input_ptr_)
        {
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++position_;
                goto exp3;
            default:
                err_handler_(json_errc::invalid_number, *this);
                ec = json_errc::expected_value;
                more_ = false;
                return;
        }
        
exp3:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                end_fraction_value(visitor, ec);
                return;
            }
            local_input_end = input_end_;
        }
        switch (*input_ptr_)
        {
            case '\r': 
                end_fraction_value(visitor, ec);
                if (ec) return;
                ++input_ptr_;
                ++position_;
                push_state(state_);
                state_ = json_parse_state::cr;
                return; 
            case '\n': 
                end_fraction_value(visitor, ec);
                if (ec) return;
                ++input_ptr_;
                ++line_;
                ++position_;
                mark_position_ = position_;
                return;   
            case ' ':case '\t':
                end_fraction_value(visitor, ec);
                if (ec) return;
                skip_space(ec);
                return;
            case '/': 
                end_fraction_value(visitor, ec);
                if (ec) return;
                push_state(state_);
                ++input_ptr_;
                ++position_;
                state_ = json_parse_state::slash;
                return;
            case '}':
                end_fraction_value(visitor, ec);
                if (ec) return;
                state_ = json_parse_state::expect_comma_or_end;
                return;
            case ']':
                end_fraction_value(visitor, ec);
                if (ec) return;
                state_ = json_parse_state::expect_comma_or_end;
                return;
            case ',':
                end_fraction_value(visitor, ec);
                if (ec) return;
                begin_member_or_element(ec);
                if (ec) return;
                ++input_ptr_;
                ++position_;
                return;
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                string_buffer_.push_back(static_cast<char>(*input_ptr_));
                ++input_ptr_;
                ++position_;
                goto exp3;
            default:
                err_handler_(json_errc::invalid_number, *this);
                ec = json_errc::invalid_number;
                more_ = false;
                return;
        }

        JSONCONS_UNREACHABLE();               
    }

    void parse_string(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        const char_type* local_input_end = input_end_;
        const char_type* sb = input_ptr_;

string_u1:
        while (true)
        {
            if (input_ptr_ == local_input_end)
            {
                string_buffer_.append(sb,input_ptr_-sb);
                position_ += (input_ptr_ - sb);
                if (!chunk_rdr_->read_chunk(*this, ec))
                {
                    return;
                }
                local_input_end = input_end_;
                sb = input_ptr_;
            }
            switch (*input_ptr_)
            {
                JSONCONS_ILLEGAL_CONTROL_CHARACTER:
                {
                    position_ += (input_ptr_ - sb + 1);
                    if (!err_handler_(json_errc::illegal_control_character, *this))
                    {
                        more_ = false;
                        ec = json_errc::illegal_control_character;
                        return;
                    }
                    // recovery - skip
                    string_buffer_.append(sb,input_ptr_-sb);
                    sb = input_ptr_ + 1;
                    break;
                }
                case '\n':
                case '\r':
                case '\t':
                {
                    position_ += (input_ptr_ - sb + 1);
                    if (!err_handler_(json_errc::illegal_character_in_string, *this))
                    {
                        more_ = false;
                        ec = json_errc::illegal_character_in_string;
                        return;
                    }
                    // recovery - skip
                    string_buffer_.append(sb,input_ptr_-sb);
                    sb = input_ptr_ + 1;
                    break;
                }
                case '\\': 
                {
                    string_buffer_.append(sb,input_ptr_-sb);
                    position_ += (input_ptr_ - sb + 1);
                    ++input_ptr_;
                    goto escape;
                }
                case '\"':
                {
                    position_ += (input_ptr_ - sb + 1);
                    if (string_buffer_.length() == 0)
                    {
                        end_string_value(sb,input_ptr_-sb, visitor, ec);
                        if (ec) {return;}
                    }
                    else
                    {
                        string_buffer_.append(sb,input_ptr_-sb);
                        end_string_value(string_buffer_.data(),string_buffer_.length(), visitor, ec);
                        if (ec) {return;}
                    }
                    ++input_ptr_;
                    return;
                }
            default:
                break;
            }
            ++input_ptr_;
        }

escape:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            //string_buffer_.append(sb,input_ptr_-sb);
            //position_ += (input_ptr_ - sb);
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
            sb = input_ptr_;
        }
        switch (*input_ptr_)
        {
        case '\"':
            string_buffer_.push_back('\"');
            sb = ++input_ptr_;
            ++position_;
            goto string_u1;
        case '\\': 
            string_buffer_.push_back('\\');
            sb = ++input_ptr_;
            ++position_;
            goto string_u1;
        case '/':
            string_buffer_.push_back('/');
            sb = ++input_ptr_;
            ++position_;
            goto string_u1;
        case 'b':
            string_buffer_.push_back('\b');
            sb = ++input_ptr_;
            ++position_;
            goto string_u1;
        case 'f':
            string_buffer_.push_back('\f');
            sb = ++input_ptr_;
            ++position_;
            goto string_u1;
        case 'n':
            string_buffer_.push_back('\n');
            sb = ++input_ptr_;
            ++position_;
            goto string_u1;
        case 'r':
            string_buffer_.push_back('\r');
            sb = ++input_ptr_;
            ++position_;
            goto string_u1;
        case 't':
            string_buffer_.push_back('\t');
            sb = ++input_ptr_;
            ++position_;
            goto string_u1;
        case 'u':
             cp_ = 0;
             ++input_ptr_;
             ++position_;
             goto escape_u1;
        default:    
            err_handler_(json_errc::illegal_escaped_character, *this);
            ec = json_errc::illegal_escaped_character;
            more_ = false;
            return;
        }

escape_u1:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            //string_buffer_.append(sb,input_ptr_-sb);
            //position_ += (input_ptr_ - sb);
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
            sb = input_ptr_;
        }
        {
            cp_ = append_to_codepoint(0, *input_ptr_, ec);
            if (ec)
            {
                return;
            }
            ++input_ptr_;
            ++position_;
            goto escape_u2;
        }

escape_u2:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            //string_buffer_.append(sb,input_ptr_-sb);
            //position_ += (input_ptr_ - sb);
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
            sb = input_ptr_;
        }
        {
            cp_ = append_to_codepoint(cp_, *input_ptr_, ec);
            if (ec)
            {
                return;
            }
            ++input_ptr_;
            ++position_;
            goto escape_u3;
        }

escape_u3:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            //string_buffer_.append(sb,input_ptr_-sb);
            //position_ += (input_ptr_ - sb);
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
            sb = input_ptr_;
        }
        {
            cp_ = append_to_codepoint(cp_, *input_ptr_, ec);
            if (ec)
            {
                return;
            }
            ++input_ptr_;
            ++position_;
            goto escape_u4;
        }

escape_u4:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            //string_buffer_.append(sb,input_ptr_-sb);
            //position_ += (input_ptr_ - sb);
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
            sb = input_ptr_;
        }
        {
            cp_ = append_to_codepoint(cp_, *input_ptr_, ec);
            if (ec)
            {
                return;
            }
            if (unicode_traits::is_high_surrogate(cp_))
            {
                ++input_ptr_;
                ++position_;
                goto escape_expect_surrogate_pair1;
            }
            else
            {
                unicode_traits::convert(&cp_, 1, string_buffer_);
                sb = ++input_ptr_;
                ++position_;
                goto string_u1;
            }
        }

escape_expect_surrogate_pair1:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            //string_buffer_.append(sb,input_ptr_-sb);
            //position_ += (input_ptr_ - sb);
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
            sb = input_ptr_;
        }
        {
            switch (*input_ptr_)
            {
            case '\\': 
                cp2_ = 0;
                ++input_ptr_;
                ++position_;
                goto escape_expect_surrogate_pair2;
            default:
                err_handler_(json_errc::expected_codepoint_surrogate_pair, *this);
                ec = json_errc::expected_codepoint_surrogate_pair;
                more_ = false;
                return;
            }
        }

escape_expect_surrogate_pair2:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            //string_buffer_.append(sb,input_ptr_-sb);
            //position_ += (input_ptr_ - sb);
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
            sb = input_ptr_;
        }
        {
            switch (*input_ptr_)
            {
            case 'u':
                ++input_ptr_;
                ++position_;
                goto escape_u5;
            default:
                err_handler_(json_errc::expected_codepoint_surrogate_pair, *this);
                ec = json_errc::expected_codepoint_surrogate_pair;
                more_ = false;
                return;
            }
        }

escape_u5:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            //string_buffer_.append(sb,input_ptr_-sb);
            //position_ += (input_ptr_ - sb);
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
            sb = input_ptr_;
        }
        {
            cp2_ = append_to_codepoint(0, *input_ptr_, ec);
            if (ec)
            {
                return;
            }
        }
        ++input_ptr_;
        ++position_;
        goto escape_u6;

escape_u6:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            //string_buffer_.append(sb,input_ptr_-sb);
            //position_ += (input_ptr_ - sb);
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
            sb = input_ptr_;
        }
        {
            cp2_ = append_to_codepoint(cp2_, *input_ptr_, ec);
            if (ec)
            {
                return;
            }
            ++input_ptr_;
            ++position_;
            goto escape_u7;
        }

escape_u7:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            //string_buffer_.append(sb,input_ptr_-sb);
            //position_ += (input_ptr_ - sb);
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
            sb = input_ptr_;
        }
        {
            cp2_ = append_to_codepoint(cp2_, *input_ptr_, ec);
            if (ec)
            {
                return;
            }
            ++input_ptr_;
            ++position_;
            goto escape_u8;
        }

escape_u8:
        if (JSONCONS_UNLIKELY(input_ptr_ >= local_input_end)) // Buffer exhausted               
        {
            //string_buffer_.append(sb,input_ptr_-sb);
            //position_ += (input_ptr_ - sb);
            if (!chunk_rdr_->read_chunk(*this, ec))
            {
                return;
            }
            local_input_end = input_end_;
            sb = input_ptr_;
        }
        {
            cp2_ = append_to_codepoint(cp2_, *input_ptr_, ec);
            if (ec)
            {
                return;
            }
            uint32_t cp = 0x10000 + ((cp_ & 0x3FF) << 10) + (cp2_ & 0x3FF);
            unicode_traits::convert(&cp, 1, string_buffer_);
            sb = ++input_ptr_;
            ++position_;
            goto string_u1;
        }

        JSONCONS_UNREACHABLE();               
    }

    void translate_conv_errc(unicode_traits::conv_errc result, std::error_code& ec)
    {
        switch (result)
        {
        case unicode_traits::conv_errc():
            break;
        case unicode_traits::conv_errc::over_long_utf8_sequence:
            more_ = err_handler_(json_errc::over_long_utf8_sequence, *this);
            if (!more_)
            {
                ec = json_errc::over_long_utf8_sequence;
                return;
            }
            break;
        case unicode_traits::conv_errc::unpaired_high_surrogate:
            more_ = err_handler_(json_errc::unpaired_high_surrogate, *this);
            if (!more_)
            {
                ec = json_errc::unpaired_high_surrogate;
                return;
            }
            break;
        case unicode_traits::conv_errc::expected_continuation_byte:
            more_ = err_handler_(json_errc::expected_continuation_byte, *this);
            if (!more_)
            {
                ec = json_errc::expected_continuation_byte;
                return;
            }
            break;
        case unicode_traits::conv_errc::illegal_surrogate_value:
            more_ = err_handler_(json_errc::illegal_surrogate_value, *this);
            if (!more_)
            {
                ec = json_errc::illegal_surrogate_value;
                return;
            }
            break;
        default:
            more_ = err_handler_(json_errc::illegal_codepoint, *this);
            if (!more_)
            {
                ec = json_errc::illegal_codepoint;
                return;
            }
            break;
        }
    }

    std::size_t line() const override
    {
        return line_;
    }

    std::size_t column() const override
    {
        return (position_ - mark_position_) + 1;
    }

    std::size_t position() const override
    {
        return saved_position_;
    }

    std::size_t end_position() const override
    {
        return position_;
    }

    std::size_t offset() const 
    {
        return input_ptr_ - begin_input_;
    }
private:

    void end_integer_value(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        if (string_buffer_[0] == '-')
        {
            end_negative_value(visitor, ec);
        }
        else
        {
            end_positive_value(visitor, ec);
        }
    }

    void end_negative_value(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        int64_t val;
        auto result = jsoncons::detail::to_integer_unchecked(string_buffer_.data(), string_buffer_.length(), val);
        if (result)
        {
            more_ = visitor.int64_value(val, semantic_tag::none, *this, ec);
        }
        else // Must be overflow
        {
            more_ = visitor.string_value(string_buffer_, semantic_tag::bigint, *this, ec);
        }
        after_value(ec);
    }

    void end_positive_value(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        uint64_t val;
        auto result = jsoncons::detail::to_integer_unchecked(string_buffer_.data(), string_buffer_.length(), val);
        if (result)
        {
            more_ = visitor.uint64_value(val, semantic_tag::none, *this, ec);
        }
        else // Must be overflow
        {
            more_ = visitor.string_value(string_buffer_, semantic_tag::bigint, *this, ec);
        }
        after_value(ec);
    }

    void end_fraction_value(basic_json_visitor<char_type>& visitor, std::error_code& ec)
    {
        JSONCONS_TRY
        {
            if (options_.lossless_number())
            {
                more_ = visitor.string_value(string_buffer_, semantic_tag::bigdec, *this, ec);
            }
            else
            {
                double d = to_double_(string_buffer_.c_str(), string_buffer_.length());
                more_ = visitor.double_value(d, semantic_tag::none, *this, ec);
            }
        }
        JSONCONS_CATCH(...)
        {
            more_ = err_handler_(json_errc::invalid_number, *this);
            if (!more_)
            {
                ec = json_errc::invalid_number;
                return;
            }
            more_ = visitor.null_value(semantic_tag::none, *this, ec); // recovery
        }

        after_value(ec);
    }

    void end_string_value(const char_type* s, std::size_t length, basic_json_visitor<char_type>& visitor, std::error_code& ec) 
    {
        string_view_type sv(s, length);
        auto result = unicode_traits::validate(s, length);
        if (result.ec != unicode_traits::conv_errc())
        {
            translate_conv_errc(result.ec,ec);
            position_ += (result.ptr - s);
            return;
        }
        switch (parent())
        {
        case json_parse_state::member_name:
            more_ = visitor.key(sv, *this, ec);
            pop_state();
            state_ = json_parse_state::expect_colon;
            break;
        case json_parse_state::object:
        case json_parse_state::array:
        {
            auto it = std::find_if(string_double_map_.begin(), string_double_map_.end(), string_maps_to_double{ sv });
            if (it != string_double_map_.end())
            {
                more_ = visitor.double_value(it->second, semantic_tag::none, *this, ec);
            }
            else
            {
                more_ = visitor.string_value(sv, semantic_tag::none, *this, ec);
            }
            state_ = json_parse_state::expect_comma_or_end;
            break;
        }
        case json_parse_state::root:
        {
            auto it = std::find_if(string_double_map_.begin(),string_double_map_.end(),string_maps_to_double{sv});
            if (it != string_double_map_.end())
            {
                more_ = visitor.double_value(it->second, semantic_tag::none, *this, ec);
            }
            else
            {
                more_ = visitor.string_value(sv, semantic_tag::none, *this, ec);
            }
            state_ = json_parse_state::accept;
            break;
        }
        default:
            more_ = err_handler_(json_errc::syntax_error, *this);
            if (!more_)
            {
                ec = json_errc::syntax_error;
                return;
            }
            break;
        }
    }

    void begin_member_or_element(std::error_code& ec) 
    {
        switch (parent())
        {
        case json_parse_state::object:
            state_ = json_parse_state::expect_member_name;
            break;
        case json_parse_state::array:
            state_ = json_parse_state::expect_value;
            break;
        case json_parse_state::root:
            break;
        default:
            more_ = err_handler_(json_errc::syntax_error, *this);
            if (!more_)
            {
                ec = json_errc::syntax_error;
                return;
            }
            break;
        }
    }

    void after_value(std::error_code& ec) 
    {
        switch (parent())
        {
        case json_parse_state::array:
        case json_parse_state::object:
            state_ = json_parse_state::expect_comma_or_end;
            break;
        case json_parse_state::root:
            state_ = json_parse_state::accept;
            break;
        default:
            more_ = err_handler_(json_errc::syntax_error, *this);
            if (!more_)
            {
                ec = json_errc::syntax_error;
                return;
            }
            break;
        }
    }

    void push_state(json_parse_state state)
    {
        state_stack_.push_back(state);
        //std::cout << "max_nesting_depth: " << options_.max_nesting_depth() << ", capacity: " << state_stack_.capacity() << ", nesting_depth: " << nesting_depth_ << ", stack size: " << state_stack_.size() << "\n";
    }

    json_parse_state pop_state()
    {
        JSONCONS_ASSERT(!state_stack_.empty())
        json_parse_state state = state_stack_.back();
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
            more_ = err_handler_(json_errc::invalid_unicode_escape_sequence, *this);
            if (!more_)
            {
                ec = json_errc::invalid_unicode_escape_sequence;
                return cp;
            }
        }
        return cp;
    }
};

using json_parser = basic_json_parser<char>;
using wjson_parser = basic_json_parser<wchar_t>;
}

#endif


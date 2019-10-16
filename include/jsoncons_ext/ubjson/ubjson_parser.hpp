// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UBJSON_UBJSON_PARSER_HPP
#define JSONCONS_UBJSON_UBJSON_PARSER_HPP

#include <string>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/config/binary_config.hpp>
#include <jsoncons_ext/ubjson/ubjson_detail.hpp>
#include <jsoncons_ext/ubjson/ubjson_error.hpp>

namespace jsoncons { namespace ubjson {

enum class parse_mode {root,before_done,array,indefinite_array,strongly_typed_array,map_key,map_value,strongly_typed_map_key,strongly_typed_map_value,indefinite_map_key,indefinite_map_value};

struct parse_state 
{
    parse_mode mode; 
    size_t length;
    uint8_t type;
    size_t index;

    parse_state(parse_mode mode, size_t length, uint8_t type = 0)
        : mode(mode), length(length), type(type), index(0)
    {
    }

    parse_state(const parse_state&) = default;
    parse_state(parse_state&&) = default;
};

template <class Src,class WorkAllocator=std::allocator<char>>
class basic_ubjson_parser : public ser_context
{
    typedef char char_type;
    typedef std::char_traits<char> char_traits_type;
    typedef WorkAllocator work_allocator_type;
    typedef typename std::allocator_traits<work_allocator_type>:: template rebind_alloc<char_type> char_allocator_type;
    typedef typename std::allocator_traits<work_allocator_type>:: template rebind_alloc<uint8_t> byte_allocator_type;
    typedef typename std::allocator_traits<work_allocator_type>:: template rebind_alloc<parse_state> parse_state_allocator_type;

    Src source_;
    size_t nesting_depth_;
    bool more_;
    bool done_;
    std::basic_string<char,std::char_traits<char>,char_allocator_type> text_buffer_;
    std::vector<parse_state,parse_state_allocator_type> state_stack_;
public:
    template <class Source>
    basic_ubjson_parser(Source&& source,
                        const WorkAllocator allocator=WorkAllocator())
       : source_(std::forward<Source>(source)), 
         nesting_depth_(0),
         more_(true), 
         done_(false),
         text_buffer_(allocator),
         state_stack_(allocator)
    {
        state_stack_.emplace_back(parse_mode::root,0);
    }

    void restart()
    {
        more_ = true;
    }

    void reset()
    {
        state_stack_.clear();
        state_stack_.emplace_back(parse_mode::root,0);
        more_ = true;
        done_ = false;
    }

    bool done() const
    {
        return done_;
    }

    bool stopped() const
    {
        return !more_;
    }

    size_t line() const override
    {
        return 0;
    }

    size_t column() const override
    {
        return source_.position();
    }

    void parse(json_content_handler& handler, std::error_code& ec)
    {
        while (!done_ && more_)
        {
            switch (state_stack_.back().mode)
            {
                case parse_mode::array:
                {
                    if (state_stack_.back().index < state_stack_.back().length)
                    {
                        ++state_stack_.back().index;
                        read_type_and_value(handler, ec);
                        if (ec)
                        {
                            return;
                        }
                    }
                    else
                    {
                        end_array(handler, ec);
                    }
                    break;
                }
                case parse_mode::strongly_typed_array:
                {
                    if (state_stack_.back().index < state_stack_.back().length)
                    {
                        ++state_stack_.back().index;
                        read_value(handler, state_stack_.back().type, ec);
                        if (ec)
                        {
                            return;
                        }
                    }
                    else
                    {
                        end_array(handler, ec);
                    }
                    break;
                }
                case parse_mode::indefinite_array:
                {
                    int c = source_.peek();
                    switch (c)
                    {
                        case Src::traits_type::eof():
                            ec = ubjson_errc::unexpected_eof;
                            more_ = false;
                            return;
                        case jsoncons::ubjson::detail::ubjson_format::end_array_marker:
                            source_.ignore(1);
                            end_array(handler, ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                        default:
                            read_type_and_value(handler, ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                    }
                    break;
                }
                case parse_mode::map_key:
                {
                    if (state_stack_.back().index < state_stack_.back().length)
                    {
                        ++state_stack_.back().index;
                        read_name(handler, ec);
                        if (ec)
                        {
                            return;
                        }
                        state_stack_.back().mode = parse_mode::map_value;
                    }
                    else
                    {
                        end_map(handler, ec);
                    }
                    break;
                }
                case parse_mode::map_value:
                {
                    state_stack_.back().mode = parse_mode::map_key;
                    read_type_and_value(handler, ec);
                    if (ec)
                    {
                        return;
                    }
                    break;
                }
                case parse_mode::strongly_typed_map_key:
                {
                    if (state_stack_.back().index < state_stack_.back().length)
                    {
                        ++state_stack_.back().index;
                        read_name(handler, ec);
                        if (ec)
                        {
                            return;
                        }
                        state_stack_.back().mode = parse_mode::strongly_typed_map_value;
                    }
                    else
                    {
                        end_map(handler, ec);
                    }
                    break;
                }
                case parse_mode::strongly_typed_map_value:
                {
                    state_stack_.back().mode = parse_mode::strongly_typed_map_key;
                    read_value(handler, state_stack_.back().type, ec);
                    if (ec)
                    {
                        return;
                    }
                    break;
                }
                case parse_mode::indefinite_map_key:
                {
                    int c = source_.peek();
                    switch (c)
                    {
                        case Src::traits_type::eof():
                            ec = ubjson_errc::unexpected_eof;
                            more_ = false;
                            return;
                        case jsoncons::ubjson::detail::ubjson_format::end_array_marker:
                            source_.ignore(1);
                            end_map(handler, ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                        default:
                            read_name(handler, ec);
                            if (ec)
                            {
                                return;
                            }
                            state_stack_.back().mode = parse_mode::indefinite_map_value;
                            break;
                    }
                    break;
                }
                case parse_mode::indefinite_map_value:
                {
                    state_stack_.back().mode = parse_mode::indefinite_map_key;
                    read_type_and_value(handler, ec);
                    if (ec)
                    {
                        return;
                    }
                    break;
                }
                case parse_mode::root:
                {
                    state_stack_.back().mode = parse_mode::before_done;
                    read_type_and_value(handler, ec);
                    if (ec)
                    {
                        return;
                    }
                    break;
                }
                case parse_mode::before_done:
                {
                    JSONCONS_ASSERT(state_stack_.size() == 1);
                    state_stack_.clear();
                    more_ = false;
                    done_ = true;
                    handler.flush();
                    break;
                }
            }
        }
    }
private:
    void read_type_and_value(json_content_handler& handler, std::error_code& ec)
    {
        if (source_.is_error())
        {
            ec = ubjson_errc::source_error;
            return;
        }   

        uint8_t type{};
        if (source_.get(type) == 0)
        {
            ec = ubjson_errc::unexpected_eof;
            return;
        }
        read_value(handler, type, ec);
    }

    void read_value(json_content_handler& handler, uint8_t type, std::error_code& ec)
    {
        switch (type)
        {
            case jsoncons::ubjson::detail::ubjson_format::null_type: 
            {
                more_ = handler.null_value(semantic_tag::none, *this);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::no_op_type: 
            {
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::true_type:
            {
                more_ = handler.bool_value(true, semantic_tag::none, *this);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::false_type:
            {
                more_ = handler.bool_value(false, semantic_tag::none, *this);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::int8_type: 
            {
                uint8_t buf[sizeof(int8_t)];
                source_.read(buf, sizeof(int8_t));
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                int8_t val = jsoncons::detail::big_to_native<int8_t>(buf,buf+sizeof(buf),&endp);
                more_ = handler.int64_value(val, semantic_tag::none, *this);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::uint8_type: 
            {
                uint8_t val{};
                if (source_.get(val) == 0)
                {
                    ec = ubjson_errc::unexpected_eof;
                    return;
                }
                more_ = handler.uint64_value(val, semantic_tag::none, *this);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::int16_type: 
            {
                uint8_t buf[sizeof(int16_t)];
                source_.read(buf, sizeof(int16_t));
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                int16_t val = jsoncons::detail::big_to_native<int16_t>(buf,buf+sizeof(buf),&endp);
                more_ = handler.int64_value(val, semantic_tag::none, *this);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::int32_type: 
            {
                uint8_t buf[sizeof(int32_t)];
                source_.read(buf, sizeof(int32_t));
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                int32_t val = jsoncons::detail::big_to_native<int32_t>(buf,buf+sizeof(buf),&endp);
                more_ = handler.int64_value(val, semantic_tag::none, *this);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::int64_type: 
            {
                uint8_t buf[sizeof(int64_t)];
                source_.read(buf, sizeof(int64_t));
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                int64_t val = jsoncons::detail::big_to_native<int64_t>(buf,buf+sizeof(buf),&endp);
                more_ = handler.int64_value(val, semantic_tag::none, *this);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::float32_type: 
            {
                uint8_t buf[sizeof(float)];
                source_.read(buf, sizeof(float));
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                float val = jsoncons::detail::big_to_native<float>(buf,buf+sizeof(buf),&endp);
                more_ = handler.double_value(val, semantic_tag::none, *this);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::float64_type: 
            {
                uint8_t buf[sizeof(double)];
                source_.read(buf, sizeof(double));
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                double val = jsoncons::detail::big_to_native<double>(buf,buf+sizeof(buf),&endp);
                more_ = handler.double_value(val, semantic_tag::none, *this);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::char_type: 
            {
                uint8_t buf[sizeof(char)];
                source_.read(buf, sizeof(char));
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return;
                }
                const uint8_t* endp;
                char c = jsoncons::detail::big_to_native<char>(buf,buf+sizeof(buf),&endp);
                auto result = unicons::validate(&c,&c+1);
                if (result.ec != unicons::conv_errc())
                {
                    ec = ubjson_errc::invalid_utf8_text_string;
                    return;
                }
                more_ = handler.string_value(basic_string_view<char>(&c,1), semantic_tag::none, *this);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::string_type: 
            {
                size_t length = get_length(ec);
                if (ec)
                {
                    return;
                }
                text_buffer_.clear();
                source_.read(std::back_inserter(text_buffer_), length);
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return;
                }
                auto result = unicons::validate(text_buffer_.begin(),text_buffer_.end());
                if (result.ec != unicons::conv_errc())
                {
                    ec = ubjson_errc::invalid_utf8_text_string;
                    return;
                }
                more_ = handler.string_value(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), semantic_tag::none, *this);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::high_precision_number_type: 
            {
                size_t length = get_length(ec);
                if (ec)
                {
                    return;
                }
                text_buffer_.clear();
                source_.read(std::back_inserter(text_buffer_), length);
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return;
                }
                if (jsoncons::detail::is_integer(text_buffer_.data(),text_buffer_.length()))
                {
                    more_ = handler.string_value(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), semantic_tag::bigint, *this);
                }
                else
                {
                    more_ = handler.string_value(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), semantic_tag::bigdec, *this);
                }
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::start_array_marker: 
            {
                begin_array(handler,ec);
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::start_object_marker: 
            {
                begin_map(handler, ec);
                break;
            }
            default:
            {
                ec = ubjson_errc::unknown_type;
                return;
            }
        }
    }

    void begin_array(json_content_handler& handler, std::error_code& ec)
    {
        if (source_.peek() == jsoncons::ubjson::detail::ubjson_format::type_marker)
        {
            source_.ignore(1);
            uint8_t item_type{};
            if (source_.get(item_type) == 0)
            {
                ec = ubjson_errc::unexpected_eof;
                return;
            }
            if (source_.peek() == jsoncons::ubjson::detail::ubjson_format::count_marker)
            {
                source_.ignore(1);
                size_t length = get_length(ec);
                state_stack_.emplace_back(parse_mode::strongly_typed_array,length,item_type);
                more_ = handler.begin_array(length, semantic_tag::none, *this);
            }
            else
            {
                ec = ubjson_errc::count_required_after_type;
                return;
            }
        }
        else if (source_.peek() == jsoncons::ubjson::detail::ubjson_format::count_marker)
        {
            source_.ignore(1);
            size_t length = get_length(ec);
            state_stack_.emplace_back(parse_mode::array,length);
            more_ = handler.begin_array(length, semantic_tag::none, *this);
        }
        else
        {
            state_stack_.emplace_back(parse_mode::indefinite_array,0);
            more_ = handler.begin_array(semantic_tag::none, *this);
        }
    }

    void end_array(json_content_handler& handler, std::error_code&)
    {
        more_ = handler.end_array(*this);
        state_stack_.pop_back();
    }

    void begin_map(json_content_handler& handler, std::error_code& ec)
    {
        if (source_.peek() == jsoncons::ubjson::detail::ubjson_format::type_marker)
        {
            source_.ignore(1);
            uint8_t item_type{};
            if (source_.get(item_type) == 0)
            {
                ec = ubjson_errc::unexpected_eof;
                return;
            }
            if (source_.peek() == jsoncons::ubjson::detail::ubjson_format::count_marker)
            {
                source_.ignore(1);
                size_t length = get_length(ec);
                state_stack_.emplace_back(parse_mode::strongly_typed_map_key,length,item_type);
                more_ = handler.begin_object(length, semantic_tag::none, *this);
            }
            else
            {
                ec = ubjson_errc::count_required_after_type;
                return;
            }
        }
        else
        {
            if (source_.peek() == jsoncons::ubjson::detail::ubjson_format::count_marker)
            {
                source_.ignore(1);
                size_t length = get_length(ec);
                state_stack_.emplace_back(parse_mode::map_key,length);
                more_ = handler.begin_object(length, semantic_tag::none, *this);
            }
            else
            {
                state_stack_.emplace_back(parse_mode::indefinite_map_key,0);
                more_ = handler.begin_object(semantic_tag::none, *this);
            }
        }
    }

    void end_map(json_content_handler& handler, std::error_code&)
    {
        more_ = handler.end_object(*this);
        state_stack_.pop_back();
    }

    size_t get_length(std::error_code& ec)
    {
        size_t length = 0;
        if (JSONCONS_UNLIKELY(source_.eof()))
        {
            ec = ubjson_errc::unexpected_eof;
            return length;
        }
        uint8_t type{};
        if (source_.get(type) == 0)
        {
            ec = ubjson_errc::unexpected_eof;
            return length;
        }
        switch (type)
        {
            case jsoncons::ubjson::detail::ubjson_format::int8_type: 
            {
                uint8_t buf[sizeof(int8_t)];
                source_.read(buf, sizeof(int8_t));
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return length;
                }
                const uint8_t* endp;
                int8_t val = jsoncons::detail::big_to_native<int8_t>(buf,buf+sizeof(buf),&endp);
                if (val >= 0)
                {
                    length = val;
                }
                else
                {
                    ec = ubjson_errc::length_cannot_be_negative;
                    return length;
                }
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::uint8_type: 
            {
                uint8_t val{};
                if (source_.get(val) == 0)
                {
                    ec = ubjson_errc::unexpected_eof;
                    return length;
                }
                length = val;
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::int16_type: 
            {
                uint8_t buf[sizeof(int16_t)];
                source_.read(buf, sizeof(int16_t));
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return length;
                }
                const uint8_t* endp;
                int16_t val = jsoncons::detail::big_to_native<int16_t>(buf,buf+sizeof(buf),&endp);
                if (val >= 0)
                {
                    length = val;
                }
                else
                {
                    ec = ubjson_errc::length_cannot_be_negative;
                    return length;
                }
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::int32_type: 
            {
                uint8_t buf[sizeof(int32_t)];
                source_.read(buf, sizeof(int32_t));
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return length;
                }
                const uint8_t* endp;
                int32_t val = jsoncons::detail::big_to_native<int32_t>(buf,buf+sizeof(buf),&endp);
                if (val >= 0)
                {
                    length = val;
                }
                else
                {
                    ec = ubjson_errc::length_cannot_be_negative;
                    return length;
                }
                break;
            }
            case jsoncons::ubjson::detail::ubjson_format::int64_type: 
            {
                uint8_t buf[sizeof(int64_t)];
                source_.read(buf, sizeof(int64_t));
                if (source_.eof())
                {
                    ec = ubjson_errc::unexpected_eof;
                    return length;
                }
                const uint8_t* endp;
                int64_t val = jsoncons::detail::big_to_native<int64_t>(buf,buf+sizeof(buf),&endp);
                if (val >= 0)
                {
                    length = (size_t)val;
                    if (length != (uint64_t)val)
                    {
                        ec = ubjson_errc::number_too_large;
                        return length;
                    }
                }
                else
                {
                    ec = ubjson_errc::length_cannot_be_negative;
                    return length;
                }
                break;
            }
            default:
            {
                ec = ubjson_errc::length_must_be_integer;
                return length;
            }
        }
        return length;
    }

    void read_name(json_content_handler& handler, std::error_code& ec)
    {
        size_t length = get_length(ec);
        if (ec)
        {
            return;
        }
        text_buffer_.clear();
        source_.read(std::back_inserter(text_buffer_), length);
        if (source_.eof())
        {
            ec = ubjson_errc::unexpected_eof;
            return;
        }
        auto result = unicons::validate(text_buffer_.begin(),text_buffer_.end());
        if (result.ec != unicons::conv_errc())
        {
            ec = ubjson_errc::invalid_utf8_text_string;
            return;
        }
        more_ = handler.name(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), *this);
    }
};

}}

#endif

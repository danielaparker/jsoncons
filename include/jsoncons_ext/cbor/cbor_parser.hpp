// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CBOR_CBOR_PARSER_HPP
#define JSONCONS_CBOR_CBOR_PARSER_HPP

#include <string>
#include <vector>
#include <memory>
#include <utility> // std::move
#include <jsoncons/json.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/staj_reader.hpp> // typed_array
#include <jsoncons/config/binary_config.hpp>
#include <jsoncons_ext/cbor/cbor_encoder.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>
#include <jsoncons_ext/cbor/cbor_detail.hpp>

namespace jsoncons { namespace cbor {

enum class parse_mode {root,before_done,array,indefinite_array,map_key,map_value,indefinite_map_key,indefinite_map_value,multi_dim};

struct mapped_string
{
    jsoncons::cbor::detail::cbor_major_type type;
    std::string s;
    std::vector<uint8_t> bytes;

    mapped_string(const std::string& s)
        : type(jsoncons::cbor::detail::cbor_major_type::text_string), s(s)
    {
    }

    mapped_string(std::string&& s)
        : type(jsoncons::cbor::detail::cbor_major_type::text_string), s(std::move(s))
    {
    }

    mapped_string(const std::vector<uint8_t>& bytes)
        : type(jsoncons::cbor::detail::cbor_major_type::byte_string), bytes(bytes)
    {
    }

    mapped_string(std::vector<uint8_t>&& bytes)
        : type(jsoncons::cbor::detail::cbor_major_type::byte_string), bytes(std::move(bytes))
    {
    }

    mapped_string(const mapped_string&) = default;

    mapped_string(mapped_string&&) = default;

    mapped_string& operator=(const mapped_string&) = default;

    mapped_string& operator=(mapped_string&&) = default;
};

typedef std::vector<mapped_string> stringref_map_type;

struct parse_state 
{
    parse_mode mode; 
    std::size_t length;
    std::size_t index;
    std::shared_ptr<stringref_map_type> stringref_map; 

    parse_state(parse_mode mode, std::size_t length)
        : mode(mode), length(length), index(0)
    {
    }

    parse_state(parse_mode mode, std::size_t length, std::shared_ptr<stringref_map_type> stringref_map)
        : mode(mode), length(length), index(0), stringref_map(stringref_map)
    {
    }

    parse_state(const parse_state&) = default;
    parse_state(parse_state&&) = default;
};

template <class Src,class WorkAllocator=std::allocator<char>>
class basic_cbor_parser : public ser_context
{
    typedef char char_type;
    typedef std::char_traits<char> char_traits_type;
    typedef WorkAllocator work_allocator_type;
    typedef typename std::allocator_traits<work_allocator_type>:: template rebind_alloc<char_type> char_allocator_type;
    typedef typename std::allocator_traits<work_allocator_type>:: template rebind_alloc<uint8_t> byte_allocator_type;
    typedef typename std::allocator_traits<work_allocator_type>:: template rebind_alloc<uint64_t> tag_allocator_type;
    typedef typename std::allocator_traits<work_allocator_type>:: template rebind_alloc<parse_state> parse_state_allocator_type;

    typedef std::basic_string<char_type,char_traits_type,char_allocator_type> string_type;

    work_allocator_type alloc_;
    Src source_;
    bool more_;
    bool done_;
    std::basic_string<char,std::char_traits<char>,char_allocator_type> text_buffer_;
    std::vector<uint8_t,byte_allocator_type> bytes_buffer_;
    std::vector<uint64_t,tag_allocator_type> tags_; 
    std::vector<parse_state,parse_state_allocator_type> state_stack_;
    typed_array<WorkAllocator> typed_array_;
    std::vector<std::size_t> shape_;
    std::size_t index_;

public:
    template <class Source>
    basic_cbor_parser(Source&& source,
                      const WorkAllocator alloc=WorkAllocator())
       : alloc_(alloc),
         source_(std::forward<Source>(source)),
         more_(true), 
         done_(false),
         text_buffer_(alloc),
         bytes_buffer_(alloc),
         tags_(alloc),
         state_stack_(alloc),
         typed_array_(alloc),
         index_(0)
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

    std::size_t line() const override
    {
        return 0;
    }

    std::size_t column() const override
    {
        return source_.position();
    }

    void parse(json_content_handler& handler, std::error_code& ec)
    {
        while (!done_ && more_)
        {
            switch (state_stack_.back().mode)
            {
                case parse_mode::multi_dim:
                {
                    if (state_stack_.back().index == 0)
                    {
                        ++state_stack_.back().index;
                        read_item(handler, ec);
                        if (ec)
                        {
                            return;
                        }
                    }
                    else
                    {
                        produce_end_multi_dim(handler, ec);
                    }
                    break;
                }
                case parse_mode::array:
                {
                    if (state_stack_.back().index < state_stack_.back().length)
                    {
                        ++state_stack_.back().index;
                        read_item(handler, ec);
                        if (ec)
                        {
                            return;
                        }
                    }
                    else
                    {
                        produce_end_array(handler, ec);
                    }
                    break;
                }
                case parse_mode::indefinite_array:
                {
                    int c = source_.peek();
                    switch (c)
                    {
                        case Src::traits_type::eof():
                            ec = cbor_errc::unexpected_eof;
                            more_ = false;
                            return;
                        case 0xff:
                            source_.ignore(1);
                            produce_end_array(handler, ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                        default:
                            read_item(handler, ec);
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
                        produce_end_map(handler, ec);
                    }
                    break;
                }
                case parse_mode::map_value:
                {
                    state_stack_.back().mode = parse_mode::map_key;
                    read_item(handler, ec);
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
                            ec = cbor_errc::unexpected_eof;
                            more_ = false;
                            return;
                        case 0xff:
                            source_.ignore(1);
                            produce_end_map(handler, ec);
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
                    read_item(handler, ec);
                    if (ec)
                    {
                        return;
                    }
                    break;
                }
                case parse_mode::root:
                {
                    state_stack_.back().mode = parse_mode::before_done;
                    read_item(handler, ec);
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
    void read_item(json_content_handler& handler, std::error_code& ec)
    {
        read_tags(ec);
        if (ec)
        {
            return;
        }
        int c = source_.peek();
        if (c == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type((uint8_t)c);

        uint8_t info = get_additional_information_value((uint8_t)c);
        switch (major_type)
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                uint64_t val = get_uint64_value(ec);
                if (ec)
                {
                    return;
                }
                if (state_stack_.back().stringref_map && !tags_.empty() && tags_.back() == 25)
                {
                    tags_.pop_back();
                    if (val >= state_stack_.back().stringref_map->size())
                    {
                        ec = cbor_errc::stringref_too_large;
                        more_ = false;
                        return;
                    }
                    stringref_map_type::size_type index = (stringref_map_type::size_type)val;
                    if (index != val)
                    {
                        ec = cbor_errc::number_too_large;
                        more_ = false;
                        return;
                    }
                    auto& str = state_stack_.back().stringref_map->at(index);
                    switch (str.type)
                    {
                        case jsoncons::cbor::detail::cbor_major_type::text_string:
                        {
                            handle_string(handler, basic_string_view<char>(str.s.data(),str.s.length()),ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                        }
                        case jsoncons::cbor::detail::cbor_major_type::byte_string:
                        {
                            handle_byte_string(handler, byte_string_view(str.bytes.data(),str.bytes.size()), ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                        }
                        default:
                            JSONCONS_UNREACHABLE();
                            break;
                    }
                }
                else
                {
                    semantic_tag tag = semantic_tag::none;
                    if (!tags_.empty())
                    {
                        if (tags_.back() == 1)
                        {
                            tag = semantic_tag::timestamp;
                        }
                        tags_.clear();
                    }
                    more_ = handler.uint64_value(val, tag, *this);
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                int64_t val = get_int64_value(ec);
                if (ec)
                {
                    return;
                }
                semantic_tag tag = semantic_tag::none;
                if (!tags_.empty())
                {
                    if (tags_.back() == 1)
                    {
                        tag = semantic_tag::timestamp;
                    }
                    tags_.clear();
                }
                more_ = handler.int64_value(val, tag, *this);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::byte_string:
            {
                bytes_buffer_ = get_byte_string(ec);
                if (ec)
                {
                    return;
                }
                handle_byte_string(handler, byte_string_view(bytes_buffer_.data(), bytes_buffer_.size()), ec);
                if (ec)
                {
                    return;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::text_string:
            {
                text_buffer_ = get_text_string(ec);
                auto result = unicons::validate(text_buffer_.begin(),text_buffer_.end());
                if (result.ec != unicons::conv_errc())
                {
                    ec = cbor_errc::invalid_utf8_text_string;
                    more_ = false;
                    return;
                }
                handle_string(handler, basic_string_view<char>(text_buffer_.data(),text_buffer_.length()),ec);
                if (ec)
                {
                    return;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::semantic_tag:
            {
                JSONCONS_UNREACHABLE();
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::simple:
            {
                switch (info)
                {
                    case 0x14:
                        more_ = handler.bool_value(false, semantic_tag::none, *this, ec);
                        source_.ignore(1);
                        break;
                    case 0x15:
                        more_ = handler.bool_value(true, semantic_tag::none, *this, ec);
                        source_.ignore(1);
                        break;
                    case 0x16:
                        more_ = handler.null_value(semantic_tag::none, *this, ec);
                        source_.ignore(1);
                        break;
                    case 0x17:
                        more_ = handler.null_value(semantic_tag::undefined, *this);
                        source_.ignore(1);
                        break;
                    case 0x19: // Half-Precision Float (two-byte IEEE 754)
                    {
                        uint64_t val = get_uint64_value(ec);
                        if (ec)
                        {
                            return;
                        }
                        more_ = handler.half_value(static_cast<uint16_t>(val), semantic_tag::none, *this, ec);
                        break;
                    }
                    case 0x1a: // Single-Precision Float (four-byte IEEE 754)
                    case 0x1b: // Double-Precision Float (eight-byte IEEE 754)
                        double val = get_double(ec);
                        if (ec)
                        {
                            return;
                        }
                        semantic_tag tag = semantic_tag::none;
                        if (!tags_.empty())
                        {
                            if (tags_.back() == 1)
                            {
                                tag = semantic_tag::timestamp;
                            }
                            tags_.clear();
                        }
                        more_ = handler.double_value(val, tag, *this);
                        break;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::array:
            {
                if (!tags_.empty())
                {
                    switch (tags_.back())
                    {
                        case 0x04:
                            text_buffer_ = get_array_as_decimal_string(ec);
                            if (ec)
                            {
                                return;
                            }
                            more_ = handler.string_value(text_buffer_, semantic_tag::bigdec);
                            tags_.pop_back();
                            break;
                        case 0x05:
                            text_buffer_ = get_array_as_hexfloat_string(ec);
                            if (ec)
                            {
                                return;
                            }
                            more_ = handler.string_value(text_buffer_, semantic_tag::bigfloat);
                            tags_.pop_back();
                            break;
                        case 40: // row major storage
                            produce_begin_multi_dim(handler, semantic_tag::multi_dim_row_major, ec);
                            break;
                        case 1040: // column major storage
                            produce_begin_multi_dim(handler, semantic_tag::multi_dim_column_major, ec);
                            break;
                        default:
                            produce_begin_array(handler, info, ec);
                            break;
                    }
                }
                else
                {
                    produce_begin_array(handler, info, ec);
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::map:
            {
                produce_begin_map(handler, info, ec);
                break;
            }
            default:
                break;
        }
        tags_.clear();
    }

    void produce_begin_array(json_content_handler& handler, uint8_t info, std::error_code& ec)
    {
        semantic_tag tag = semantic_tag::none;
        auto stringref_map = state_stack_.back().stringref_map;
        for (auto t : tags_)
        {
            switch (t)
            {
                case 0x05:
                    tag = semantic_tag::bigfloat;
                    break;
                case 0x100: // 256 (stringref-namespace)
                    stringref_map = std::make_shared<stringref_map_type>();
                    break;
                default:
                    break;
            }
        }
        tags_.clear();
        switch (info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length:
            {
                state_stack_.emplace_back(parse_mode::indefinite_array,0,stringref_map);
                more_ = handler.begin_array(tag, *this);
                source_.ignore(1);
                break;
            }
            default: // definite length
            {
                std::size_t len = get_size(ec);
                if (ec)
                {
                    return;
                }
                state_stack_.emplace_back(parse_mode::array,len,stringref_map);
                more_ = handler.begin_array(len, tag, *this);
                break;
            }
        }
    }

    void produce_end_array(json_content_handler& handler, std::error_code&)
    {
        more_ = handler.end_array(*this);
        state_stack_.pop_back();
    }

    void produce_begin_map(json_content_handler& handler, uint8_t info, std::error_code& ec)
    {
        auto stringref_map = state_stack_.back().stringref_map;
        for (auto t : tags_)
        {
            switch (t)
            {
                case 0x100: // 256 (stringref-namespace)
                    stringref_map = std::make_shared<stringref_map_type>();
                    break;
                default:
                    break;
            }
        }
        tags_.clear();
        switch (info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length: 
            {
                state_stack_.emplace_back(parse_mode::indefinite_map_key,0,stringref_map);
                more_ = handler.begin_object(semantic_tag::none, *this, ec);
                source_.ignore(1);
                break;
            }
            default: // definite_length
            {
                std::size_t len = get_size(ec);
                if (ec)
                {
                    return;
                }
                state_stack_.emplace_back(parse_mode::map_key,len,stringref_map);
                more_ = handler.begin_object(len, semantic_tag::none, *this, ec);
                break;
            }
        }
    }

    void produce_end_map(json_content_handler& handler, std::error_code&)
    {
        more_ = handler.end_object(*this);
        state_stack_.pop_back();
    }

    void read_name(json_content_handler& handler, std::error_code& ec)
    {
        read_tags(ec);
        if (ec)
        {
            return;
        }
        jsoncons::cbor::detail::cbor_major_type major_type;
        int c = source_.peek();
        switch (c)
        {
            case Src::traits_type::eof():
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return;
            default:
                major_type = get_major_type((uint8_t)c);
                break;
        }
        switch (major_type)
        {
            case jsoncons::cbor::detail::cbor_major_type::text_string:
            {
                text_buffer_ = get_text_string(ec);
                if (ec)
                {
                    return;
                }
                auto result = unicons::validate(text_buffer_.begin(),text_buffer_.end());
                if (result.ec != unicons::conv_errc())
                {
                    ec = cbor_errc::invalid_utf8_text_string;
                    more_ = false;
                    return;
                }
                more_ = handler.name(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), *this);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::byte_string:
            {
                bytes_buffer_ = get_byte_string(ec);
                if (ec)
                {
                    return;
                }
                text_buffer_.clear();
                encode_base64url(bytes_buffer_.begin(),bytes_buffer_.end(),text_buffer_);
                more_ = handler.name(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), *this);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                if (state_stack_.back().stringref_map && !tags_.empty() && tags_.back() == 25)
                {
                    uint64_t ref = get_uint64_value(ec);
                    if (ec)
                    {
                        return;
                    }
                    if (ref >= state_stack_.back().stringref_map->size())
                    {
                        ec = cbor_errc::stringref_too_large;
                        more_ = false;
                        return;
                    }

                    stringref_map_type::size_type index = (stringref_map_type::size_type)ref;
                    if (index != ref)
                    {
                        ec = cbor_errc::number_too_large;
                        more_ = false;
                        return;
                    }
                    auto& val = state_stack_.back().stringref_map->at(index);
                    switch (val.type)
                    {
                        case jsoncons::cbor::detail::cbor_major_type::text_string:
                        {
                            more_ = handler.name(basic_string_view<char>(val.s.data(),val.s.length()), *this);
                            break;
                        }
                        case jsoncons::cbor::detail::cbor_major_type::byte_string:
                        {
                            text_buffer_.clear();
                            encode_base64url(val.bytes.begin(),val.bytes.end(),text_buffer_);
                            more_ = handler.name(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), *this);
                            break;
                        }
                        default:
                            JSONCONS_UNREACHABLE();
                            break;
                    }
                    tags_.pop_back();
                    break;
                }
            }
                JSONCONS_FALLTHROUGH;
            default:
            {
                text_buffer_.clear();
                json_string_encoder encoder(text_buffer_);
                basic_cbor_parser<Src> reader(std::move(source_));

                reader.parse(encoder, ec);
                source_ = std::move(reader.source_);
                auto result = unicons::validate(text_buffer_.begin(),text_buffer_.end());
                if (result.ec != unicons::conv_errc())
                {
                    ec = cbor_errc::invalid_utf8_text_string;
                    more_ = false;
                    return;
                }
                more_ = handler.name(basic_string_view<char>(text_buffer_.data(),text_buffer_.length()), *this);
            }
        }
    }

    string_type get_text_string(std::error_code& ec)
    {
        string_type s;

        jsoncons::cbor::detail::cbor_major_type major_type;
        uint8_t info;
        int c = source_.peek();
        switch (c)
        {
            case Src::traits_type::eof():
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return s;
            default:
                major_type = get_major_type((uint8_t)c);
                info = get_additional_information_value((uint8_t)c);
                break;
        }
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::text_string);
        auto func = [&](std::size_t length, std::error_code& ec)
        {
            s.reserve(s.size()+length);
            source_.read(std::back_inserter(s), length);
            if (source_.eof())
            {
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return;
            }
        };
        iterate_string_chunks( func, ec);
        if (state_stack_.back().stringref_map && 
            info != jsoncons::cbor::detail::additional_info::indefinite_length &&
            s.length() >= jsoncons::cbor::detail::min_length_for_stringref(state_stack_.back().stringref_map->size()))
        {
            state_stack_.back().stringref_map->emplace_back(s);
        }
        
        return s;
    }

    std::size_t get_size(std::error_code& ec)
    {
        uint64_t u = get_uint64_value(ec);
        if (ec)
        {
            return 0;
        }
        std::size_t len = (std::size_t)u;
        if (len != u)
        {
            ec = cbor_errc::number_too_large;
            more_ = false;
        }
        return len;
    }

    std::vector<uint8_t> get_byte_string(std::error_code& ec)
    {
        std::vector<uint8_t> v;

        jsoncons::cbor::detail::cbor_major_type major_type;
        uint8_t info;
        int c = source_.peek();
        switch (c)
        {
            case Src::traits_type::eof():
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return v;
            default:
                major_type = get_major_type((uint8_t)c);
                info = get_additional_information_value((uint8_t)c);
                break;
        }
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::byte_string);

        switch(info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length:
            {
                auto func = [&](std::size_t length, std::error_code& ec)
                {
                    size_t offset = v.size();
                    v.resize(v.size()+length);
                    source_.read(v.data()+offset, length);
                    if (source_.eof())
                    {
                        ec = cbor_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                };
                iterate_string_chunks( func, ec);
                break;
            }
            default:
            {
                std::size_t length = get_size(ec);
                if (ec)
                {
                    more_ = false;
                    return v;
                }
                v.resize(length);
                source_.read(v.data(), length);
                if (source_.eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return v;
                }
                if (state_stack_.back().stringref_map &&
                    v.size() >= jsoncons::cbor::detail::min_length_for_stringref(state_stack_.back().stringref_map->size()))
                {
                    state_stack_.back().stringref_map->emplace_back(v);
                }
                break;
            }
        }
        return v;
    }

    template <class Function>
    void iterate_string_chunks(Function func, std::error_code& ec)
    {
        int c = source_.peek();
        if (c == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }

        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type((uint8_t)c);
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::text_string || major_type == jsoncons::cbor::detail::cbor_major_type::byte_string);
        uint8_t info = get_additional_information_value((uint8_t)c);

        switch (info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length:
            {
                source_.ignore(1);
                bool done = false;
                while (!done)
                {
                    int test = source_.peek();
                    switch (test)
                    {
                        case Src::traits_type::eof():
                            ec = cbor_errc::unexpected_eof;
                            more_ = false;
                            return;
                        case 0xff:
                            done = true;
                            break;
                        default:
                            iterate_string_chunks( func, ec);
                            if (ec)
                            {
                                return;
                            }
                            break;
                    }
                }
                source_.ignore(1);
                break;
            }
            default: // definite length
            {
                std::size_t length = get_size(ec);
                if (ec)
                {
                    return;
                }
                func(length, ec);
                if (ec)
                {
                    return;
                }
                break;
            }
        }
    }

    uint64_t get_uint64_value(std::error_code& ec)
    {
        uint64_t val = 0;
        if (JSONCONS_UNLIKELY(source_.eof()))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return val;
        }
        const uint8_t* endp = nullptr;

        uint8_t type{};
        if (source_.get(type) == 0)
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return 0;
        }
        uint8_t info = get_additional_information_value(type);
        switch (info)
        {
            case JSONCONS_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
            {
                val = info;
                break;
            }

            case 0x18: // Unsigned integer (one-byte uint8_t follows)
            {
                uint8_t c{};
                source_.get(c);
                if (source_.eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return val;
                }
                val = c;
                break;
            }

            case 0x19: // Unsigned integer (two-byte uint16_t follows)
            {
                uint8_t buf[sizeof(uint16_t)];
                source_.read(buf, sizeof(uint16_t));
                val = jsoncons::detail::big_to_native<uint16_t>(buf,buf+sizeof(buf),&endp);
                break;
            }

            case 0x1a: // Unsigned integer (four-byte uint32_t follows)
            {
                uint8_t buf[sizeof(uint32_t)];
                source_.read(buf, sizeof(uint32_t));
                val = jsoncons::detail::big_to_native<uint32_t>(buf,buf+sizeof(buf),&endp);
                break;
            }

            case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
            {
                uint8_t buf[sizeof(uint64_t)];
                source_.read(buf, sizeof(uint64_t));
                val = jsoncons::detail::big_to_native<uint64_t>(buf,buf+sizeof(buf),&endp);
                break;
            }
            default:
                break;
        }
        return val;
    }

    int64_t get_int64_value(std::error_code& ec)
    {
        int64_t val = 0;
        if (JSONCONS_UNLIKELY(source_.eof()))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return val;
        }
        const uint8_t* endp = nullptr;

        uint8_t info = get_additional_information_value((uint8_t)source_.peek());
        switch (get_major_type((uint8_t)source_.peek()))
        {
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
                source_.ignore(1);
                switch (info)
                {
                    case JSONCONS_CBOR_0x00_0x17: // 0x00..0x17 (0..23)
                    {
                        val = static_cast<int8_t>(- 1 - info);
                        break;
                    }
                    case 0x18: // Negative integer (one-byte uint8_t follows)
                        {
                            uint8_t c{};
                            source_.get(c);
                            if (source_.eof())
                            {
                                ec = cbor_errc::unexpected_eof;
                                more_ = false;
                                return val;
                            }
                            val = static_cast<int64_t>(-1)- c;
                            break;
                        }

                    case 0x19: // Negative integer -1-n (two-byte uint16_t follows)
                        {
                            uint8_t buf[sizeof(uint16_t)];
                            if (source_.read(buf, sizeof(uint16_t)) != sizeof(uint16_t))
                            {
                                return val;
                            }
                            auto x = jsoncons::detail::big_to_native<uint16_t>(buf,buf+sizeof(buf),&endp);
                            val = static_cast<int64_t>(-1)- x;
                            break;
                        }

                    case 0x1a: // Negative integer -1-n (four-byte uint32_t follows)
                        {
                            uint8_t buf[sizeof(uint32_t)];
                            if (source_.read(buf, sizeof(uint32_t)) != sizeof(uint32_t))
                            {
                                return val;
                            }
                            auto x = jsoncons::detail::big_to_native<uint32_t>(buf,buf+sizeof(buf),&endp);
                            val = static_cast<int64_t>(-1)- x;
                            break;
                        }

                    case 0x1b: // Negative integer -1-n (eight-byte uint64_t follows)
                        {
                            uint8_t buf[sizeof(uint64_t)];
                            if (source_.read(buf, sizeof(uint64_t)) != sizeof(uint64_t))
                            {
                                return val;
                            }
                            auto x = jsoncons::detail::big_to_native<uint64_t>(buf,buf+sizeof(buf),&endp);
                            val = static_cast<int64_t>(-1)- static_cast<int64_t>(x);
                            break;
                        }
                }
                break;

                case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
                {
                    uint64_t x = get_uint64_value(ec);
                    if (ec)
                    {
                        return 0;
                    }
                    if (x <= static_cast<uint64_t>((std::numeric_limits<int64_t>::max)()))
                    {
                        val = x;
                    }
                    else
                    {
                        // error;
                    }
                    
                    break;
                }
                break;
            default:
                break;
        }

        return val;
    }

    double get_double(std::error_code& ec)
    {
        double val = 0;
        if (JSONCONS_UNLIKELY(source_.eof()))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return val;
        }
        const uint8_t* endp = nullptr;

        uint8_t type{};
        if (source_.get(type) == 0)
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return 0;
        }
        uint8_t info = get_additional_information_value(type);
        switch (info)
        {
        case 0x1a: // Single-Precision Float (four-byte IEEE 754)
            {
                uint8_t buf[sizeof(float)];
                source_.read(buf, sizeof(float));
                if (source_.eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return 0;
                }
                val = jsoncons::detail::big_to_native<float>(buf,buf+sizeof(buf),&endp);
                break;
            }

        case 0x1b: //  Double-Precision Float (eight-byte IEEE 754)
            {
                uint8_t buf[sizeof(double)];
                source_.read(buf, sizeof(double));
                if (source_.eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return 0;
                }
                val = jsoncons::detail::big_to_native<double>(buf,buf+sizeof(buf),&endp);
                break;
            }
            default:
                break;
        }
        
        return val;
    }

    string_type get_array_as_decimal_string(std::error_code& ec)
    {
        string_type s;

        int c;
        if ((c=source_.get()) == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return s;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type((uint8_t)c);
        uint8_t info = get_additional_information_value((uint8_t)c);
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::array);
        JSONCONS_ASSERT(info == 2);

        if ((c=source_.peek()) == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return s;
        }
        int64_t exponent = 0;
        switch (get_major_type((uint8_t)c))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                exponent = get_uint64_value(ec);
                if (ec)
                {
                    return s;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                exponent = get_int64_value(ec);
                if (ec)
                {
                    return s;
                }
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_bigdec;
                more_ = false;
                return s;
            }
        }

        switch (get_major_type((uint8_t)source_.peek()))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                uint64_t val = get_uint64_value(ec);
                if (ec)
                {
                    return s;
                }
                jsoncons::detail::print_uinteger(val, s);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                int64_t val = get_int64_value(ec);
                if (ec)
                {
                    return s;
                }
                jsoncons::detail::print_integer(val, s);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::semantic_tag:
            {
                if ((c=source_.get()) == Src::traits_type::eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return s;
                }
                uint8_t tag = get_additional_information_value((uint8_t)c);
                if ((c=source_.peek()) == Src::traits_type::eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return s;
                }

                if (get_major_type((uint8_t)c) == jsoncons::cbor::detail::cbor_major_type::byte_string)
                {
                    std::vector<uint8_t> v = get_byte_string(ec);
                    if (ec)
                    {
                        return s;
                    }
                    if (tag == 2)
                    {
                        bignum n(1, v.data(), v.size());
                        n.dump(s);
                    }
                    else if (tag == 3)
                    {
                        bignum n(-1, v.data(), v.size());
                        n.dump(s);
                    }
                }
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_bigdec;
                more_ = false;
                return s;
            }
        }

        string_type result;
        if (s.size() > 0)
        {
            if (s[0] == '-')
            {
                result.push_back('-');
                jsoncons::detail::prettify_string(s.c_str()+1, s.size()-1, (int)exponent, -4, 17, result);
            }
            else
            {
                jsoncons::detail::prettify_string(s.c_str(), s.size(), (int)exponent, -4, 17, result);
            }
        }
        return result;
    }

    string_type get_array_as_hexfloat_string(std::error_code& ec)
    {
        string_type s;

        int c;
        if ((c=source_.get()) == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return s;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type((uint8_t)c);
        uint8_t info = get_additional_information_value((uint8_t)c);
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::array);
        JSONCONS_ASSERT(info == 2);

        if ((c=source_.peek()) == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return s;
        }
        int64_t exponent = 0;
        switch (get_major_type((uint8_t)c))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                exponent = get_uint64_value(ec);
                if (ec)
                {
                    return s;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                exponent = get_int64_value(ec);
                if (ec)
                {
                    return s;
                }
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_bigfloat;
                more_ = false;
                return s;
            }
        }

        switch (get_major_type((uint8_t)source_.peek()))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                uint64_t val = get_uint64_value(ec);
                if (ec)
                {
                    return s;
                }
                s.push_back('0');
                s.push_back('x');
                jsoncons::detail::uinteger_to_hex_string(val, s);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                int64_t val = get_int64_value(ec);
                if (ec)
                {
                    return s;
                }
                s.push_back('-');
                s.push_back('0');
                s.push_back('x');
                jsoncons::detail::uinteger_to_hex_string(static_cast<uint64_t>(-val), s);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::semantic_tag:
            {
                if ((c=source_.get()) == Src::traits_type::eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return s;
                }
                uint8_t tag = get_additional_information_value((uint8_t)c);
                if ((c=source_.peek()) == Src::traits_type::eof())
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return s;
                }

                if (get_major_type((uint8_t)c) == jsoncons::cbor::detail::cbor_major_type::byte_string)
                {
                    std::vector<uint8_t> v = get_byte_string(ec);
                    if (ec)
                    {
                        return s;
                    }
                    if (tag == 2)
                    {
                        s.push_back('-');
                        s.push_back('0');
                        s.push_back('x');
                        bignum n(1, v.data(), v.size());
                        n.dump_hex_string(s);
                    }
                    else if (tag == 3)
                    {
                        s.push_back('-');
                        s.push_back('0');
                        bignum n(-1, v.data(), v.size());
                        n.dump_hex_string(s);
                        s[2] = 'x';
                    }
                }
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_bigfloat;
                more_ = false;
                return s;
            }
        }

        s.push_back('p');
        if (exponent >=0)
        {
            jsoncons::detail::uinteger_to_hex_string(static_cast<uint64_t>(exponent), s);
        }
        else
        {
            s.push_back('-');
            jsoncons::detail::uinteger_to_hex_string(static_cast<uint64_t>(-exponent), s);
        }
        return s;
    }

    static jsoncons::cbor::detail::cbor_major_type get_major_type(uint8_t type)
    {
        static const uint8_t major_type_shift = 0x05;
        uint8_t value = type >> major_type_shift;
        return static_cast<jsoncons::cbor::detail::cbor_major_type>(value);
    }

    static uint8_t get_additional_information_value(uint8_t type)
    {
        static const uint8_t additional_information_mask = (1U << 5) - 1;
        uint8_t value = type & additional_information_mask;
        return value;
    }

    void read_tags(std::error_code& ec)
    {
        int c = source_.peek();
        if (c == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type((uint8_t)c);
        while (major_type == jsoncons::cbor::detail::cbor_major_type::semantic_tag)
        {
            uint64_t val = get_uint64_value(ec);
            if (ec)
            {
                return;
            } 
            tags_.push_back(val);
            c = source_.peek();
            if (c == Src::traits_type::eof())
            {
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return;
            }
            major_type = get_major_type((uint8_t)c);
        }
    }

    void handle_string(json_content_handler& handler, const basic_string_view<char>& v, std::error_code&)
    {
        semantic_tag tag = semantic_tag::none;
        if (!tags_.empty())
        {
            switch (tags_.back())
            {
                case 0:
                    tag = semantic_tag::datetime;
                    break;
                case 32:
                    tag = semantic_tag::uri;
                    break;
                case 33:
                    tag = semantic_tag::base64url;
                    break;
                case 34:
                    tag = semantic_tag::base64;
                    break;
                default:
                    break;
            }
            tags_.clear();
        }
        more_ = handler.string_value(v, tag, *this);
    }

    void handle_byte_string(json_content_handler& handler, const byte_string_view& v, std::error_code& ec)
    {
        if (!tags_.empty())
        {
            switch (tags_.back())
            {
                case 0x2:
                {
                    bignum n(1, v.data(), v.size());
                    text_buffer_.clear();
                    n.dump(text_buffer_);
                    more_ = handler.string_value(text_buffer_, semantic_tag::bigint, *this);
                    break;
                }
                case 0x3:
                {
                    bignum n(-1, v.data(), v.size());
                    text_buffer_.clear();
                    n.dump(text_buffer_);
                    more_ = handler.string_value(text_buffer_, semantic_tag::bigint, *this);
                    break;
                }
                case 0x15:
                {
                    more_ = handler.byte_string_value(v, semantic_tag::base64url, *this);
                    break;
                }
                case 0x16:
                {
                    more_ = handler.byte_string_value(v, semantic_tag::base64, *this);
                    break;
                }
                case 0x17:
                {
                    more_ = handler.byte_string_value(v, semantic_tag::base16, *this);
                    break;
                }
                case 0x40:
                {
                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    std::size_t size = v.size();
                    typed_array_ = typed_array<WorkAllocator>(uint8_array_arg,size,alloc_);
                    for (std::size_t i = 0; p < last; ++p, ++i)
                    {
                        typed_array_.data(uint8_array_arg)[i] = *p;
                    }
                    more_ = handler.typed_array(typed_array_.data(uint8_array_arg), semantic_tag::none, *this, ec);
                    break;
                }
                case 0x44:
                {
                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    std::size_t size = v.size();
                    typed_array_ = typed_array<WorkAllocator>(uint8_array_arg,size,alloc_);
                    for (std::size_t i = 0; p < last; ++p, ++i)
                    {
                        typed_array_.data(uint8_array_arg)[i] = *p;
                    }
                    more_ = handler.typed_array(typed_array_.data(uint8_array_arg), semantic_tag::clamped, *this);
                    break;
                }
                case 0x41:
                case 0x45:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = std::size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    std::size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<WorkAllocator>(uint16_array_arg,size,alloc_);
                    for (std::size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        uint16_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<uint16_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<uint16_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(uint16_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(uint16_array_arg), semantic_tag::none, *this, ec);
                    break;
                }
                case 0x42:
                case 0x46:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = std::size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    std::size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<WorkAllocator>(uint32_array_arg,size,alloc_);
                    for (std::size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        uint32_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<uint32_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<uint32_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(uint32_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(uint32_array_arg), semantic_tag::none, *this, ec);
                    break;
                }
                case 0x43:
                case 0x47:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = std::size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    std::size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<WorkAllocator>(uint64_array_arg,size,alloc_);
                    for (std::size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        uint64_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<uint64_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<uint64_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(uint64_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(uint64_array_arg), semantic_tag::none, *this, ec);
                    break;
                }
                case 0x48:
                {
                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    std::size_t size = v.size();
                    typed_array_ = typed_array<WorkAllocator>(int8_array_arg,size,alloc_);
                    for (std::size_t i = 0; p < last; ++p, ++i)
                    {
                        typed_array_.data(int8_array_arg)[i] = (int8_t)*p;
                    }
                    more_ = handler.typed_array(typed_array_.data(int8_array_arg), semantic_tag::none, *this, ec);
                    break;
                }
                case 0x49:
                case 0x4d:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = std::size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    std::size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<WorkAllocator>(int16_array_arg,size,alloc_);
                    for (std::size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        int16_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<int16_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<int16_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(int16_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(int16_array_arg), semantic_tag::none, *this, ec);
                    break;
                }
                case 0x4a:
                case 0x4e:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = std::size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    std::size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<WorkAllocator>(int32_array_arg,size,alloc_);
                    for (std::size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        int32_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<int32_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<int32_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(int32_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(int32_array_arg), semantic_tag::none, *this, ec);
                    break;
                }
                case 0x4b:
                case 0x4f:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = std::size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    std::size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<WorkAllocator>(int64_array_arg,size,alloc_);
                    for (std::size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        int64_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<int64_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<int64_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(int64_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(int64_array_arg), semantic_tag::none, *this, ec);
                    break;
                }
                case 0x50:
                case 0x54:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = std::size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    std::size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<WorkAllocator>(half_array_arg,size,alloc_);
                    for (std::size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        uint16_t val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<uint16_t>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<uint16_t>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(half_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(half_arg, typed_array_.data(half_array_arg), semantic_tag::none, *this, ec);
                    break;
                }
                case 0x51:
                case 0x55:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = std::size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    std::size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<WorkAllocator>(float_array_arg,size,alloc_);
                    for (std::size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        float val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<float>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<float>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(float_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(float_array_arg), semantic_tag::none, *this, ec);
                    break;
                }
                case 0x52:
                case 0x56:
                {
                    const uint8_t tag = (uint8_t)tags_.back();
                    const uint8_t e = (tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift; 
                    const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
                    const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

                    const size_t bytes_per_elem = std::size_t(1) << (f + ll);

                    const uint8_t* p = v.data();
                    const uint8_t* last = v.data() + v.size();

                    std::size_t size = v.size()/bytes_per_elem;
                    typed_array_ = typed_array<WorkAllocator>(double_array_arg,size,alloc_);
                    for (std::size_t i = 0; p < last; p += bytes_per_elem, ++i)
                    {
                        const uint8_t* endp = nullptr;
                        double val{ 0 };
                        switch (e)
                        {
                            case 0: val = jsoncons::detail::big_to_native<double>(p,p+bytes_per_elem,&endp);break;
                            case 1: val = jsoncons::detail::little_to_native<double>(p,p+bytes_per_elem,&endp);break;
                            default: break;
                        }
                        typed_array_.data(double_array_arg)[i] = val;
                    }
                    more_ = handler.typed_array(typed_array_.data(double_array_arg), semantic_tag::none, *this, ec);
                    break;
                }
                default:
                    more_ = handler.byte_string_value(v, semantic_tag::none, *this, ec);
                    break;
            }
            tags_.clear();
        }
        else
        {
            more_ = handler.byte_string_value(v, semantic_tag::none, *this, ec);
        }
    }

    void produce_begin_multi_dim(json_content_handler& handler, 
                                 semantic_tag tag,
                                 std::error_code& ec)
    {
        int c;
        if ((c=source_.get()) == Src::traits_type::eof())
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type((uint8_t)c);
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::array);
        uint8_t info = get_additional_information_value((uint8_t)c);
       
        read_shape(info, ec);   
        if (ec)
        {
            return;
        }

        state_stack_.emplace_back(parse_mode::multi_dim, 0);
        more_ = handler.begin_multi_dim(shape_, tag, *this, ec);
    }

    void produce_end_multi_dim(json_content_handler& handler, std::error_code& ec)
    {
        more_ = handler.end_multi_dim(*this, ec);
        state_stack_.pop_back();
    }

    void read_shape(uint8_t info, std::error_code& ec)
    {
        shape_.clear();
        switch (info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length:
            {
                while (true)
                {
                    int c = source_.peek();
                    switch (c)
                    {
                        case Src::traits_type::eof():
                            ec = cbor_errc::unexpected_eof;
                            more_ = false;
                            return;
                        case 0xff:
                            source_.ignore(1);
                            break;
                        default:
                        {
                            std::size_t dim = get_size(ec);
                            if (ec)
                            {
                                return;
                            }
                            shape_.push_back(dim);
                            break;
                        }
                    }
                }
                break;
            }
            default:
            {
                std::size_t size = get_size(ec);
                if (ec)
                {
                    return;
                }
                for (std::size_t i = 0; more_ && i < size; ++i)
                {
                    std::size_t dim = get_size(ec);
                    if (ec)
                    {
                        return;
                    }
                    shape_.push_back(dim);
                }
                break;
            }
        }
    }
};

}}

#endif

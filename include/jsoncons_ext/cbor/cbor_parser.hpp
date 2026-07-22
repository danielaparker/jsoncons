// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CBOR_CBOR_PARSER_HPP
#define JSONCONS_EXT_CBOR_CBOR_PARSER_HPP

#include <bitset> // std::bitset
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <system_error>
#include <utility> // std::move
#include <vector>

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/generic_visitor.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/typed_array.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/ser_utils.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/utility/binary.hpp>
#include <jsoncons/utility/unicode_traits.hpp>

#include <jsoncons_ext/cbor/cbor_detail.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>
#include <jsoncons_ext/cbor/cbor_options.hpp>
#include <jsoncons_ext/cbor/cbor_typed_array_iterator.hpp>

namespace jsoncons { 
namespace cbor {

enum class parse_mode {root,accept,array,typed_array,indefinite_array,map_key,map_value,indefinite_map_key,indefinite_map_value,multi_dim};

template <typename Source,typename Allocator>
class basic_cbor_parser;

struct parse_state 
{
    parse_mode mode; 
    std::size_t length;
    bool pop_stringref_map_stack;
    std::size_t index{0};

    parse_state(parse_mode mode, std::size_t length, bool pop_stringref_map_stack = false) noexcept
        : mode(mode), length(length), pop_stringref_map_stack(pop_stringref_map_stack)
    {
    }

    parse_state(const parse_state&) = default;
    parse_state(parse_state&&) = default;
    parse_state& operator=(const parse_state&) = default;
    parse_state& operator=(parse_state&&) = default;
    
    ~parse_state() = default;
};

template <typename Source,typename Allocator=std::allocator<char>>
class basic_cbor_parser : public ser_context
{
    using char_type = char;
    using char_traits_type = std::char_traits<char>;
    using allocator_type = Allocator;
    using char_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<char_type>;
    using byte_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<uint8_t>;                  
    using tag_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<uint64_t>;                 
    using parse_state_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<parse_state>;                         
    using string_type = std::basic_string<char_type,char_traits_type,char_allocator_type>;
    using string_view_type = jsoncons::basic_string_view<char_type>;
    using byte_string_type = std::vector<uint8_t,byte_allocator_type>;

    struct mapped_string
    {
        using allocator_type = Allocator;

        jsoncons::cbor::detail::cbor_major_type type;
        string_type str;
        byte_string_type bytes;

        mapped_string(const string_type& str, const allocator_type& alloc = allocator_type())
            : type(jsoncons::cbor::detail::cbor_major_type::text_string), 
              str(str.data(), str.size(), alloc), bytes(alloc)
        {
        }

        mapped_string(const string_view_type& sv, const allocator_type& alloc = allocator_type())
            : type(jsoncons::cbor::detail::cbor_major_type::text_string), str(sv.data(), sv.size(), alloc), bytes(alloc)
        {
        }

        mapped_string(string_type&& str, const allocator_type& alloc = allocator_type())
            : type(jsoncons::cbor::detail::cbor_major_type::text_string), str(std::move(str), alloc), bytes(alloc)
        {
        }

        mapped_string(const byte_string_type& bytes, 
            const allocator_type& alloc = allocator_type())
            : type(jsoncons::cbor::detail::cbor_major_type::byte_string), str(alloc), bytes(bytes,alloc)
        {
        }

        mapped_string(const byte_string_view& bytes,
            const allocator_type& alloc = allocator_type())
            : type(jsoncons::cbor::detail::cbor_major_type::byte_string), str(alloc), bytes(bytes.begin(),bytes.end(),alloc)
        {
        }

        mapped_string(byte_string_type&& bytes, 
            const allocator_type& alloc = allocator_type())
            : type(jsoncons::cbor::detail::cbor_major_type::byte_string), str(alloc), bytes(std::move(bytes),alloc)
        {
        }

        mapped_string(const mapped_string&) = default;
        mapped_string(mapped_string&&) = default;

        mapped_string& operator=(const mapped_string&) = default;
        mapped_string& operator=(mapped_string&&) = default;

        mapped_string(const mapped_string& other, const allocator_type& alloc) 
            :  type(other.type), str(other.str,alloc), bytes(other.bytes,alloc)
        {
        }

        mapped_string(mapped_string&& other, const allocator_type& alloc) noexcept
            : type(other.type), str(std::move(other.str), alloc), bytes(std::move(other.bytes), alloc)
        {
        }

        ~mapped_string() = default;
    };

    using mapped_string_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<mapped_string>;                           
    using stringref_map = std::vector<mapped_string, mapped_string_allocator_type>;
    using stringref_map_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<stringref_map>;                           

    enum {stringref_tag, // 25
          stringref_namespace_tag, // 256
          item_tag,
          num_of_tags};

    bool more_{true};
    bool done_{false};
    bool cursor_mode_{false};
    int mark_level_{0};
    uint64_t raw_tag_{0};
    int nesting_depth_{0};

    std::bitset<num_of_tags> other_tags_;
    allocator_type alloc_;
    Source source_;
    int max_nesting_depth_;
    string_type text_buffer_;
    byte_string_type bytes_buffer_;
    std::vector<parse_state,parse_state_allocator_type> state_stack_;
    std::vector<std::shared_ptr<typed_array_iterator>> typed_array_stack_;
    std::vector<stringref_map,stringref_map_allocator_type> stringref_map_stack_;
    mdarray_order order_{};
    typed_array_tags array_tag_{};
    semantic_tag typed_array_tag_{};
    std::vector<std::size_t> extents_;
    std::size_t mdarray_size_{0};

    struct read_byte_string_from_buffer
    {
        byte_string_view bytes;

        read_byte_string_from_buffer(const byte_string_view& b)
            : bytes(b)
        {
        }
        template <typename Container>
        void operator()(Container& c, std::error_code&)
        {
            c.clear();
            c.reserve(bytes.size());
            for (auto b : bytes)
            {
                c.push_back(b);
            }
        }

        byte_string_view view(std::error_code&)
        {
            return bytes;
        }
    };

    struct read_byte_string_from_source
    {
        basic_cbor_parser<Source,Allocator>* source;

        read_byte_string_from_source(basic_cbor_parser<Source,Allocator>* source)
            : source(source)
        {
        }
        template <typename Container>
        void operator()(Container& cont, std::error_code& ec)
        {
            source->read_byte_string(cont,ec);
        }

        byte_string_view view(std::error_code& ec)
        {
            return source->read_byte_string_view(ec);
        }
    };

public:
    template <typename Sourceable>
    basic_cbor_parser(Sourceable&& source,
                      const cbor_decode_options& options = cbor_decode_options(),
                      const Allocator& alloc = Allocator())
       : alloc_(alloc),
         source_(std::forward<Sourceable>(source)),
         max_nesting_depth_(options.max_nesting_depth()),
         text_buffer_(alloc),
         bytes_buffer_(alloc),
         state_stack_(alloc),
         stringref_map_stack_(alloc)
    {
        state_stack_.emplace_back(parse_mode::root,0);
    }
    
    basic_cbor_parser(const basic_cbor_parser&) = delete;
    basic_cbor_parser(basic_cbor_parser&&) = delete;
    basic_cbor_parser& operator=(const basic_cbor_parser&) = delete;
    basic_cbor_parser& operator=(basic_cbor_parser&&) = delete;
    
    ~basic_cbor_parser() = default;

    void restart()
    {
        more_ = true;
    }

    void reset()
    {
        more_ = true;
        done_ = false;
        text_buffer_.clear();
        bytes_buffer_.clear();
        raw_tag_ = 0;
        state_stack_.clear();
        state_stack_.emplace_back(parse_mode::root,0);
        stringref_map_stack_.clear();
        nesting_depth_ = 0;
    }

    bool is_typed_array() const
    {
        return state_stack_.back().mode == parse_mode::typed_array;
    }

    bool is_multi_dim() const
    {
        return state_stack_.size() >=2 && state_stack_[state_stack_.size()-2].mode == parse_mode::multi_dim;
    }
    mdarray_order order() const
    {
        JSONCONS_ASSERT(!typed_array_stack_.empty());
        return typed_array_stack_.back()->order();
    }

    typed_array_tags array_tag() const
    {
        JSONCONS_ASSERT(!typed_array_stack_.empty());
        return typed_array_stack_.back()->array_tag();
    }

    jsoncons::span<uint8_t> array_buffer()
    {
        JSONCONS_ASSERT(!typed_array_stack_.empty());
        return typed_array_stack_.back()->array_buffer();
    }

    jsoncons::span<const std::size_t> extents() const 
    {
        JSONCONS_ASSERT(!typed_array_stack_.empty());
        return typed_array_stack_.back()->extents();
    }

    template <typename Sourceable>
    void reset(Sourceable&& source)
    {
        source_ = std::forward<Sourceable>(source);
        reset();
    }

    void cursor_mode(bool value)
    {
        cursor_mode_ = value;
    }

    int level() const
    {
        return static_cast<int>(state_stack_.size());
    }

    int mark_level() const 
    {
        return mark_level_;
    }

    void mark_level(int value)
    {
        mark_level_ = value;
    }

    bool done() const
    {
        return done_;
    }

    bool stopped() const
    {
        return !more_;
    }

    std::size_t line() const final
    {
        return 0;
    }

    std::size_t column() const final
    {
        return source_.position();
    }
    
    uint64_t raw_tag() const
    {
        return raw_tag_;
    }

    void to_end_array()
    {
        state_stack_.pop_back();
    }

    void parse(generic_visitor& visitor, std::error_code& ec)
    {
        while (!done_ && more_)
        {
            switch (state_stack_.back().mode)
            {
                case parse_mode::multi_dim:
                {
                    JSONCONS_ASSERT(!typed_array_stack_.empty());
                    typed_array_stack_.pop_back();
                    state_stack_.pop_back();
                    break;
                }
                case parse_mode::typed_array:
                {
                    JSONCONS_ASSERT(!typed_array_stack_.empty());
                    read_typed_array_item(visitor, ec);
                    auto iter = typed_array_stack_.back();
                    if (iter->done())
                    {
                        if (!is_multi_dim())
                        {
                            typed_array_stack_.pop_back();
                        }
                        state_stack_.pop_back();
                    }
                    break;
                }
                case parse_mode::array:
                {
                    if (is_multi_dim())
                    {
                        JSONCONS_ASSERT(!typed_array_stack_.empty());
                        auto iter = typed_array_stack_.back();
                        if (!iter->done())
                        {
                            iter->next(visitor, *this, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                        }
                        else
                        {
                            if (iter->count() != state_stack_.back().length)
                            {
                                //std::cout << state_stack_.back().index << "!=" << state_stack_.back().length << "\n";
                                ec = cbor_errc::bad_mdarray;
                                return;
                            }
                            end_classical_array_storage(ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                        }
                    }
                    else
                    {
                        if (state_stack_.back().index < state_stack_.back().length)
                        {
                            ++state_stack_.back().index;
                            read_item(visitor, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                        }
                        else
                        {
                            end_array(visitor, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                        }
                    }
                    break;
                }
                case parse_mode::indefinite_array:
                {
                    if (is_multi_dim()) 
                    {
                        JSONCONS_ASSERT(!typed_array_stack_.empty());
                        auto iter = typed_array_stack_.back();
                        if (!iter->done())
                        {
                            iter->next(visitor, *this, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                        }
                        else
                        {
                            auto c = source_.peek();
                            if (JSONCONS_UNLIKELY(c.eof))
                            {
                                ec = cbor_errc::unexpected_eof;
                                more_ = false;
                                return;
                            }
                            if (c.value == 0xff)
                            {
                                source_.ignore(1);
                            }
                            else
                            {
                                ec = cbor_errc::bad_mdarray;
                                return;
                            }
                            end_classical_array_storage(ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                        }
                    }
                    else
                    {
                        auto c = source_.peek();
                        if (JSONCONS_UNLIKELY(c.eof))
                        {
                            ec = cbor_errc::unexpected_eof;
                            more_ = false;
                            return;
                        }
                        if (c.value == 0xff)
                        {
                            source_.ignore(1);
                            end_array(visitor, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                        }
                        else
                        {
                            read_item(visitor, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                        }
                    }
                    break;
                }
                case parse_mode::map_key:
                {
                    if (state_stack_.back().index < state_stack_.back().length)
                    {
                        ++state_stack_.back().index;
                        state_stack_.back().mode = parse_mode::map_value;
                        read_item(visitor, ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            return;
                        }
                    }
                    else
                    {
                        end_object(visitor, ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            return;
                        }
                    }
                    break;
                }
                case parse_mode::map_value:
                {
                    state_stack_.back().mode = parse_mode::map_key;
                    read_item(visitor, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        return;
                    }
                    break;
                }
                case parse_mode::indefinite_map_key:
                {
                    auto c = source_.peek();
                    if (JSONCONS_UNLIKELY(c.eof))
                    {
                        ec = cbor_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    if (c.value == 0xff)
                    {
                        source_.ignore(1);
                        end_object(visitor, ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            return;
                        }
                    }
                    else
                    {
                        state_stack_.back().mode = parse_mode::indefinite_map_value;
                        read_item(visitor, ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            return;
                        }
                    }
                    break;
                }
                case parse_mode::indefinite_map_value:
                {
                    state_stack_.back().mode = parse_mode::indefinite_map_key;
                    read_item(visitor, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        return;
                    }
                    break;
                }
                case parse_mode::root:
                {
                    state_stack_.back().mode = parse_mode::accept;
                    read_item(visitor, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        return;
                    }
                    break;
                }
                case parse_mode::accept:
                {
                    JSONCONS_ASSERT(state_stack_.size() == 1);
                    state_stack_.clear();
                    more_ = false;
                    done_ = true;
                    visitor.flush();
                    break;
                }
            }
        }
    }

    void read_typed_array_item(generic_visitor& visitor, std::error_code& ec)
    {
        JSONCONS_ASSERT(!typed_array_stack_.empty());
        auto iter = typed_array_stack_.back();
        if (!iter->done())
        {
            iter->next(visitor, *this, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
            more_ = !cursor_mode_;
        }
    }

    void read_item(generic_visitor& visitor, std::error_code& ec)
    {
        read_tags(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            return;
        }
        auto c = source_.peek();
        if (JSONCONS_UNLIKELY(c.eof))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type(c.value);
        uint8_t info = get_additional_information_value(c.value);

        switch (major_type)
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                uint64_t val = read_uint64(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                if (!stringref_map_stack_.empty() && other_tags_[stringref_tag])
                {
                    other_tags_[stringref_tag] = false;
                    if (val >= stringref_map_stack_.back().size())
                    {
                        ec = cbor_errc::stringref_too_large;
                        more_ = false;
                        return;
                    }
                    auto index = static_cast<typename stringref_map::size_type>(val);
                    if (index != val)
                    {
                        ec = cbor_errc::number_too_large;
                        more_ = false;
                        return;
                    }
                    auto& str = stringref_map_stack_.back().at(index);
                    switch (str.type)
                    {
                        case jsoncons::cbor::detail::cbor_major_type::text_string:
                        {
                            handle_string(visitor, jsoncons::string_view(str.str.data(),str.str.length()),ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                            break;
                        }
                        case jsoncons::cbor::detail::cbor_major_type::byte_string:
                        {
                            read_byte_string_from_buffer read(byte_string_view(str.bytes));
                            read_byte_string(read, visitor, ec);
                            if (JSONCONS_UNLIKELY(ec))
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
                    if (other_tags_[item_tag])
                    {
                        if (raw_tag_ == 1)
                        {
                            tag = semantic_tag::epoch_second;
                        }
                        other_tags_[item_tag] = false;
                    }
                    visitor.uint64_value(val, tag, *this, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        return;
                    }
                    more_ = !cursor_mode_;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                int64_t val = read_int64(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                semantic_tag tag = semantic_tag::none;
                if (other_tags_[item_tag])
                {
                    if (raw_tag_ == 1)
                    {
                        tag = semantic_tag::epoch_second;
                    }
                    other_tags_[item_tag] = false;
                }
                visitor.int64_value(val, tag, *this, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                more_ = !cursor_mode_;
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::byte_string:
            {
                read_byte_string_from_source read(this);
                read_byte_string(read, visitor, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::text_string:
            {
                auto sv = read_text_string_view(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                auto result = unicode_traits::validate(sv.data(),sv.size());
                if (result.ec != unicode_traits::unicode_errc())
                {
                    ec = cbor_errc::invalid_utf8_text_string;
                    more_ = false;
                    return;
                }
                handle_string(visitor, sv, ec);
                if (JSONCONS_UNLIKELY(ec))
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
                        visitor.bool_value(false, semantic_tag::none, *this, ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            return;
                        }
                        more_ = !cursor_mode_;
                        source_.ignore(1);
                        break;
                    case 0x15:
                        visitor.bool_value(true, semantic_tag::none, *this, ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            return;
                        }
                        more_ = !cursor_mode_;
                        source_.ignore(1);
                        break;
                    case 0x16:
                        visitor.null_value(semantic_tag::none, *this, ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            return;
                        }
                        more_ = !cursor_mode_;
                        source_.ignore(1);
                        break;
                    case 0x17:
                        visitor.null_value(semantic_tag::undefined, *this, ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            return;
                        }
                        more_ = !cursor_mode_;
                        source_.ignore(1);
                        break;
                    case 0x19: // Half-Precision Float (two-byte IEEE 754)
                    {
                        uint64_t val = read_uint64(ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            return;
                        }
                        visitor.half_value(static_cast<uint16_t>(val), semantic_tag::none, *this, ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            return;
                        }
                        more_ = !cursor_mode_;
                        break;
                    }
                    case 0x1a: // Single-Precision Float (four-byte IEEE 754)
                    case 0x1b: // Double-Precision Float (eight-byte IEEE 754)
                    {
                        double val = read_double(ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            return;
                        }
                        semantic_tag tag = semantic_tag::none;
                        if (other_tags_[item_tag])
                        {
                            if (raw_tag_ == 1)
                            {
                                tag = semantic_tag::epoch_second;
                            }
                            other_tags_[item_tag] = false;
                        }
                        visitor.double_value(val, tag, *this, ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            return;
                        }
                        more_ = !cursor_mode_;
                        break;
                    }
                    default:
                    {
                        ec = cbor_errc::unknown_type;
                        more_ = false;
                        return;
                    }
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::array:
            {
                if (other_tags_[item_tag])
                {
                    switch (raw_tag_)
                    {
                        case 0x04:
                            text_buffer_.clear();
                            read_decimal_fraction(text_buffer_, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                            visitor.string_value(text_buffer_, semantic_tag::bigdec, *this, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                            more_ = !cursor_mode_;
                            break;
                        case 0x05:
                            text_buffer_.clear();
                            read_bigfloat(text_buffer_, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                            visitor.string_value(text_buffer_, semantic_tag::bigfloat, *this, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                            more_ = !cursor_mode_;
                            break;
                        case 40: // row-major storage
                            order_ = mdarray_order::row_major;
                            read_mdarray_header(visitor, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                            break;
                        case 1040: // column-major storage
                            order_ = mdarray_order::column_major;
                            read_mdarray_header(visitor, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                            break;
                        default:
                            begin_array(visitor, info, ec);
                            if (JSONCONS_UNLIKELY(ec))
                            {
                                return;
                            }
                            break;
                    }
                    other_tags_[item_tag] = false;
                }
                else
                {
                    begin_array(visitor, info, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        return;
                    }
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::map:
            {
                begin_object(visitor, info, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                break;
            }
            default:
                break;
        }
        other_tags_[item_tag] = false;
    }
private:

    void begin_array(generic_visitor& visitor, uint8_t info, std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(++nesting_depth_ > max_nesting_depth_))
        {
            ec = cbor_errc::max_nesting_depth_exceeded;
            more_ = false;
            return;
        } 
        semantic_tag tag = semantic_tag::none;
        bool pop_stringref_map_stack = false;
        if (other_tags_[stringref_namespace_tag])
        {
            stringref_map_stack_.emplace_back();
            other_tags_[stringref_namespace_tag] = false;
            pop_stringref_map_stack = true;
        }
        switch (info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length:
            {
                state_stack_.emplace_back(parse_mode::indefinite_array,0,pop_stringref_map_stack);
                visitor.begin_array(tag, *this, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                more_ = !cursor_mode_;
                source_.ignore(1);
                break;
            }
            default: // definite length
            {
                std::size_t len = read_size(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                state_stack_.emplace_back(parse_mode::array,len,pop_stringref_map_stack);
                visitor.begin_array(len, tag, *this, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                more_ = !cursor_mode_;
                break;
            }
        }
    }

    void end_array(generic_visitor& visitor, std::error_code& ec)
    {
        --nesting_depth_;

        visitor.end_array(*this, ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            return;
        }
        more_ = !cursor_mode_;
        if (level() == mark_level_)
        {
            more_ = false;
        }
        if (state_stack_.back().pop_stringref_map_stack)
        {
            stringref_map_stack_.pop_back();
        }
        state_stack_.pop_back();
    }

    void begin_classical_array_storage(uint8_t info, std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(++nesting_depth_ > max_nesting_depth_))
        {
            ec = cbor_errc::max_nesting_depth_exceeded;
            more_ = false;
            return;
        }
        bool pop_stringref_map_stack = false;
        if (other_tags_[stringref_namespace_tag])
        {
            stringref_map_stack_.emplace_back();
            other_tags_[stringref_namespace_tag] = false;
            pop_stringref_map_stack = true;
        }
        switch (info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length:
            {
                state_stack_.emplace_back(parse_mode::indefinite_array, 0, pop_stringref_map_stack);
                more_ = !cursor_mode_;
                source_.ignore(1);
                break;
            }
            default: // definite length
            {
                std::size_t len = read_size(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                state_stack_.emplace_back(parse_mode::array, len, pop_stringref_map_stack);
                more_ = !cursor_mode_;
                break;
            }
        }
    }

    void end_classical_array_storage(std::error_code&)
    {
        --nesting_depth_;

        more_ = !cursor_mode_;
        if (level() == mark_level_)
        {
            more_ = false;
        }
        if (state_stack_.back().pop_stringref_map_stack)
        {
            stringref_map_stack_.pop_back();
        }
        state_stack_.pop_back();
    }

    void begin_object(generic_visitor& visitor, uint8_t info, std::error_code& ec)
    {
        if (JSONCONS_UNLIKELY(++nesting_depth_ > max_nesting_depth_))
        {
            ec = cbor_errc::max_nesting_depth_exceeded;
            more_ = false;
            return;
        } 
        bool pop_stringref_map_stack = false;
        if (other_tags_[stringref_namespace_tag])
        {
            stringref_map_stack_.emplace_back();
            other_tags_[stringref_namespace_tag] = false;
            pop_stringref_map_stack = true;
        }
        switch (info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length: 
            {
                state_stack_.emplace_back(parse_mode::indefinite_map_key,0,pop_stringref_map_stack);
                visitor.begin_object(semantic_tag::none, *this, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                more_ = !cursor_mode_;
                source_.ignore(1);
                break;
            }
            default: // definite_length
            {
                std::size_t len = read_size(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                state_stack_.emplace_back(parse_mode::map_key,len,pop_stringref_map_stack);
                visitor.begin_object(len, semantic_tag::none, *this, ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                more_ = !cursor_mode_;
                break;
            }
        }
    }

    void end_object(generic_visitor& visitor, std::error_code& ec)
    {
        --nesting_depth_;
        visitor.end_object(*this, ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            return;
        }
        more_ = !cursor_mode_;
        if (level() == mark_level_)
        {
            more_ = false;
        }
        if (state_stack_.back().pop_stringref_map_stack)
        {
            stringref_map_stack_.pop_back();
        }
        state_stack_.pop_back();
    }

    string_view_type read_text_string_view(std::error_code& ec)
    {
        auto c = source_.peek();
        if (JSONCONS_UNLIKELY(c.eof))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return string_view_type();
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type(c.value);
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::text_string);
        uint8_t info = get_additional_information_value(c.value);

        if (info == jsoncons::cbor::detail::additional_info::indefinite_length)
        {
            text_buffer_.clear();
            source_.ignore(1);
            iterate_string_chunks(text_buffer_, major_type, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return string_view_type();
            }
            return string_view_type(text_buffer_.data(), text_buffer_.size());
        }

        std::size_t length = read_size(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            return string_view_type();
        }
        auto data = source_.read_span(length, text_buffer_);
        if (data.size() != length)
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return string_view_type();
        }
        string_view_type sv(reinterpret_cast<const char_type*>(data.data()), data.size());
        if (!stringref_map_stack_.empty() &&
            sv.length() >= jsoncons::cbor::detail::min_length_for_stringref(stringref_map_stack_.back().size()))
        {
            stringref_map_stack_.back().emplace_back(mapped_string(sv,alloc_));
        }
        return sv;
    }

    void read_text_string(string_type& str, std::error_code& ec)
    {
        auto c = source_.peek();
        if (JSONCONS_UNLIKELY(c.eof))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type(c.value);
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::text_string);
        uint8_t info = get_additional_information_value(c.value);

        if (info == jsoncons::cbor::detail::additional_info::indefinite_length)
        {
            source_.ignore(1);
            iterate_string_chunks(str, major_type, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
        }
        else
        {
            std::size_t length = read_size(ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
            if (source_reader<Source>::read(source_, str, length) != length)
            {
                ec = cbor_errc::unexpected_eof;
            }
        }

        if (!stringref_map_stack_.empty() && 
            info != jsoncons::cbor::detail::additional_info::indefinite_length &&
            str.length() >= jsoncons::cbor::detail::min_length_for_stringref(stringref_map_stack_.back().size()))
        {
            stringref_map_stack_.back().emplace_back(mapped_string(str,alloc_));
        }
    }

    byte_string_view read_byte_string_view(std::error_code& ec)
    {
        auto c = source_.peek();
        if (JSONCONS_UNLIKELY(c.eof))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return byte_string_view();
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type(c.value);
        uint8_t info = get_additional_information_value(c.value);

        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::byte_string);

        if (info == jsoncons::cbor::detail::additional_info::indefinite_length)
        {
            bytes_buffer_.clear();
            source_.ignore(1);
            iterate_string_chunks(bytes_buffer_, major_type, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return byte_string_view();
            }
            return byte_string_view(bytes_buffer_.data(), bytes_buffer_.size());
        }

        std::size_t length = read_size(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            return byte_string_view();
        }
        auto data = source_.read_span(length, bytes_buffer_);
        if (data.size() != length)
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return byte_string_view();
        }
        
        byte_string_view bytes(data.data(), data.size());
        if (!stringref_map_stack_.empty() &&
            bytes.size() >= jsoncons::cbor::detail::min_length_for_stringref(stringref_map_stack_.back().size()))
        {
            stringref_map_stack_.back().emplace_back(mapped_string(bytes, alloc_));
        }
        return bytes;
    }

    std::size_t read_size(std::error_code& ec)
    {
        uint64_t u = read_uint64(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            return 0;
        }
        std::size_t len = static_cast<std::size_t>(u);
        if (len != u)
        {
            ec = cbor_errc::number_too_large;
            more_ = false;
        }
        return len;
    }

    void read_byte_string(byte_string_type& v, std::error_code& ec)
    {
        v.clear();
        auto c = source_.peek();
        if (JSONCONS_UNLIKELY(c.eof))
        {
            ec = cbor_errc::unexpected_eof;
            return;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type(c.value);
        uint8_t info = get_additional_information_value(c.value);

        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::byte_string);

        if (info == jsoncons::cbor::detail::additional_info::indefinite_length)
        {
            source_.ignore(1);
            iterate_string_chunks(v, major_type, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
        }
        else 
        {
            std::size_t length = read_size(ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
            if (source_reader<Source>::read(source_, v, length) != length)
            {
                ec = cbor_errc::unexpected_eof;
                return;
            }
            if (!stringref_map_stack_.empty() &&
                v.size() >= jsoncons::cbor::detail::min_length_for_stringref(stringref_map_stack_.back().size()))
            {
                stringref_map_stack_.back().emplace_back(mapped_string(v, alloc_));
            }
        }
    }

    template <typename Container>
    void iterate_string_chunks(Container& v, jsoncons::cbor::detail::cbor_major_type type, std::error_code& ec)
    {
        bool done = false;
        while (!done)
        {
            auto c = source_.peek();
            if (JSONCONS_UNLIKELY(c.eof))
            {
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return;
            }
            if (c.value == 0xff)
            {
                done = true;
                source_.ignore(1);
                continue;
            }

            jsoncons::cbor::detail::cbor_major_type major_type = get_major_type(c.value);
            if (major_type != type)
            {
                ec = cbor_errc::illegal_chunked_string;
                more_ = false;
                return;
            }
            uint8_t info = get_additional_information_value(c.value);
            if (info == jsoncons::cbor::detail::additional_info::indefinite_length)
            {
                ec = cbor_errc::illegal_chunked_string;
                more_ = false;
                return;
            }

            std::size_t length = read_size(ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
            const std::size_t offset = v.size();
            if (source_reader<Source>::read(source_, v, length) != length)
            {
                ec = cbor_errc::unexpected_eof;
            }
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
            if (type == jsoncons::cbor::detail::cbor_major_type::text_string)
            {
                // RFC 8949 3.2.3: each chunk of an indefinite-length text
                // string must itself be well-formed UTF-8, so a code point
                // may not be split across chunks.
                auto result = unicode_traits::validate(
                    reinterpret_cast<const char*>(v.data()) + offset, length);
                if (result.ec != unicode_traits::unicode_errc())
                {
                    ec = cbor_errc::invalid_utf8_text_string;
                    more_ = false;
                    return;
                }
            }
        }
    }

    uint64_t read_uint64(std::error_code& ec)
    {
        uint64_t val = 0;

        uint8_t initial_b;
        if (source_.read(&initial_b, 1) == 0)
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return 0;
        }
        uint8_t info = get_additional_information_value(initial_b);
        switch (info)
        {
            case JSONCONS_EXT_CBOR_0x00_0x17: // Integer 0x00..0x17 (0..23)
            {
                val = info;
                break;
            }

            case 0x18: // Unsigned integer (one-byte uint8_t follows)
            {
                uint8_t b;
                if (source_.read(&b, 1) == 0)
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return val;
                }
                val = b;
                break;
            }

            case 0x19: // Unsigned integer (two-byte uint16_t follows)
            {
                uint8_t buf[sizeof(uint16_t)];
                if (source_.read(buf, sizeof(uint16_t)) != sizeof(uint16_t))
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return val;
                }
                val = binary::big_to_native<uint16_t>(buf, sizeof(buf));
                break;
            }

            case 0x1a: // Unsigned integer (four-byte uint32_t follows)
            {
                uint8_t buf[sizeof(uint32_t)];
                if (source_.read(buf, sizeof(uint32_t)) != sizeof(uint32_t))
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return val;
                }
                val = binary::big_to_native<uint32_t>(buf, sizeof(buf));
                break;
            }

            case 0x1b: // Unsigned integer (eight-byte uint64_t follows)
            {
                uint8_t buf[sizeof(uint64_t)];
                if (source_.read(buf, sizeof(uint64_t)) != sizeof(uint64_t))
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return val;
                }
                val = binary::big_to_native<uint64_t>(buf, sizeof(buf));
                break;
            }
            default:
                break;
        }
        return val;
    }

    int64_t read_int64(std::error_code& ec)
    {
        int64_t val = 0;

        auto ch = source_.peek();
        if (ch.eof)
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return val;
        }

        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type(ch.value);
        uint8_t info = get_additional_information_value(ch.value);
        switch (major_type)
        {
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
                source_.ignore(1);
                switch (info)
                {
                    case JSONCONS_EXT_CBOR_0x00_0x17: // 0x00..0x17 (0..23)
                    {
                        val = static_cast<int8_t>(- 1 - info);
                        break;
                    }
                    case 0x18: // Negative integer (one-byte uint8_t follows)
                        {
                            uint8_t b;
                            if (source_.read(&b, 1) == 0)
                            {
                                ec = cbor_errc::unexpected_eof;
                                more_ = false;
                                return val;
                            }
                            val = static_cast<int64_t>(-1) - static_cast<int64_t>(b);
                            break;
                        }

                    case 0x19: // Negative integer -1-n (two-byte uint16_t follows)
                        {
                            uint8_t buf[sizeof(uint16_t)];
                            if (source_.read(buf, sizeof(uint16_t)) != sizeof(uint16_t))
                            {
                                ec = cbor_errc::unexpected_eof;
                                more_ = false;
                                return val;
                            }
                            auto x = binary::big_to_native<uint16_t>(buf, sizeof(buf));
                            val = static_cast<int64_t>(-1)- x;
                            break;
                        }

                    case 0x1a: // Negative integer -1-n (four-byte uint32_t follows)
                        {
                            uint8_t buf[sizeof(uint32_t)];
                            if (source_.read(buf, sizeof(uint32_t)) != sizeof(uint32_t))
                            {
                                ec = cbor_errc::unexpected_eof;
                                more_ = false;
                                return val;
                            }
                            auto x = binary::big_to_native<uint32_t>(buf, sizeof(buf));
                            val = static_cast<int64_t>(-1)- x;
                            break;
                        }

                    case 0x1b: // Negative integer -1-n (eight-byte uint64_t follows)
                        {
                            uint8_t buf[sizeof(uint64_t)];
                            if (source_.read(buf, sizeof(uint64_t)) != sizeof(uint64_t))
                            {
                                ec = cbor_errc::unexpected_eof;
                                more_ = false;
                                return val;
                            }
                            auto x = binary::big_to_native<uint64_t>(buf, sizeof(buf));
                            val = static_cast<int64_t>(-1)- static_cast<int64_t>(x);
                            break;
                        }
                }
                break;

                case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
                {
                    uint64_t x = read_uint64(ec);
                    if (JSONCONS_UNLIKELY(ec))
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

    double read_double(std::error_code& ec)
    {
        double val = 0;

        uint8_t b;
        if (source_.read(&b, 1) == 0)
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return 0;
        }
        uint8_t info = get_additional_information_value(b);
        switch (info)
        {
        case 0x1a: // Single-Precision Float (four-byte IEEE 754)
            {
                uint8_t buf[sizeof(float)];
                if (source_.read(buf, sizeof(float)) !=sizeof(float)) 
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return 0;
                }
                val = binary::big_to_native<float>(buf, sizeof(buf));
                break;
            }

        case 0x1b: //  Double-Precision Float (eight-byte IEEE 754)
            {
                uint8_t buf[sizeof(double)];
                if (source_.read(buf, sizeof(double)) != sizeof(double))
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return 0;
                }
                val = binary::big_to_native<double>(buf, sizeof(buf));
                break;
            }
            default:
                break;
        }
        
        return val;
    }

    void read_decimal_fraction(string_type& result, std::error_code& ec)
    {
        std::size_t size = read_size(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            return;
        }
        if (size != 2)
        {
            ec = cbor_errc::invalid_decimal_fraction;
            more_ = false;
            return;
        }

        auto c = source_.peek();
        if (JSONCONS_UNLIKELY(c.eof))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        int32_t exponent = 0;
        switch (get_major_type(c.value))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                auto u = read_uint64(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                if (u > static_cast<uint64_t>((std::numeric_limits<int>::max)()))
                {
                    ec = cbor_errc::invalid_decimal_fraction;
                    more_ = false;
                    return;
                }
                exponent = static_cast<int>(u);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                auto u = read_int64(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                if (u < static_cast<int64_t>((std::numeric_limits<int>::min)()) || u > static_cast<int64_t>((std::numeric_limits<int>::max)()))
                {
                    ec = cbor_errc::invalid_decimal_fraction;
                    more_ = false;
                    return;
                }
                exponent = static_cast<int>(u);
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_decimal_fraction;
                more_ = false;
                return;
            }
        }

        string_type str(alloc_);

        c = source_.peek();
        if (JSONCONS_UNLIKELY(c.eof))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }

        switch (get_major_type(c.value))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                uint64_t val = read_uint64(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                jsoncons::from_integer(val, str);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                int64_t val = read_int64(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                jsoncons::from_integer(val, str);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::semantic_tag:
            {
                uint8_t b;
                if (source_.read(&b, 1) == 0)
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return;
                }
                uint8_t tag = get_additional_information_value(b);
                c = source_.peek();
                if (JSONCONS_UNLIKELY(c.eof))
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return;
                }

                if (get_major_type(c.value) == jsoncons::cbor::detail::cbor_major_type::byte_string)
                {
                    bytes_buffer_.clear();
                    read_byte_string(bytes_buffer_, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    if (tag == 2)
                    {
                        bigint n = bigint::from_bytes_be(1, bytes_buffer_.data(), bytes_buffer_.size());
                        n.write_string(str);
                    }
                    else if (tag == 3)
                    {
                        bigint n = bigint::from_bytes_be(1, bytes_buffer_.data(), bytes_buffer_.size());
                        n = -1 - n;
                        n.write_string(str);
                    }
                }
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_decimal_fraction;
                more_ = false;
                return;
            }
        }

        if (str.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)()))
        {
            ec = cbor_errc::invalid_decimal_fraction;
            more_ = false;
            return;
        }
        int length = static_cast<int>(str.size());
        if (length > 0)
        {
            if (str[0] == '-')
            {
                result.push_back('-');
                jsoncons::prettify_string(str.data()+1, length-1, exponent, -4, 17, result);
            }
            else
            {
                jsoncons::prettify_string(str.data(), length, exponent, -4, 17, result);
            }
        }
        else
        {
            ec = cbor_errc::invalid_decimal_fraction;
            more_ = false;
            return;
        }
    }

    void read_bigfloat(string_type& str, std::error_code& ec)
    {
        std::size_t size = read_size(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            return;
        }
        if (size != 2)
        {
            ec = cbor_errc::invalid_bigfloat;
            more_ = false;
            return;
        }

        auto c = source_.peek();
        if (JSONCONS_UNLIKELY(c.eof))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        int64_t exponent = 0;
        switch (get_major_type(c.value))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                exponent = read_uint64(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                exponent = read_int64(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_bigfloat;
                more_ = false;
                return;
            }
        }

        c = source_.peek();
        if (JSONCONS_UNLIKELY(c.eof))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        switch (get_major_type(c.value))
        {
            case jsoncons::cbor::detail::cbor_major_type::unsigned_integer:
            {
                uint64_t val = read_uint64(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                str.push_back('0');
                str.push_back('x');
                jsoncons::integer_to_hex(val, str);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::negative_integer:
            {
                int64_t val = read_int64(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    return;
                }
                str.push_back('-');
                str.push_back('0');
                str.push_back('x');
                jsoncons::integer_to_hex(static_cast<uint64_t>(-1 - val) + 1u, str);
                break;
            }
            case jsoncons::cbor::detail::cbor_major_type::semantic_tag:
            {
                uint8_t b;
                if (source_.read(&b, 1) == 0)
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return;
                }
                uint8_t tag = get_additional_information_value(b);

                c = source_.peek();
                if (JSONCONS_UNLIKELY(c.eof))
                {
                    ec = cbor_errc::unexpected_eof;
                    more_ = false;
                    return;
                }

                if (get_major_type(c.value) == jsoncons::cbor::detail::cbor_major_type::byte_string)
                {
                    bytes_buffer_.clear(); 
                    read_byte_string(bytes_buffer_, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    if (tag == 2)
                    {
                        str.push_back('0');
                        str.push_back('x');
                        bigint n = bigint::from_bytes_be(1, bytes_buffer_.data(), bytes_buffer_.size());
                        n.write_string_hex(str);
                    }
                    else if (tag == 3)
                    {
                        str.push_back('-');
                        str.push_back('0');
                        bigint n = bigint::from_bytes_be(1, bytes_buffer_.data(), bytes_buffer_.size());
                        n = -1 - n;
                        n.write_string_hex(str);
                        str[2] = 'x'; // overwrite minus
                    }
                }
                break;
            }
            default:
            {
                ec = cbor_errc::invalid_bigfloat;
                more_ = false;
                return;
            }
        }

        str.push_back('p');
        if (exponent >=0)
        {
            jsoncons::integer_to_hex(static_cast<uint64_t>(exponent), str);
        }
        else
        {
            str.push_back('-');
            jsoncons::integer_to_hex(static_cast<uint64_t>(-1 - exponent) + 1u, str);
        }
    }

    static jsoncons::cbor::detail::cbor_major_type get_major_type(uint8_t type)
    {
        static constexpr uint8_t major_type_shift = 0x05;
        uint8_t value = type >> major_type_shift;
        return static_cast<jsoncons::cbor::detail::cbor_major_type>(value);
    }

    static uint8_t get_additional_information_value(uint8_t type)
    {
        static constexpr uint8_t additional_information_mask = (1U << 5) - 1;
        uint8_t value = type & additional_information_mask;
        return value;
    }

    void read_tags(std::error_code& ec)
    {
        auto c = source_.peek();
        if (JSONCONS_UNLIKELY(c.eof))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type(c.value);

        while (major_type == jsoncons::cbor::detail::cbor_major_type::semantic_tag)
        {
            uint64_t val = read_uint64(ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
            switch(val)
            {
                case 25: // stringref
                    other_tags_[stringref_tag] = true;
                    break;
                case 256: // stringref-namespace
                    other_tags_[stringref_namespace_tag] = true;
                    break;
                default:
                    other_tags_[item_tag] = true;
                    raw_tag_ = val;
                    break;
            }
            c = source_.peek();
            if (JSONCONS_UNLIKELY(c.eof))
            {
                ec = cbor_errc::unexpected_eof;
                more_ = false;
                return;
            }
            major_type = get_major_type(c.value);
        }
    }

    void handle_string(generic_visitor& visitor, const jsoncons::string_view& v, std::error_code& ec)
    {
        semantic_tag tag = semantic_tag::none;
        if (other_tags_[item_tag])
        {
            switch (raw_tag_)
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
            other_tags_[item_tag] = false;
        }
        visitor.string_value(v, tag, *this, ec);
        more_ = !cursor_mode_;
    }

    static jsoncons::endian get_typed_array_endianness(const uint8_t tag)
    {
        return ((tag & detail::cbor_array_tags_e_mask) >> detail::cbor_array_tags_e_shift) == 0 ? jsoncons::endian::big : jsoncons::endian::little; 
    }

    static std::size_t get_typed_array_bytes_per_element(const uint8_t tag)
    {
        const uint8_t f = (tag & detail::cbor_array_tags_f_mask) >> detail::cbor_array_tags_f_shift; 
        const uint8_t ll = (tag & detail::cbor_array_tags_ll_mask) >> detail::cbor_array_tags_ll_shift; 

        return std::size_t(1) << (f + ll); 
    }

    template <typename Read>
    void read_byte_string(Read read, generic_visitor& visitor, std::error_code& ec)
    {
        if (other_tags_[item_tag])
        {
            switch (raw_tag_)
            {
                case 0x2:
                {
                    auto bytes = read.view(ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    bigint n = bigint::from_bytes_be(1, bytes.data(), bytes.size());
                    text_buffer_.clear();
                    n.write_string(text_buffer_);
                    visitor.string_value(text_buffer_, semantic_tag::bigint, *this, ec);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x3:
                {
                    auto bytes = read.view(ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    bigint n = bigint::from_bytes_be(1, bytes.data(), bytes.size());
                    n = -1 - n;
                    text_buffer_.clear();
                    n.write_string(text_buffer_);
                    visitor.string_value(text_buffer_, semantic_tag::bigint, *this, ec);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x15:
                case 0x16:
                case 0x17:
                {
                    auto bytes = read.view(ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    const semantic_tag tag = raw_tag_ == 0x15 ? semantic_tag::base64url
                        : raw_tag_ == 0x16 ? semantic_tag::base64
                        : semantic_tag::base16;
                    visitor.byte_string_value(bytes, tag, *this, ec);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x40:
                {
                    array_tag_ = typed_array_tags::uint8;
                    byte_string_type array_buffer(alloc_);
                    read(array_buffer, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    auto ta = typed_array_cast<const uint8_t>(array_buffer);
                    if (!cursor_mode_ && state_stack_.back().mode == parse_mode::multi_dim) 
                    {
                        if (mdarray_size_ != ta.size())
                        {
                            ec = cbor_errc::bad_extents;
                            more_ = false;
                            return;
                        }
                        auto iter = std::make_shared<mdarray_iterator<uint8_t,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::uint8, extents_, order_, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    else
                    {
                        auto iter = std::make_shared<oned_typed_array_iterator<uint8_t,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::uint8, semantic_tag::none, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    state_stack_.emplace_back(parse_mode::typed_array, ta.size(), false);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x44:
                {
                    array_tag_ = typed_array_tags::uint8;
                    typed_array_tag_ = semantic_tag::clamped;
                    byte_string_type array_buffer(alloc_);
                    read(array_buffer, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    auto ta = typed_array_cast<const uint8_t>(array_buffer);
                    if (!cursor_mode_ && state_stack_.back().mode == parse_mode::multi_dim) 
                    {
                        if (mdarray_size_ != ta.size())
                        {
                            ec = cbor_errc::bad_extents;
                            more_ = false;
                            return;
                        }
                        auto iter = std::make_shared<mdarray_iterator<uint8_t,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::uint8, extents_, order_, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    else
                    {
                        auto iter = std::make_shared<oned_typed_array_iterator<uint8_t,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::uint8, semantic_tag::clamped, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    state_stack_.emplace_back(parse_mode::typed_array, ta.size(), false);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x41:
                case 0x45:
                {
                    array_tag_ = typed_array_tags::uint16;
                    byte_string_type array_buffer(alloc_);
                    read(array_buffer, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    const uint8_t tag = (uint8_t)raw_tag_;
                    jsoncons::endian e = get_typed_array_endianness(tag); 
                    auto ta = typed_array_cast<uint16_t>(array_buffer);
                    if (e != jsoncons::endian::native)
                    {
                        for (std::size_t i = 0; i < ta.size(); ++i)
                        {
                            ta[i] = binary::byte_swap<uint16_t>(ta[i]);
                        }
                    }
                    if (!cursor_mode_ && state_stack_.back().mode == parse_mode::multi_dim) 
                    {
                        if (mdarray_size_ != ta.size())
                        {
                            ec = cbor_errc::bad_extents;
                            more_ = false;
                            return;
                        }
                        auto iter = std::make_shared<mdarray_iterator<uint16_t,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::uint16, extents_, order_, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    else
                    {
                        auto iter = std::make_shared<oned_typed_array_iterator<uint16_t,jsoncons::identity,Allocator>>(std::move(array_buffer),
                            typed_array_tags::uint16, semantic_tag::none, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    state_stack_.emplace_back(parse_mode::typed_array, ta.size(), false);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x42:
                case 0x46:
                {
                    array_tag_ = typed_array_tags::uint32;
                    byte_string_type array_buffer(alloc_);
                    read(array_buffer, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    const uint8_t tag = (uint8_t)raw_tag_;
                    jsoncons::endian e = get_typed_array_endianness(tag);
                    auto ta = typed_array_cast<uint32_t>(array_buffer);
                    if (e != jsoncons::endian::native)
                    {
                        for (std::size_t i = 0; i < ta.size(); ++i)
                        {
                            ta[i] = binary::byte_swap<uint32_t>(ta[i]);
                        }
                    }
                    if (!cursor_mode_ && state_stack_.back().mode == parse_mode::multi_dim) 
                    {
                        if (mdarray_size_ != ta.size())
                        {
                            ec = cbor_errc::bad_extents;
                            more_ = false;
                            return;
                        }
                        auto iter = std::make_shared<mdarray_iterator<uint32_t,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::uint32, extents_, order_, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    else
                    {
                        auto iter = std::make_shared<oned_typed_array_iterator<uint32_t,jsoncons::identity,Allocator>>(std::move(array_buffer),
                            typed_array_tags::uint32, semantic_tag::none, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    state_stack_.emplace_back(parse_mode::typed_array, ta.size(), false);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x43:
                case 0x47:
                {
                    array_tag_ = typed_array_tags::uint64;
                    byte_string_type array_buffer(alloc_);
                    read(array_buffer, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    const uint8_t tag = (uint8_t)raw_tag_;
                    jsoncons::endian e = get_typed_array_endianness(tag); 
                    auto ta = typed_array_cast<uint64_t>(array_buffer);
                    if (e != jsoncons::endian::native)
                    {
                        for (std::size_t i = 0; i < ta.size(); ++i)
                        {
                            ta[i] = binary::byte_swap<uint64_t>(ta[i]);
                        }
                    }
                    if (!cursor_mode_ && state_stack_.back().mode == parse_mode::multi_dim) 
                    {
                        if (mdarray_size_ != ta.size())
                        {
                            ec = cbor_errc::bad_extents;
                            more_ = false;
                            return;
                        }
                        auto iter = std::make_shared<mdarray_iterator<uint64_t,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::uint64, extents_, order_, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    else
                    {
                        auto iter = std::make_shared<oned_typed_array_iterator<uint64_t,jsoncons::identity,Allocator>>(std::move(array_buffer),
                            typed_array_tags::uint64, semantic_tag::none, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    state_stack_.emplace_back(parse_mode::typed_array, ta.size(), false);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x48:
                {
                    array_tag_ = typed_array_tags::int8;
                    byte_string_type array_buffer(alloc_);
                    read(array_buffer, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    auto ta = typed_array_cast<int8_t>(array_buffer);
                    if (!cursor_mode_ && state_stack_.back().mode == parse_mode::multi_dim) 
                    {
                        if (mdarray_size_ != ta.size())
                        {
                            ec = cbor_errc::bad_extents;
                            more_ = false;
                            return;
                        }
                        auto iter = std::make_shared<mdarray_iterator<int8_t,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::int8, extents_, order_, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    else
                    {
                        auto iter = std::make_shared<oned_typed_array_iterator<int8_t,jsoncons::identity,Allocator>>(std::move(array_buffer),
                            typed_array_tags::int8, semantic_tag::none, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    state_stack_.emplace_back(parse_mode::typed_array, ta.size(), false);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x49:
                case 0x4d:
                {
                    array_tag_ = typed_array_tags::int16;
                    byte_string_type array_buffer(alloc_);
                    read(array_buffer, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    const uint8_t tag = (uint8_t)raw_tag_;
                    jsoncons::endian e = get_typed_array_endianness(tag); 
                    auto ta = typed_array_cast<int16_t>(array_buffer);
                    if (e != jsoncons::endian::native)
                    {
                        for (std::size_t i = 0; i < ta.size(); ++i)
                        {
                            ta[i] = binary::byte_swap<int16_t>(ta[i]);
                        }
                    }
                    if (!cursor_mode_ && state_stack_.back().mode == parse_mode::multi_dim) 
                    {
                        if (mdarray_size_ != ta.size())
                        {
                            ec = cbor_errc::bad_extents;
                            more_ = false;
                            return;
                        }
                        auto iter = std::make_shared<mdarray_iterator<int16_t,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::int16, extents_, order_, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    else
                    {
                        auto iter = std::make_shared<oned_typed_array_iterator<int16_t,jsoncons::identity,Allocator>>(std::move(array_buffer),
                            typed_array_tags::int16, semantic_tag::none, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    state_stack_.emplace_back(parse_mode::typed_array, ta.size(), false);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x4a:
                case 0x4e:
                {
                    array_tag_ = typed_array_tags::int32;
                    byte_string_type array_buffer(alloc_);
                    read(array_buffer, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    const uint8_t tag = (uint8_t)raw_tag_;
                    jsoncons::endian e = get_typed_array_endianness(tag); 
                    auto ta = typed_array_cast<int32_t>(array_buffer);
                    if (e != jsoncons::endian::native)
                    {
                        for (std::size_t i = 0; i < ta.size(); ++i)
                        {
                            ta[i] = binary::byte_swap<int32_t>(ta[i]);
                        }
                    }
                    if (!cursor_mode_ && state_stack_.back().mode == parse_mode::multi_dim) 
                    {
                        if (mdarray_size_ != ta.size())
                        {
                            ec = cbor_errc::bad_extents;
                            more_ = false;
                            return;
                        }
                        auto iter = std::make_shared<mdarray_iterator<int32_t,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::int32, extents_, order_, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    else
                    {
                        auto iter = std::make_shared<oned_typed_array_iterator<int32_t,jsoncons::identity,Allocator>>(std::move(array_buffer),
                            typed_array_tags::int32, semantic_tag::none, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    state_stack_.emplace_back(parse_mode::typed_array, ta.size(), false);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x4b:
                case 0x4f:
                {
                    array_tag_ = typed_array_tags::int64;
                    byte_string_type array_buffer(alloc_);
                    read(array_buffer, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    const uint8_t tag = (uint8_t)raw_tag_;
                    jsoncons::endian e = get_typed_array_endianness(tag); 
                    auto ta = typed_array_cast<int64_t>(array_buffer);
                    if (e != jsoncons::endian::native)
                    {
                        for (std::size_t i = 0; i < ta.size(); ++i)
                        {
                            ta[i] = binary::byte_swap<int64_t>(ta[i]);
                        }
                    }
                    if (!cursor_mode_ && state_stack_.back().mode == parse_mode::multi_dim) 
                    {
                        if (mdarray_size_ != ta.size())
                        {
                            ec = cbor_errc::bad_extents;
                            more_ = false;
                            return;
                        }
                        auto iter = std::make_shared<mdarray_iterator<int64_t,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::int64, extents_, order_, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    else
                    {
                        auto iter = std::make_shared<oned_typed_array_iterator<int64_t,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::int64, semantic_tag::none, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    state_stack_.emplace_back(parse_mode::typed_array, ta.size(), false);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x50:
                case 0x54:
                {
                    array_tag_ = typed_array_tags::half_float;
                    byte_string_type array_buffer(alloc_);
                    read(array_buffer, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    const uint8_t tag = (uint8_t)raw_tag_;
                    jsoncons::endian e = get_typed_array_endianness(tag); 
                    auto ta = typed_array_cast<uint16_t>(array_buffer);
                    if (e != jsoncons::endian::native)
                    {
                        for (std::size_t i = 0; i < ta.size(); ++i)
                        {
                            ta[i] = binary::byte_swap<uint16_t>(ta[i]);
                        }
                    }
                    if (!cursor_mode_ && state_stack_.back().mode == parse_mode::multi_dim) 
                    {
                        if (mdarray_size_ != ta.size())
                        {
                            ec = cbor_errc::bad_extents;
                            more_ = false;
                            return;
                        }
                        auto iter = std::make_shared<mdarray_iterator<uint16_t,decode_half,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::half_float, extents_, order_, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    else
                    {
                        auto iter = std::make_shared<oned_typed_array_iterator<uint16_t,decode_half,Allocator>>(std::move(array_buffer),
                            typed_array_tags::half_float, semantic_tag::none, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    state_stack_.emplace_back(parse_mode::typed_array, ta.size(), false);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x51:
                case 0x55:
                {
                    array_tag_ = typed_array_tags::float32;
                    byte_string_type array_buffer(alloc_);
                    read(array_buffer, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    const uint8_t tag = (uint8_t)raw_tag_;
                    jsoncons::endian e = get_typed_array_endianness(tag); 
                    auto ta = typed_array_cast<float>(array_buffer);
                    if (e != jsoncons::endian::native)
                    {
                        for (std::size_t i = 0; i < ta.size(); ++i)
                        {
                            ta[i] = binary::byte_swap<float>(ta[i]);
                        }
                    }
                    if (!cursor_mode_ && state_stack_.back().mode == parse_mode::multi_dim) 
                    {
                        if (mdarray_size_ != ta.size())
                        {
                            ec = cbor_errc::bad_extents;
                            more_ = false;
                            return;
                        }
                        auto iter = std::make_shared<mdarray_iterator<float,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::float32, extents_, order_, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    else
                    {
                        auto iter = std::make_shared<oned_typed_array_iterator<float,jsoncons::identity,Allocator>>(std::move(array_buffer),
                            typed_array_tags::float32, semantic_tag::none, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    state_stack_.emplace_back(parse_mode::typed_array, ta.size(), false);
                    more_ = !cursor_mode_;
                    break;
                }
                case 0x52:
                case 0x56:
                {
                    array_tag_ = typed_array_tags::float64;
                    byte_string_type array_buffer(alloc_);
                    read(array_buffer, ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    const uint8_t tag = (uint8_t)raw_tag_;
                    jsoncons::endian e = get_typed_array_endianness(tag); 
                    auto ta = typed_array_cast<double>(array_buffer);
                    if (e != jsoncons::endian::native)
                    {
                        for (std::size_t i = 0; i < ta.size(); ++i)
                        {
                            ta[i] = binary::byte_swap<double>(ta[i]);
                        }
                    }
                    if (!cursor_mode_ && state_stack_.back().mode == parse_mode::multi_dim) 
                    {
                        if (mdarray_size_ != ta.size())
                        {
                            ec = cbor_errc::bad_extents;
                            more_ = false;
                            return;
                        }
                        auto iter = std::make_shared<mdarray_iterator<double,jsoncons::identity,Allocator>>(std::move(array_buffer), 
                            typed_array_tags::float64, extents_, order_, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    else
                    {
                        auto iter = std::make_shared<oned_typed_array_iterator<double,jsoncons::identity,Allocator>>(std::move(array_buffer),
                            typed_array_tags::float64, semantic_tag::none, alloc_);
                        typed_array_stack_.push_back(iter);
                        iter->next(visitor, *this, ec);
                    }
                    state_stack_.emplace_back(parse_mode::typed_array, ta.size(), false);
                    more_ = !cursor_mode_;
                    break;
                }
                default:
                {
                    auto bytes = read.view(ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    visitor.byte_string_value(bytes, raw_tag_, *this, ec);
                    more_ = !cursor_mode_;
                    break;
                }
            }
            other_tags_[item_tag] = false;
        }
        else
        {
            auto bytes = read.view(ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
            visitor.byte_string_value(bytes, semantic_tag::none, *this, ec);
            more_ = !cursor_mode_;
        }
    }

    void read_mdarray_header(generic_visitor& visitor, std::error_code& ec)
    {
        uint8_t b;
        if (source_.read(&b, 1) == 0)
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        jsoncons::cbor::detail::cbor_major_type major_type = get_major_type(b);
        JSONCONS_ASSERT(major_type == jsoncons::cbor::detail::cbor_major_type::array);
        uint8_t info = get_additional_information_value(b);
       
        read_extents(ec);   
        if (JSONCONS_UNLIKELY(ec))
        {
            return;
        }

        read_tags(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            return;
        }
        auto c = source_.peek();
        if (JSONCONS_UNLIKELY(c.eof))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        major_type = get_major_type(c.value);
        info = get_additional_information_value(c.value);
        state_stack_.emplace_back(parse_mode::multi_dim, 0);
        ++state_stack_.back().index;

        if (major_type == jsoncons::cbor::detail::cbor_major_type::array && order_ == mdarray_order::row_major) 
        {
            begin_classical_array_storage(info, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
            auto iter = std::make_shared<cbor_mdarray_row_major_iterator<Source,Allocator>>(extents_, this, cursor_mode_, alloc_);
            typed_array_stack_.push_back(iter);
            if (!iter->done())
            {
                iter->next(visitor, *this, ec);
            }
        }
        else if (major_type == jsoncons::cbor::detail::cbor_major_type::array && order_ == mdarray_order::column_major) 
        {
            begin_classical_array_storage(info, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
            auto iter = std::make_shared<cbor_mdarray_column_major_iterator<Source,Allocator>>(extents_, this, cursor_mode_, alloc_);
            typed_array_stack_.push_back(iter);
            if (!iter->done())
            {
                iter->next(visitor, *this, ec);
            }
        }
        else if (major_type == jsoncons::cbor::detail::cbor_major_type::byte_string)
        {
            std::size_t typed_array_count = typed_array_stack_.size();
            read_byte_string_from_source read(this);
            read_byte_string(read, visitor, ec);
            if (JSONCONS_UNLIKELY(ec))
            {
                return;
            }
            if (typed_array_stack_.size() == typed_array_count)
            {
                // Byte string storage must be a typed array
                ec = cbor_errc::bad_mdarray;
                return;
            }
        }
        else
        {
            ec = cbor_errc::bad_mdarray;
            return;
        }
        // cursor case
    }

    void read_extents(std::error_code& ec)
    {
        extents_.clear();

        auto b = source_.peek();
        if (JSONCONS_UNLIKELY(b.eof))
        {
            ec = cbor_errc::unexpected_eof;
            more_ = false;
            return;
        }
        if (get_major_type(b.value) != jsoncons::cbor::detail::cbor_major_type::array)
        {
            ec = cbor_errc::bad_extents;
            more_ = false;
            return;
        }
        uint8_t info = get_additional_information_value(b.value);

        switch (info)
        {
            case jsoncons::cbor::detail::additional_info::indefinite_length:
            {
                source_.ignore(1);
                bool done = false;
                while (!done)
                {
                    auto c = source_.peek();
                    if (JSONCONS_UNLIKELY(c.eof))
                    {
                        ec = cbor_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    if (c.value == 0xff)
                    {
                        source_.ignore(1);
                        done = true;
                    }
                    else
                    {
                        std::size_t extent = read_size(ec);
                        if (JSONCONS_UNLIKELY(ec))
                        {
                            more_ = false;
                            return;
                        }
                        extents_.push_back(extent);
                    }
                }
                break;
            }
            default:
            {
                std::size_t size = read_size(ec);
                if (JSONCONS_UNLIKELY(ec))
                {
                    more_ = false;
                    return;
                }
                for (std::size_t i = 0; more_ && i < size; ++i)
                {
                    std::size_t extent = read_size(ec);
                    if (JSONCONS_UNLIKELY(ec))
                    {
                        more_ = false;
                        return;
                    }
                    extents_.push_back(extent);
                }
                break;
            }
        }
        auto r = calculate_mdarray_size(extents_);
        if (!r || *r == 0)
        {
            ec = cbor_errc::bad_extents;
            more_ = false;
            return;
        }
        mdarray_size_ = *r;
    }
};

} // namespace cbor
} // namespace jsoncons

#endif

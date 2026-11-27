// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CBOR_CBOR_ONDEMAND_HPP
#define JSONCONS_EXT_CBOR_CBOR_ONDEMAND_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <string>
#include <vector>
#include <atomic>

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/utility/unicode_traits.hpp>
#include <jsoncons_ext/cbor/cbor_common.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>

// Zero-copy access to CBOR data in its encoded form. Scanning checks
// structural well-formedness once; the resulting items cannot fail.
// Tags are exposed, not interpreted: semantics such as bignums, string
// references and typed arrays belong to the parser and cursor.

namespace jsoncons {
namespace cbor {
namespace ondemand {

inline
std::size_t tag_size(uint8_t initial) noexcept
{
    unsigned ai = initial & 0x1F;
    return ai < 24 ? 1 : 1 + (1u << (ai - 24));
}

inline
bool cbor_int_to_uint64(const uint8_t* p, const uint8_t* end, uint64_t& value, cbor_errc& ec)
{
    assert(p < end);
    uint8_t info = get_additional_info(*p); 
    if (info < 24)
    {
        value = info;
        return true;
    }

    uint64_t length = 0;
    switch (info)
    {
        case 24: length = 1; break;
        case 25: length = 2; break;
        case 26: length = 4; break;
        case 27: length = 8; break;
        default:
            ec = cbor_errc::reserved_additional_info;
            return false;
    }

    if (static_cast<std::size_t>(end - p) < length)
    {
        ec = cbor_errc::unexpected_eof;
        return false;
    }

    value = 0;
    for (std::size_t i = 0; i < length; ++i)
    {
        value = (value << 8) | static_cast<uint64_t>(p[i]);
    }
    p += length;
    return true;
}

enum class access_errc
{
    success = 0,
    invalid_type = 1,
    indefinite,
    invalid_utf8,
    result_out_of_range,
    key_not_found, 
};

class access_error_category_impl
    : public std::error_category
{
public:
    const char* name() const noexcept override
    {
        return "jsoncons::cbor::ondemand::access";
    }
    std::string message(int ec) const override
    {
        switch (static_cast<access_errc>(ec))
        {
            case access_errc::success:
                return "Success";
            case access_errc::invalid_type:
                return "Invalid type";
            case access_errc::indefinite:
                return "Indefinite length";
            case access_errc::result_out_of_range:
                return "Result out of range";
            case access_errc::invalid_utf8:
                return "Invalid UTF-8";
            case access_errc::key_not_found:
                return "Key not found";
            default:
                return "Unknown access error";
        }
    }
};

inline 
const std::error_category& access_error_category() noexcept
{
  static const access_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(access_errc e) noexcept
{
    return std::error_code(static_cast<int>(e), access_error_category());
}

class access_error
{
    access_errc code_{};
    std::size_t position_{};
    
public:
    access_error() noexcept = default;

    access_error(access_errc ec) noexcept
        : code_{ec}
    {
    }
    
    access_error(access_errc ec, std::size_t position) noexcept
        : code_{ec}, position_{position}
    {
    }

    access_error(const access_error& other) noexcept = default;

    access_error(access_error&& other) noexcept = default;

    access_error& operator=(const access_error& other) noexcept = default;

    access_error& operator=(access_error&& other) noexcept = default;
    
    access_errc code() const noexcept
    {
        return code_;
    }
    std::size_t position() const noexcept
    {
        return position_;
    }

    std::string message() const
    {
        std::string str(make_error_code(code_).message());
        str.append(" at position ");
        append_to_string(position_, str);
        return str;
    }
};

template <typename T>
using access_result = expected<T,access_error>;

constexpr int default_max_nesting_depth = 1024;

// The major type of an item's head, after any leading tags. `simple` is
// major type 7: simple values and floats, told apart by the argument.
enum class cbor_type : uint8_t
{
    unsigned_integer = 0,
    negative_integer = 0x20,
    byte_string      = 0x40,
    text_string      = 0x60,
    array            = 0x80,
    map              = 0xa0,
    boolean          = 0xf5,
    null             = 0xf6,
    undefined        = 0xf7,
    float16          = 0xf9,
    float32          = 0xfa,
    float64          = 0xfb,
    simple           = 0xe0,
};                   

struct read_error
{
    cbor_errc code_{};
    std::size_t position_{0};
    
public:
    read_error() noexcept = default;

    read_error(cbor_errc ec) noexcept
        : code_{ec}
    {
    }
    
    read_error(cbor_errc ec, std::size_t position) noexcept
        : code_{ec}, position_{position}
    {
    }

    read_error(const read_error& other) noexcept = default;

    read_error(read_error&& other) noexcept = default;

    read_error& operator=(const read_error& other) noexcept = default;

    read_error& operator=(read_error&& other) noexcept = default;
    
    cbor_errc code() const noexcept
    {
        return code_;
    }
    std::size_t position() const noexcept
    {
        return position_;
    }

    std::string message() const
    {
        std::string str(make_error_code(code_).message());
        str.append(" at position ");
        append_to_string(position_, str);
        return str;
    }
};

inline
bool read_uint(const uint8_t*& p, const uint8_t* end, uint8_t info, uint64_t& value, cbor_errc& ec)
{
    if (info < 24)
    {
        value = info;
        return true;
    }

    std::size_t length = 0;
    switch (info)
    {
        case 24: length = 1; break;
        case 25: length = 2; break;
        case 26: length = 4; break;
        case 27: length = 8; break;
        default:
            ec = cbor_errc::reserved_additional_info;
            return false;
    }

    if (static_cast<std::size_t>(end - p) < length)
    {
        ec = cbor_errc::unexpected_eof;
        return false;
    }

    value = 0;
    for (std::size_t i = 0; i < length; ++i)
    {
        value = (value << 8) | static_cast<uint64_t>(p[i]);
    }
    p += length;
    return true;
}

class reverse_tag_iterator
{
    const uint8_t* begin_ = nullptr;
    const uint8_t* current_ = nullptr;
    std::uint64_t value_{0};

    static const uint8_t* previous(const uint8_t* begin, const uint8_t* /*end*/) noexcept
    {
        const uint8_t* last = nullptr;

        for (auto p = begin; get_major_type(*p) == cbor_major_type::semantic_tag; p += tag_size(*p))
        {
            last = p;
        }

        return last;
    }

    static uint64_t tag_value(const uint8_t* p, const uint8_t* end) noexcept
    {
        if (p == nullptr || p == end)
        {
            return 0;
        }

        cbor_errc ec{};
        uint64_t value{};
        const bool ok = cbor_int_to_uint64(p, end, value, ec);
        assert(ok);
        (void)ok;
        return value;
    }

public:
    using iterator_category = std::input_iterator_tag;
    using value_type = uint64_t;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = uint64_t;

    reverse_tag_iterator() noexcept = default;

    reverse_tag_iterator(
        const uint8_t* begin,
        const uint8_t* end) noexcept
        : begin_(begin), current_(previous(begin, end)), value_(tag_value(current_,end))
    {
    }

    reference operator*() const noexcept
    {
        return value_;
    }

    reverse_tag_iterator& operator++() noexcept
    {
        const uint8_t* end = current_; 
        current_ = previous(begin_, current_);
        value_ = tag_value(current_,end);
        return *this;
    }

    reverse_tag_iterator operator++(int) noexcept
    {
        auto tmp = *this;
        ++*this;
        return tmp;
    }

    friend bool operator==(
        const reverse_tag_iterator& a,
        const reverse_tag_iterator& b) noexcept
    {
        return a.current_ == b.current_;
    }
};

class reverse_tag_view
{
    const uint8_t* first_{nullptr};
    const uint8_t* stop_{nullptr};
public:
    using iterator = reverse_tag_iterator;
    using const_iterator = reverse_tag_iterator;

    iterator begin() const noexcept { return reverse_tag_iterator(first_, stop_); }
    iterator end() const noexcept { return reverse_tag_iterator(); }
    bool empty() const noexcept { return first_ == stop_; }

private:
    friend class cbor_item;
    reverse_tag_view(const uint8_t* first, const uint8_t* stop) noexcept : first_(first), stop_(stop) {}
};

namespace ondemand_detail {

    struct item_head
    {
        cbor::cbor_major_type major_type{};
        uint8_t additional_info{0};
        mutable std::atomic<bool> valid_utf8{};
        uint64_t value{0};
        const uint8_t* content;

        item_head() noexcept = default;

        item_head(const item_head& other)
            : major_type(other.major_type),
              additional_info(other.additional_info),
              valid_utf8(other.valid_utf8.load(std::memory_order_relaxed)),
              value(other.value)
        {
        }

        item_head& operator=(const item_head& other)
        {
            if (this != &other)
            {
                major_type = other.major_type;
                additional_info = other.additional_info;
                value = other.value;
                valid_utf8.store(
                    other.valid_utf8.load(std::memory_order_relaxed),
                    std::memory_order_relaxed
                );
            }
            return *this;
        }

        bool indefinite() const noexcept
        {
            return additional_info == cbor::detail::additional_info::indefinite_length;
        }
    };

    inline cbor_major_type major_type(const item_head& head) noexcept
    {
        return head.major_type;
    }

    inline uint8_t additional_info(const item_head& head) noexcept
    {
        return head.additional_info;
    }

    inline cbor_type type(const item_head& head) noexcept
    {
        if (head.major_type != cbor_major_type::simple)
        {
            return static_cast<cbor_type>(static_cast<uint8_t>(head.major_type) << 5); 
        }
        else
        {
            switch (head.additional_info)
            {
                case 20:
                case 21:
                    return cbor_type::boolean;
                case 22:
                    return cbor_type::null;
                case 23:
                    return cbor_type::undefined;
                case 25:
                    return cbor_type::float16;
                case 26:
                    return cbor_type::float32;
                case 27:
                    return cbor_type::float64;
                default:
                    return cbor_type::simple;
            }
        }
    }

    inline 
    access_result<uint64_t> get_uint64(const item_head& head) noexcept
    {
        using result_type = access_result<uint64_t>;
        if (head.major_type != cbor::cbor_major_type::unsigned_integer)
        {
            return result_type(jsoncons::unexpect, access_errc::invalid_type);
        }
        return result_type{head.value};
    }

    inline 
    access_result<int64_t> get_int64(const item_head& head) noexcept
    {
        using result_type = access_result<int64_t>;
        const uint64_t int64_max = static_cast<uint64_t>((std::numeric_limits<int64_t>::max)());
        if (head.major_type == cbor::cbor_major_type::unsigned_integer)
        {
            if (head.value <= int64_max)
            {
                return result_type{static_cast<int64_t>(head.value)};
            }
            else
            {
                return result_type(jsoncons::unexpect, access_errc::result_out_of_range);
            }
        }
        if (head.major_type == cbor::cbor_major_type::negative_integer)
        {
            if (head.value <= int64_max)
            {
                return result_type{-1 - static_cast<int64_t>(head.value)};
            }
            else
            {
                return result_type(jsoncons::unexpect, access_errc::result_out_of_range);
            }
        }
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }

    inline 
    access_result<bool> get_bool(const item_head& head) noexcept
    {
        using result_type = access_result<bool>;

        if (type(head) != cbor_type::boolean)
        {
            return result_type(jsoncons::unexpect, access_errc::invalid_type);
        }        
        return head.additional_info == 21;
    }

    inline 
    access_result<double> get_double(const item_head& head) noexcept
    {
        using result_type = access_result<double>;

        if (head.major_type != cbor::cbor_major_type::simple)
        {
            return result_type(jsoncons::unexpect, access_errc::invalid_type);
        }
        switch (head.additional_info)
        {
            case 25:
                return binary::decode_half(static_cast<uint16_t>(head.value));
            case 26:
            {
                const uint32_t bits = static_cast<uint32_t>(head.value);
                float single;
                std::memcpy(&single, &bits, sizeof single);
                return result_type{single};
            }
            case 27:
            {
                const uint64_t bits = head.value;
                double value;
                std::memcpy(&value, &bits, sizeof value);
                return result_type{value};
            }
            default:
                return result_type(jsoncons::unexpect, access_errc::invalid_type);
        }
    }

    inline 
    access_result<jsoncons::string_view> get_string(const item_head& head, const uint8_t* content) noexcept
    {
        using result_type = access_result<jsoncons::string_view>;
        if (head.major_type != cbor::cbor_major_type::text_string)
        {
            return result_type(jsoncons::unexpect, access_errc::invalid_type);
        }
        if (head.indefinite())
        {
            return result_type(jsoncons::unexpect, access_errc::indefinite);
        }
        const char* s = reinterpret_cast<const char*>(content);
        std::size_t length = static_cast<std::size_t>(head.value);

        if (head.valid_utf8.load(std::memory_order_relaxed))
        {
            return result_type{jsoncons::in_place, s, length};
        }

        if (unicode_traits::validate(s, length).ec != unicode_traits::unicode_errc())
        {
            return result_type(jsoncons::unexpect, access_errc::invalid_utf8);
        }
        head.valid_utf8.store(true, std::memory_order_relaxed);
        return result_type{jsoncons::in_place, s, length};
    }

    inline 
    access_result<jsoncons::span<const uint8_t>> get_byte_string(const item_head& head, const uint8_t* content) noexcept
    {
        using result_type = access_result<span<const uint8_t>>;
        if (head.major_type != cbor::cbor_major_type::byte_string)
        {
            return result_type(jsoncons::unexpect, access_errc::invalid_type);
        }
        if (head.indefinite())
        {
            return result_type(jsoncons::unexpect, access_errc::indefinite);
        }
        return result_type{jsoncons::in_place, content, static_cast<std::size_t>(head.value)};
    }

    JSONCONS_FORCE_INLINE bool read_head(const uint8_t*& p, const uint8_t* end, item_head& head, cbor_errc& ec)
    {
        if (p == end)
        {
            ec = cbor_errc::unexpected_eof;
            return false;
        }

        const uint8_t initial = *p++;
        head.major_type = static_cast<cbor::cbor_major_type>(initial >> 5);
        head.additional_info = initial & 0x1f;
        head.value = 0;

        if (head.additional_info == cbor::detail::additional_info::indefinite_length)
        {
            switch (head.major_type)
            {
                case cbor::cbor_major_type::byte_string:
                case cbor::cbor_major_type::text_string:
                case cbor::cbor_major_type::array:
                case cbor::cbor_major_type::map:
                    break;
                default:
                    ec = cbor_errc::unknown_type;
                    return false;
            }
        }
        else 
        {
            if (!read_uint(p, end, head.additional_info, head.value, ec))
                return false;
            // RFC 8949 3.3: a two-byte encoding of a simple value below 32
            // is not well-formed.
            if (head.major_type == cbor::cbor_major_type::simple &&
                head.additional_info == 24 && head.value < 32)
            {
                ec = cbor_errc::unknown_type;
                return false;
            }
        }

        return true;
    }

    // Head of the item's content, past any leading semantic tags.
    JSONCONS_FORCE_INLINE bool read_value_head(const uint8_t*& p, const uint8_t* end, item_head& head, cbor_errc& ec)
    {
        while (get_major_type(*p) == cbor_major_type::semantic_tag)
        {
            std::size_t size = tag_size(*p);
            if (size == 0)
            {
                break;
            }
            if ((std::size_t(end - p)) < size)
            {
                break;
            }
            p += size;
        }
        if (!read_head(p, end, head, ec))
        {
            return false;
        }

        return true;
    }

    inline bool skip_string_chunks(const uint8_t*& p, const uint8_t* end,
                                  cbor::cbor_major_type expected_major, cbor_errc& ec)
    {
        for (;;)
        {
            if (p >= end)
            {
                ec = cbor_errc::unexpected_eof;
                return false;
            }
            if (*p == 0xff)
            {
                ++p;
                return true;
            }

            item_head chunk;
            if (!read_head(p, end, chunk, ec))
            {
                return false;
            }
            if (chunk.major_type != expected_major || chunk.indefinite())
            {
                ec = cbor_errc::illegal_chunked_string;
                return false;
            }
            if (static_cast<uint64_t>(end - p) < chunk.value)
            {
                ec = cbor_errc::unexpected_eof;
                return false;
            }
            p += static_cast<std::size_t>(chunk.value);
        }
    }

    // Skips the payload of any non-container head in place.
    JSONCONS_FORCE_INLINE bool skip_scalar_or_string(const item_head& head, const uint8_t*& p, const uint8_t* end, cbor_errc& ec)
    {
        switch (head.major_type)
        {
            case cbor::cbor_major_type::unsigned_integer:
            case cbor::cbor_major_type::negative_integer:
            case cbor::cbor_major_type::simple:
                return true;

            case cbor::cbor_major_type::byte_string:
            case cbor::cbor_major_type::text_string:
                if (head.indefinite())
                {
                    return skip_string_chunks(p, end, head.major_type, ec);
                }
                if (static_cast<uint64_t>(end - p) < head.value)
                {
                    ec = cbor_errc::unexpected_eof;
                    return false;
                }
                p += static_cast<std::size_t>(head.value);
                return true;

            default:
                ec = cbor_errc::unknown_type;
                return false;
        }
    }

    // Items left in an open container, or a marker for an indefinite-length
    // one, which ends at a break: arrays allow it before any element, maps
    // only between entries.
    constexpr uint64_t indefinite_array_marker = UINT64_MAX;
    constexpr uint64_t indefinite_map_key_marker = UINT64_MAX - 1;
    constexpr uint64_t indefinite_map_value_marker = UINT64_MAX - 2;

    // Definite counts are capped just past what the remaining input could
    // hold, as every item takes at least one byte. A capped container still
    // fails at its first malformed item, or with EOF if there is none, and
    // no count collides with a marker.
    inline uint64_t remaining_items(const item_head& head, std::size_t available) noexcept
    {
        const bool is_map = head.major_type == cbor::cbor_major_type::map;
        if (head.indefinite())
        {
            return is_map ? indefinite_map_key_marker : indefinite_array_marker;
        }
        const uint64_t limit = static_cast<uint64_t>(available);
        return is_map ? 2 * (std::min)(head.value, limit / 2 + 1)
                      : (std::min)(head.value, limit + 1);
    }

    class pending_stack
    {
        static constexpr std::size_t local_capacity = 32;

        uint64_t local_[local_capacity];   // only [0, size_) is ever read
        std::vector<uint64_t> spill_;
        std::size_t size_{0};
    public:
        bool empty() const noexcept
        {
            return size_ == 0;
        }

        std::size_t size() const noexcept
        {
            return size_;
        }

        void push(uint64_t value)
        {
            if (size_ < local_capacity)
            {
                local_[size_] = value;
            }
            else
            {
                spill_.push_back(value);
            }
            ++size_;
        }

        uint64_t pop() noexcept
        {
            --size_;
            if (size_ >= local_capacity)
            {
                const uint64_t value = spill_.back();
                spill_.pop_back();
                return value;
            }
            return local_[size_];
        }
    };

    // Skips the array or map whose head has just been read. Iterative, so
    // depth is bounded by max_nesting_depth rather than the call stack. The
    // innermost level's count is `current`; enclosing levels are pushed on
    // `open`, above a root level of zero.
    inline bool skip_container(const uint8_t*& p, const uint8_t* end, item_head head,
        int max_nesting_depth, cbor_errc& ec)
    {
        const std::size_t depth_limit = max_nesting_depth > 0 ? static_cast<std::size_t>(max_nesting_depth) : 0;

        pending_stack open;
        uint64_t current = 0;

        for (;;)
        {
            // Open the container in `head`.
            if (JSONCONS_UNLIKELY(open.size() >= depth_limit))
            {
                ec = cbor_errc::max_nesting_depth_exceeded;
                return false;
            }
            const uint64_t remaining = remaining_items(head, static_cast<std::size_t>(end - p));
            if (remaining != 0)
            {
                open.push(current);
                current = remaining;
            }

            // Walk items in place until the next container head.
            for (;;)
            {
                // Close finished levels, then count off the next item.
                if (current != 0 && current < indefinite_map_value_marker)
                {
                    --current;
                }
                else if (current == 0)
                {
                    if (open.empty())
                    {
                        return true;
                    }
                    current = open.pop();
                    continue;
                }
                else if (current == indefinite_map_value_marker)
                {
                    current = indefinite_map_key_marker;
                }
                else // at an element or entry boundary, where a break may close the level
                {
                    if (p >= end)
                    {
                        ec = cbor_errc::unexpected_eof;
                        return false;
                    }
                    if (*p == 0xff)
                    {
                        ++p;
                        current = open.pop();
                        continue;
                    }
                    if (current == indefinite_map_key_marker)
                    {
                        current = indefinite_map_value_marker;
                    }
                }

                if (!read_value_head(p, end, head, ec))
                {
                    return false;
                }
                if (head.major_type == cbor::cbor_major_type::array ||
                    head.major_type == cbor::cbor_major_type::map)
                {
                    break;
                }
                if (!skip_scalar_or_string(head, p, end, ec))
                {
                    return false;
                }
            }
        }
    }

    // Returns the end of a checked item whose head has just been read.
    // Definite-length containers need no stack: their counts add up to one
    // count of items left. Indefinite-length containers use skip_container.
    inline const uint8_t* skip_checked(const uint8_t* p, const uint8_t* end, item_head head)
    {
        const uint8_t* const content = p;
        const item_head first = head;
        cbor_errc ec{};
        uint64_t left = 0;
        for (;;)
        {
            if (head.major_type == cbor::cbor_major_type::array ||
                head.major_type == cbor::cbor_major_type::map)
            {
                if (head.indefinite())
                {
                    p = content;
                    const bool ok = skip_container(p, end, first, (std::numeric_limits<int>::max)(), ec);
                    assert(ok);
                    (void)ok;
                    return p;
                }
                left += head.major_type == cbor::cbor_major_type::map ? 2 * head.value : head.value;
            }
            else
            {
                const bool ok = skip_scalar_or_string(head, p, end, ec);
                assert(ok);
                (void)ok;
            }
            if (left == 0)
            {
                return p;
            }
            --left;
            const bool ok = read_value_head(p, end, head, ec);
            assert(ok);
            (void)ok;
        }
    }

    inline bool validate_utf8(const uint8_t* p, std::size_t length) noexcept
    {
        const auto result = unicode_traits::validate(reinterpret_cast<const char*>(p), length);
        return result.ec == unicode_traits::unicode_errc();
    }

    // Size of an encoded head, from its additional information.
    inline std::size_t head_size(const item_head& head) noexcept
    {
        return head.additional_info < 24 || head.indefinite()
            ? 1 : 1 + (std::size_t(1) << (head.additional_info - 24));
    }

    struct item_access;

} // namespace ondemand_detail

// A zero-copy view of one complete, structurally well-formed CBOR item:
// its leading tags, head and content. Obtained only from read_sequence, read_item
// or children(). Borrows the scanned bytes and must not outlive them.
class cbor_item
{
    span<const uint8_t> bytes_;
    ondemand_detail::item_head head_;
    const uint8_t* content_;   // first byte after the untagged head
public:
    // The item's complete encoding, including leading tags.
    span<const uint8_t> encoded_bytes() const noexcept
    {
        return bytes_;
    }

    cbor_major_type major_type() const noexcept
    {
        return ondemand_detail::major_type(head_);
    }

    uint8_t additional_info() const noexcept
    {
        return ondemand_detail::additional_info(head_);
    }

    cbor_type type() const noexcept
    {
        return ondemand_detail::type(head_);
    }

    // The head's argument (RFC 8949 3): an integer's value, a string's
    // length, a container's count, or a simple value's number (a float's
    // bit pattern). Zero when indefinite.
    uint64_t argument() const noexcept
    {
        return head_.value;
    }

    bool indefinite() const noexcept
    {
        return head_.indefinite();
    }

    class tag_iterator;
    class tag_view;
    class child_iterator;
    class child_range;

    // The item's leading tag numbers, outermost first.
    tag_view tags() const noexcept;
    reverse_tag_view reverse_tags() const noexcept;

    // The data items this item is made of: an array's elements, a map's
    // keys and values alternating, or the definite-length chunks of an
    // indefinite-length string (RFC 8949 3.2.3). Empty for anything else.
    child_range children() const noexcept;

    // The typed accessors return false, leaving `value` unchanged, if the
    // item is not of that kind or range.
    access_result<uint64_t> get_uint64() const noexcept
    {
        return ondemand_detail::get_uint64(head_);
    }

    access_result<int64_t> get_int64() const noexcept
    {
        return ondemand_detail::get_int64(head_);
    }

    access_result<bool> get_bool() const noexcept
    {
        return ondemand_detail::get_bool(head_);
    }

    access_result<double> get_double() const noexcept
    {
        return ondemand_detail::get_double(head_);
    }

    // Zero-copy string content. False for indefinite-length strings,
    // whose chunks are their children.
    access_result<jsoncons::string_view> get_string() const noexcept
    {
        return ondemand_detail::get_string(head_, content_);
    }

#if defined(JSONCONS_HAS_CHAR8)
    access_result<jsoncons::u8string_view> get_u8string() const noexcept
    {
        auto str = ondemand_detail::get_string(head_, content_);
        if (!str)
        {
            return access_result<jsoncons::u8string_view>(jsoncons::unexpect, str.error());
        }
        return access_result<jsoncons::u8string_view>(jsoncons::u8string_view(reinterpret_cast<const char8_t*>((*str).data()), (*str).size()));
    }
#endif

    access_result<span<const uint8_t>> get_byte_string() const noexcept
    {
        return ondemand_detail::get_byte_string(head_, content_);
    }

    // Copies string content, assembling chunked strings. `value` is replaced
    // on success, unchanged on failure, and may alias the item's bytes.

    template <typename CharBuffer,typename std::enable_if<ext_traits::is_narrow_char_buffer<CharBuffer>::value, int>::type = 0>
    access_result<void> get_string(CharBuffer& buf) const;

    template <typename ByteBuffer,typename std::enable_if<ext_traits::is_byte_buffer<ByteBuffer>::value, int>::type = 0>
    access_result<void> get_byte_string(ByteBuffer& buf) const;

    access_result<cbor_item> at(const char* key) const noexcept
    {
        return at(jsoncons::string_view(key));
    }

    template <typename StringViewLike,typename std::enable_if<ext_traits::is_string_view_like<StringViewLike>::value
        && std::is_same<typename StringViewLike::value_type,char>::value, int>::type = 0>
    access_result<cbor_item> at(StringViewLike key) const noexcept;

    template <typename UnsignedInteger,typename std::enable_if<ext_traits::is_unsigned_integer<UnsignedInteger>::value, 
        int>::type = 0>
    access_result<cbor_item> at(UnsignedInteger key) const noexcept;

    access_result<std::pair<std::int64_t,cbor_item>> get_bigdecimal() const noexcept;

    access_result<std::pair<std::int64_t,cbor_item>> get_bigfloat() const noexcept;

    access_result<std::pair<bool,jsoncons::span<const uint8_t>>> get_bignum() const noexcept;

private:
    friend struct ondemand_detail::item_access;

    cbor_item() noexcept = default;

    cbor_item(span<const uint8_t> bytes, const ondemand_detail::item_head& head, const uint8_t* content) noexcept
        : bytes_(bytes), head_(head), content_(content)
    {
    }

    // A string's content: its payload, or its chunks' payloads in order.
    std::size_t content_size() const noexcept;
    void copy_content(uint8_t* out) const noexcept;
};

namespace ondemand_detail {

    struct item_access
    {
        static cbor_item make(span<const uint8_t> bytes, const item_head& head, const uint8_t* content) noexcept
        {
            return cbor_item(bytes, head, content);
        }
    };

} // namespace ondemand_detail

// Iterates an item's leading tag numbers.
class cbor_item::tag_iterator
{

    const uint8_t* pos_{nullptr};    // nullptr when exhausted
    const uint8_t* stop_{nullptr};   // first byte of the untagged head
    uint64_t value_{0};
public:
    using value_type = uint64_t;
    using reference = uint64_t;
    using pointer = void;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    tag_iterator() noexcept = default;

    uint64_t operator*() const noexcept
    {
        assert(pos_ != nullptr);
        return value_;
    }

    tag_iterator& operator++() noexcept
    {
        advance();
        return *this;
    }

    tag_iterator operator++(int) noexcept
    {
        tag_iterator temp = *this;
        advance();
        return temp;
    }

    friend bool operator==(const tag_iterator& a, const tag_iterator& b) noexcept
    {
        return a.pos_ == b.pos_;
    }

    friend bool operator!=(const tag_iterator& a, const tag_iterator& b) noexcept
    {
        return a.pos_ != b.pos_;
    }

private:
    friend class tag_view;
    friend class reverse_tag_view;

    tag_iterator(const uint8_t* pos, const uint8_t* stop) noexcept
        : pos_(pos), stop_(stop), value_(0)
    {
        advance();
    }

    void advance() noexcept
    {
        if (pos_ == nullptr || pos_ >= stop_)
        {
            pos_ = nullptr;
            return;
        }
        ondemand_detail::item_head head;
        cbor_errc ec{};
        const uint8_t* p = pos_;

        const bool ok = ondemand_detail::read_head(p, stop_, head, ec);
        assert(ok && head.major_type == cbor::cbor_major_type::semantic_tag);
        (void)ok;
        value_ = head.value;
        pos_ = p;
    }
};

class cbor_item::tag_view
{
    const uint8_t* first_{nullptr};
    const uint8_t* stop_{nullptr};
public:
    using iterator = tag_iterator;
    using const_iterator = tag_iterator;

    iterator begin() const noexcept { return tag_iterator(first_, stop_); }
    iterator end() const noexcept { return tag_iterator(); }
    bool empty() const noexcept { return first_ == stop_; }

private:
    friend class cbor_item;
    tag_view(const uint8_t* first, const uint8_t* stop) noexcept : first_(first), stop_(stop) {}
};

inline cbor_item::tag_view cbor_item::tags() const noexcept
{
    // Leading tags occupy [start of encoding, start of untagged head).
    return tag_view(bytes_.data(), content_ - ondemand_detail::head_size(head_));
}

inline reverse_tag_view cbor_item::reverse_tags() const noexcept
{
    // Leading tags occupy [start of encoding, start of untagged head).
    return reverse_tag_view(bytes_.data(), content_ - ondemand_detail::head_size(head_));
}

// Iterates an item's children, measuring each once. The last child of a
// definite-length container ends where its parent does, so is not measured.
class cbor_item::child_iterator
{
    const uint8_t* pos_;    // current child's begin, nullptr when exhausted
    const uint8_t* next_;   // current child's end, the next child's begin
    const uint8_t* end_;    // the parent's end
    uint64_t remaining_;    // raw children left; indefinite ends at a break
    ondemand_detail::item_head head_;
    const uint8_t* content_;
public:
    using value_type = cbor_item;
    using reference = cbor_item;
    using pointer = void;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::input_iterator_tag;

    child_iterator() noexcept
        : pos_(nullptr), next_(nullptr), end_(nullptr), remaining_(0), content_(nullptr)
    {
    }

    cbor_item operator*() const noexcept
    {
        assert(pos_ != nullptr);
        return ondemand_detail::item_access::make(
            span<const uint8_t>(pos_, static_cast<std::size_t>(next_ - pos_)),
            head_, content_);
    }

    child_iterator& operator++()
    {
        advance();
        return *this;
    }

    child_iterator operator++(int)
    {
        child_iterator temp = *this;
        advance();
        return temp;
    }

    friend bool operator==(const child_iterator& a, const child_iterator& b) noexcept
    {
        return a.pos_ == b.pos_;
    }

    friend bool operator!=(const child_iterator& a, const child_iterator& b) noexcept
    {
        return a.pos_ != b.pos_;
    }

private:
    friend class child_range;

    child_iterator(const uint8_t* first, const uint8_t* end, uint64_t remaining)
        : pos_(nullptr), next_(first), end_(end), remaining_(remaining), content_(nullptr)
    {
        advance();
    }

    void advance()
    {
        pos_ = next_;
        if (remaining_ == ondemand_detail::indefinite_array_marker)
        {
            if (*pos_ == 0xff)
            {
                pos_ = nullptr;
                return;
            }
        }
        else if (remaining_ == 0)
        {
            pos_ = nullptr;
            return;
        }
        else
        {
            --remaining_;
        }

        const uint8_t* p = pos_;
        cbor_errc ec{};
        const bool ok = ondemand_detail::read_value_head(p, end_, head_, ec);
        assert(ok);
        (void)ok;
        content_ = p;
        next_ = remaining_ == 0 ? end_ : ondemand_detail::skip_checked(p, end_, head_);
    }
};

class cbor_item::child_range
{
    const uint8_t* first_{nullptr};
    const uint8_t* end_{nullptr};
    uint64_t remaining_{0};
public:
    using iterator = child_iterator;
    using const_iterator = child_iterator;

    child_iterator begin() const
    {
        return first_ == nullptr ? child_iterator() : child_iterator(first_, end_, remaining_);
    }

    child_iterator end() const noexcept
    {
        return child_iterator();
    }

    bool empty() const noexcept
    {
        return first_ == nullptr ||
            (remaining_ == ondemand_detail::indefinite_array_marker
                ? *first_ == 0xff : remaining_ == 0);
    }

private:
    friend class cbor_item;
    child_range(const uint8_t* first, const uint8_t* end, uint64_t remaining) noexcept
        : first_(first), end_(end), remaining_(remaining)
    {
    }
};

inline cbor_item::child_range cbor_item::children() const noexcept
{
    const uint8_t* const end = bytes_.data() + bytes_.size();
    switch (head_.major_type)
    {
        case cbor::cbor_major_type::array:
        case cbor::cbor_major_type::map:
            if (!head_.indefinite())
            {
                const bool is_map = head_.major_type == cbor::cbor_major_type::map;
                return child_range(content_, end, is_map ? 2 * head_.value : head_.value);
            }
            return child_range(content_, end, ondemand_detail::indefinite_array_marker);
        case cbor::cbor_major_type::byte_string:
        case cbor::cbor_major_type::text_string:
            if (head_.indefinite())
            {
                return child_range(content_, end, ondemand_detail::indefinite_array_marker);
            }
            break;
        default:
            break;
    }
    return child_range(nullptr, nullptr, 0);
}

inline std::size_t cbor_item::content_size() const noexcept
{
    if (!head_.indefinite())
    {
        return static_cast<std::size_t>(head_.value);
    }
    std::size_t size = 0;
    for (const cbor_item chunk : children())
    {
        size += static_cast<std::size_t>(chunk.head_.value);
    }
    return size;
}

inline void cbor_item::copy_content(uint8_t* out) const noexcept
{
    if (!head_.indefinite())
    {
        if (head_.value != 0)
        {
            std::memcpy(out, content_, static_cast<std::size_t>(head_.value));
        }
        return;
    }
    for (const cbor_item chunk : children())
    {
        chunk.copy_content(out);
        out += static_cast<std::size_t>(chunk.head_.value);
    }
}

struct cbor_sequence
{
    cbor_item head;
    span<const uint8_t> tail;
};

// Scans the first item in `input`, checking its structure, and returns it
// with the bytes that follow. Errors report where scanning stopped.
// Allocates only when nesting exceeds 32 open containers.
inline expected<cbor_sequence, read_error> read_sequence(span<const uint8_t> input,
    int max_nesting_depth = default_max_nesting_depth)
{
    if (input.empty())
    {
        return expected<cbor_sequence, read_error>(unexpect, cbor_errc::unexpected_eof, 0);
    }
    const uint8_t* p = input.data();
    const uint8_t* end = p + input.size();
    cbor_errc ec{};

    ondemand_detail::item_head head;
    bool ok = ondemand_detail::read_value_head(p, end, head, ec);
    const uint8_t* content = p;
    if (ok)
    {
        if (head.major_type == cbor::cbor_major_type::array ||
            head.major_type == cbor::cbor_major_type::map)
        {
            ok = ondemand_detail::skip_container(p, end, head, max_nesting_depth, ec);
        }
        else
        {
            ok = ondemand_detail::skip_scalar_or_string(head, p, end, ec);
        }
    }
    if (!ok)
    {
        return expected<cbor_sequence, read_error>(unexpect,
            ec, static_cast<std::size_t>(p - input.data()));
    }
    return cbor_sequence{
        ondemand_detail::item_access::make(
            span<const uint8_t>(input.data(), static_cast<std::size_t>(p - input.data())), head, content),
        span<const uint8_t>(p, static_cast<std::size_t>(end - p))};
}

// Like read_sequence, but `input` must hold exactly one item: trailing bytes are
// cbor_errc::trailing_data.
inline expected<cbor_item, read_error> read_item(span<const uint8_t> input,
    int max_nesting_depth = default_max_nesting_depth)
{
    auto scanned = read_sequence(input, max_nesting_depth);
    if (!scanned)
    {
        return expected<cbor_item, read_error>(unexpect, scanned.error());
    }
    if (!scanned.value().tail.empty())
    {
        return expected<cbor_item, read_error>(unexpect,
            cbor_errc::trailing_data, scanned.value().head.encoded_bytes().size());
    }
    return scanned.value().head;
}

// Reject temporary owning containers, whose items would dangle.
template <typename Container>
expected<cbor_sequence, read_error> read_sequence(const Container&&, int = default_max_nesting_depth) = delete;
template <typename Container>
expected<cbor_item, read_error> read_item(const Container&&, int = default_max_nesting_depth) = delete;

// True if `text_item` is a text string of well-formed UTF-8, checking each
// chunk of an indefinite-length string separately (RFC 8949 3.2.3).
inline bool validate_text(const cbor_item& text_item) noexcept
{
    if (text_item.type() != cbor_type::text_string)
    {
        return false;
    }
    auto str = text_item.get_string();
    if (str)
    {
        return true;
    }
    if (str.error().code() == access_errc::invalid_utf8)
    {
        return false;
    }
    if (str.error().code() == access_errc::indefinite)
    {
        for (const cbor_item chunk : text_item.children())
        {
            auto r = chunk.get_string();
            if (!r)
            {
                return false;
            }
        }
        return true;
    }

    return true; // not text
}

template <typename CharBuffer,typename std::enable_if<ext_traits::is_narrow_char_buffer<CharBuffer>::value, int>::type>
access_result<void> cbor_item::get_string(CharBuffer& buf) const
{
    using char_type = typename CharBuffer::value_type;
    using result_type = access_result<void>;
    if (head_.major_type != cbor::cbor_major_type::text_string)
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }

    if (head_.indefinite())
    {
        std::size_t length = 0;
        for (const cbor_item chunk : children())
        {
            auto r = chunk.get_string();
            if (!r)
            {
                return result_type(jsoncons::unexpect, r.error().code());
            }
            length += (*r).size();
        }
        buf.resize(length);
        if (length > 0)
        {
            std::size_t pos = 0;
            for (const cbor_item chunk : children())
            {
                auto r = chunk.get_string();
                if (!r)
                {
                    return result_type(jsoncons::unexpect, r.error().code(), r.error().position());
                }
                auto sv = *r;
                if (!sv.empty())
                {
                    std::memcpy(reinterpret_cast<void*>(&(buf[0]) + pos), reinterpret_cast<const char_type*>(sv.data()), sv.size());
                    pos += sv.size();
                }
            }
        }
    }
    else
    {
        auto r = get_string();
        if (!r)
        {
            return result_type(jsoncons::unexpect, r.error().code(), r.error().position());
        }
        auto sv = *r;
        buf.resize(sv.size());
        if (!sv.empty())
        {
            std::memcpy(reinterpret_cast<void*>(&(buf[0])), reinterpret_cast<const char_type*>(sv.data()), sv.size());
        }
    }
    return result_type{};
}

template <typename ByteBuffer, typename std::enable_if<ext_traits::is_byte_buffer<ByteBuffer>::value, int>::type>
access_result<void> cbor_item::get_byte_string(ByteBuffer& buf) const
{
    using byte_type = typename ByteBuffer::value_type;
    using result_type = access_result<void>;
    if (head_.major_type != cbor::cbor_major_type::byte_string)
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }

    if (head_.indefinite())
    {
        std::size_t length = 0;
        for (const cbor_item chunk : children())
        {
            auto r = chunk.get_byte_string();
            if (!r)
            {
                return result_type(jsoncons::unexpect, r.error().code(), r.error().position());
            }
            length += (*r).size();
        }
        buf.resize(length);
        if (length > 0)
        {
            std::size_t pos = 0;
            for (const cbor_item chunk : children())
            {
                auto r = chunk.get_byte_string();
                if (!r)
                {
                    return result_type(jsoncons::unexpect, r.error().code(), r.error().position());
                }
                auto bsv = *r;
                std::memcpy((buf.data() + pos), reinterpret_cast<const byte_type*>(bsv.data()), bsv.size());
                pos += bsv.size();
            }
        }
    }
    else
    {
        auto r = get_byte_string();
        if (!r)
        {
            return result_type(jsoncons::unexpect, r.error().code(), r.error().position());
        }
        auto bsv = *r;
        buf.resize(bsv.size());
        if (!bsv.empty())
        {
            std::memcpy(buf.data(), reinterpret_cast<const byte_type*>(bsv.data()), bsv.size());
        }
    }

    return result_type{};
}

template <typename StringViewLike,typename std::enable_if<ext_traits::is_string_view_like<StringViewLike>::value
    && std::is_same<typename StringViewLike::value_type,char>::value, int>::type>
access_result<cbor_item> cbor_item::at(StringViewLike key) const noexcept
{
    using result_type = access_result<cbor_item>;

    if (head_.major_type != cbor::cbor_major_type::map)
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }
    bool is_key = true;
    access_result<jsoncons::string_view> k;
    for (auto child : children())
    {
        if (is_key)
        {
            k = child.get_string();
        }
        else 
        {
            if (k && *k == key)
            {
                return result_type{child};
            }
        }
        is_key = !is_key;
    }

    return result_type(jsoncons::unexpect, access_errc::key_not_found);
}

template <typename UnsignedInteger,typename std::enable_if<ext_traits::is_unsigned_integer<UnsignedInteger>::value, 
    int>::type>
access_result<cbor_item> cbor_item::at(UnsignedInteger key) const noexcept
{
    using result_type = access_result<cbor_item>;

    if (head_.major_type == cbor::cbor_major_type::map)
    {
        bool is_key = true;
        access_result<uint64_t> k;
        for (auto child : children())
        {
            if (is_key)
            {
                k = child.get_uint64();
            }
            else 
            {
                if (k && *k == key)
                {
                    return result_type{child};
                }
            }
            is_key = !is_key;
        }

        return result_type(jsoncons::unexpect, access_errc::key_not_found);
    }
    else if (head_.major_type == cbor::cbor_major_type::array)
    {
        std::size_t index = 0;
        for (auto child : children())
        {
            if (index++ == key)
            {
                return result_type{child};
            }
        }

        return result_type(jsoncons::unexpect, access_errc::result_out_of_range);
    }
    else
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }
}

access_result<std::pair<int64_t,cbor_item>> cbor_item::get_bigdecimal() const noexcept
{
    using value_type = std::pair<int64_t, cbor_item>;
    using result_type = access_result<value_type>;
    if (!(head_.major_type == cbor::cbor_major_type::array && head_.value == 2))
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }

    if (reverse_tags().empty())
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }
    uint64_t tag = *(reverse_tags().begin());
    if (tag != 4)
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }

    std::int64_t exponent{};
    cbor_item mantissa{};

    std::size_t index = 0;
    for (auto child : children())
    {
        if (index == 0)
        {
            auto r = child.get_int64();
            if (!r)
            {
                return result_type(jsoncons::unexpect, r.error());
            }
            exponent = *r;
        }
        if (index == 1)
        {
            if (child.type() == cbor_type::byte_string)
            {
                if (child.reverse_tags().empty())
                {
                    return result_type(jsoncons::unexpect, access_errc::invalid_type);
                }
                uint64_t child_tag = *(child.reverse_tags().begin());
                if (!(child_tag == 2 || child_tag == 3))
                {
                    return result_type(jsoncons::unexpect, access_errc::invalid_type);
                }
            }
            else if (!(child.type() == cbor_type::unsigned_integer || child.type() == cbor_type::negative_integer))
            {
                return result_type(jsoncons::unexpect, access_errc::invalid_type);
            }
            mantissa = child;
        }
        ++index;
    }

    return result_type{std::pair<int64_t,cbor_item>{exponent, mantissa}};
}

access_result<std::pair<int64_t,cbor_item>> cbor_item::get_bigfloat() const noexcept
{
    using value_type = std::pair<int64_t, cbor_item>;
    using result_type = access_result<value_type>;
    if (!(head_.major_type == cbor::cbor_major_type::array && head_.value == 2))
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }
    if (reverse_tags().empty())
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }
    uint64_t tag = *(reverse_tags().begin());
    if (tag != 5)
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }

    std::int64_t exponent{};
    cbor_item mantissa{};

    std::size_t index = 0;
    for (auto child : children())
    {
        if (index == 0)
        {
            auto r = child.get_int64();
            if (!r)
            {
                return result_type(jsoncons::unexpect, r.error());
            }
            exponent = *r;
        }
        if (index == 1)
        {
            if (child.type() == cbor_type::byte_string)
            {
                if (child.reverse_tags().empty())
                {
                    return result_type(jsoncons::unexpect, access_errc::invalid_type);
                }
                uint64_t child_tag = *(child.reverse_tags().begin());
                if (!(child_tag == 2 || child_tag == 3))
                {
                    return result_type(jsoncons::unexpect, access_errc::invalid_type);
                }
            }
            else if (!(child.type() == cbor_type::unsigned_integer || child.type() == cbor_type::negative_integer))
            {
                return result_type(jsoncons::unexpect, access_errc::invalid_type);
            }
            mantissa = child;
        }
        ++index;
    }

    return result_type{std::pair<int64_t,cbor_item>{exponent, mantissa}};
}

access_result<std::pair<bool,jsoncons::span<const uint8_t>>> cbor_item::get_bignum() const noexcept
{
    using value_type = std::pair<bool,jsoncons::span<const uint8_t>>;
    using result_type = access_result<value_type>;

    if (reverse_tags().empty())
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }
    uint64_t tag = *(reverse_tags().begin());
    if (!(tag == 2 || tag == 3))
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }
    if (major_type() != cbor_major_type::byte_string)
    {
        return result_type(jsoncons::unexpect, access_errc::invalid_type);
    }
    return result_type{value_type{(tag == 3), jsoncons::span<const uint8_t>{content_, head_.value}}};
}

} // namespace ondemand
} // namespace cbor
} // namespace jsoncons

#endif

// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CBOR_CBOR_VIEW_HPP
#define JSONCONS_EXT_CBOR_CBOR_VIEW_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/utility/unicode_traits.hpp>
#include <jsoncons_ext/cbor/cbor_detail.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>

// Zero-copy access to CBOR data in its encoded form. Scanning checks
// structural well-formedness once; the resulting items cannot fail.
// Tags are exposed, not interpreted: semantics such as bignums, string
// references and typed arrays belong to the parser and cursor.

namespace jsoncons {
namespace cbor {
namespace view {

    constexpr int default_max_nesting_depth = 1024;

    // The major type of an item's head, after any leading tags. `simple` is
    // major type 7: simple values and floats, told apart by the argument.
    enum class item_kind : uint8_t
    {
        unsigned_integer,
        negative_integer,
        byte_string,
        text_string,
        array,
        map,
        simple
    };

    struct scan_error
    {
        cbor_errc code;
        std::size_t offset;
    };

    namespace detail_view {

        struct item_head
        {
            cbor::detail::cbor_major_type major_type{};
            uint8_t additional_info{0};
            uint64_t value{0};

            bool indefinite() const noexcept
            {
                return additional_info == cbor::detail::additional_info::indefinite_length;
            }
        };

        inline item_kind kind(const item_head& head) noexcept
        {
            switch (head.major_type)
            {
                case cbor::detail::cbor_major_type::unsigned_integer: return item_kind::unsigned_integer;
                case cbor::detail::cbor_major_type::negative_integer: return item_kind::negative_integer;
                case cbor::detail::cbor_major_type::byte_string:      return item_kind::byte_string;
                case cbor::detail::cbor_major_type::text_string:      return item_kind::text_string;
                case cbor::detail::cbor_major_type::array:            return item_kind::array;
                case cbor::detail::cbor_major_type::map:              return item_kind::map;
                default:                                              return item_kind::simple;
            }
        }

        inline bool uint64_value(const item_head& head, uint64_t& value) noexcept
        {
            if (head.major_type != cbor::detail::cbor_major_type::unsigned_integer)
            {
                return false;
            }
            value = head.value;
            return true;
        }

        inline bool int64_value(const item_head& head, int64_t& value) noexcept
        {
            const uint64_t int64_max = static_cast<uint64_t>((std::numeric_limits<int64_t>::max)());
            if (head.major_type == cbor::detail::cbor_major_type::unsigned_integer && head.value <= int64_max)
            {
                value = static_cast<int64_t>(head.value);
                return true;
            }
            if (head.major_type == cbor::detail::cbor_major_type::negative_integer && head.value <= int64_max)
            {
                value = -1 - static_cast<int64_t>(head.value);
                return true;
            }
            return false;
        }

        inline bool bool_value(const item_head& head, bool& value) noexcept
        {
            if (head.major_type != cbor::detail::cbor_major_type::simple ||
                (head.additional_info != 20 && head.additional_info != 21))
            {
                return false;
            }
            value = head.additional_info == 21;
            return true;
        }

        inline bool double_value(const item_head& head, double& value) noexcept
        {
            if (head.major_type != cbor::detail::cbor_major_type::simple)
            {
                return false;
            }
            switch (head.additional_info)
            {
                case 25:
                    value = binary::decode_half(static_cast<uint16_t>(head.value));
                    return true;
                case 26:
                {
                    const uint32_t bits = static_cast<uint32_t>(head.value);
                    float single;
                    std::memcpy(&single, &bits, sizeof single);
                    value = single;
                    return true;
                }
                case 27:
                {
                    const uint64_t bits = head.value;
                    std::memcpy(&value, &bits, sizeof value);
                    return true;
                }
                default:
                    return false;
            }
        }

        inline bool text(const item_head& head, const uint8_t* content, string_view& value) noexcept
        {
            if (head.major_type != cbor::detail::cbor_major_type::text_string || head.indefinite())
            {
                return false;
            }
            value = string_view(reinterpret_cast<const char*>(content), static_cast<std::size_t>(head.value));
            return true;
        }

        inline bool bytes(const item_head& head, const uint8_t* content, span<const uint8_t>& value) noexcept
        {
            if (head.major_type != cbor::detail::cbor_major_type::byte_string || head.indefinite())
            {
                return false;
            }
            value = span<const uint8_t>(content, static_cast<std::size_t>(head.value));
            return true;
        }

        JSONCONS_FORCE_INLINE bool read_uint(const uint8_t*& p, const uint8_t* end, uint8_t info, uint64_t& value, cbor_errc& ec)
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

        JSONCONS_FORCE_INLINE bool read_head(const uint8_t*& p, const uint8_t* end, item_head& head, cbor_errc& ec)
        {
            if (p == end)
            {
                ec = cbor_errc::unexpected_eof;
                return false;
            }

            const uint8_t initial = *p++;
            head.major_type = static_cast<cbor::detail::cbor_major_type>(initial >> 5);
            head.additional_info = initial & 0x1f;
            head.value = 0;

            if (head.additional_info == cbor::detail::additional_info::indefinite_length)
            {
                switch (head.major_type)
                {
                    case cbor::detail::cbor_major_type::byte_string:
                    case cbor::detail::cbor_major_type::text_string:
                    case cbor::detail::cbor_major_type::array:
                    case cbor::detail::cbor_major_type::map:
                        break;
                    default:
                        ec = cbor_errc::unknown_type;
                        return false;
                }
                return true;
            }
            if (!read_uint(p, end, head.additional_info, head.value, ec))
            {
                return false;
            }
            // RFC 8949 3.3: a two-byte encoding of a simple value below 32
            // is not well-formed.
            if (head.major_type == cbor::detail::cbor_major_type::simple &&
                head.additional_info == 24 && head.value < 32)
            {
                ec = cbor_errc::unknown_type;
                return false;
            }
            return true;
        }

        // Head of the item's content, past any leading semantic tags.
        JSONCONS_FORCE_INLINE bool read_value_head(const uint8_t*& p, const uint8_t* end, item_head& head, cbor_errc& ec)
        {
            do
            {
                if (!read_head(p, end, head, ec))
                {
                    return false;
                }
            }
            while (head.major_type == cbor::detail::cbor_major_type::semantic_tag);
            return true;
        }

        inline bool skip_string_chunks(const uint8_t*& p, const uint8_t* end,
                                      cbor::detail::cbor_major_type expected_major, cbor_errc& ec)
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
                case cbor::detail::cbor_major_type::unsigned_integer:
                case cbor::detail::cbor_major_type::negative_integer:
                case cbor::detail::cbor_major_type::simple:
                    return true;

                case cbor::detail::cbor_major_type::byte_string:
                case cbor::detail::cbor_major_type::text_string:
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
            const bool is_map = head.major_type == cbor::detail::cbor_major_type::map;
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
                    if (head.major_type == cbor::detail::cbor_major_type::array ||
                        head.major_type == cbor::detail::cbor_major_type::map)
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
                if (head.major_type == cbor::detail::cbor_major_type::array ||
                    head.major_type == cbor::detail::cbor_major_type::map)
                {
                    if (head.indefinite())
                    {
                        p = content;
                        const bool ok = skip_container(p, end, first, (std::numeric_limits<int>::max)(), ec);
                        assert(ok);
                        (void)ok;
                        return p;
                    }
                    left += head.major_type == cbor::detail::cbor_major_type::map ? 2 * head.value : head.value;
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

    } // namespace detail_view

    // A zero-copy view of one complete, structurally well-formed CBOR item:
    // its leading tags, head and content. Obtained only from scan, parse_item
    // or children(). Borrows the scanned bytes and must not outlive them.
    class item
    {
    public:
        // The item's complete encoding, including leading tags.
        span<const uint8_t> encoded_bytes() const noexcept
        {
            return bytes_;
        }

        item_kind kind() const noexcept
        {
            return detail_view::kind(head_);
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
        class tag_range;
        class child_iterator;
        class child_range;

        // The item's leading tag numbers, outermost first.
        tag_range tags() const noexcept;

        // The data items this item is made of: an array's elements, a map's
        // keys and values alternating, or the definite-length chunks of an
        // indefinite-length string (RFC 8949 3.2.3). Empty for anything else.
        child_range children() const noexcept;

        // The typed accessors return false, leaving `value` unchanged, if the
        // item is not of that kind or range.
        bool uint64_value(uint64_t& value) const noexcept
        {
            return detail_view::uint64_value(head_, value);
        }

        bool int64_value(int64_t& value) const noexcept
        {
            return detail_view::int64_value(head_, value);
        }

        bool bool_value(bool& value) const noexcept
        {
            return detail_view::bool_value(head_, value);
        }

        bool double_value(double& value) const noexcept
        {
            return detail_view::double_value(head_, value);
        }

        // Zero-copy string content. False for indefinite-length strings,
        // whose chunks are their children.
        bool text(string_view& value) const noexcept
        {
            return detail_view::text(head_, content_, value);
        }

        bool bytes(span<const uint8_t>& value) const noexcept
        {
            return detail_view::bytes(head_, content_, value);
        }

        // Copies string content, assembling chunked strings. `value` is replaced
        // on success, unchanged on failure, and may alias the item's bytes.
        bool text(std::string& value) const;
        bool bytes(std::vector<uint8_t>& value) const;

    private:
        friend struct detail_view::item_access;

        item(span<const uint8_t> bytes, const detail_view::item_head& head, const uint8_t* content) noexcept
            : bytes_(bytes), head_(head), content_(content)
        {
        }

        // A string's content: its payload, or its chunks' payloads in order.
        std::size_t content_size() const noexcept;
        void copy_content(uint8_t* out) const noexcept;

        span<const uint8_t> bytes_;
        detail_view::item_head head_;
        const uint8_t* content_;   // first byte after the untagged head
    };

    namespace detail_view {

        struct item_access
        {
            static item make(span<const uint8_t> bytes, const item_head& head, const uint8_t* content) noexcept
            {
                return item(bytes, head, content);
            }
        };

    } // namespace detail_view

    // Iterates an item's leading tag numbers.
    class item::tag_iterator
    {
    public:
        using value_type = uint64_t;
        using reference = uint64_t;
        using pointer = void;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        tag_iterator() noexcept : pos_(nullptr), stop_(nullptr), value_(0) {}

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
        friend class tag_range;

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
            detail_view::item_head head;
            cbor_errc ec{};
            const uint8_t* p = pos_;
            const bool ok = detail_view::read_head(p, stop_, head, ec);
            assert(ok && head.major_type == cbor::detail::cbor_major_type::semantic_tag);
            (void)ok;
            value_ = head.value;
            pos_ = p;
        }

        const uint8_t* pos_;    // nullptr when exhausted
        const uint8_t* stop_;   // first byte of the untagged head
        uint64_t value_;
    };

    class item::tag_range
    {
    public:
        using iterator = tag_iterator;
        using const_iterator = tag_iterator;

        tag_iterator begin() const noexcept { return tag_iterator(first_, stop_); }
        tag_iterator end() const noexcept { return tag_iterator(); }
        bool empty() const noexcept { return first_ == stop_; }

    private:
        friend class item;
        tag_range(const uint8_t* first, const uint8_t* stop) noexcept : first_(first), stop_(stop) {}

        const uint8_t* first_;
        const uint8_t* stop_;
    };

    inline item::tag_range item::tags() const noexcept
    {
        // Leading tags occupy [start of encoding, start of untagged head).
        return tag_range(bytes_.data(), content_ - detail_view::head_size(head_));
    }

    // Iterates an item's children, measuring each once. The last child of a
    // definite-length container ends where its parent does, so is not measured.
    class item::child_iterator
    {
    public:
        using value_type = item;
        using reference = item;
        using pointer = void;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        child_iterator() noexcept
            : pos_(nullptr), next_(nullptr), end_(nullptr), remaining_(0), content_(nullptr)
        {
        }

        item operator*() const noexcept
        {
            assert(pos_ != nullptr);
            return detail_view::item_access::make(
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
            if (remaining_ == detail_view::indefinite_array_marker)
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
            const bool ok = detail_view::read_value_head(p, end_, head_, ec);
            assert(ok);
            (void)ok;
            content_ = p;
            next_ = remaining_ == 0 ? end_ : detail_view::skip_checked(p, end_, head_);
        }

        const uint8_t* pos_;    // current child's begin, nullptr when exhausted
        const uint8_t* next_;   // current child's end, the next child's begin
        const uint8_t* end_;    // the parent's end
        uint64_t remaining_;    // raw children left; indefinite ends at a break
        detail_view::item_head head_;
        const uint8_t* content_;
    };

    class item::child_range
    {
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
                (remaining_ == detail_view::indefinite_array_marker
                    ? *first_ == 0xff : remaining_ == 0);
        }

    private:
        friend class item;
        child_range(const uint8_t* first, const uint8_t* end, uint64_t remaining) noexcept
            : first_(first), end_(end), remaining_(remaining)
        {
        }

        const uint8_t* first_;
        const uint8_t* end_;
        uint64_t remaining_;
    };

    inline item::child_range item::children() const noexcept
    {
        const uint8_t* const end = bytes_.data() + bytes_.size();
        switch (head_.major_type)
        {
            case cbor::detail::cbor_major_type::array:
            case cbor::detail::cbor_major_type::map:
                if (!head_.indefinite())
                {
                    const bool is_map = head_.major_type == cbor::detail::cbor_major_type::map;
                    return child_range(content_, end, is_map ? 2 * head_.value : head_.value);
                }
                return child_range(content_, end, detail_view::indefinite_array_marker);
            case cbor::detail::cbor_major_type::byte_string:
            case cbor::detail::cbor_major_type::text_string:
                if (head_.indefinite())
                {
                    return child_range(content_, end, detail_view::indefinite_array_marker);
                }
                break;
            default:
                break;
        }
        return child_range(nullptr, nullptr, 0);
    }

    inline std::size_t item::content_size() const noexcept
    {
        if (!head_.indefinite())
        {
            return static_cast<std::size_t>(head_.value);
        }
        std::size_t size = 0;
        for (const item chunk : children())
        {
            size += static_cast<std::size_t>(chunk.head_.value);
        }
        return size;
    }

    inline void item::copy_content(uint8_t* out) const noexcept
    {
        if (!head_.indefinite())
        {
            if (head_.value != 0)
            {
                std::memcpy(out, content_, static_cast<std::size_t>(head_.value));
            }
            return;
        }
        for (const item chunk : children())
        {
            chunk.copy_content(out);
            out += static_cast<std::size_t>(chunk.head_.value);
        }
    }

    inline bool item::text(std::string& value) const
    {
        if (head_.major_type != cbor::detail::cbor_major_type::text_string)
        {
            return false;
        }
        std::string content(content_size(), '\0');
        if (!content.empty())
        {
            copy_content(reinterpret_cast<uint8_t*>(&content[0]));
        }
        value = std::move(content);
        return true;
    }

    inline bool item::bytes(std::vector<uint8_t>& value) const
    {
        if (head_.major_type != cbor::detail::cbor_major_type::byte_string)
        {
            return false;
        }
        std::vector<uint8_t> content(content_size());
        if (!content.empty())
        {
            copy_content(content.data());
        }
        value = std::move(content);
        return true;
    }

    struct scan_result
    {
        item first;
        span<const uint8_t> remainder;
    };

    // Scans the first item in `input`, checking its structure, and returns it
    // with the bytes that follow. Errors report where scanning stopped.
    // Allocates only when nesting exceeds 32 open containers.
    inline expected<scan_result, scan_error> scan(span<const uint8_t> input,
        int max_nesting_depth = default_max_nesting_depth)
    {
        if (input.empty())
        {
            return expected<scan_result, scan_error>(unexpect,
                scan_error{cbor_errc::unexpected_eof, 0});
        }
        const uint8_t* p = input.data();
        const uint8_t* end = p + input.size();
        cbor_errc ec{};

        detail_view::item_head head;
        bool ok = detail_view::read_value_head(p, end, head, ec);
        const uint8_t* content = p;
        if (ok)
        {
            if (head.major_type == cbor::detail::cbor_major_type::array ||
                head.major_type == cbor::detail::cbor_major_type::map)
            {
                ok = detail_view::skip_container(p, end, head, max_nesting_depth, ec);
            }
            else
            {
                ok = detail_view::skip_scalar_or_string(head, p, end, ec);
            }
        }
        if (!ok)
        {
            return expected<scan_result, scan_error>(unexpect,
                scan_error{ec, static_cast<std::size_t>(p - input.data())});
        }
        return scan_result{
            detail_view::item_access::make(
                span<const uint8_t>(input.data(), static_cast<std::size_t>(p - input.data())), head, content),
            span<const uint8_t>(p, static_cast<std::size_t>(end - p))};
    }

    // Like scan, but `input` must hold exactly one item: trailing bytes are
    // cbor_errc::trailing_data.
    inline expected<item, scan_error> parse_item(span<const uint8_t> input,
        int max_nesting_depth = default_max_nesting_depth)
    {
        auto scanned = scan(input, max_nesting_depth);
        if (!scanned)
        {
            return expected<item, scan_error>(unexpect, scanned.error());
        }
        if (!scanned.value().remainder.empty())
        {
            return expected<item, scan_error>(unexpect,
                scan_error{cbor_errc::trailing_data, scanned.value().first.encoded_bytes().size()});
        }
        return scanned.value().first;
    }

    // Reject temporary owning containers, whose items would dangle.
    template <typename Container>
    expected<scan_result, scan_error> scan(const Container&&, int = default_max_nesting_depth) = delete;
    template <typename Container>
    expected<item, scan_error> parse_item(const Container&&, int = default_max_nesting_depth) = delete;

    // True if `text_item` is a text string of well-formed UTF-8, checking each
    // chunk of an indefinite-length string separately (RFC 8949 3.2.3).
    inline bool validate_text(const item& text_item) noexcept
    {
        string_view text;
        if (text_item.text(text))
        {
            return detail_view::validate_utf8(reinterpret_cast<const uint8_t*>(text.data()), text.size());
        }
        if (text_item.kind() != item_kind::text_string)
        {
            return false;
        }
        for (const item chunk : text_item.children())
        {
            if (!chunk.text(text) ||
                !detail_view::validate_utf8(reinterpret_cast<const uint8_t*>(text.data()), text.size()))
            {
                return false;
            }
        }
        return true;
    }

} // namespace view
} // namespace cbor
} // namespace jsoncons

#endif

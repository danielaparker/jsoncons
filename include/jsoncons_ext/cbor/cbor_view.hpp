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
#include <system_error>
#include <type_traits>
#include <vector>

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/detail/expected.hpp>
#include <jsoncons/utility/unicode_traits.hpp>
#include <jsoncons_ext/cbor/cbor_detail.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>

// A deliberately narrow, zero-copy facility for reading the *encoded*
// structure of CBOR data in place. Scanning validates well-formedness
// once; everything after that operates on checked `item` views and
// cannot fail structurally. Semantic interpretation (tag 2 bignums,
// string references, typed arrays, ...) belongs to the parser and
// cursor layers, not here: tags are exposed, never interpreted.

namespace jsoncons {
namespace cbor {
namespace view {

    constexpr int default_max_nesting_depth = 1024;

    // The wire-level kind of an item, after its leading semantic tags.
    // `simple` covers RFC 8949 major type 7: simple values and floating
    // point, distinguished by the head argument.
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
        std::size_t offset;   // bytes consumed when the error was detected
    };

    using jsoncons::detail::expected;
    using jsoncons::detail::unexpect;

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

        inline int sign(int value) noexcept
        {
            return (value > 0) - (value < 0);
        }

        inline int compare_size(std::size_t a, std::size_t b) noexcept
        {
            return a < b ? -1 : (a > b ? 1 : 0);
        }

        inline int compare_bytes(span<const uint8_t> a, span<const uint8_t> b) noexcept
        {
            const std::size_t n = (std::min)(a.size(), b.size());
            if (n != 0)
            {
                int r = std::memcmp(a.data(), b.data(), n);
                if (r != 0)
                {
                    return sign(r);
                }
            }
            return compare_size(a.size(), b.size());
        }

        JSONCONS_FORCE_INLINE bool read_uint(const uint8_t*& p, const uint8_t* end, uint8_t info, uint64_t& value, std::error_code& ec)
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
                    ec = cbor_errc::unknown_type;
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

        JSONCONS_FORCE_INLINE bool read_head(const uint8_t*& p, const uint8_t* end, item_head& head, std::error_code& ec)
        {
            if (p >= end)
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
        JSONCONS_FORCE_INLINE bool read_value_head(const uint8_t*& p, const uint8_t* end, item_head& head, std::error_code& ec)
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
                                      cbor::detail::cbor_major_type expected_major, std::error_code& ec)
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
        JSONCONS_FORCE_INLINE bool skip_scalar_or_string(const item_head& head, const uint8_t*& p, const uint8_t* end, std::error_code& ec)
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

        // Remaining raw items in an open container, or a marker for an
        // indefinite-length one, which ends at a break byte instead: arrays
        // accept a break before any element, maps only between entries, never
        // between a key and its value. Counts admitted by skip_container are
        // bounded by the bytes remaining in the input, so a count can never
        // collide with a marker.
        constexpr uint64_t indefinite_array_marker = UINT64_MAX;
        constexpr uint64_t indefinite_map_key_marker = UINT64_MAX - 1;
        constexpr uint64_t indefinite_map_value_marker = UINT64_MAX - 2;

        class pending_stack
        {
            static constexpr std::size_t local_capacity = 32;

            uint64_t local_[local_capacity];
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

            void clear() noexcept
            {
                spill_.clear();
                size_ = 0;
            }

            void push(uint64_t count)
            {
                if (size_ < local_capacity)
                {
                    local_[size_] = count;
                }
                else
                {
                    spill_.push_back(count);
                }
                ++size_;
            }

            uint64_t pop() noexcept
            {
                --size_;
                if (size_ >= local_capacity)
                {
                    const uint64_t count = spill_.back();
                    spill_.pop_back();
                    return count;
                }
                return local_[size_];
            }
        };

        // Iterative walk of the array or map whose head has just been read,
        // so nested input is bounded by max_nesting_depth alone and can never
        // exhaust the call stack. The innermost level's state stays in a
        // local; enclosing levels wait in `open`, with a synthetic root
        // level of zero at the bottom.
        inline bool skip_container(const uint8_t*& p, const uint8_t* end, item_head head,
            int max_nesting_depth, pending_stack& open, std::error_code& ec)
        {
            const std::size_t depth_limit = max_nesting_depth > 0 ? static_cast<std::size_t>(max_nesting_depth) : 0;

            open.clear();
            uint64_t current = 0;

            for (;;)
            {
                // Open the container in `head`.
                if (JSONCONS_UNLIKELY(open.size() >= depth_limit))
                {
                    ec = cbor_errc::max_nesting_depth_exceeded;
                    return false;
                }
                if (head.indefinite())
                {
                    open.push(current);
                    current = head.major_type == cbor::detail::cbor_major_type::map
                        ? indefinite_map_key_marker : indefinite_array_marker;
                }
                else
                {
                    // Every unread item occupies at least one byte, so a
                    // count the remaining input cannot hold is EOF now.
                    const bool is_map = head.major_type == cbor::detail::cbor_major_type::map;
                    const uint64_t avail = static_cast<uint64_t>(end - p);
                    if (head.value > (is_map ? avail / 2 : avail))
                    {
                        ec = cbor_errc::unexpected_eof;
                        return false;
                    }
                    const uint64_t count = is_map ? 2 * head.value : head.value;
                    if (count != 0)
                    {
                        open.push(current);
                        current = count;
                    }
                }

                // Walk items in place until the next container head.
                for (;;)
                {
                    // Retire finished levels, then reserve the next unread item.
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

        inline bool skip_item(const uint8_t*& p, const uint8_t* end, int max_nesting_depth,
            pending_stack& open, std::error_code& ec)
        {
            item_head head;
            if (!read_value_head(p, end, head, ec))
            {
                return false;
            }
            if (head.major_type == cbor::detail::cbor_major_type::array ||
                head.major_type == cbor::detail::cbor_major_type::map)
            {
                return skip_container(p, end, head, max_nesting_depth, open, ec);
            }
            return skip_scalar_or_string(head, p, end, ec);
        }

        // Skips one item over bytes already validated by a scan, surfacing
        // the untagged head and content position so callers can build items
        // without reparsing. Cannot fail; the depth bound was enforced when
        // the bytes were scanned. Shallow items take no workspace at all.
        inline const uint8_t* skip_checked(const uint8_t* p, const uint8_t* end,
            item_head& head, const uint8_t*& content) noexcept
        {
            std::error_code ec;
            bool ok = read_value_head(p, end, head, ec);
            assert(ok);
            content = p;
            if (head.major_type == cbor::detail::cbor_major_type::array ||
                head.major_type == cbor::detail::cbor_major_type::map)
            {
                pending_stack open;
                ok = skip_container(p, end, head, (std::numeric_limits<int>::max)(), open, ec);
            }
            else
            {
                ok = skip_scalar_or_string(head, p, end, ec);
            }
            assert(ok && !ec);
            (void)ok;
            return p;
        }

        inline bool validate_utf8(const uint8_t* p, std::size_t length) noexcept
        {
            const auto result = unicode_traits::validate(reinterpret_cast<const char*>(p), length);
            return result.ec == unicode_traits::unicode_errc();
        }

        struct item_access;
        struct scan_access;

    } // namespace detail_view

    // A checked, zero-copy view of one complete, well-formed encoded CBOR
    // item: its leading semantic tags, head, and content. Obtained from
    // scan_prefix or parse_exact, or by iterating a container item; never
    // constructed from unvalidated bytes. Borrows the scanned input, so it
    // must not outlive the bytes it was scanned from.
    class item
    {
    public:
        // The item's full encoding, leading tags included. Scanning another
        // copy of these bytes yields an identical item.
        span<const uint8_t> encoded_bytes() const noexcept
        {
            return bytes_;
        }

        item_kind kind() const noexcept
        {
            switch (head_.major_type)
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

        // The head's argument (RFC 8949 3): the integer's value, a string's
        // length, a container's count, or a simple value's number, including
        // the bit patterns of floating-point values. Zero when indefinite.
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
        class element_iterator;
        class element_range;
        class chunk_iterator;
        class chunk_range;
        class entry_iterator;
        class entry_range;

        // The item's leading semantic tags, outermost first. Empty for
        // untagged items. Tags are exposed, never interpreted: deciding what
        // a tag means is the caller's or a higher layer's concern.
        tag_range tags() const noexcept;

        // The elements of an array item, in order; empty unless kind() is
        // array. Iteration is read-only and borrows this item's bytes.
        element_range elements() const noexcept;

        // The entries of a map item, in order; empty unless kind() is map.
        entry_range entries() const noexcept;

        // The content of a string item as contiguous spans: one span for a
        // definite-length string, one per chunk for an indefinite-length
        // one. Empty unless kind() is byte_string or text_string.
        chunk_range chunks() const noexcept;

        // The typed accessors below return false, leaving `value` untouched,
        // when this item is not of the requested kind. Well-formedness was
        // established when the item was scanned, so kind is the only thing
        // that can be wrong.

        bool uint64_value(uint64_t& value) const noexcept
        {
            if (head_.major_type != cbor::detail::cbor_major_type::unsigned_integer)
            {
                return false;
            }
            value = head_.value;
            return true;
        }

        bool int64_value(int64_t& value) const noexcept
        {
            const uint64_t int64_max = static_cast<uint64_t>((std::numeric_limits<int64_t>::max)());
            if (head_.major_type == cbor::detail::cbor_major_type::unsigned_integer && head_.value <= int64_max)
            {
                value = static_cast<int64_t>(head_.value);
                return true;
            }
            if (head_.major_type == cbor::detail::cbor_major_type::negative_integer && head_.value <= int64_max)
            {
                value = -1 - static_cast<int64_t>(head_.value);
                return true;
            }
            return false;
        }

        bool bool_value(bool& value) const noexcept
        {
            if (head_.major_type != cbor::detail::cbor_major_type::simple ||
                (head_.additional_info != 20 && head_.additional_info != 21))
            {
                return false;
            }
            value = head_.additional_info == 21;
            return true;
        }

        bool double_value(double& value) const noexcept
        {
            if (head_.major_type != cbor::detail::cbor_major_type::simple)
            {
                return false;
            }
            switch (head_.additional_info)
            {
                case 25:
                    value = binary::decode_half(static_cast<uint16_t>(head_.value));
                    return true;
                case 26:
                {
                    const uint32_t bits = static_cast<uint32_t>(head_.value);
                    float single;
                    std::memcpy(&single, &bits, sizeof single);
                    value = single;
                    return true;
                }
                case 27:
                {
                    const uint64_t bits = head_.value;
                    std::memcpy(&value, &bits, sizeof value);
                    return true;
                }
                default:
                    return false;
            }
        }

        // Zero-copy string content. False for indefinite-length (chunked)
        // strings, whose content is not contiguous; use chunks() or the
        // copying overloads for those.
        bool text(string_view& value) const noexcept
        {
            if (head_.major_type != cbor::detail::cbor_major_type::text_string || head_.indefinite())
            {
                return false;
            }
            value = string_view(reinterpret_cast<const char*>(content_), static_cast<std::size_t>(head_.value));
            return true;
        }

        bool bytes(span<const uint8_t>& value) const noexcept
        {
            if (head_.major_type != cbor::detail::cbor_major_type::byte_string || head_.indefinite())
            {
                return false;
            }
            value = span<const uint8_t>(content_, static_cast<std::size_t>(head_.value));
            return true;
        }

        // Copying string content, assembling chunked strings. Transactional:
        // `value` is replaced on success and untouched on failure, and may
        // alias this item's underlying bytes.
        bool text(std::string& value) const;
        bool bytes(std::vector<uint8_t>& value) const;

    private:
        friend struct detail_view::item_access;

        explicit item(span<const uint8_t> bytes) noexcept
            : bytes_(bytes)
        {
            const uint8_t* p = bytes.data();
            const uint8_t* end = p + bytes.size();
            std::error_code ec;
            const bool ok = detail_view::read_value_head(p, end, head_, ec);
            assert(ok && !ec);
            (void)ok;
            content_ = p;
        }

        item(span<const uint8_t> bytes, const detail_view::item_head& head, const uint8_t* content) noexcept
            : bytes_(bytes), head_(head), content_(content)
        {
        }

        span<const uint8_t> bytes_;
        detail_view::item_head head_;
        const uint8_t* content_;   // first byte after the untagged head
    };

    namespace detail_view {

        struct item_access
        {
            static item make(span<const uint8_t> bytes) noexcept
            {
                return item(bytes);
            }

            static item make(span<const uint8_t> bytes, const item_head& head, const uint8_t* content) noexcept
            {
                return item(bytes, head, content);
            }
        };

    } // namespace detail_view

    // Iterates the values of an item's leading semantic tags.
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

        tag_iterator& operator++()
        {
            advance();
            return *this;
        }

        tag_iterator operator++(int)
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
        friend class item;
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
            item_head head;
            std::error_code ec;
            const uint8_t* p = pos_;
            const bool ok = detail_view::read_head(p, stop_, head, ec);
            assert(ok && head.major_type == cbor::detail::cbor_major_type::semantic_tag);
            (void)ok;
            value_ = head.value;
            pos_ = p;
        }

        using item_head = detail_view::item_head;
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
        const uint8_t* p = bytes_.data();
        const uint8_t* stop = p;
        detail_view::item_head head;
        std::error_code ec;
        while (detail_view::read_head(p, bytes_.data() + bytes_.size(), head, ec) &&
               head.major_type == cbor::detail::cbor_major_type::semantic_tag)
        {
            stop = p;
        }
        return tag_range(bytes_.data(), stop);
    }

    // Iterates the elements of an array item as checked items.
    class item::element_iterator
    {
    public:
        using value_type = item;
        using reference = item;
        using pointer = void;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        element_iterator() noexcept : pos_(nullptr), end_(nullptr), remaining_(0), current_(nullptr), content_(nullptr) {}

        item operator*() const noexcept
        {
            assert(pos_ != nullptr);
            return detail_view::item_access::make(
                span<const uint8_t>(pos_, static_cast<std::size_t>(current_ - pos_)), head_, content_);
        }

        element_iterator& operator++()
        {
            pos_ = current_;
            advance();
            return *this;
        }

        element_iterator operator++(int)
        {
            element_iterator temp = *this;
            ++(*this);
            return temp;
        }

        friend bool operator==(const element_iterator& a, const element_iterator& b) noexcept
        {
            return a.pos_ == b.pos_;
        }

        friend bool operator!=(const element_iterator& a, const element_iterator& b) noexcept
        {
            return a.pos_ != b.pos_;
        }

    private:
        friend class item;
        friend class element_range;

        element_iterator(const uint8_t* pos, const uint8_t* end, uint64_t remaining) noexcept
            : pos_(pos), end_(end), remaining_(remaining), current_(nullptr), content_(nullptr)
        {
            advance();
        }

        void advance() noexcept
        {
            if (pos_ == nullptr)
            {
                return;
            }
            if (remaining_ == detail_view::indefinite_array_marker)
            {
                assert(pos_ < end_);
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
            current_ = detail_view::skip_checked(pos_, end_, head_, content_);
        }

        const uint8_t* pos_;       // start of the current element; nullptr at end
        const uint8_t* end_;
        uint64_t remaining_;       // count left, or indefinite_array_marker
        const uint8_t* current_;   // end of the current element
        detail_view::item_head head_;
        const uint8_t* content_;
    };

    class item::element_range
    {
    public:
        using iterator = element_iterator;
        using const_iterator = element_iterator;

        element_iterator begin() const noexcept { return element_iterator(first_, end_, remaining_); }
        element_iterator end() const noexcept { return element_iterator(); }
        bool empty() const noexcept { return begin() == this->end(); }

    private:
        friend class item;
        element_range(const uint8_t* first, const uint8_t* end, uint64_t remaining) noexcept
            : first_(first), end_(end), remaining_(remaining) {}

        const uint8_t* first_;
        const uint8_t* end_;
        uint64_t remaining_;
    };

    inline item::element_range item::elements() const noexcept
    {
        if (head_.major_type != cbor::detail::cbor_major_type::array)
        {
            return element_range(nullptr, nullptr, 0);
        }
        return element_range(content_, bytes_.data() + bytes_.size(),
            head_.indefinite() ? detail_view::indefinite_array_marker : head_.value);
    }

    // One entry of a map item.
    struct map_entry
    {
        item key;
        item value;
    };

    // Iterates the entries of a map item as checked key and value items.
    class item::entry_iterator
    {
    public:
        using value_type = map_entry;
        using reference = map_entry;
        using pointer = void;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        entry_iterator() noexcept : pos_(nullptr), end_(nullptr), remaining_(0), key_end_(nullptr), value_end_(nullptr),
            key_content_(nullptr), value_content_(nullptr) {}

        map_entry operator*() const noexcept
        {
            assert(pos_ != nullptr);
            return map_entry{
                detail_view::item_access::make(
                    span<const uint8_t>(pos_, static_cast<std::size_t>(key_end_ - pos_)), key_head_, key_content_),
                detail_view::item_access::make(
                    span<const uint8_t>(key_end_, static_cast<std::size_t>(value_end_ - key_end_)), value_head_, value_content_)};
        }

        entry_iterator& operator++()
        {
            pos_ = value_end_;
            advance();
            return *this;
        }

        entry_iterator operator++(int)
        {
            entry_iterator temp = *this;
            ++(*this);
            return temp;
        }

        friend bool operator==(const entry_iterator& a, const entry_iterator& b) noexcept
        {
            return a.pos_ == b.pos_;
        }

        friend bool operator!=(const entry_iterator& a, const entry_iterator& b) noexcept
        {
            return a.pos_ != b.pos_;
        }

    private:
        friend class item;
        friend class entry_range;

        entry_iterator(const uint8_t* pos, const uint8_t* end, uint64_t remaining) noexcept
            : pos_(pos), end_(end), remaining_(remaining), key_end_(nullptr), value_end_(nullptr),
              key_content_(nullptr), value_content_(nullptr)
        {
            advance();
        }

        void advance() noexcept
        {
            if (pos_ == nullptr)
            {
                return;
            }
            if (remaining_ == detail_view::indefinite_map_key_marker)
            {
                assert(pos_ < end_);
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
            key_end_ = detail_view::skip_checked(pos_, end_, key_head_, key_content_);
            value_end_ = detail_view::skip_checked(key_end_, end_, value_head_, value_content_);
        }

        const uint8_t* pos_;        // start of the current entry's key; nullptr at end
        const uint8_t* end_;
        uint64_t remaining_;        // entry count left, or indefinite_map_key_marker
        const uint8_t* key_end_;
        const uint8_t* value_end_;
        detail_view::item_head key_head_;
        const uint8_t* key_content_;
        detail_view::item_head value_head_;
        const uint8_t* value_content_;
    };

    class item::entry_range
    {
    public:
        using iterator = entry_iterator;
        using const_iterator = entry_iterator;

        entry_iterator begin() const noexcept { return entry_iterator(first_, end_, remaining_); }
        entry_iterator end() const noexcept { return entry_iterator(); }
        bool empty() const noexcept { return begin() == this->end(); }

    private:
        friend class item;
        entry_range(const uint8_t* first, const uint8_t* end, uint64_t remaining) noexcept
            : first_(first), end_(end), remaining_(remaining) {}

        const uint8_t* first_;
        const uint8_t* end_;
        uint64_t remaining_;
    };

    inline item::entry_range item::entries() const noexcept
    {
        if (head_.major_type != cbor::detail::cbor_major_type::map)
        {
            return entry_range(nullptr, nullptr, 0);
        }
        return entry_range(content_, bytes_.data() + bytes_.size(),
            head_.indefinite() ? detail_view::indefinite_map_key_marker : head_.value);
    }

    // Iterates the contiguous spans of a string item's content.
    class item::chunk_iterator
    {
    public:
        using value_type = span<const uint8_t>;
        using reference = span<const uint8_t>;
        using pointer = void;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        chunk_iterator() noexcept : pos_(nullptr), end_(nullptr) {}

        span<const uint8_t> operator*() const noexcept
        {
            assert(pos_ != nullptr);
            return current_;
        }

        chunk_iterator& operator++()
        {
            advance();
            return *this;
        }

        chunk_iterator operator++(int)
        {
            chunk_iterator temp = *this;
            advance();
            return temp;
        }

        friend bool operator==(const chunk_iterator& a, const chunk_iterator& b) noexcept
        {
            return a.pos_ == b.pos_;
        }

        friend bool operator!=(const chunk_iterator& a, const chunk_iterator& b) noexcept
        {
            return a.pos_ != b.pos_;
        }

    private:
        friend class item;
        friend class chunk_range;

        // Definite string: one chunk covering the whole payload.
        chunk_iterator(span<const uint8_t> payload) noexcept
            : pos_(payload.data()), end_(nullptr), current_(payload)
        {
        }

        // Indefinite string: one chunk per definite-length piece.
        chunk_iterator(const uint8_t* pos, const uint8_t* end) noexcept
            : pos_(pos), end_(end)
        {
            advance();
        }

        void advance() noexcept
        {
            if (pos_ == nullptr)
            {
                return;
            }
            if (end_ == nullptr || *pos_ == 0xff)   // single definite chunk consumed, or break
            {
                pos_ = nullptr;
                return;
            }
            detail_view::item_head head;
            std::error_code ec;
            const uint8_t* p = pos_;
            const bool ok = detail_view::read_head(p, end_, head, ec);
            assert(ok && !head.indefinite());
            (void)ok;
            current_ = span<const uint8_t>(p, static_cast<std::size_t>(head.value));
            pos_ = p + static_cast<std::size_t>(head.value);
        }

        const uint8_t* pos_;    // nullptr when exhausted
        const uint8_t* end_;    // nullptr for the single definite chunk
        span<const uint8_t> current_;
    };

    class item::chunk_range
    {
    public:
        using iterator = chunk_iterator;
        using const_iterator = chunk_iterator;

        chunk_iterator begin() const noexcept
        {
            if (first_ == nullptr)
            {
                return chunk_iterator();
            }
            if (indefinite_)
            {
                return chunk_iterator(first_, end_);
            }
            return chunk_iterator(span<const uint8_t>(first_, static_cast<std::size_t>(end_ - first_)));
        }

        chunk_iterator end() const noexcept { return chunk_iterator(); }
        bool empty() const noexcept { return begin() == this->end(); }

    private:
        friend class item;
        chunk_range(const uint8_t* first, const uint8_t* end, bool indefinite) noexcept
            : first_(first), end_(end), indefinite_(indefinite) {}

        const uint8_t* first_;
        const uint8_t* end_;
        bool indefinite_;
    };

    inline item::chunk_range item::chunks() const noexcept
    {
        if (head_.major_type != cbor::detail::cbor_major_type::byte_string &&
            head_.major_type != cbor::detail::cbor_major_type::text_string)
        {
            return chunk_range(nullptr, nullptr, false);
        }
        if (head_.indefinite())
        {
            return chunk_range(content_, bytes_.data() + bytes_.size(), true);
        }
        return chunk_range(content_, content_ + static_cast<std::size_t>(head_.value), false);
    }

    inline bool item::text(std::string& value) const
    {
        if (head_.major_type != cbor::detail::cbor_major_type::text_string)
        {
            return false;
        }
        std::size_t total = 0;
        for (chunk_iterator it = chunks().begin(); it != chunk_iterator(); ++it)
        {
            total += (*it).size();
        }
        std::string content;
        content.reserve(total);
        for (chunk_iterator it = chunks().begin(); it != chunk_iterator(); ++it)
        {
            const span<const uint8_t> chunk = *it;
            content.append(reinterpret_cast<const char*>(chunk.data()), chunk.size());
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
        std::size_t total = 0;
        for (chunk_iterator it = chunks().begin(); it != chunk_iterator(); ++it)
        {
            total += (*it).size();
        }
        std::vector<uint8_t> content;
        content.reserve(total);
        for (chunk_iterator it = chunks().begin(); it != chunk_iterator(); ++it)
        {
            const span<const uint8_t> chunk = *it;
            content.insert(content.end(), chunk.data(), chunk.data() + chunk.size());
        }
        value = std::move(content);
        return true;
    }

    // Reusable scanning state: the depth policy and the walker's workspace.
    // Scanning allocates only when nesting exceeds 32 open containers, and
    // a reused context retains that capacity.
    class scan_context
    {
    public:
        scan_context() noexcept
            : max_nesting_depth_(default_max_nesting_depth)
        {
        }

        explicit scan_context(int max_nesting_depth) noexcept
            : max_nesting_depth_(max_nesting_depth)
        {
        }

        int max_nesting_depth() const noexcept
        {
            return max_nesting_depth_;
        }

    private:
        friend struct detail_view::scan_access;

        int max_nesting_depth_;
        detail_view::pending_stack workspace_;
    };

    namespace detail_view {

        struct scan_access
        {
            static pending_stack& workspace(scan_context& context) noexcept
            {
                return context.workspace_;
            }
        };

        // All error codes assigned by the walker are cbor_errc values.
        inline cbor_errc to_cbor_errc(const std::error_code& ec) noexcept
        {
            return static_cast<cbor_errc>(ec.value());
        }

    } // namespace detail_view

    struct scan_result
    {
        item first;
        span<const uint8_t> remainder;
    };

    // Scans the first item in `input`, validating its well-formedness, and
    // returns it together with the remaining bytes. On failure the error
    // holds the offending code and the offset at which scanning stopped.
    inline expected<scan_result, scan_error> scan_prefix(span<const uint8_t> input, scan_context& context)
    {
        const uint8_t* p = input.data();
        const uint8_t* end = p + input.size();
        std::error_code ec;
        if (!detail_view::skip_item(p, end, context.max_nesting_depth(),
                detail_view::scan_access::workspace(context), ec))
        {
            return expected<scan_result, scan_error>(unexpect,
                scan_error{detail_view::to_cbor_errc(ec), static_cast<std::size_t>(p - input.data())});
        }
        return scan_result{
            detail_view::item_access::make(span<const uint8_t>(input.data(), static_cast<std::size_t>(p - input.data()))),
            span<const uint8_t>(p, static_cast<std::size_t>(end - p))};
    }

    inline expected<scan_result, scan_error> scan_prefix(span<const uint8_t> input)
    {
        scan_context context;
        return scan_prefix(input, context);
    }

    // Scans `input`, which must hold exactly one item: trailing bytes are
    // an error (cbor_errc::trailing_data), unlike scan_prefix.
    inline expected<item, scan_error> parse_exact(span<const uint8_t> input, scan_context& context)
    {
        auto scanned = scan_prefix(input, context);
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

    inline expected<item, scan_error> parse_exact(span<const uint8_t> input)
    {
        scan_context context;
        return parse_exact(input, context);
    }

    // Guard against scanning a temporary container: the resulting views
    // would dangle immediately. Non-owning view types (spans, string views)
    // are trivially copyable and pass through.
    template <typename Container, typename std::enable_if<!std::is_trivially_copyable<Container>::value, int>::type = 0>
    expected<scan_result, scan_error> scan_prefix(const Container&&, scan_context&) = delete;
    template <typename Container, typename std::enable_if<!std::is_trivially_copyable<Container>::value, int>::type = 0>
    expected<scan_result, scan_error> scan_prefix(const Container&&) = delete;
    template <typename Container, typename std::enable_if<!std::is_trivially_copyable<Container>::value, int>::type = 0>
    expected<item, scan_error> parse_exact(const Container&&, scan_context&) = delete;
    template <typename Container, typename std::enable_if<!std::is_trivially_copyable<Container>::value, int>::type = 0>
    expected<item, scan_error> parse_exact(const Container&&) = delete;

    // Deterministic encoding orders over the encoded bytes of items.
    // The *_compare forms return a three-way result; the *_less forms are
    // strict-weak-order predicates for sorting and ordered containers.

    struct bytewise_compare
    {
        int operator()(span<const uint8_t> a, span<const uint8_t> b) const noexcept
        {
            return detail_view::compare_bytes(a, b);
        }

        int operator()(const item& a, const item& b) const noexcept
        {
            return (*this)(a.encoded_bytes(), b.encoded_bytes());
        }
    };

    struct length_first_compare
    {
        int operator()(span<const uint8_t> a, span<const uint8_t> b) const noexcept
        {
            const int r = detail_view::compare_size(a.size(), b.size());
            return r != 0 ? r : detail_view::compare_bytes(a, b);
        }

        int operator()(const item& a, const item& b) const noexcept
        {
            return (*this)(a.encoded_bytes(), b.encoded_bytes());
        }
    };

    struct bytewise_less
    {
        bool operator()(span<const uint8_t> a, span<const uint8_t> b) const noexcept
        {
            return bytewise_compare()(a, b) < 0;
        }

        bool operator()(const item& a, const item& b) const noexcept
        {
            return bytewise_compare()(a, b) < 0;
        }
    };

    struct length_first_less
    {
        bool operator()(span<const uint8_t> a, span<const uint8_t> b) const noexcept
        {
            return length_first_compare()(a, b) < 0;
        }

        bool operator()(const item& a, const item& b) const noexcept
        {
            return length_first_compare()(a, b) < 0;
        }
    };

    // True if `map_item` is a map whose keys are strictly ascending in the
    // given order, RFC 8949 4.2 deterministic encoding style. Duplicate
    // keys are not ascending. Does not allocate.
    template <typename Compare = bytewise_compare>
    bool map_keys_sorted(const item& map_item, Compare compare = Compare())
    {
        if (map_item.kind() != item_kind::map)
        {
            return false;
        }
        span<const uint8_t> prev;
        for (item::entry_iterator it = map_item.entries().begin(); it != item::entry_iterator(); ++it)
        {
            const span<const uint8_t> key = (*it).key.encoded_bytes();
            if (!prev.empty() && compare(prev, key) >= 0)
            {
                return false;
            }
            prev = key;
        }
        return true;
    }

    // True if `text_item` is a text string whose content is well-formed
    // UTF-8. Each chunk of an indefinite-length text string must itself be
    // well-formed (RFC 8949 3.2.3). No other function validates text.
    inline bool validate_text(const item& text_item) noexcept
    {
        if (text_item.kind() != item_kind::text_string)
        {
            return false;
        }
        for (item::chunk_iterator it = text_item.chunks().begin(); it != item::chunk_iterator(); ++it)
        {
            const span<const uint8_t> chunk = *it;
            if (!detail_view::validate_utf8(chunk.data(), chunk.size()))
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

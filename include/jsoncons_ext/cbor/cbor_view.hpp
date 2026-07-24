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
#include <vector>

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/detail/expected.hpp>
#include <jsoncons/utility/unicode_traits.hpp>
#include <jsoncons_ext/cbor/cbor_detail.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>

// A deliberately narrow, zero-copy facility for reading the *encoded*
// structure of CBOR data in place. Scanning validates structural well-formedness
// once; everything after that operates on checked items or navigation state and
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

    enum class position_role : uint8_t
    {
        root,
        array_element,
        map_key,
        map_value
    };

    // The CBOR major type
    enum class major_type : uint8_t
    {
        unsigned_integer = 0,
        negative_integer = 1,
        byte_string = 2,
        text_string = 3,
        array = 4,
        map = 5,
        semantic_tag = 6,
        simple = 7
    };

    struct scan_error
    {
        cbor_errc code;
        std::size_t offset;
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

            uint64_t local_[local_capacity]{};
            std::vector<uint64_t> spill_;
            std::size_t size_{0};
            std::size_t peak_size_{0};
        public:
            bool empty() const noexcept
            {
                return size_ == 0;
            }

            std::size_t size() const noexcept
            {
                return size_;
            }

            std::size_t peak_size() const noexcept
            {
                return peak_size_;
            }

            void clear() noexcept
            {
                spill_.clear();
                size_ = 0;
                peak_size_ = 0;
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
                if (size_ > peak_size_)
                {
                    peak_size_ = size_;
                }
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


        inline bool validate_utf8(const uint8_t* p, std::size_t length) noexcept
        {
            const auto result = unicode_traits::validate(reinterpret_cast<const char*>(p), length);
            return result.ec == unicode_traits::unicode_errc();
        }

        struct item_access;
        struct scan_access;
        struct navigator_access;

    } // namespace detail_view

    class scan_context;

    // A checked, zero-copy view of one complete, structurally well-formed CBOR
    // item: its leading semantic tags, head, and content. Obtained from
    // scan_prefix, parse_exact, wire_cursor, or navigator::finish_item; never
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
            return detail_view::kind(head_);
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
        class chunk_iterator;
        class chunk_range;
        class child_iterator;
        class child_range;

        // The item's leading semantic tags, outermost first. Empty for
        // untagged items. Tags are exposed, never interpreted: deciding what
        // a tag means is the caller's or a higher layer's concern.
        tag_range tags() const noexcept;

        // The content of a string item as contiguous spans: one span for a
        // definite-length string, one per chunk for an indefinite-length
        // one. Empty unless kind() is byte_string or text_string.
        chunk_range chunks() const noexcept;

        // Raw children of an array or map: elements, or keys and values alternating.
        child_range children(scan_context& context) const noexcept;

        // The typed accessors return false, leaving `value` untouched, on a
        // kind mismatch. Structural well-formedness was established by scanning.
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

        // Zero-copy string content. False for indefinite-length strings.
        bool text(string_view& value) const noexcept
        {
            return detail_view::text(head_, content_, value);
        }

        bool bytes(span<const uint8_t>& value) const noexcept
        {
            return detail_view::bytes(head_, content_, value);
        }

        // Copying string content, assembling chunked strings. Transactional:
        // `value` is replaced on success and untouched on failure, and may
        // alias this item's underlying bytes.
        bool text(std::string& value) const;
        bool bytes(std::vector<uint8_t>& value) const;

    private:
        friend struct detail_view::item_access;

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
            static item make(span<const uint8_t> bytes, const item_head& head, const uint8_t* content) noexcept
            {
                return item(bytes, head, content);
            }
        };

    } // namespace detail_view

    // ---- Low-level tier ----
    // The wire-level head decoding the checked item layer is built on, for
    // consumers that need sub-item head access. Prefer the checked item layer
    // unless you are walking objects yourself with intention.

    struct item_head
    {
        view::major_type major_type{};
        uint8_t additional_info{0};
        uint64_t value{0};

        bool indefinite() const noexcept
        {
            return additional_info == 31;   // additional-info 31 = indefinite length
        }
    };

    // Offset-based access to unchecked CBOR wire data. The cursor borrows its
    // input and never exposes a mutable pointer/end pair.
    class wire_cursor
    {
    public:

        explicit wire_cursor(span<const uint8_t> input) noexcept
            : begin_(input.data()), current_(input.data()), end_(input.data() + input.size())
        {
        }

        template <typename Container>
        wire_cursor(const Container&&) = delete;

        std::size_t position() const noexcept
        {
            return static_cast<std::size_t>(current_ - begin_);
        }

        span<const uint8_t> remaining() const noexcept
        {
            return span<const uint8_t>(current_, static_cast<std::size_t>(end_ - current_));
        }

        // Reads one head and advances past that head only. Tags are returned
        // as their own heads. On failure, position() is the reported offset.
        expected<item_head, scan_error> read_head() noexcept
        {
            if (current_ >= end_)
            {
                return expected<item_head, scan_error>(unexpect,
                    scan_error{cbor_errc::unexpected_eof, position()});
            }

            detail_view::item_head h;
            std::error_code ec;
            const bool ok = detail_view::read_head(current_, end_, h, ec);
            if (!ok)
            {
                return expected<item_head, scan_error>(unexpect,
                    scan_error{static_cast<cbor_errc>(ec.value()), position()});
            }

            item_head head;
            head.major_type = static_cast<view::major_type>(static_cast<uint8_t>(h.major_type));
            head.additional_info = h.additional_info;
            head.value = h.value;
            return head;
        }

        // Reads and validates one complete item. This is the fallible boundary
        // at which the supplied scanning workspace may grow.
        expected<item, scan_error> read_item(scan_context& context);

        // Validates and passes over one complete item, returning its encoded
        // bytes unparsed. Costs and failure reporting match read_item.
        expected<span<const uint8_t>, scan_error> skip_item(scan_context& context);

        // Advances past `count` bytes of already-measured content, such as a
        // definite string payload after its head. False, leaving the position
        // unchanged, when fewer bytes remain.
        bool skip(std::size_t count) noexcept
        {
            if (static_cast<std::size_t>(end_ - current_) < count)
            {
                return false;
            }
            current_ += count;
            return true;
        }

    private:
        const uint8_t* begin_;
        const uint8_t* current_;
        const uint8_t* end_;
    };

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
        friend class navigator;
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

    using tag_range = item::tag_range;

    class navigator
    {
        static constexpr std::size_t npos = (std::numeric_limits<std::size_t>::max)();

        struct node_state
        {
            std::size_t begin{0};
            std::size_t head_begin{0};
            std::size_t content_begin{0};
            std::size_t end{npos};
            detail_view::item_head head{};
            position_role role{position_role::root};
        };

        struct navigation_frame
        {
            node_state container{};
            uint64_t remaining{0};
        };

    public:
        navigator(navigator&&) noexcept = default;
        navigator& operator=(navigator&&) noexcept = default;
        navigator(const navigator&) = delete;
        navigator& operator=(const navigator&) = delete;

        item_kind kind() const noexcept
        {
            return detail_view::kind(current_.head);
        }

        uint64_t argument() const noexcept
        {
            return current_.head.value;
        }

        bool indefinite() const noexcept
        {
            return current_.head.indefinite();
        }

        tag_range tags() const noexcept
        {
            return tag_range(input_.data() + current_.begin,
                             input_.data() + current_.head_begin);
        }

        position_role role() const noexcept
        {
            return current_.role;
        }

        std::size_t depth() const noexcept
        {
            return active_depth_;
        }

        bool uint64_value(uint64_t& value) const noexcept
        {
            return detail_view::uint64_value(current_.head, value);
        }

        bool int64_value(int64_t& value) const noexcept
        {
            return detail_view::int64_value(current_.head, value);
        }

        bool bool_value(bool& value) const noexcept
        {
            return detail_view::bool_value(current_.head, value);
        }

        bool double_value(double& value) const noexcept
        {
            return detail_view::double_value(current_.head, value);
        }

        bool text(string_view& value) const noexcept
        {
            return detail_view::text(current_.head,
                                     input_.data() + current_.content_begin, value);
        }

        bool bytes(span<const uint8_t>& value) const noexcept
        {
            return detail_view::bytes(current_.head,
                                      input_.data() + current_.content_begin, value);
        }

        bool enter() noexcept;
        bool next() noexcept;
        bool leave() noexcept;

        void rewind() noexcept
        {
            current_ = root_;
            active_depth_ = 0;
        }

        item finish_item() noexcept;

        bool extent_known() const noexcept
        {
            return current_.end != npos;
        }

        expected<span<const uint8_t>, scan_error> reset_prefix(
            span<const uint8_t> input,
            int max_nesting_depth = default_max_nesting_depth);

        expected<span<const uint8_t>, scan_error> reset_exact(
            span<const uint8_t> input,
            int max_nesting_depth = default_max_nesting_depth);

        template <typename Container>
        expected<span<const uint8_t>, scan_error> reset_prefix(
            const Container&&,
            int = default_max_nesting_depth) = delete;

        template <typename Container>
        expected<span<const uint8_t>, scan_error> reset_exact(
            const Container&&,
            int = default_max_nesting_depth) = delete;

    private:
        friend struct detail_view::navigator_access;

        node_state read_node(std::size_t begin, position_role role,
                             std::size_t right_fence = npos) const noexcept;
        void initialize_frame(navigation_frame& frame, const node_state& container) noexcept;
        bool take_child(navigation_frame& frame, std::size_t& offset,
                        position_role& role, std::size_t& right_fence) noexcept;
        void establish_end(node_state& node) noexcept;

        navigator(span<const uint8_t> root, std::size_t peak_depth,
                  detail_view::pending_stack&& validation_workspace)
            : input_(root), frames_(peak_depth),
              validation_workspace_(std::move(validation_workspace)), active_depth_(0)
        {
            initialize_root();
        }

        void initialize_root() noexcept
        {
            root_ = read_node(0, position_role::root, input_.size());
            current_ = root_;
            active_depth_ = 0;
        }

        void prepare_checked(span<const uint8_t> root, std::size_t peak_depth)
        {
            frames_.resize(peak_depth);
            input_ = root;
            initialize_root();
        }

        span<const uint8_t> input_;
        node_state root_;
        node_state current_;
        std::vector<navigation_frame> frames_;
        detail_view::pending_stack validation_workspace_;
        std::size_t active_depth_;
    };

    namespace detail_view {

        struct navigator_access
        {
            static navigator make(span<const uint8_t> root, std::size_t peak_depth,
                                  pending_stack&& validation_workspace)
            {
                return navigator(root, peak_depth, std::move(validation_workspace));
            }
        };

    } // namespace detail_view

    struct navigation_result
    {
        navigator first;
        span<const uint8_t> remainder;
    };


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

            static std::size_t peak_depth(scan_context& context) noexcept
            {
                return context.workspace_.peak_size();
            }
        };

        // All error codes assigned by the walker are cbor_errc values.
        inline cbor_errc to_cbor_errc(const std::error_code& ec) noexcept
        {
            return static_cast<cbor_errc>(ec.value());
        }

    } // namespace detail_view

    // Iterates the raw children of a checked container item, measuring each
    // child once to yield it as a complete checked item.
    class item::child_iterator
    {
    public:
        using value_type = item;
        using reference = item;
        using pointer = void;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::input_iterator_tag;

        child_iterator() noexcept
            : pos_(nullptr), next_(nullptr), end_(nullptr), remaining_(0),
              content_(nullptr), context_(nullptr)
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

        child_iterator(const uint8_t* first, const uint8_t* end, uint64_t remaining,
                       scan_context& context) noexcept
            : pos_(nullptr), next_(first), end_(end), remaining_(remaining),
              content_(nullptr), context_(&context)
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
            std::error_code ec;
            bool ok = detail_view::read_value_head(p, end_, head_, ec);
            assert(ok && !ec);
            content_ = p;
            ok = head_.major_type == cbor::detail::cbor_major_type::array ||
                 head_.major_type == cbor::detail::cbor_major_type::map
                ? detail_view::skip_container(p, end_, head_,
                      (std::numeric_limits<int>::max)(),
                      detail_view::scan_access::workspace(*context_), ec)
                : detail_view::skip_scalar_or_string(head_, p, end_, ec);
            assert(ok && !ec);
            (void)ok;
            next_ = p;
        }

        const uint8_t* pos_;    // current child's begin, nullptr when exhausted
        const uint8_t* next_;   // current child's end, the next child's begin
        const uint8_t* end_;    // the parent's end
        uint64_t remaining_;    // raw children left; indefinite ends at a break
        detail_view::item_head head_;
        const uint8_t* content_;
        scan_context* context_;
    };

    class item::child_range
    {
    public:
        using iterator = child_iterator;
        using const_iterator = child_iterator;

        child_iterator begin() const
        {
            if (first_ == nullptr)
            {
                return child_iterator();
            }
            return child_iterator(first_, end_, remaining_, *context_);
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
        child_range(const uint8_t* first, const uint8_t* end, uint64_t remaining,
                    scan_context* context) noexcept
            : first_(first), end_(end), remaining_(remaining), context_(context)
        {
        }

        const uint8_t* first_;
        const uint8_t* end_;
        uint64_t remaining_;
        scan_context* context_;
    };

    inline item::child_range item::children(scan_context& context) const noexcept
    {
        if (head_.major_type != cbor::detail::cbor_major_type::array &&
            head_.major_type != cbor::detail::cbor_major_type::map)
        {
            return child_range(nullptr, nullptr, 0, &context);
        }
        const uint64_t remaining = head_.indefinite()
            ? detail_view::indefinite_array_marker
            : (head_.major_type == cbor::detail::cbor_major_type::map
                ? 2 * head_.value : head_.value);
        return child_range(content_, bytes_.data() + bytes_.size(), remaining, &context);
    }

    struct scan_result
    {
        item first;
        span<const uint8_t> remainder;
    };

    // Scans the first item in `input`, validating structural well-formedness, and
    // returns it together with the remaining bytes. On failure the error
    // holds the offending code and the offset at which scanning stopped.
    inline expected<scan_result, scan_error> scan_prefix(span<const uint8_t> input, scan_context& context)
    {
        const uint8_t* p = input.data();
        const uint8_t* end = p + input.size();
        std::error_code ec;

        detail_view::pending_stack& workspace = detail_view::scan_access::workspace(context);
        workspace.clear();
        // Parse the head once and keep it, so the item is built below without
        // re-reading its own head.
        detail_view::item_head head;
        if (!detail_view::read_value_head(p, end, head, ec))
        {
            return expected<scan_result, scan_error>(unexpect,
                scan_error{detail_view::to_cbor_errc(ec), static_cast<std::size_t>(p - input.data())});
        }
        const uint8_t* content = p;

        const bool ok = (head.major_type == cbor::detail::cbor_major_type::array ||
                         head.major_type == cbor::detail::cbor_major_type::map)
            ? detail_view::skip_container(p, end, head, context.max_nesting_depth(),
                  workspace, ec)
            : detail_view::skip_scalar_or_string(head, p, end, ec);
        if (!ok)
        {
            return expected<scan_result, scan_error>(unexpect,
                scan_error{detail_view::to_cbor_errc(ec), static_cast<std::size_t>(p - input.data())});
        }
        return scan_result{
            detail_view::item_access::make(
                span<const uint8_t>(input.data(), static_cast<std::size_t>(p - input.data())), head, content),
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

    inline expected<item, scan_error> wire_cursor::read_item(scan_context& context)
    {
        const std::size_t start = position();
        auto scanned = scan_prefix(remaining(), context);
        if (!scanned)
        {
            scan_error error = scanned.error();
            const std::size_t available = static_cast<std::size_t>(end_ - current_);
            current_ += (std::min)(error.offset, available);
            error.offset += start;
            return expected<item, scan_error>(unexpect, error);
        }

        current_ += scanned.value().first.encoded_bytes().size();
        return scanned.value().first;
    }

    inline expected<span<const uint8_t>, scan_error> wire_cursor::skip_item(scan_context& context)
    {
        const uint8_t* const start = current_;
        std::error_code ec;
        const bool ok = detail_view::skip_item(current_, end_, context.max_nesting_depth(),
            detail_view::scan_access::workspace(context), ec);
        if (!ok)
        {
            return expected<span<const uint8_t>, scan_error>(unexpect,
                scan_error{detail_view::to_cbor_errc(ec), position()});
        }
        return span<const uint8_t>(start, static_cast<std::size_t>(current_ - start));
    }


    inline expected<navigation_result, scan_error> navigate_prefix(
        span<const uint8_t> input,
        int max_nesting_depth = default_max_nesting_depth)
    {
        scan_context context(max_nesting_depth);
        auto scanned = scan_prefix(input, context);
        if (!scanned)
        {
            return expected<navigation_result, scan_error>(unexpect, scanned.error());
        }

        navigator result = detail_view::navigator_access::make(
            scanned.value().first.encoded_bytes(),
            detail_view::scan_access::peak_depth(context),
            std::move(detail_view::scan_access::workspace(context)));
        return navigation_result{std::move(result), scanned.value().remainder};
    }

    inline expected<navigator, scan_error> navigate_exact(
        span<const uint8_t> input,
        int max_nesting_depth = default_max_nesting_depth)
    {
        auto navigated = navigate_prefix(input, max_nesting_depth);
        if (!navigated)
        {
            return expected<navigator, scan_error>(unexpect, navigated.error());
        }
        if (!navigated.value().remainder.empty())
        {
            return expected<navigator, scan_error>(unexpect,
                scan_error{cbor_errc::trailing_data,
                    input.size() - navigated.value().remainder.size()});
        }
        return std::move(navigated.value().first);
    }

    inline expected<span<const uint8_t>, scan_error> navigator::reset_prefix(
        span<const uint8_t> input, int max_nesting_depth)
    {
        scan_context context(max_nesting_depth);
        detail_view::scan_access::workspace(context) = std::move(validation_workspace_);
        auto scanned = scan_prefix(input, context);
        const std::size_t peak_depth = detail_view::scan_access::peak_depth(context);
        validation_workspace_ = std::move(detail_view::scan_access::workspace(context));
        if (!scanned)
        {
            return expected<span<const uint8_t>, scan_error>(unexpect, scanned.error());
        }

        prepare_checked(scanned.value().first.encoded_bytes(), peak_depth);
        return scanned.value().remainder;
    }

    inline expected<span<const uint8_t>, scan_error> navigator::reset_exact(
        span<const uint8_t> input, int max_nesting_depth)
    {
        scan_context context(max_nesting_depth);
        detail_view::scan_access::workspace(context) = std::move(validation_workspace_);
        auto scanned = scan_prefix(input, context);
        const std::size_t peak_depth = detail_view::scan_access::peak_depth(context);
        validation_workspace_ = std::move(detail_view::scan_access::workspace(context));
        if (!scanned)
        {
            return expected<span<const uint8_t>, scan_error>(unexpect, scanned.error());
        }
        if (!scanned.value().remainder.empty())
        {
            return expected<span<const uint8_t>, scan_error>(unexpect,
                scan_error{cbor_errc::trailing_data,
                    scanned.value().first.encoded_bytes().size()});
        }

        prepare_checked(scanned.value().first.encoded_bytes(), peak_depth);
        return span<const uint8_t>();
    }

    inline navigator::node_state navigator::read_node(
        std::size_t begin, position_role role, std::size_t right_fence) const noexcept
    {
        const uint8_t* const base = input_.data();
        const uint8_t* p = base + begin;
        const uint8_t* const input_end = base + input_.size();
        detail_view::item_head head;
        std::error_code ec;
        std::size_t head_begin = begin;
        bool ok;
        do
        {
            head_begin = static_cast<std::size_t>(p - base);
            ok = detail_view::read_head(p, input_end, head, ec);
            assert(ok && !ec);
        }
        while (head.major_type == cbor::detail::cbor_major_type::semantic_tag);
        (void)ok;

        node_state node;
        node.begin = begin;
        node.head_begin = head_begin;
        node.content_begin = static_cast<std::size_t>(p - base);
        node.head = head;
        node.role = role;

        switch (head.major_type)
        {
            case cbor::detail::cbor_major_type::unsigned_integer:
            case cbor::detail::cbor_major_type::negative_integer:
            case cbor::detail::cbor_major_type::simple:
                node.end = node.content_begin;
                break;
            case cbor::detail::cbor_major_type::byte_string:
            case cbor::detail::cbor_major_type::text_string:
                if (!head.indefinite())
                {
                    node.end = node.content_begin + static_cast<std::size_t>(head.value);
                }
                else if (right_fence != npos)
                {
                    node.end = right_fence;
                }
                else if (input_[node.content_begin] == 0xff)
                {
                    node.end = node.content_begin + 1;
                }
                break;
            case cbor::detail::cbor_major_type::array:
            case cbor::detail::cbor_major_type::map:
                if (right_fence != npos)
                {
                    node.end = right_fence;
                }
                else if (!head.indefinite() && head.value == 0)
                {
                    node.end = node.content_begin;
                }
                else if (head.indefinite() && input_[node.content_begin] == 0xff)
                {
                    node.end = node.content_begin + 1;
                }
                break;
            default:
                assert(false);
                break;
        }
        return node;
    }

    inline void navigator::initialize_frame(
        navigation_frame& frame, const node_state& container) noexcept
    {
        frame.container = container;
        if (container.head.indefinite())
        {
            frame.remaining = container.head.major_type == cbor::detail::cbor_major_type::map
                ? detail_view::indefinite_map_key_marker
                : detail_view::indefinite_array_marker;
        }
        else
        {
            frame.remaining = container.head.major_type == cbor::detail::cbor_major_type::map
                ? 2 * container.head.value
                : container.head.value;
        }
    }

    inline bool navigator::take_child(
        navigation_frame& frame, std::size_t& offset,
        position_role& role, std::size_t& right_fence) noexcept
    {
        right_fence = npos;
        if (frame.remaining == 0)
        {
            if (frame.container.end != npos)
            {
                assert(offset == frame.container.end);
                offset = frame.container.end;
            }
            else
            {
                frame.container.end = offset;
            }
            return false;
        }

        if (frame.remaining == detail_view::indefinite_array_marker ||
            frame.remaining == detail_view::indefinite_map_key_marker)
        {
            if (input_[offset] == 0xff)
            {
                frame.container.end = ++offset;
                frame.remaining = 0;
                return false;
            }
        }

        if (frame.remaining < detail_view::indefinite_map_value_marker)
        {
            role = frame.container.head.major_type == cbor::detail::cbor_major_type::map
                ? ((frame.remaining & 1) == 0 ? position_role::map_key : position_role::map_value)
                : position_role::array_element;
            --frame.remaining;
            if (frame.remaining == 0 && frame.container.end != npos)
            {
                right_fence = frame.container.end;
            }
        }
        else if (frame.remaining == detail_view::indefinite_array_marker)
        {
            role = position_role::array_element;
        }
        else if (frame.remaining == detail_view::indefinite_map_key_marker)
        {
            role = position_role::map_key;
            frame.remaining = detail_view::indefinite_map_value_marker;
        }
        else
        {
            role = position_role::map_value;
            frame.remaining = detail_view::indefinite_map_key_marker;
        }
        return true;
    }

    inline void navigator::establish_end(node_state& node) noexcept
    {
        if (node.end != npos)
        {
            return;
        }

        const uint8_t* const base = input_.data();
        const uint8_t* p = base + node.content_begin;
        const uint8_t* const end = base + input_.size();
        std::error_code ec;
        const bool is_container = node.head.major_type == cbor::detail::cbor_major_type::array ||
                                  node.head.major_type == cbor::detail::cbor_major_type::map;
        const bool ok = is_container
            ? detail_view::skip_container(p, end, node.head,
                  (std::numeric_limits<int>::max)(), validation_workspace_, ec)
            : detail_view::skip_scalar_or_string(node.head, p, end, ec);
        assert(ok && !ec);
        (void)ok;
        node.end = static_cast<std::size_t>(p - base);
    }

    inline bool navigator::enter() noexcept
    {
        if (current_.head.major_type != cbor::detail::cbor_major_type::array &&
            current_.head.major_type != cbor::detail::cbor_major_type::map)
        {
            return false;
        }
        if ((!current_.head.indefinite() && current_.head.value == 0) ||
            (current_.head.indefinite() && input_[current_.content_begin] == 0xff))
        {
            return false;
        }

        assert(active_depth_ < frames_.size());
        navigation_frame& frame = frames_[active_depth_];
        initialize_frame(frame, current_);
        std::size_t offset = current_.content_begin;
        position_role child_role = position_role::array_element;
        std::size_t right_fence = npos;
        const bool has_child = take_child(frame, offset, child_role, right_fence);
        assert(has_child);
        (void)has_child;
        current_ = read_node(offset, child_role, right_fence);
        ++active_depth_;
        return true;
    }

    inline bool navigator::next() noexcept
    {
        if (active_depth_ == 0)
        {
            return false;
        }

        navigation_frame& frame = frames_[active_depth_ - 1];
        if (frame.remaining == 0)
        {
            if (frame.container.end == npos)
            {
                establish_end(current_);
                frame.container.end = current_.end;
            }
            return false;
        }
        establish_end(current_);
        std::size_t offset = current_.end;
        position_role child_role = position_role::array_element;
        std::size_t right_fence = npos;
        if (!take_child(frame, offset, child_role, right_fence))
        {
            return false;
        }

        current_ = read_node(offset, child_role, right_fence);
        return true;
    }

    inline bool navigator::leave() noexcept
    {
        if (active_depth_ == 0)
        {
            return false;
        }

        navigation_frame& frame = frames_[active_depth_ - 1];
        if (frame.container.end == npos)
        {
            establish_end(current_);
            std::size_t offset = current_.end;
            position_role child_role = position_role::array_element;
            std::size_t right_fence = npos;
            while (take_child(frame, offset, child_role, right_fence))
            {
                node_state unread = read_node(offset, child_role, right_fence);
                establish_end(unread);
                offset = unread.end;
            }
        }

        current_ = frame.container;
        --active_depth_;
        return true;
    }

    inline item navigator::finish_item() noexcept
    {
        establish_end(current_);
        return detail_view::item_access::make(
            span<const uint8_t>(input_.data() + current_.begin,
                                current_.end - current_.begin),
            current_.head, input_.data() + current_.content_begin);
    }

    template <typename Container>
    expected<navigation_result, scan_error> navigate_prefix(
        const Container&&,
        int = default_max_nesting_depth) = delete;

    template <typename Container>
    expected<navigator, scan_error> navigate_exact(
        const Container&&,
        int = default_max_nesting_depth) = delete;


    // Guard against temporary owning containers; the concrete span overloads
    // remain available for explicit non-owning views.
    template <typename Container>
    expected<scan_result, scan_error> scan_prefix(const Container&&, scan_context&) = delete;
    template <typename Container>
    expected<scan_result, scan_error> scan_prefix(const Container&&) = delete;
    template <typename Container>
    expected<item, scan_error> parse_exact(const Container&&, scan_context&) = delete;
    template <typename Container>
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


    // Span-based order entry points for raw-span consumers (e.g. collation over
    // byte-string keys). Each validates its input(s) then orders the encoded
    // bytes; on malformed input the error carries the code and byte offset.
    // Trailing bytes after the first item are tolerated.

    template <typename Order = bytewise_compare>
    expected<int, scan_error> compare(span<const uint8_t> a, span<const uint8_t> b,
        Order order = Order(), int max_nesting_depth = default_max_nesting_depth)
    {
        scan_context context(max_nesting_depth);
        auto ra = scan_prefix(a, context);
        if (!ra)
        {
            return expected<int, scan_error>(unexpect, ra.error());
        }
        auto rb = scan_prefix(b, context);
        if (!rb)
        {
            return expected<int, scan_error>(unexpect, rb.error());
        }
        return order(ra.value().first.encoded_bytes(), rb.value().first.encoded_bytes());
    }

    template <typename Order = bytewise_compare>
    expected<bool, scan_error> map_keys_sorted(span<const uint8_t> input,
        Order order = Order(), int max_nesting_depth = default_max_nesting_depth)
    {
        scan_context context(max_nesting_depth);
        auto scanned = scan_prefix(input, context);
        if (!scanned)
        {
            return expected<bool, scan_error>(unexpect, scanned.error());
        }
        const item map_item = scanned.value().first;
        if (map_item.kind() != item_kind::map)
        {
            return false;
        }

        span<const uint8_t> previous;
        bool is_key = true;
        for (item child : map_item.children(context))
        {
            if (is_key)
            {
                const span<const uint8_t> key = child.encoded_bytes();
                if (!previous.empty() && order(previous, key) >= 0)
                {
                    return false;
                }
                previous = key;
            }
            is_key = !is_key;
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

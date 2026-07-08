// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CBOR_CBOR_VIEW_HPP
#define JSONCONS_EXT_CBOR_CBOR_VIEW_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <system_error>
#include <vector>

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons_ext/cbor/cbor_detail.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>

namespace jsoncons {
namespace cbor {
namespace view {

    constexpr int default_max_nesting_depth = 1024;

    struct item_head
    {
        detail::cbor_major_type major_type{};
        uint8_t additional_info{0};
        uint64_t value{0};

        bool indefinite() const noexcept
        {
            return additional_info == detail::additional_info::indefinite_length;
        }
    };

    struct map_entry_view
    {
        span<const uint8_t> key;
        span<const uint8_t> value;
    };

    namespace detail_view {

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

        inline bool read_uint(const uint8_t*& p, const uint8_t* end, uint8_t info, uint64_t& value, std::error_code& ec)
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

        inline bool read_head(const uint8_t*& p, const uint8_t* end, item_head& head, std::error_code& ec)
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
            return read_uint(p, end, head.additional_info, head.value, ec);
        }

        inline bool skip_item(const uint8_t*& p, const uint8_t* end, int depth, std::error_code& ec);

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

        inline bool skip_container_items(const uint8_t*& p, const uint8_t* end, const item_head& head,
                                         std::size_t items_per_entry, int depth, std::error_code& ec)
        {
            if (head.indefinite())
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
                    for (std::size_t i = 0; i < items_per_entry; ++i)
                    {
                        if (!skip_item(p, end, depth, ec))
                        {
                            return false;
                        }
                    }
                }
            }

            for (uint64_t i = 0; i < head.value; ++i)
            {
                for (std::size_t j = 0; j < items_per_entry; ++j)
                {
                    if (!skip_item(p, end, depth, ec))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        inline bool skip_item(const uint8_t*& p, const uint8_t* end, int depth, std::error_code& ec)
        {
            item_head head;
            do
            {
                if (!read_head(p, end, head, ec))
                {
                    return false;
                }
            }
            while (head.major_type == cbor::detail::cbor_major_type::semantic_tag);

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

                case cbor::detail::cbor_major_type::array:
                case cbor::detail::cbor_major_type::map:
                {
                    if (JSONCONS_UNLIKELY(depth <= 0))
                    {
                        ec = cbor_errc::max_nesting_depth_exceeded;
                        return false;
                    }
                    const std::size_t items_per_entry =
                        head.major_type == cbor::detail::cbor_major_type::map ? 2 : 1;
                    return skip_container_items(p, end, head, items_per_entry, depth - 1, ec);
                }

                default:
                    break;
            }

            ec = cbor_errc::unknown_type;
            return false;
        }

        inline span<const uint8_t> read_item_span(const uint8_t*& p, const uint8_t* end, int depth, std::error_code& ec)
        {
            const uint8_t* first = p;
            if (!skip_item(p, end, depth, ec))
            {
                return span<const uint8_t>();
            }
            return span<const uint8_t>(first, static_cast<std::size_t>(p - first));
        }

        inline bool read_map_entries(span<const uint8_t> item, std::vector<map_entry_view>& entries,
            int depth, std::error_code& ec)
        {
            const uint8_t* p = item.data();
            const uint8_t* end = p + item.size();
            item_head head;
            if (!read_head(p, end, head, ec))
            {
                return false;
            }
            if (head.major_type != cbor::detail::cbor_major_type::map)
            {
                ec = cbor_errc::unknown_type;
                return false;
            }
            if (JSONCONS_UNLIKELY(depth <= 0))
            {
                ec = cbor_errc::max_nesting_depth_exceeded;
                return false;
            }

            if (!head.indefinite())
            {
                if (static_cast<std::size_t>(head.value) != head.value)
                {
                    ec = cbor_errc::number_too_large;
                    return false;
                }
                entries.reserve(static_cast<std::size_t>(
                    (std::min)(head.value, static_cast<uint64_t>(end - p) / 2)));
                for (uint64_t i = 0; i < head.value; ++i)
                {
                    auto key = read_item_span(p, end, depth - 1, ec);
                    if (ec) { return false; }
                    auto value = read_item_span(p, end, depth - 1, ec);
                    if (ec) { return false; }
                    entries.push_back(map_entry_view{key, value});
                }
                return true;
            }

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
                auto key = read_item_span(p, end, depth - 1, ec);
                if (ec) { return false; }
                auto value = read_item_span(p, end, depth - 1, ec);
                if (ec) { return false; }
                entries.push_back(map_entry_view{key, value});
            }
        }

    } // namespace detail_view

    struct bytewise_order
    {
        int operator()(span<const uint8_t> a, span<const uint8_t> b) const noexcept
        {
            return detail_view::compare_bytes(a, b);
        }
    };

    struct length_first_order
    {
        int operator()(span<const uint8_t> a, span<const uint8_t> b) const noexcept
        {
            const int r = detail_view::compare_size(a.size(), b.size());
            return r != 0 ? r : detail_view::compare_bytes(a, b);
        }
    };

    inline bool skip_item(const uint8_t*& p, const uint8_t* end, std::error_code& ec,
        int max_nesting_depth = default_max_nesting_depth)
    {
        return detail_view::skip_item(p, end, max_nesting_depth, ec);
    }

    inline span<const uint8_t> item_span(span<const uint8_t> input, std::error_code& ec,
        int max_nesting_depth = default_max_nesting_depth)
    {
        const uint8_t* p = input.data();
        return detail_view::read_item_span(p, p + input.size(), max_nesting_depth, ec);
    }

    inline bool map_entries(span<const uint8_t> input, std::vector<map_entry_view>& entries, std::error_code& ec,
        int max_nesting_depth = default_max_nesting_depth)
    {
        entries.clear();
        return detail_view::read_map_entries(input, entries, max_nesting_depth, ec);
    }

    template <typename Order = bytewise_order>
    int compare(span<const uint8_t> a, span<const uint8_t> b, std::error_code& ec,
        Order order = Order(), int max_nesting_depth = default_max_nesting_depth)
    {
        const uint8_t* ap = a.data();
        auto ia = detail_view::read_item_span(ap, ap + a.size(), max_nesting_depth, ec);
        if (ec)
        {
            return 0;
        }
        const uint8_t* bp = b.data();
        auto ib = detail_view::read_item_span(bp, bp + b.size(), max_nesting_depth, ec);
        if (ec)
        {
            return 0;
        }
        return order(ia, ib);
    }

    template <typename Order = bytewise_order>
    int compare(span<const uint8_t> a, span<const uint8_t> b)
    {
        std::error_code ec;
        int result = compare(a, b, ec, Order());
        if (JSONCONS_UNLIKELY(ec))
        {
            JSONCONS_THROW(ser_error(ec));
        }
        return result;
    }

    template <typename Order = bytewise_order>
    bool map_keys_sorted(span<const uint8_t> input, std::error_code& ec,
        Order order = Order(), int max_nesting_depth = default_max_nesting_depth)
    {
        std::vector<map_entry_view> entries;
        if (!detail_view::read_map_entries(input, entries, max_nesting_depth, ec))
        {
            return false;
        }
        for (std::size_t i = 1; i < entries.size(); ++i)
        {
            if (order(entries[i-1].key, entries[i].key) >= 0)
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

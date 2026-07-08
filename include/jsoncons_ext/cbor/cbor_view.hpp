// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_CBOR_CBOR_VIEW_HPP
#define JSONCONS_EXT_CBOR_CBOR_VIEW_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <system_error>
#include <utility>
#include <vector>

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/utility/binary.hpp>
#include <jsoncons_ext/cbor/cbor_detail.hpp>
#include <jsoncons_ext/cbor/cbor_error.hpp>

namespace jsoncons {
namespace cbor {
namespace view {

    enum class type_rank : uint8_t
    {
        null_value = 0,
        false_value = 1,
        true_value = 2,
        undefined_value = 3,
        simple_value = 4,
        number_value = 5,
        text_string = 6,
        byte_string = 7,
        array = 8,
        map = 9,
        tag = 10
    };

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

        inline int compare_uint64(uint64_t a, uint64_t b) noexcept
        {
            return a < b ? -1 : (a > b ? 1 : 0);
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

        inline bool skip_item(const uint8_t*& p, const uint8_t* end, std::error_code& ec);

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

        inline bool skip_array_items(const uint8_t*& p, const uint8_t* end, const item_head& head, std::error_code& ec)
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
                    if (!skip_item(p, end, ec))
                    {
                        return false;
                    }
                }
            }

            for (uint64_t i = 0; i < head.value; ++i)
            {
                if (!skip_item(p, end, ec))
                {
                    return false;
                }
            }
            return true;
        }

        inline bool skip_map_items(const uint8_t*& p, const uint8_t* end, const item_head& head, std::error_code& ec)
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
                    if (!skip_item(p, end, ec) || !skip_item(p, end, ec))
                    {
                        return false;
                    }
                }
            }

            for (uint64_t i = 0; i < head.value; ++i)
            {
                if (!skip_item(p, end, ec) || !skip_item(p, end, ec))
                {
                    return false;
                }
            }
            return true;
        }

        inline bool skip_item(const uint8_t*& p, const uint8_t* end, std::error_code& ec)
        {
            item_head head;
            if (!read_head(p, end, head, ec))
            {
                return false;
            }

            switch (head.major_type)
            {
                case cbor::detail::cbor_major_type::unsigned_integer:
                case cbor::detail::cbor_major_type::negative_integer:
                    return true;

                case cbor::detail::cbor_major_type::simple:
                    if (head.indefinite())
                    {
                        ec = cbor_errc::unknown_type;
                        return false;
                    }
                    return !head.indefinite();

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
                    return skip_array_items(p, end, head, ec);

                case cbor::detail::cbor_major_type::map:
                    return skip_map_items(p, end, head, ec);

                case cbor::detail::cbor_major_type::semantic_tag:
                    return skip_item(p, end, ec);
            }

            ec = cbor_errc::unknown_type;
            return false;
        }

        inline span<const uint8_t> read_item_span(const uint8_t*& p, const uint8_t* end, std::error_code& ec)
        {
            const uint8_t* first = p;
            if (!skip_item(p, end, ec))
            {
                return span<const uint8_t>();
            }
            return span<const uint8_t>(first, static_cast<std::size_t>(p - first));
        }

        inline type_rank rank_for(const item_head& head) noexcept
        {
            switch (head.major_type)
            {
                case cbor::detail::cbor_major_type::unsigned_integer:
                case cbor::detail::cbor_major_type::negative_integer:
                    return type_rank::number_value;
                case cbor::detail::cbor_major_type::byte_string:
                    return type_rank::byte_string;
                case cbor::detail::cbor_major_type::text_string:
                    return type_rank::text_string;
                case cbor::detail::cbor_major_type::array:
                    return type_rank::array;
                case cbor::detail::cbor_major_type::map:
                    return type_rank::map;
                case cbor::detail::cbor_major_type::semantic_tag:
                    return type_rank::tag;
                case cbor::detail::cbor_major_type::simple:
                {
                    const uint64_t simple = head.additional_info == 24 ? head.value : head.additional_info;
                    switch (simple)
                    {
                        case 20: return type_rank::false_value;
                        case 21: return type_rank::true_value;
                        case 22: return type_rank::null_value;
                        case 23: return type_rank::undefined_value;
                        default:
                            return (head.additional_info == 25 || head.additional_info == 26 || head.additional_info == 27)
                                ? type_rank::number_value
                                : type_rank::simple_value;
                    }
                }
            }
            return type_rank::simple_value;
        }

        inline bool parse_number(span<const uint8_t> item, long double& value, std::error_code& ec)
        {
            const uint8_t* p = item.data();
            const uint8_t* end = p + item.size();
            item_head head;
            if (!read_head(p, end, head, ec))
            {
                return false;
            }

            switch (head.major_type)
            {
                case cbor::detail::cbor_major_type::unsigned_integer:
                    value = static_cast<long double>(head.value);
                    return true;

                case cbor::detail::cbor_major_type::negative_integer:
                    value = -1.0L - static_cast<long double>(head.value);
                    return true;

                case cbor::detail::cbor_major_type::simple:
                    switch (head.additional_info)
                    {
                        case 25:
                            value = static_cast<long double>(binary::decode_half(static_cast<uint16_t>(head.value)));
                            return true;
                        case 26:
                        {
                            uint32_t bits = static_cast<uint32_t>(head.value);
                            float f;
                            std::memcpy(&f, &bits, sizeof(f));
                            value = static_cast<long double>(f);
                            return true;
                        }
                        case 27:
                        {
                            uint64_t bits = head.value;
                            double d;
                            std::memcpy(&d, &bits, sizeof(d));
                            value = static_cast<long double>(d);
                            return true;
                        }
                        default:
                            break;
                    }
                    break;

                default:
                    break;
            }

            ec = cbor_errc::unknown_type;
            return false;
        }

        inline int compare_numbers(span<const uint8_t> a, span<const uint8_t> b, std::error_code& ec)
        {
            long double av = 0;
            long double bv = 0;
            if (!parse_number(a, av, ec) || !parse_number(b, bv, ec))
            {
                return 0;
            }

            const bool anan = std::isnan(av);
            const bool bnan = std::isnan(bv);
            if (anan || bnan)
            {
                return anan == bnan ? 0 : (anan ? -1 : 1);
            }
            if (av == 0.0L && bv == 0.0L)
            {
                return 0;
            }
            return av < bv ? -1 : (av > bv ? 1 : 0);
        }

        class string_chunk_reader
        {
            cbor::detail::cbor_major_type major_type_{};
            const uint8_t* p_{nullptr};
            const uint8_t* end_{nullptr};
            span<const uint8_t> payload_;
            bool indefinite_{false};
            bool done_{false};

        public:
            string_chunk_reader(span<const uint8_t> item, std::error_code& ec)
            {
                p_ = item.data();
                end_ = p_ + item.size();
                item_head head;
                if (!read_head(p_, end_, head, ec))
                {
                    done_ = true;
                    return;
                }
                major_type_ = head.major_type;
                if (major_type_ != cbor::detail::cbor_major_type::byte_string &&
                    major_type_ != cbor::detail::cbor_major_type::text_string)
                {
                    ec = cbor_errc::unknown_type;
                    done_ = true;
                    return;
                }
                indefinite_ = head.indefinite();
                if (!indefinite_)
                {
                    if (static_cast<uint64_t>(end_ - p_) < head.value)
                    {
                        ec = cbor_errc::unexpected_eof;
                        done_ = true;
                        return;
                    }
                    payload_ = span<const uint8_t>(p_, static_cast<std::size_t>(head.value));
                    p_ += static_cast<std::size_t>(head.value);
                }
            }

            bool next(span<const uint8_t>& chunk, std::error_code& ec)
            {
                chunk = span<const uint8_t>();
                if (done_)
                {
                    return false;
                }
                if (!indefinite_)
                {
                    done_ = true;
                    chunk = payload_;
                    return true;
                }

                for (;;)
                {
                    if (p_ >= end_)
                    {
                        ec = cbor_errc::unexpected_eof;
                        done_ = true;
                        return false;
                    }
                    if (*p_ == 0xff)
                    {
                        ++p_;
                        done_ = true;
                        return false;
                    }

                    item_head head;
                    if (!read_head(p_, end_, head, ec))
                    {
                        done_ = true;
                        return false;
                    }
                    if (head.major_type != major_type_ || head.indefinite())
                    {
                        ec = cbor_errc::illegal_chunked_string;
                        done_ = true;
                        return false;
                    }
                    if (static_cast<uint64_t>(end_ - p_) < head.value)
                    {
                        ec = cbor_errc::unexpected_eof;
                        done_ = true;
                        return false;
                    }
                    chunk = span<const uint8_t>(p_, static_cast<std::size_t>(head.value));
                    p_ += static_cast<std::size_t>(head.value);
                    return true;
                }
            }
        };

        inline int compare_string_items(span<const uint8_t> a, span<const uint8_t> b, std::error_code& ec)
        {
            string_chunk_reader ar(a, ec);
            if (ec) { return 0; }
            string_chunk_reader br(b, ec);
            if (ec) { return 0; }

            span<const uint8_t> ac;
            span<const uint8_t> bc;
            std::size_t ai = 0;
            std::size_t bi = 0;
            bool ahas = false;
            bool bhas = false;

            for (;;)
            {
                while (!ahas || ai == ac.size())
                {
                    ahas = ar.next(ac, ec);
                    if (ec) { return 0; }
                    ai = 0;
                    if (!ahas || ac.size() != 0)
                    {
                        break;
                    }
                }
                while (!bhas || bi == bc.size())
                {
                    bhas = br.next(bc, ec);
                    if (ec) { return 0; }
                    bi = 0;
                    if (!bhas || bc.size() != 0)
                    {
                        break;
                    }
                }

                if (!ahas || !bhas)
                {
                    return ahas == bhas ? 0 : (ahas ? 1 : -1);
                }

                const std::size_t n = (std::min)(ac.size() - ai, bc.size() - bi);
                if (n != 0)
                {
                    int r = std::memcmp(ac.data() + ai, bc.data() + bi, n);
                    if (r != 0)
                    {
                        return sign(r);
                    }
                    ai += n;
                    bi += n;
                }
            }
        }

        inline int compare_items(span<const uint8_t> a, span<const uint8_t> b, std::error_code& ec);

        inline int compare_arrays(span<const uint8_t> a, span<const uint8_t> b, std::error_code& ec)
        {
            const uint8_t* ap = a.data();
            const uint8_t* bp = b.data();
            const uint8_t* aend = ap + a.size();
            const uint8_t* bend = bp + b.size();
            item_head ah;
            item_head bh;
            if (!read_head(ap, aend, ah, ec) || !read_head(bp, bend, bh, ec))
            {
                return 0;
            }

            uint64_t ai = 0;
            uint64_t bi = 0;
            for (;;)
            {
                if (ah.indefinite() && ap >= aend)
                {
                    ec = cbor_errc::unexpected_eof;
                    return 0;
                }
                if (bh.indefinite() && bp >= bend)
                {
                    ec = cbor_errc::unexpected_eof;
                    return 0;
                }
                const bool adone = ah.indefinite() ? (*ap == 0xff) : (ai == ah.value);
                const bool bdone = bh.indefinite() ? (*bp == 0xff) : (bi == bh.value);
                if (adone || bdone)
                {
                    return adone == bdone ? 0 : (adone ? -1 : 1);
                }

                auto av = read_item_span(ap, aend, ec);
                if (ec) { return 0; }
                auto bv = read_item_span(bp, bend, ec);
                if (ec) { return 0; }
                ++ai;
                ++bi;

                int c = compare_items(av, bv, ec);
                if (ec || c != 0)
                {
                    return c;
                }
            }
        }

        inline bool read_map_entries(span<const uint8_t> item, std::vector<map_entry_view>& entries, std::error_code& ec)
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

            if (!head.indefinite())
            {
                if (static_cast<std::size_t>(head.value) != head.value)
                {
                    ec = cbor_errc::number_too_large;
                    return false;
                }
                entries.reserve(static_cast<std::size_t>(head.value));
                for (uint64_t i = 0; i < head.value; ++i)
                {
                    auto key = read_item_span(p, end, ec);
                    if (ec) { return false; }
                    auto value = read_item_span(p, end, ec);
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
                auto key = read_item_span(p, end, ec);
                if (ec) { return false; }
                auto value = read_item_span(p, end, ec);
                if (ec) { return false; }
                entries.push_back(map_entry_view{key, value});
            }
        }

        inline int compare_maps(span<const uint8_t> a, span<const uint8_t> b, std::error_code& ec)
        {
            std::vector<map_entry_view> ae;
            std::vector<map_entry_view> be;
            if (!read_map_entries(a, ae, ec) || !read_map_entries(b, be, ec))
            {
                return 0;
            }

            auto less = [&ec](const map_entry_view& x, const map_entry_view& y)
            {
                int ck = compare_items(x.key, y.key, ec);
                if (ec || ck != 0)
                {
                    return ck < 0;
                }
                return compare_items(x.value, y.value, ec) < 0;
            };
            std::sort(ae.begin(), ae.end(), less);
            if (ec) { return 0; }
            std::sort(be.begin(), be.end(), less);
            if (ec) { return 0; }

            const std::size_t n = (std::min)(ae.size(), be.size());
            for (std::size_t i = 0; i < n; ++i)
            {
                int ck = compare_items(ae[i].key, be[i].key, ec);
                if (ec || ck != 0)
                {
                    return ck;
                }
                int cv = compare_items(ae[i].value, be[i].value, ec);
                if (ec || cv != 0)
                {
                    return cv;
                }
            }
            return compare_size(ae.size(), be.size());
        }

        inline int compare_tags(span<const uint8_t> a, span<const uint8_t> b, std::error_code& ec)
        {
            const uint8_t* ap = a.data();
            const uint8_t* bp = b.data();
            const uint8_t* aend = ap + a.size();
            const uint8_t* bend = bp + b.size();
            item_head ah;
            item_head bh;
            if (!read_head(ap, aend, ah, ec) || !read_head(bp, bend, bh, ec))
            {
                return 0;
            }
            int tag_compare = compare_uint64(ah.value, bh.value);
            if (tag_compare != 0)
            {
                return tag_compare;
            }
            auto av = read_item_span(ap, aend, ec);
            if (ec) { return 0; }
            auto bv = read_item_span(bp, bend, ec);
            if (ec) { return 0; }
            return compare_items(av, bv, ec);
        }

        inline int compare_items(span<const uint8_t> a, span<const uint8_t> b, std::error_code& ec)
        {
            if (a.size() == b.size() && compare_bytes(a, b) == 0)
            {
                return 0;
            }

            const uint8_t* ap = a.data();
            const uint8_t* bp = b.data();
            item_head ah;
            item_head bh;
            if (!read_head(ap, ap + a.size(), ah, ec) || !read_head(bp, bp + b.size(), bh, ec))
            {
                return 0;
            }

            const type_rank ar = rank_for(ah);
            const type_rank br = rank_for(bh);
            if (ar != br)
            {
                return static_cast<uint8_t>(ar) < static_cast<uint8_t>(br) ? -1 : 1;
            }

            switch (ar)
            {
                case type_rank::null_value:
                case type_rank::false_value:
                case type_rank::true_value:
                case type_rank::undefined_value:
                    return 0;

                case type_rank::simple_value:
                    return compare_uint64(ah.additional_info == 24 ? ah.value : ah.additional_info,
                                          bh.additional_info == 24 ? bh.value : bh.additional_info);

                case type_rank::number_value:
                    return compare_numbers(a, b, ec);

                case type_rank::text_string:
                case type_rank::byte_string:
                    return compare_string_items(a, b, ec);

                case type_rank::array:
                    return compare_arrays(a, b, ec);

                case type_rank::map:
                    return compare_maps(a, b, ec);

                case type_rank::tag:
                    return compare_tags(a, b, ec);
            }
            return 0;
        }

    } // namespace detail_view

    inline bool skip_item(const uint8_t*& p, const uint8_t* end, std::error_code& ec)
    {
        return detail_view::skip_item(p, end, ec);
    }

    inline span<const uint8_t> item_span(span<const uint8_t> input, std::error_code& ec)
    {
        const uint8_t* p = input.data();
        return detail_view::read_item_span(p, p + input.size(), ec);
    }

    inline bool map_entries(span<const uint8_t> input, std::vector<map_entry_view>& entries, std::error_code& ec)
    {
        entries.clear();
        return detail_view::read_map_entries(input, entries, ec);
    }

    inline int compare(span<const uint8_t> a, span<const uint8_t> b, std::error_code& ec)
    {
        return detail_view::compare_items(a, b, ec);
    }

    inline int compare(span<const uint8_t> a, span<const uint8_t> b)
    {
        std::error_code ec;
        int result = compare(a, b, ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            JSONCONS_THROW(ser_error(ec));
        }
        return result;
    }

} // namespace view
} // namespace cbor
} // namespace jsoncons

#endif

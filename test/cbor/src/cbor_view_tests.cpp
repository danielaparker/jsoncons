// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif

#include <jsoncons_ext/cbor/cbor_view.hpp>

#include <system_error>
#include <vector>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("cbor raw view utilities")
{
    SECTION("item span reads one item")
    {
        std::vector<uint8_t> data = {0x01,0x02};
        std::error_code ec;
        auto item = cbor::view::item_span(jsoncons::span<const uint8_t>(data), ec);
        REQUIRE_FALSE(ec);
        REQUIRE(item.size() == 1);
        CHECK(item[0] == 0x01);
    }

    SECTION("trailing bytes after the first item are ignored")
    {
        std::vector<uint8_t> a = {0x01,0x63,'a','b','c'};
        std::vector<uint8_t> b = {0x01};
        std::error_code ec;
        CHECK(cbor::view::compare(jsoncons::span<const uint8_t>(a), jsoncons::span<const uint8_t>(b), ec) == 0);
        CHECK_FALSE(ec);
    }

    SECTION("nesting at the default depth bound succeeds")
    {
        std::vector<uint8_t> data(cbor::view::default_max_nesting_depth, 0x81);
        data.push_back(0x01);
        std::error_code ec;
        auto item = cbor::view::item_span(jsoncons::span<const uint8_t>(data), ec);
        REQUIRE_FALSE(ec);
        CHECK(item.size() == data.size());
    }

    SECTION("nesting past the depth bound is rejected")
    {
        std::vector<uint8_t> data(cbor::view::default_max_nesting_depth + 1, 0x81);
        data.push_back(0x01);
        std::error_code ec;
        auto item = cbor::view::item_span(jsoncons::span<const uint8_t>(data), ec);
        CHECK(item.empty());
        CHECK(ec == cbor::cbor_errc::max_nesting_depth_exceeded);

        ec.clear();
        item = cbor::view::item_span(jsoncons::span<const uint8_t>(data), ec, 4000);
        REQUIRE_FALSE(ec);
        CHECK(item.size() == data.size());
    }

    SECTION("tag chains do not consume nesting depth or stack")
    {
        std::vector<uint8_t> data(1000000, 0xc0);
        data.push_back(0x01);
        std::error_code ec;
        auto item = cbor::view::item_span(jsoncons::span<const uint8_t>(data), ec);
        REQUIRE_FALSE(ec);
        CHECK(item.size() == data.size());
    }

    SECTION("map claiming a huge entry count is rejected")
    {
        std::vector<uint8_t> data = {0xba,0xff,0xff,0xff,0xff};
        std::vector<cbor::view::map_entry_view> entries;
        std::error_code ec;
        CHECK_FALSE(cbor::view::map_entries(jsoncons::span<const uint8_t>(data), entries, ec));
        CHECK(ec == cbor::cbor_errc::unexpected_eof);
    }

    SECTION("array claiming a huge element count is rejected")
    {
        std::vector<uint8_t> data = {0x9a,0xff,0xff,0xff,0xff};
        std::error_code ec;
        auto item = cbor::view::item_span(jsoncons::span<const uint8_t>(data), ec);
        CHECK(item.empty());
        CHECK(ec == cbor::cbor_errc::unexpected_eof);
    }

    SECTION("definite and indefinite nesting can mix")
    {
        // [_ {1: [2]}, [], {} ] followed by trailing bytes
        std::vector<uint8_t> data = {0x9f,0xa1,0x01,0x81,0x02,0x80,0xa0,0xff,0x63,'a','b','c'};
        std::error_code ec;
        auto item = cbor::view::item_span(jsoncons::span<const uint8_t>(data), ec);
        REQUIRE_FALSE(ec);
        CHECK(item.size() == 8);
    }

    SECTION("indefinite map break may not split a key from its value")
    {
        std::vector<uint8_t> dangling_key = {0xbf,0x01,0xff};
        std::error_code ec;
        auto item = cbor::view::item_span(jsoncons::span<const uint8_t>(dangling_key), ec);
        CHECK(item.empty());
        CHECK(ec == cbor::cbor_errc::unknown_type);

        std::vector<uint8_t> complete_entry = {0xbf,0x01,0x02,0xff};
        ec.clear();
        item = cbor::view::item_span(jsoncons::span<const uint8_t>(complete_entry), ec);
        REQUIRE_FALSE(ec);
        CHECK(item.size() == complete_entry.size());
    }

    SECTION("map entries expose key and value spans")
    {
        std::vector<uint8_t> data = {0xa2,0x01,0x61,0x61,0x61,0x6b,0x02};
        std::vector<cbor::view::map_entry_view> entries;
        std::error_code ec;
        REQUIRE(cbor::view::map_entries(jsoncons::span<const uint8_t>(data), entries, ec));
        REQUIRE_FALSE(ec);
        REQUIRE(entries.size() == 2);
        CHECK(entries[0].key.data() == data.data() + 1);
        CHECK(entries[0].key.size() == 1);
        CHECK(entries[0].value.data() == data.data() + 2);
        CHECK(entries[0].value.size() == 2);
        CHECK(entries[1].key.data() == data.data() + 4);
        CHECK(entries[1].key.size() == 2);
        CHECK(entries[1].value.data() == data.data() + 6);
        CHECK(entries[1].value.size() == 1);
    }

    SECTION("invalid indefinite integer is rejected")
    {
        std::vector<uint8_t> data = {0x1f};
        std::error_code ec;
        auto item = cbor::view::item_span(jsoncons::span<const uint8_t>(data), ec);
        CHECK(item.empty());
        CHECK(ec == cbor::cbor_errc::unknown_type);
    }
}

TEST_CASE("cbor view items compare in deterministic encoding key orders")
{
    const std::vector<std::vector<uint8_t>> bytewise_sorted = {
        {0x0a},
        {0x18,0x64},
        {0x20},
        {0x61,0x7a},
        {0x62,0x61,0x61},
        {0x81,0x18,0x64},
        {0x81,0x20},
        {0xf4}
    };

    const std::vector<std::vector<uint8_t>> length_first_sorted = {
        {0x0a},
        {0x20},
        {0xf4},
        {0x18,0x64},
        {0x61,0x7a},
        {0x81,0x20},
        {0x62,0x61,0x61},
        {0x81,0x18,0x64}
    };

    SECTION("bytewise order matches the RFC 8949 4.2.1 example")
    {
        for (std::size_t i = 0; i < bytewise_sorted.size(); ++i)
        {
            for (std::size_t j = 0; j < bytewise_sorted.size(); ++j)
            {
                std::error_code ec;
                const int r = cbor::view::compare(jsoncons::span<const uint8_t>(bytewise_sorted[i]),
                    jsoncons::span<const uint8_t>(bytewise_sorted[j]), ec);
                REQUIRE_FALSE(ec);
                CHECK((i < j ? r < 0 : (i > j ? r > 0 : r == 0)));
            }
        }
    }

    SECTION("length-first order matches the RFC 8949 4.2.3 example")
    {
        for (std::size_t i = 0; i < length_first_sorted.size(); ++i)
        {
            for (std::size_t j = 0; j < length_first_sorted.size(); ++j)
            {
                std::error_code ec;
                const int r = cbor::view::compare(jsoncons::span<const uint8_t>(length_first_sorted[i]),
                    jsoncons::span<const uint8_t>(length_first_sorted[j]), ec, cbor::view::length_first_order());
                REQUIRE_FALSE(ec);
                CHECK((i < j ? r < 0 : (i > j ? r > 0 : r == 0)));
            }
        }
    }

    SECTION("other orderings can be plugged in")
    {
        struct reverse_bytewise_order
        {
            int operator()(jsoncons::span<const uint8_t> a, jsoncons::span<const uint8_t> b) const noexcept
            {
                return -cbor::view::bytewise_order()(a, b);
            }
        };

        std::vector<uint8_t> ten = {0x0a};
        std::vector<uint8_t> hundred = {0x18,0x64};
        std::error_code ec;
        CHECK(cbor::view::compare(jsoncons::span<const uint8_t>(ten), jsoncons::span<const uint8_t>(hundred), ec) < 0);
        REQUIRE_FALSE(ec);
        CHECK(cbor::view::compare(jsoncons::span<const uint8_t>(ten), jsoncons::span<const uint8_t>(hundred), ec, reverse_bytewise_order()) > 0);
        REQUIRE_FALSE(ec);
    }
}

TEST_CASE("cbor view map_keys_sorted")
{
    SECTION("detects deterministically ordered keys")
    {
        std::vector<uint8_t> sorted_map = {0xa2,0x01,0x61,0x61,0x61,0x6b,0x02};
        std::vector<uint8_t> unsorted_map = {0xa2,0x61,0x6b,0x02,0x01,0x61,0x61};
        std::error_code ec;
        CHECK(cbor::view::map_keys_sorted(jsoncons::span<const uint8_t>(sorted_map), ec));
        CHECK_FALSE(ec);
        CHECK_FALSE(cbor::view::map_keys_sorted(jsoncons::span<const uint8_t>(unsorted_map), ec));
        CHECK_FALSE(ec);
    }

    SECTION("duplicate keys are not sorted")
    {
        std::vector<uint8_t> dup_map = {0xa2,0x01,0x00,0x01,0x01};
        std::error_code ec;
        CHECK_FALSE(cbor::view::map_keys_sorted(jsoncons::span<const uint8_t>(dup_map), ec));
        CHECK_FALSE(ec);
    }

    SECTION("bytewise and length-first orders can disagree")
    {
        std::vector<uint8_t> m = {0xa2,0x18,0x64,0x00,0x20,0x00};
        std::error_code ec;
        CHECK(cbor::view::map_keys_sorted(jsoncons::span<const uint8_t>(m), ec));
        CHECK_FALSE(ec);
        CHECK_FALSE(cbor::view::map_keys_sorted(jsoncons::span<const uint8_t>(m), ec, cbor::view::length_first_order()));
        CHECK_FALSE(ec);
    }
}

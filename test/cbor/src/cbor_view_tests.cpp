// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif

#include <jsoncons_ext/cbor/cbor_view.hpp>

#include <algorithm>
#include <limits>
#include <string>
#include <vector>
#include <catch/catch.hpp>

using namespace jsoncons;

namespace {

    cbor::view::item parse(const std::vector<uint8_t>& data)
    {
        auto result = cbor::view::parse_exact(jsoncons::span<const uint8_t>(data.data(), data.size()));
        REQUIRE(result.has_value());
        return result.value();
    }

} // namespace

TEST_CASE("cbor view scan_prefix and parse_exact")
{
    SECTION("scan_prefix returns the first item and the remainder")
    {
        std::vector<uint8_t> data = {0x01,0x02};
        auto result = cbor::view::scan_prefix(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        CHECK(result.value().first.encoded_bytes().data() == data.data());
        CHECK(result.value().first.encoded_bytes().size() == 1);
        CHECK(result.value().remainder.data() == data.data() + 1);
        CHECK(result.value().remainder.size() == 1);
    }

    SECTION("scan_prefix walks a sequence of items")
    {
        std::vector<uint8_t> data = {0x01,0x61,'a',0x80};
        jsoncons::span<const uint8_t> rest(data.data(), data.size());
        std::size_t count = 0;
        while (!rest.empty())
        {
            auto result = cbor::view::scan_prefix(rest);
            REQUIRE(result.has_value());
            rest = result.value().remainder;
            ++count;
        }
        CHECK(count == 3);
    }

    SECTION("parse_exact rejects trailing bytes with their offset")
    {
        std::vector<uint8_t> data = {0x01,0x02};
        auto result = cbor::view::parse_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::trailing_data);
        CHECK(result.error().offset == 1);

        std::vector<uint8_t> exact = {0x01};
        CHECK(cbor::view::parse_exact(jsoncons::span<const uint8_t>(exact)).has_value());
    }

    SECTION("empty input fails at offset zero")
    {
        auto result = cbor::view::scan_prefix(jsoncons::span<const uint8_t>());
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::unexpected_eof);
        CHECK(result.error().offset == 0);
    }

    SECTION("errors carry the offset where scanning stopped")
    {
        // [1, <invalid head 0x1f>]
        std::vector<uint8_t> data = {0x82,0x01,0x1f};
        auto result = cbor::view::scan_prefix(jsoncons::span<const uint8_t>(data));
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::unknown_type);
        CHECK(result.error().offset == 3);
    }

    SECTION("a scan_context is reusable across scans")
    {
        cbor::view::scan_context context(4000);
        std::vector<uint8_t> deep(2000, 0x81);
        deep.push_back(0x01);
        std::vector<uint8_t> shallow = {0x01};

        CHECK(cbor::view::parse_exact(jsoncons::span<const uint8_t>(deep), context).has_value());
        CHECK(cbor::view::parse_exact(jsoncons::span<const uint8_t>(shallow), context).has_value());
        CHECK(cbor::view::parse_exact(jsoncons::span<const uint8_t>(deep), context).has_value());
    }
}

TEST_CASE("cbor view scanning validates well-formedness")
{
    SECTION("nesting at the default depth bound succeeds")
    {
        std::vector<uint8_t> data(cbor::view::default_max_nesting_depth, 0x81);
        data.push_back(0x01);
        auto result = cbor::view::parse_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        CHECK(result.value().encoded_bytes().size() == data.size());
    }

    SECTION("nesting past the depth bound is rejected")
    {
        std::vector<uint8_t> data(cbor::view::default_max_nesting_depth + 1, 0x81);
        data.push_back(0x01);
        auto result = cbor::view::parse_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::max_nesting_depth_exceeded);

        cbor::view::scan_context deeper(4000);
        CHECK(cbor::view::parse_exact(jsoncons::span<const uint8_t>(data), deeper).has_value());
    }

    SECTION("tag chains do not consume nesting depth or stack")
    {
        std::vector<uint8_t> data(1000000, 0xc0);
        data.push_back(0x01);
        auto result = cbor::view::parse_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());

        std::size_t count = 0;
        for (uint64_t tag : result.value().tags())
        {
            CHECK(tag == 0);
            ++count;
        }
        CHECK(count == 1000000);
    }

    SECTION("containers claiming huge counts are rejected")
    {
        std::vector<uint8_t> array = {0x9a,0xff,0xff,0xff,0xff};
        auto array_result = cbor::view::scan_prefix(jsoncons::span<const uint8_t>(array));
        REQUIRE_FALSE(array_result.has_value());
        CHECK(array_result.error().code == cbor::cbor_errc::unexpected_eof);

        std::vector<uint8_t> map = {0xba,0xff,0xff,0xff,0xff};
        auto map_result = cbor::view::scan_prefix(jsoncons::span<const uint8_t>(map));
        REQUIRE_FALSE(map_result.has_value());
        CHECK(map_result.error().code == cbor::cbor_errc::unexpected_eof);
    }

    SECTION("definite and indefinite nesting can mix")
    {
        // [_ {1: [2]}, [], {} ] followed by trailing bytes
        std::vector<uint8_t> data = {0x9f,0xa1,0x01,0x81,0x02,0x80,0xa0,0xff,0x63,'a','b','c'};
        auto result = cbor::view::scan_prefix(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        CHECK(result.value().first.encoded_bytes().size() == 8);
        CHECK(result.value().remainder.size() == 4);
    }

    SECTION("an indefinite map break may not split a key from its value")
    {
        std::vector<uint8_t> dangling_key = {0xbf,0x01,0xff};
        auto result = cbor::view::scan_prefix(jsoncons::span<const uint8_t>(dangling_key));
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::unknown_type);

        std::vector<uint8_t> complete_entry = {0xbf,0x01,0x02,0xff};
        CHECK(cbor::view::parse_exact(jsoncons::span<const uint8_t>(complete_entry)).has_value());
    }

    SECTION("an invalid indefinite integer is rejected")
    {
        std::vector<uint8_t> data = {0x1f};
        auto result = cbor::view::scan_prefix(jsoncons::span<const uint8_t>(data));
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::unknown_type);
    }

    SECTION("a two-byte simple value below 32 is not well-formed")
    {
        // RFC 8949 3.3
        std::vector<uint8_t> below = {0xf8,0x13};
        auto below_result = cbor::view::scan_prefix(jsoncons::span<const uint8_t>(below));
        REQUIRE_FALSE(below_result.has_value());
        CHECK(below_result.error().code == cbor::cbor_errc::unknown_type);

        std::vector<uint8_t> nested = {0x81,0xf8,0x00};
        CHECK_FALSE(cbor::view::scan_prefix(jsoncons::span<const uint8_t>(nested)).has_value());

        std::vector<uint8_t> at_32 = {0xf8,0x20};
        auto at_32_result = cbor::view::parse_exact(jsoncons::span<const uint8_t>(at_32));
        REQUIRE(at_32_result.has_value());
        CHECK(at_32_result.value().kind() == cbor::view::item_kind::simple);
        CHECK(at_32_result.value().argument() == 32);
    }
}

TEST_CASE("cbor view item exposes wire structure")
{
    SECTION("kinds follow the untagged major type")
    {
        CHECK(parse({0x00}).kind() == cbor::view::item_kind::unsigned_integer);
        CHECK(parse({0x20}).kind() == cbor::view::item_kind::negative_integer);
        CHECK(parse({0x41,0x01}).kind() == cbor::view::item_kind::byte_string);
        CHECK(parse({0x61,'a'}).kind() == cbor::view::item_kind::text_string);
        CHECK(parse({0x80}).kind() == cbor::view::item_kind::array);
        CHECK(parse({0xa0}).kind() == cbor::view::item_kind::map);
        CHECK(parse({0xf4}).kind() == cbor::view::item_kind::simple);
    }

    SECTION("argument exposes the head argument")
    {
        CHECK(parse({0x0a}).argument() == 10);
        CHECK(parse({0x18,0x64}).argument() == 100);
        CHECK(parse({0x63,'a','b','c'}).argument() == 3);
        CHECK(parse({0x82,0x01,0x02}).argument() == 2);
        CHECK(parse({0xf6}).argument() == 22);   // null
        CHECK(parse({0xf7}).argument() == 23);   // undefined
    }

    SECTION("indefinite length is visible")
    {
        std::vector<uint8_t> data = {0x9f,0x01,0xff};
        cbor::view::item scanned = parse(data);
        CHECK(scanned.indefinite());
        CHECK(scanned.argument() == 0);
        CHECK_FALSE(parse({0x81,0x01}).indefinite());
    }

    SECTION("tags are exposed, not interpreted")
    {
        std::vector<uint8_t> data = {0xd8,0x40,0xc2,0x42,0x01,0x02};   // tag 64, tag 2, bytes
        cbor::view::item tagged = parse(data);
        CHECK(tagged.kind() == cbor::view::item_kind::byte_string);

        std::vector<uint64_t> tags;
        for (uint64_t tag : tagged.tags())
        {
            tags.push_back(tag);
        }
        CHECK((tags == std::vector<uint64_t>{64,2}));

        std::vector<uint8_t> untagged = {0x01};
        CHECK(parse(untagged).tags().empty());
    }

    SECTION("encoded_bytes covers the whole item, tags included")
    {
        std::vector<uint8_t> data = {0xc1,0x0a};
        cbor::view::item tagged = parse(data);
        CHECK(tagged.encoded_bytes().data() == data.data());
        CHECK(tagged.encoded_bytes().size() == data.size());
    }
}

TEST_CASE("cbor view typed accessors")
{
    SECTION("uint64_value")
    {
        uint64_t v = 0;
        CHECK(parse({0x00}).uint64_value(v));
        CHECK(v == 0);
        CHECK(parse({0x1b,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}).uint64_value(v));
        CHECK(v == (std::numeric_limits<uint64_t>::max)());

        CHECK_FALSE(parse({0x20}).uint64_value(v));
        CHECK_FALSE(parse({0x61,'a'}).uint64_value(v));
    }

    SECTION("int64_value")
    {
        int64_t v = 0;
        CHECK(parse({0x0a}).int64_value(v));
        CHECK(v == 10);
        CHECK(parse({0x20}).int64_value(v));
        CHECK(v == -1);
        CHECK(parse({0x1b,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff}).int64_value(v));
        CHECK(v == (std::numeric_limits<int64_t>::max)());
        CHECK(parse({0x3b,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff}).int64_value(v));
        CHECK(v == (std::numeric_limits<int64_t>::min)());

        CHECK_FALSE(parse({0x1b,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00}).int64_value(v));
        CHECK_FALSE(parse({0x3b,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00}).int64_value(v));
    }

    SECTION("bool_value")
    {
        bool v = false;
        CHECK(parse({0xf5}).bool_value(v));
        CHECK(v);
        CHECK(parse({0xf4}).bool_value(v));
        CHECK_FALSE(v);
        CHECK_FALSE(parse({0xf6}).bool_value(v));
    }

    SECTION("double_value")
    {
        double v = 0;
        CHECK(parse({0xf9,0x3c,0x00}).double_value(v));
        CHECK(v == 1.0);
        CHECK(parse({0xf9,0x7c,0x00}).double_value(v));
        CHECK(v == std::numeric_limits<double>::infinity());
        CHECK(parse({0xfa,0x3f,0xc0,0x00,0x00}).double_value(v));
        CHECK(v == 1.5);
        CHECK(parse({0xfb,0x3f,0xf1,0x99,0x99,0x99,0x99,0x99,0x9a}).double_value(v));
        CHECK(v == 1.1);

        CHECK_FALSE(parse({0x01}).double_value(v));
    }

    SECTION("a tagged scalar still reads, with its tag visible")
    {
        std::vector<uint8_t> data = {0xc1,0x0a};   // tag 1 (epoch time) 10
        cbor::view::item tagged = parse(data);
        CHECK_FALSE(tagged.tags().empty());
        uint64_t v = 0;
        CHECK(tagged.uint64_value(v));
        CHECK(v == 10);
    }

    SECTION("text views definite strings in place")
    {
        std::vector<uint8_t> data = {0x63,'a','b','c'};
        cbor::view::item text_item = parse(data);
        jsoncons::string_view sv;
        REQUIRE(text_item.text(sv));
        CHECK(std::string(sv.data(), sv.size()) == "abc");
        CHECK(reinterpret_cast<const uint8_t*>(sv.data()) == data.data() + 1);

        std::vector<uint8_t> empty = {0x60};
        REQUIRE(parse(empty).text(sv));
        CHECK(sv.empty());

        std::vector<uint8_t> chunked = {0x7f,0x61,'a',0xff};
        CHECK_FALSE(parse(chunked).text(sv));
        std::vector<uint8_t> bytes = {0x41,0x01};
        CHECK_FALSE(parse(bytes).text(sv));
    }

    SECTION("bytes views definite strings in place")
    {
        std::vector<uint8_t> data = {0x43,0x01,0x02,0x03};
        jsoncons::span<const uint8_t> bs;
        REQUIRE(parse(data).bytes(bs));
        CHECK(bs.data() == data.data() + 1);
        CHECK(bs.size() == 3);
    }
}

TEST_CASE("cbor view copying accessors are transactional")
{
    SECTION("text assembles chunked strings")
    {
        std::string s;
        REQUIRE(parse({0x63,'a','b','c'}).text(s));
        CHECK(s == "abc");
        REQUIRE(parse({0x7f,0x62,'h','e',0x63,'l','l','o',0xff}).text(s));
        CHECK(s == "hello");
        REQUIRE(parse({0x7f,0xff}).text(s));
        CHECK(s.empty());
    }

    SECTION("bytes assembles chunked strings")
    {
        std::vector<uint8_t> buffer;
        REQUIRE(parse({0x5f,0x41,0x01,0x42,0x02,0x03,0xff}).bytes(buffer));
        CHECK((buffer == std::vector<uint8_t>{0x01,0x02,0x03}));
    }

    SECTION("the destination is untouched on kind mismatch")
    {
        std::string s = "sentinel";
        CHECK_FALSE(parse({0x41,0x01}).text(s));
        CHECK(s == "sentinel");

        std::vector<uint8_t> buffer = {0xaa};
        CHECK_FALSE(parse({0x61,'a'}).bytes(buffer));
        CHECK((buffer == std::vector<uint8_t>{0xaa}));
    }

    SECTION("the destination may alias the scanned bytes")
    {
        std::vector<uint8_t> data = {0x42,0x01,0x02};
        auto result = cbor::view::parse_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        REQUIRE(result.value().bytes(data));   // the item borrows `data` itself
        CHECK((data == std::vector<uint8_t>{0x01,0x02}));
    }
}


TEST_CASE("cbor view string chunks")
{
    SECTION("a definite string is one chunk in place")
    {
        std::vector<uint8_t> data = {0x63,'a','b','c'};
        std::size_t count = 0;
        for (jsoncons::span<const uint8_t> chunk : parse(data).chunks())
        {
            CHECK(chunk.data() == data.data() + 1);
            CHECK(chunk.size() == 3);
            ++count;
        }
        CHECK(count == 1);
    }

    SECTION("an indefinite string is one chunk per piece")
    {
        std::vector<uint8_t> data = {0x7f,0x62,'h','e',0x63,'l','l','o',0xff};
        std::vector<std::size_t> sizes;
        for (jsoncons::span<const uint8_t> chunk : parse(data).chunks())
        {
            sizes.push_back(chunk.size());
        }
        CHECK((sizes == std::vector<std::size_t>{2,3}));

        CHECK(parse({0x5f,0xff}).chunks().empty());
    }

    SECTION("non-strings have no chunks")
    {
        CHECK(parse({0x01}).chunks().empty());
        CHECK(parse({0x81,0x41,0x01}).chunks().empty());
    }
}

TEST_CASE("cbor view deterministic encoding orders")
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

    SECTION("bytewise_compare matches the RFC 8949 4.2.1 example")
    {
        for (std::size_t i = 0; i < bytewise_sorted.size(); ++i)
        {
            for (std::size_t j = 0; j < bytewise_sorted.size(); ++j)
            {
                const int r = cbor::view::bytewise_compare()(parse(bytewise_sorted[i]), parse(bytewise_sorted[j]));
                CHECK((i < j ? r < 0 : (i > j ? r > 0 : r == 0)));
            }
        }
    }

    SECTION("length_first_compare matches the RFC 8949 4.2.3 example")
    {
        for (std::size_t i = 0; i < length_first_sorted.size(); ++i)
        {
            for (std::size_t j = 0; j < length_first_sorted.size(); ++j)
            {
                const int r = cbor::view::length_first_compare()(parse(length_first_sorted[i]), parse(length_first_sorted[j]));
                CHECK((i < j ? r < 0 : (i > j ? r > 0 : r == 0)));
            }
        }
    }

    SECTION("the less predicates sort into the deterministic orders")
    {
        struct bytewise_vector_less
        {
            bool operator()(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b) const
            {
                return cbor::view::bytewise_less()(jsoncons::span<const uint8_t>(a), jsoncons::span<const uint8_t>(b));
            }
        };

        std::vector<std::vector<uint8_t>> shuffled = bytewise_sorted;
        std::reverse(shuffled.begin(), shuffled.end());
        std::sort(shuffled.begin(), shuffled.end(), bytewise_vector_less());
        CHECK(shuffled == bytewise_sorted);
    }

    SECTION("other orders can be plugged in")
    {
        struct reverse_bytewise_compare
        {
            int operator()(const cbor::view::item& a, const cbor::view::item& b) const noexcept
            {
                return -cbor::view::bytewise_compare()(a, b);
            }
        };

        std::vector<uint8_t> ten = {0x0a};
        std::vector<uint8_t> hundred = {0x18,0x64};
        CHECK(cbor::view::bytewise_compare()(parse(ten), parse(hundred)) < 0);
        CHECK(reverse_bytewise_compare()(parse(ten), parse(hundred)) > 0);
    }
}


TEST_CASE("cbor view validate_text")
{
    SECTION("well-formed UTF-8 passes")
    {
        CHECK(cbor::view::validate_text(parse({0x63,'a','b','c'})));
        CHECK(cbor::view::validate_text(parse({0x62,0xc3,0xa9})));
        CHECK(cbor::view::validate_text(parse({0x7f,0x62,0xc3,0xa9,0x61,'a',0xff})));
    }

    SECTION("ill-formed UTF-8 is rejected")
    {
        CHECK_FALSE(cbor::view::validate_text(parse({0x62,0xc3,0x28})));
        CHECK_FALSE(cbor::view::validate_text(parse({0x61,0x80})));
    }

    SECTION("a multibyte sequence may not straddle chunks")
    {
        // RFC 8949 3.2.3
        CHECK_FALSE(cbor::view::validate_text(parse({0x7f,0x61,0xc3,0x61,0xa9,0xff})));
    }

    SECTION("non-text items are not valid text")
    {
        CHECK_FALSE(cbor::view::validate_text(parse({0x01})));
        CHECK_FALSE(cbor::view::validate_text(parse({0x41,0x61})));
    }
}

static_assert(std::is_move_constructible<cbor::view::navigator>::value,
              "navigator must be movable");
static_assert(!std::is_copy_constructible<cbor::view::navigator>::value,
              "navigator must not be copyable");

TEST_CASE("cbor view navigator construction and root access")
{
    SECTION("navigate_prefix returns a checked root and remainder")
    {
        std::vector<uint8_t> data = {0xc1,0x18,0x2a,0x01};   // tag(1) 42, then 1
        auto result = cbor::view::navigate_prefix(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        CHECK(result.value().first.kind() == cbor::view::item_kind::unsigned_integer);
        CHECK(result.value().first.argument() == 42);
        CHECK(result.value().first.role() == cbor::view::position_role::root);
        CHECK(result.value().first.depth() == 0);
        CHECK(result.value().first.extent_known());
        REQUIRE(result.value().remainder.size() == 1);
        CHECK(result.value().remainder.data() == data.data() + 3);

        std::vector<uint64_t> tags;
        for (uint64_t tag : result.value().first.tags())
        {
            tags.push_back(tag);
        }
        REQUIRE(tags.size() == 1);
        CHECK(tags[0] == 1);

        uint64_t value = 0;
        CHECK(result.value().first.uint64_value(value));
        CHECK(value == 42);
    }

    SECTION("navigate_exact exposes scalar and string content")
    {
        std::vector<uint8_t> text_data = {0x63,'a','d','a'};
        auto text_result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(text_data));
        REQUIRE(text_result.has_value());
        jsoncons::string_view text;
        CHECK(text_result.value().text(text));
        CHECK(text == "ada");

        std::vector<uint8_t> bytes_data = {0x42,0x01,0x02};
        auto bytes_result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(bytes_data));
        REQUIRE(bytes_result.has_value());
        jsoncons::span<const uint8_t> bytes;
        CHECK(bytes_result.value().bytes(bytes));
        REQUIRE(bytes.size() == 2);
        CHECK(bytes[0] == 1);
        CHECK(bytes[1] == 2);

        std::vector<uint8_t> boolean_data = {0xf5};
        auto boolean_result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(boolean_data));
        REQUIRE(boolean_result.has_value());
        bool boolean = false;
        CHECK(boolean_result.value().bool_value(boolean));
        CHECK(boolean);
    }

    SECTION("navigate_exact rejects trailing data")
    {
        std::vector<uint8_t> data = {0x01,0x02};
        auto result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::trailing_data);
        CHECK(result.error().offset == 1);
    }

    SECTION("reset is transactional and reuses the navigator")
    {
        std::vector<uint8_t> first = {0x01};
        auto made = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(first));
        REQUIRE(made.has_value());
        cbor::view::navigator navigator = std::move(made.value());

        std::vector<uint8_t> sequence = {0x02,0x03};
        auto remainder = navigator.reset_prefix(jsoncons::span<const uint8_t>(sequence));
        REQUIRE(remainder.has_value());
        REQUIRE(remainder.value().size() == 1);
        uint64_t value = 0;
        REQUIRE(navigator.uint64_value(value));
        CHECK(value == 2);

        std::vector<uint8_t> malformed = {0x19,0x01};
        auto malformed_result = navigator.reset_exact(jsoncons::span<const uint8_t>(malformed));
        REQUIRE_FALSE(malformed_result.has_value());
        value = 0;
        REQUIRE(navigator.uint64_value(value));
        CHECK(value == 2);

        auto trailing_result = navigator.reset_exact(jsoncons::span<const uint8_t>(sequence));
        REQUIRE_FALSE(trailing_result.has_value());
        CHECK(trailing_result.error().code == cbor::cbor_errc::trailing_data);
        value = 0;
        REQUIRE(navigator.uint64_value(value));
        CHECK(value == 2);

        std::vector<uint8_t> replacement = {0x81,0x04};
        REQUIRE(navigator.reset_exact(jsoncons::span<const uint8_t>(replacement)).has_value());
        CHECK(navigator.kind() == cbor::view::item_kind::array);
        CHECK(navigator.argument() == 1);
        CHECK(navigator.role() == cbor::view::position_role::root);
        CHECK(navigator.depth() == 0);
        std::vector<uint8_t> deep(100, 0x81);
        deep.push_back(0x05);
        REQUIRE(navigator.reset_exact(jsoncons::span<const uint8_t>(deep), 200).has_value());
        REQUIRE(navigator.reset_exact(jsoncons::span<const uint8_t>(replacement), 200).has_value());
        REQUIRE(navigator.reset_exact(jsoncons::span<const uint8_t>(deep), 200).has_value());
        for (std::size_t i = 0; i < 100; ++i)
        {
            REQUIRE(navigator.enter());
        }
        CHECK(navigator.argument() == 5);

    }
}

TEST_CASE("cbor view navigator movement")
{
    SECTION("walks nested definite arrays without prefinishing parents")
    {
        std::vector<uint8_t> data = {0x83,0x01,0x82,0x02,0x03,0x04}; // [1,[2,3],4]
        auto result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        cbor::view::navigator navigator = std::move(result.value());

        REQUIRE(navigator.enter());
        CHECK(navigator.depth() == 1);
        CHECK(navigator.role() == cbor::view::position_role::array_element);
        CHECK(navigator.argument() == 1);

        REQUIRE(navigator.next());
        CHECK(navigator.kind() == cbor::view::item_kind::array);
        CHECK_FALSE(navigator.extent_known());
        REQUIRE(navigator.enter());
        CHECK(navigator.depth() == 2);
        CHECK(navigator.argument() == 2);
        REQUIRE(navigator.next());
        CHECK(navigator.argument() == 3);
        CHECK_FALSE(navigator.next());
        CHECK_FALSE(navigator.next());
        REQUIRE(navigator.leave());
        CHECK(navigator.depth() == 1);
        CHECK(navigator.kind() == cbor::view::item_kind::array);
        CHECK(navigator.extent_known());

        REQUIRE(navigator.next());
        CHECK(navigator.argument() == 4);
        CHECK(navigator.extent_known());
        CHECK_FALSE(navigator.next());
        REQUIRE(navigator.leave());
        CHECK(navigator.depth() == 0);
        CHECK(navigator.kind() == cbor::view::item_kind::array);
        CHECK_FALSE(navigator.leave());
    }

    SECTION("next skips an unopened container once")
    {
        std::vector<uint8_t> data = {0x82,0x82,0x01,0x02,0x03}; // [[1,2],3]
        auto result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        cbor::view::navigator navigator = std::move(result.value());
        REQUIRE(navigator.enter());
        CHECK(navigator.kind() == cbor::view::item_kind::array);
        CHECK_FALSE(navigator.extent_known());
        REQUIRE(navigator.next());
        CHECK(navigator.argument() == 3);
        CHECK(navigator.extent_known());
    }

    SECTION("early leave skips only the unread remainder")
    {
        std::vector<uint8_t> data = {0x82,0x82,0x01,0x02,0x03}; // [[1,2],3]
        auto result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        cbor::view::navigator navigator = std::move(result.value());
        REQUIRE(navigator.enter());
        REQUIRE(navigator.enter());
        CHECK(navigator.argument() == 1);
        REQUIRE(navigator.leave());
        CHECK(navigator.kind() == cbor::view::item_kind::array);
        CHECK(navigator.extent_known());
        REQUIRE(navigator.next());
        CHECK(navigator.argument() == 3);
    }

    SECTION("map roles alternate for definite and indefinite maps")
    {
        const std::vector<std::vector<uint8_t>> maps = {
            {0xa2,0x01,0x02,0x03,0x04},
            {0xbf,0x01,0x02,0x03,0x04,0xff}
        };
        for (const auto& data : maps)
        {
            auto result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(data));
            REQUIRE(result.has_value());
            cbor::view::navigator navigator = std::move(result.value());
            REQUIRE(navigator.enter());
            CHECK(navigator.role() == cbor::view::position_role::map_key);
            CHECK(navigator.argument() == 1);
            REQUIRE(navigator.next());
            CHECK(navigator.role() == cbor::view::position_role::map_value);
            CHECK(navigator.argument() == 2);
            REQUIRE(navigator.next());
            CHECK(navigator.role() == cbor::view::position_role::map_key);
            CHECK(navigator.argument() == 3);
            REQUIRE(navigator.next());
            CHECK(navigator.role() == cbor::view::position_role::map_value);
            CHECK(navigator.argument() == 4);
            CHECK_FALSE(navigator.next());
            CHECK_FALSE(navigator.next());
            REQUIRE(navigator.leave());
            CHECK(navigator.role() == cbor::view::position_role::root);
        }
    }

    SECTION("empty and non-container enter leave position unchanged")
    {
        const std::vector<std::vector<uint8_t>> values = {
            {0x01}, {0x80}, {0x9f,0xff}, {0xa0}, {0xbf,0xff}
        };
        for (const auto& data : values)
        {
            auto result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(data));
            REQUIRE(result.has_value());
            cbor::view::navigator navigator = std::move(result.value());
            const cbor::view::item_kind kind = navigator.kind();
            CHECK_FALSE(navigator.enter());
            CHECK(navigator.kind() == kind);
            CHECK(navigator.depth() == 0);
        }
    }

    SECTION("finish_item caches an unknown extent and descent remains legal")
    {
        std::vector<uint8_t> data = {0x82,0x82,0x01,0x02,0x03}; // [[1,2],3]
        auto result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        cbor::view::navigator navigator = std::move(result.value());
        REQUIRE(navigator.enter());
        CHECK_FALSE(navigator.extent_known());
        cbor::view::item nested = navigator.finish_item();
        CHECK(navigator.extent_known());
        CHECK(nested.encoded_bytes().data() == data.data() + 1);
        CHECK(nested.encoded_bytes().size() == 3);
        REQUIRE(navigator.enter());
        CHECK(navigator.argument() == 1);
    }

    SECTION("right fences make the final payload immediately finishable")
    {
        std::vector<uint8_t> data = {
            0x87,0x01,0x02,0x03,0x04,0x05,0x06,
            0x82,0x81,0x07,0x81,0x08
        };
        auto result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        cbor::view::navigator navigator = std::move(result.value());
        REQUIRE(navigator.enter());
        for (int i = 0; i < 6; ++i)
        {
            REQUIRE(navigator.next());
        }
        CHECK(navigator.kind() == cbor::view::item_kind::array);
        CHECK(navigator.extent_known());
        cbor::view::item payload = navigator.finish_item();
        CHECK(payload.encoded_bytes().data() == data.data() + 7);
        CHECK(payload.encoded_bytes().size() == 5);

        REQUIRE(navigator.enter());
        CHECK(navigator.kind() == cbor::view::item_kind::array);
        CHECK_FALSE(navigator.extent_known());
        REQUIRE(navigator.enter());
        CHECK(navigator.argument() == 7);
        CHECK(navigator.extent_known());
    }

    SECTION("rewind restores the checked root")
    {
        std::vector<uint8_t> data = {0x81,0x81,0x01};
        auto result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        cbor::view::navigator navigator = std::move(result.value());
        REQUIRE(navigator.enter());
        REQUIRE(navigator.enter());
        CHECK(navigator.depth() == 2);
        navigator.rewind();
        CHECK(navigator.depth() == 0);
        CHECK(navigator.role() == cbor::view::position_role::root);
        CHECK(navigator.kind() == cbor::view::item_kind::array);
        REQUIRE(navigator.enter());
        CHECK(navigator.depth() == 1);
    }

    SECTION("deep movement uses the validation-sized workspace")
    {
        const std::size_t depth = 100;
        std::vector<uint8_t> data(depth, 0x81);
        data.push_back(0x01);
        auto result = cbor::view::navigate_exact(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        cbor::view::navigator navigator = std::move(result.value());
        for (std::size_t i = 0; i < depth; ++i)
        {
            REQUIRE(navigator.enter());
            CHECK(navigator.depth() == i + 1);
        }
        CHECK(navigator.argument() == 1);
        for (std::size_t i = 0; i < depth; ++i)
        {
            REQUIRE(navigator.leave());
        }
        CHECK(navigator.depth() == 0);
    }
}



TEST_CASE("cbor view wire cursor")
{
    SECTION("read_head decodes one head and advances past it only")
    {
        std::vector<uint8_t> data = {0x82,0x01,0x02};   // [1, 2]
        cbor::view::wire_cursor cursor{jsoncons::span<const uint8_t>(data)};
        CHECK(cursor.position() == 0);
        CHECK(cursor.remaining().size() == data.size());

        auto result = cursor.read_head();
        REQUIRE(result.has_value());
        CHECK(result.value().major_type == cbor::view::major_type::array);
        CHECK(result.value().value == 2);
        CHECK(cursor.position() == 1);   // past the head, not the elements
        CHECK(cursor.remaining().data() == data.data() + 1);
    }

    SECTION("read_head surfaces tags as their own heads")
    {
        std::vector<uint8_t> data = {0xc1,0x0a};   // tag 1, 10
        cbor::view::wire_cursor cursor{jsoncons::span<const uint8_t>(data)};

        auto tag = cursor.read_head();
        REQUIRE(tag.has_value());
        CHECK(tag.value().major_type == cbor::view::major_type::semantic_tag);
        CHECK(tag.value().value == 1);

        auto value = cursor.read_head();
        REQUIRE(value.has_value());
        CHECK(value.value().major_type == cbor::view::major_type::unsigned_integer);
        CHECK(value.value().value == 10);
        CHECK(cursor.remaining().empty());
    }

    SECTION("read_head reports malformed and truncated heads")
    {
        cbor::view::wire_cursor empty{jsoncons::span<const uint8_t>()};
        auto no_head = empty.read_head();
        REQUIRE_FALSE(no_head.has_value());
        CHECK(no_head.error().code == cbor::cbor_errc::unexpected_eof);
        CHECK(no_head.error().offset == 0);
        CHECK(empty.position() == 0);

        std::vector<uint8_t> truncated = {0x19,0x01};
        cbor::view::wire_cursor cursor{jsoncons::span<const uint8_t>(truncated)};
        auto result = cursor.read_head();
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::unexpected_eof);
        CHECK(result.error().offset == 1);
        CHECK(cursor.position() == 1);
    }

    SECTION("read_item returns and advances past one complete item")
    {
        std::vector<uint8_t> data = {0x82,0x01,0x02,0x63,'a','b','c'};   // [1,2] then "abc"
        cbor::view::wire_cursor cursor{jsoncons::span<const uint8_t>(data)};
        cbor::view::scan_context context;

        auto array = cursor.read_item(context);
        REQUIRE(array.has_value());
        CHECK(array.value().kind() == cbor::view::item_kind::array);
        CHECK(array.value().encoded_bytes().size() == 3);
        CHECK(cursor.position() == 3);

        auto text = cursor.read_item(context);
        REQUIRE(text.has_value());
        CHECK(text.value().kind() == cbor::view::item_kind::text_string);
        CHECK(cursor.position() == data.size());
        CHECK(cursor.remaining().empty());
    }

    SECTION("read_item honours the depth bound and reports an absolute offset")
    {
        std::vector<uint8_t> data = {0x01,0x81,0x81,0x01};   // 1 then [[1]]
        cbor::view::wire_cursor cursor{jsoncons::span<const uint8_t>(data)};
        cbor::view::scan_context default_context;
        REQUIRE(cursor.read_item(default_context).has_value());

        cbor::view::scan_context context(1);
        auto result = cursor.read_item(context);
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::max_nesting_depth_exceeded);
        CHECK(result.error().offset == 3);
        CHECK(cursor.position() == 3);
    }

    SECTION("skip_item passes over one item, returning its bytes unparsed")
    {
        std::vector<uint8_t> data = {0x82,0x01,0x02,0x63,'a','b','c'};   // [1,2] then "abc"
        cbor::view::wire_cursor cursor{jsoncons::span<const uint8_t>(data)};
        cbor::view::scan_context context;

        auto skipped = cursor.skip_item(context);
        REQUIRE(skipped.has_value());
        CHECK(skipped.value().data() == data.data());
        CHECK(skipped.value().size() == 3);
        CHECK(cursor.position() == 3);

        auto text = cursor.read_item(context);
        REQUIRE(text.has_value());
        CHECK(text.value().kind() == cbor::view::item_kind::text_string);
        CHECK(cursor.remaining().empty());
    }

    SECTION("skip_item honours the depth bound like read_item")
    {
        std::vector<uint8_t> data = {0x01,0x81,0x81,0x01};   // 1 then [[1]]
        cbor::view::wire_cursor cursor{jsoncons::span<const uint8_t>(data)};
        cbor::view::scan_context default_context;
        REQUIRE(cursor.skip_item(default_context).has_value());

        cbor::view::scan_context context(1);
        auto result = cursor.skip_item(context);
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::max_nesting_depth_exceeded);
        CHECK(result.error().offset == 3);
        CHECK(cursor.position() == 3);
    }

    SECTION("skip advances past measured content only when it is available")
    {
        std::vector<uint8_t> data = {0x63,'a','b','c'};
        cbor::view::wire_cursor cursor{jsoncons::span<const uint8_t>(data)};
        auto head = cursor.read_head();
        REQUIRE(head.has_value());
        REQUIRE(cursor.skip(static_cast<std::size_t>(head.value().value)));
        CHECK(cursor.position() == data.size());
        CHECK(cursor.remaining().empty());
        CHECK(cursor.skip(0));
        CHECK_FALSE(cursor.skip(1));
        CHECK(cursor.position() == data.size());
    }

}

TEST_CASE("cbor view span-based order entry points")
{
    struct reverse_bytewise
    {
        int operator()(jsoncons::span<const uint8_t> a, jsoncons::span<const uint8_t> b) const noexcept
        {
            return -cbor::view::bytewise_compare()(a, b);
        }
    };

    SECTION("compare validates each span then orders, tolerating trailing bytes")
    {
        std::vector<uint8_t> ten = {0x0a};
        std::vector<uint8_t> hundred = {0x18,0x64};
        std::vector<uint8_t> ten_plus = {0x0a,0x63,'x','y','z'};   // 10 then trailing
        auto lt = cbor::view::compare(jsoncons::span<const uint8_t>(ten), jsoncons::span<const uint8_t>(hundred));
        REQUIRE(lt.has_value());
        CHECK(lt.value() < 0);
        auto eq = cbor::view::compare(jsoncons::span<const uint8_t>(ten), jsoncons::span<const uint8_t>(ten_plus));
        REQUIRE(eq.has_value());
        CHECK(eq.value() == 0);
        auto rev = cbor::view::compare(jsoncons::span<const uint8_t>(ten), jsoncons::span<const uint8_t>(hundred), reverse_bytewise());
        REQUIRE(rev.has_value());
        CHECK(rev.value() > 0);
    }

    SECTION("compare reports malformed input with an offset")
    {
        std::vector<uint8_t> good = {0x01};
        std::vector<uint8_t> bad = {0x82,0x01,0x1f};   // [1, <invalid>]
        auto result = cbor::view::compare(jsoncons::span<const uint8_t>(good), jsoncons::span<const uint8_t>(bad));
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::unknown_type);
        CHECK(result.error().offset == 3);
    }

    SECTION("map_keys_sorted validates and checks map keys")
    {
        std::vector<uint8_t> sorted_map = {0xa2,0x01,0x61,0x61,0x61,0x6b,0x02};
        std::vector<uint8_t> unsorted_map = {0xa2,0x61,0x6b,0x02,0x01,0x61,0x61};
        std::vector<uint8_t> duplicate_map = {0xa2,0x01,0x00,0x01,0x01};
        auto sorted = cbor::view::map_keys_sorted(jsoncons::span<const uint8_t>(sorted_map));
        REQUIRE(sorted.has_value());
        CHECK(sorted.value());
        auto unsorted = cbor::view::map_keys_sorted(jsoncons::span<const uint8_t>(unsorted_map));
        REQUIRE(unsorted.has_value());
        CHECK_FALSE(unsorted.value());
        auto duplicate = cbor::view::map_keys_sorted(jsoncons::span<const uint8_t>(duplicate_map));
        REQUIRE(duplicate.has_value());
        CHECK_FALSE(duplicate.value());

        std::vector<uint8_t> length_order = {0xa2,0x18,0x64,0x00,0x20,0x00};
        auto bytewise = cbor::view::map_keys_sorted(jsoncons::span<const uint8_t>(length_order));
        auto length_first = cbor::view::map_keys_sorted(
            jsoncons::span<const uint8_t>(length_order), cbor::view::length_first_compare());
        REQUIRE(bytewise.has_value());
        REQUIRE(length_first.has_value());
        CHECK(bytewise.value());
        CHECK_FALSE(length_first.value());

        std::vector<uint8_t> array = {0x81,0x01};
        auto not_map = cbor::view::map_keys_sorted(jsoncons::span<const uint8_t>(array));
        REQUIRE(not_map.has_value());
        CHECK_FALSE(not_map.value());

        std::vector<uint8_t> malformed = {0xa1,0x01};   // key, no value
        auto bad = cbor::view::map_keys_sorted(jsoncons::span<const uint8_t>(malformed));
        REQUIRE_FALSE(bad.has_value());
        CHECK(bad.error().code == cbor::cbor_errc::unexpected_eof);
    }
}

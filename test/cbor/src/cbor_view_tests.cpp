// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif

#include <jsoncons_ext/cbor/cbor_view.hpp>

#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>
#include <catch/catch.hpp>

using namespace jsoncons;

namespace {

    cbor::view::item parse(const std::vector<uint8_t>& data)
    {
        auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data.data(), data.size()));
        REQUIRE(result.has_value());
        return result.value();
    }

    cbor::cbor_errc parse_errc(const std::vector<uint8_t>& data)
    {
        auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data.data(), data.size()));
        return result ? cbor::cbor_errc::success : result.error().code;
    }

} // namespace

TEST_CASE("cbor view scan and parse_item")
{
    SECTION("scan returns the first item and the remainder")
    {
        std::vector<uint8_t> data = {0x01,0x02};
        auto result = cbor::view::scan(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        CHECK(result.value().first.encoded_bytes().data() == data.data());
        CHECK(result.value().first.encoded_bytes().size() == 1);
        CHECK(result.value().remainder.data() == data.data() + 1);
        CHECK(result.value().remainder.size() == 1);
    }

    SECTION("scan walks a sequence of items")
    {
        std::vector<uint8_t> data = {0x01,0x61,'a',0x80};
        jsoncons::span<const uint8_t> rest(data.data(), data.size());
        std::size_t count = 0;
        while (!rest.empty())
        {
            auto result = cbor::view::scan(rest);
            REQUIRE(result.has_value());
            rest = result.value().remainder;
            ++count;
        }
        CHECK(count == 3);
    }

    SECTION("parse_item rejects trailing bytes with their offset")
    {
        std::vector<uint8_t> data = {0x01,0x02};
        auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data));
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::trailing_data);
        CHECK(result.error().offset == 1);

        std::vector<uint8_t> exact = {0x01};
        CHECK(cbor::view::parse_item(jsoncons::span<const uint8_t>(exact)).has_value());
    }

    SECTION("empty input fails at offset zero")
    {
        auto result = cbor::view::scan(jsoncons::span<const uint8_t>());
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::unexpected_eof);
        CHECK(result.error().offset == 0);
    }

    SECTION("errors carry the offset where scanning stopped")
    {
        // [1, <invalid head 0x1f>]
        std::vector<uint8_t> data = {0x82,0x01,0x1f};
        auto result = cbor::view::scan(jsoncons::span<const uint8_t>(data));
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::unknown_type);
        CHECK(result.error().offset == 3);
    }
}

TEST_CASE("cbor view scanning validates well-formedness")
{
    SECTION("nesting at the default depth bound succeeds")
    {
        std::vector<uint8_t> data(cbor::view::default_max_nesting_depth, 0x81);
        data.push_back(0x01);
        auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        CHECK(result.value().encoded_bytes().size() == data.size());
    }

    SECTION("nesting past the depth bound is rejected")
    {
        std::vector<uint8_t> data(cbor::view::default_max_nesting_depth + 1, 0x81);
        data.push_back(0x01);
        auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data));
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::max_nesting_depth_exceeded);

        CHECK(cbor::view::parse_item(jsoncons::span<const uint8_t>(data), 4000).has_value());
    }

    SECTION("tag chains do not consume nesting depth or stack")
    {
        std::vector<uint8_t> data(1000000, 0xc0);
        data.push_back(0x01);
        auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data));
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
        auto array_result = cbor::view::scan(jsoncons::span<const uint8_t>(array));
        REQUIRE_FALSE(array_result.has_value());
        CHECK(array_result.error().code == cbor::cbor_errc::unexpected_eof);

        std::vector<uint8_t> map = {0xba,0xff,0xff,0xff,0xff};
        auto map_result = cbor::view::scan(jsoncons::span<const uint8_t>(map));
        REQUIRE_FALSE(map_result.has_value());
        CHECK(map_result.error().code == cbor::cbor_errc::unexpected_eof);
    }

    SECTION("a count beyond the input fails at its first malformed item, else EOF")
    {
        CHECK(parse_errc({0x91,0xff}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_errc({0xb1,0x01,0xff}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_errc({0x9b,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0xff}) == cbor::cbor_errc::unknown_type);
        CHECK(parse_errc({0x91,0x01}) == cbor::cbor_errc::unexpected_eof);
    }

    SECTION("definite and indefinite nesting can mix")
    {
        // [_ {1: [2]}, [], {} ] followed by trailing bytes
        std::vector<uint8_t> data = {0x9f,0xa1,0x01,0x81,0x02,0x80,0xa0,0xff,0x63,'a','b','c'};
        auto result = cbor::view::scan(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        CHECK(result.value().first.encoded_bytes().size() == 8);
        CHECK(result.value().remainder.size() == 4);
    }

    SECTION("an indefinite map break may not split a key from its value")
    {
        std::vector<uint8_t> dangling_key = {0xbf,0x01,0xff};
        auto result = cbor::view::scan(jsoncons::span<const uint8_t>(dangling_key));
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::unknown_type);

        std::vector<uint8_t> complete_entry = {0xbf,0x01,0x02,0xff};
        CHECK(cbor::view::parse_item(jsoncons::span<const uint8_t>(complete_entry)).has_value());
    }

    SECTION("an invalid indefinite integer is rejected")
    {
        std::vector<uint8_t> data = {0x1f};
        auto result = cbor::view::scan(jsoncons::span<const uint8_t>(data));
        REQUIRE_FALSE(result.has_value());
        CHECK(result.error().code == cbor::cbor_errc::unknown_type);
    }

    SECTION("a two-byte simple value below 32 is not well-formed")
    {
        // RFC 8949 3.3
        std::vector<uint8_t> below = {0xf8,0x13};
        auto below_result = cbor::view::scan(jsoncons::span<const uint8_t>(below));
        REQUIRE_FALSE(below_result.has_value());
        CHECK(below_result.error().code == cbor::cbor_errc::unknown_type);

        std::vector<uint8_t> nested = {0x81,0xf8,0x00};
        CHECK_FALSE(cbor::view::scan(jsoncons::span<const uint8_t>(nested)).has_value());

        std::vector<uint8_t> at_32 = {0xf8,0x20};
        auto at_32_result = cbor::view::parse_item(jsoncons::span<const uint8_t>(at_32));
        REQUIRE(at_32_result.has_value());
        CHECK(at_32_result.value().kind() == cbor::view::item_kind::simple);
        CHECK(at_32_result.value().argument() == 32);
    }
}

TEST_CASE("cbor view rejects the RFC 8949 compliance vectors")
{
    // The vectors of cbor_compliance_tests.cpp, through parse_item.
    using cbor::cbor_errc;

    CHECK(parse_errc({0x18}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x19}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x19,0x00}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x1a}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x1a,0x00}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x1a,0x00,0x00}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x1a,0x00,0x00,0x00}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x1b,0x00,0x00,0x00}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x1c}) == cbor_errc::reserved_additional_info);
    CHECK(parse_errc({0x1d}) == cbor_errc::reserved_additional_info);
    CHECK(parse_errc({0x1e}) == cbor_errc::reserved_additional_info);
    CHECK(parse_errc({0xfc}) == cbor_errc::reserved_additional_info);
    CHECK(parse_errc({0xfd}) == cbor_errc::reserved_additional_info);
    CHECK(parse_errc({0xfe}) == cbor_errc::reserved_additional_info);
    CHECK(parse_errc({0x44,01,02,03}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x5f}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x5f,0x01,0xff}) == cbor_errc::illegal_chunked_string);
    CHECK(parse_errc({0x64,0x49,0x45,0x54}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x74,0x32,0x30,0x31,0x33}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x7f,0x01,0xff}) == cbor_errc::illegal_chunked_string);
    CHECK(parse_errc({0x7f,0x65,0x73,0x74,0x72,0x65,0x61,0x64,0x6d,0x69,0x6e}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x62,0xc0,0xae}) == cbor_errc::success);   // validate_text's concern
    CHECK(parse_errc({0x81}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x82,0x01}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x81,0x81,0x81,0x81,0x81}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc(std::vector<uint8_t>(160, 0x81)) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x81,0xfe}) == cbor_errc::reserved_additional_info);
    CHECK(parse_errc({0x9f}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x9f,0x01}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0x9f,0xfe,0xff}) == cbor_errc::reserved_additional_info);
    CHECK(parse_errc({0x91,0xff}) == cbor_errc::unknown_type);
    CHECK(parse_errc({0xa1}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0xa1,0xfe,0x01}) == cbor_errc::reserved_additional_info);
    CHECK(parse_errc({0xa1,0x61,0x61}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0xa1,0x61,0x61,0xfe}) == cbor_errc::reserved_additional_info);
    CHECK(parse_errc({0xa2,0x01,0x02}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0xbf}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0xbf,0x00,0x01,0x03,0xff}) == cbor_errc::unknown_type);
    CHECK(parse_errc({0xbf,0x61,0x61}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0xbf,0x61,0x61,0x01}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0xbf,0xfe,0x01}) == cbor_errc::reserved_additional_info);
    CHECK(parse_errc({0xbf,0x01,0xfe}) == cbor_errc::reserved_additional_info);
    CHECK(parse_errc({0xa1,0xff}) == cbor_errc::unknown_type);
    CHECK(parse_errc({0xa1,0x00,0xff}) == cbor_errc::unknown_type);
    CHECK(parse_errc({0xff}) == cbor_errc::unknown_type);
    CHECK(parse_errc({0x18}) == cbor_errc::unexpected_eof);
    CHECK(parse_errc({0xff}) == cbor_errc::unknown_type);
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
        auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        REQUIRE(result.value().bytes(data));   // the item borrows `data` itself
        CHECK((data == std::vector<uint8_t>{0x01,0x02}));
    }
}


TEST_CASE("cbor view string chunks")
{
    SECTION("a definite string has no children; its content is in place")
    {
        std::vector<uint8_t> data = {0x63,'a','b','c'};
        CHECK(parse(data).children().empty());
        jsoncons::span<const uint8_t> content;
        CHECK_FALSE(parse(data).bytes(content));
        jsoncons::string_view text;
        REQUIRE(parse(data).text(text));
        CHECK(reinterpret_cast<const uint8_t*>(text.data()) == data.data() + 1);
        CHECK(text.size() == 3);
    }

    SECTION("an indefinite string's children are its chunks")
    {
        std::vector<uint8_t> data = {0x7f,0x62,'h','e',0x63,'l','l','o',0xff};
        std::vector<std::size_t> sizes;
        for (cbor::view::item chunk : parse(data).children())
        {
            CHECK(chunk.kind() == cbor::view::item_kind::text_string);
            CHECK_FALSE(chunk.indefinite());
            jsoncons::string_view text;
            REQUIRE(chunk.text(text));
            sizes.push_back(text.size());
        }
        CHECK((sizes == std::vector<std::size_t>{2,3}));

        CHECK(parse({0x5f,0xff}).children().empty());
        CHECK_FALSE(parse({0x5f,0x41,0x01,0xff}).children().empty());
    }

    SECTION("scalars have no children")
    {
        CHECK(parse({0x01}).children().empty());
        CHECK(parse({0xf5}).children().empty());
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

TEST_CASE("cbor view item children")
{
    SECTION("array children are checked items in order")
    {
        std::vector<uint8_t> data = {0x83,0x01,0x82,0x02,0x03,0x63,'a','b','c'};   // [1,[2,3],"abc"]
        auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());

        std::vector<cbor::view::item> children;
        for (cbor::view::item child : result.value().children())
        {
            children.push_back(child);
        }
        REQUIRE(children.size() == 3);
        CHECK(children[0].kind() == cbor::view::item_kind::unsigned_integer);
        CHECK(children[0].argument() == 1);
        CHECK(children[1].kind() == cbor::view::item_kind::array);
        CHECK(children[1].encoded_bytes().data() == data.data() + 2);
        CHECK(children[1].encoded_bytes().size() == 3);
        CHECK(children[2].kind() == cbor::view::item_kind::text_string);
        jsoncons::string_view text;
        CHECK(children[2].text(text));
        CHECK(text == "abc");

        std::vector<uint64_t> nested;
        for (cbor::view::item grandchild : children[1].children())
        {
            uint64_t value = 0;
            REQUIRE(grandchild.uint64_value(value));
            nested.push_back(value);
        }
        REQUIRE(nested.size() == 2);
        CHECK(nested[0] == 2);
        CHECK(nested[1] == 3);
    }

    SECTION("map children alternate keys and values")
    {
        const std::vector<std::vector<uint8_t>> maps = {
            {0xa2,0x01,0x02,0x03,0x04},
            {0xbf,0x01,0x02,0x03,0x04,0xff}
        };
        for (const auto& data : maps)
        {
            auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data));
            REQUIRE(result.has_value());
            std::vector<uint64_t> values;
            for (cbor::view::item child : result.value().children())
            {
                uint64_t value = 0;
                REQUIRE(child.uint64_value(value));
                values.push_back(value);
            }
            REQUIRE(values.size() == 4);
            CHECK(values[0] == 1);
            CHECK(values[1] == 2);
            CHECK(values[2] == 3);
            CHECK(values[3] == 4);
        }
    }

    SECTION("empty containers and non-containers have no children")
    {
        const std::vector<std::vector<uint8_t>> values = {
            {0x01}, {0x63,'a','d','a'}, {0x80}, {0x9f,0xff}, {0xa0}, {0xbf,0xff}
        };
        for (const auto& data : values)
        {
            auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data));
            REQUIRE(result.has_value());
            CHECK(result.value().children().empty());
            CHECK(result.value().children().begin() == cbor::view::item::child_iterator());
        }
    }

    SECTION("indefinite container children stop at the break")
    {
        std::vector<uint8_t> data = {0x9f,0x01,0x81,0x02,0xff};   // [_ 1,[2]]
        auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        std::size_t count = 0;
        for (cbor::view::item child : result.value().children())
        {
            (void)child;
            ++count;
        }
        CHECK(count == 2);
    }

    SECTION("a deep child is measured past 32 open containers")
    {
        const std::size_t depth = 100;
        std::vector<uint8_t> data = {0x82};          // [deep, 7]
        data.insert(data.end(), depth, 0x81);
        data.push_back(0x01);
        data.push_back(0x07);
        auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data), 200);
        REQUIRE(result.has_value());
        auto it = result.value().children().begin();
        REQUIRE(it != cbor::view::item::child_iterator());
        CHECK((*it).encoded_bytes().size() == depth + 1);
        ++it;
        REQUIRE(it != cbor::view::item::child_iterator());
        uint64_t value = 0;
        CHECK((*it).uint64_value(value));
        CHECK(value == 7);
        ++it;
        CHECK(it == cbor::view::item::child_iterator());
    }

    SECTION("children are measured across mixed definite and indefinite nesting")
    {
        // [[1, [_ 2, [3]], 4], {_ 5: [_ ]}, [[[6]]], 7]
        std::vector<uint8_t> data = {0x84,
            0x83,0x01,0x9f,0x02,0x81,0x03,0xff,0x04,
            0xbf,0x05,0x9f,0xff,0xff,
            0x81,0x81,0x81,0x06,
            0x07};
        auto result = cbor::view::parse_item(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        std::vector<std::size_t> offsets;
        std::vector<std::size_t> sizes;
        for (cbor::view::item child : result.value().children())
        {
            offsets.push_back(static_cast<std::size_t>(child.encoded_bytes().data() - data.data()));
            sizes.push_back(child.encoded_bytes().size());
        }
        CHECK((offsets == std::vector<std::size_t>{1,9,14,18}));
        CHECK((sizes == std::vector<std::size_t>{8,5,4,1}));
    }

    SECTION("the last child of a definite container ends where its parent does")
    {
        // [1, {"a": [2, 3]}] then trailing bytes
        std::vector<uint8_t> data = {0x82,0x01,0xa1,0x61,'a',0x82,0x02,0x03,0x01,0x02};
        auto result = cbor::view::scan(jsoncons::span<const uint8_t>(data));
        REQUIRE(result.has_value());
        cbor::view::item last = result.value().first;
        std::size_t levels = 0;
        while (!last.children().empty())
        {
            for (cbor::view::item child : last.children())
            {
                last = child;
            }
            ++levels;
        }
        CHECK(levels == 3);
        CHECK(last.encoded_bytes().data() == data.data() + 7);
        CHECK(last.encoded_bytes().size() == 1);
    }
}


// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/nonstd/more_type_traits.hpp>
#include <jsoncons/utility/conversion.hpp>
#include <cmath>
#include <limits>
#include <vector>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("convert into string")
{
    std::vector<uint8_t> bytes = {'f','o','o','b','a','r'};

    SECTION("from jsoncons::byte_string into string")
    {
        std::string expected = "Zm9vYmFy";

        std::string s;
        bytes_to_string(bytes.begin(), bytes.end(), jsoncons::semantic_tag::base64url, s);
        
        CHECK(expected == s);
    }
    SECTION("from byte string into wstring")
    {
        std::wstring expected = L"Zm9vYmFy";

        std::wstring s;
        bytes_to_string(bytes.begin(), bytes.end(), jsoncons::semantic_tag::base64url, s);

        CHECK(expected == s);
    }
}

TEST_CASE("convert into list-like")
{
    SECTION("from string")
    {
        std::vector<uint8_t> expected = {'f','o','o','b','a','r'};

        jsoncons::string_view sv("Zm9vYmFy");
        std::vector<uint8_t> v;
        auto r = string_to_bytes(sv.begin(), sv.end(), jsoncons::semantic_tag::base64url, v);
        REQUIRE(r.ec == conv_errc{});
        
        CHECK(expected == v);
    }
}

TEST_CASE("in_integer_range")
{
    SECTION("int64_t")
    {
        CHECK(in_integer_range<int64_t>(-9223372036854775808.0));
        CHECK(in_integer_range<int64_t>(9223372036854775295.0));
        CHECK_FALSE(in_integer_range<int64_t>(9223372036854775808.0));
        CHECK_FALSE(in_integer_range<int64_t>(-9223372036854777856.0));
    }
    SECTION("uint64_t")
    {
        CHECK(in_integer_range<uint64_t>(-0.5));
        CHECK(in_integer_range<uint64_t>(18446744073709549568.0));
        CHECK_FALSE(in_integer_range<uint64_t>(-1.5));
        CHECK_FALSE(in_integer_range<uint64_t>(18446744073709551616.0));
    }
    SECTION("int32_t")
    {
        CHECK(in_integer_range<int32_t>(2147483647.4));
        CHECK(in_integer_range<int32_t>(-2147483648.5));
        CHECK_FALSE(in_integer_range<int32_t>(2147483648.0));
        CHECK_FALSE(in_integer_range<int32_t>(-2147483649.0));
    }
    SECTION("not a number")
    {
        CHECK_FALSE(in_integer_range<int64_t>(std::nan("")));
        CHECK_FALSE(in_integer_range<int64_t>(std::numeric_limits<double>::infinity()));
        CHECK_FALSE(in_integer_range<int64_t>(-std::numeric_limits<double>::infinity()));
    }
}

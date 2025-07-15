// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>

#include <map>
#include <utility>
#include <vector>

#include <jsoncons/reflect/decode_traits.hpp>

#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("decode_traits primitive")
{
    SECTION("is_primitive")
    {
        CHECK(ext_traits::is_primitive<uint64_t>::value);
    }
    SECTION("uint64_t")
    {
        std::string input = R"(1000)";

        std::error_code ec;

        json_string_cursor cursor(input);
        auto result = reflect::decode_traits<uint64_t>::try_decode(make_alloc_set(), cursor);
        REQUIRE(result);

        CHECK(result.value() == 1000);
    }
    SECTION("vector of uint64_t")
    {
        std::string input = R"([1000,1001,1002])";

        json_string_cursor cursor(input);
        auto result = reflect::decode_traits<std::vector<uint64_t>>::try_decode(make_alloc_set(), cursor);
        REQUIRE(result);

        std::vector<uint64_t>& val(*result);
        REQUIRE(3 == val.size());
        CHECK(val[0] == 1000);
        CHECK(val[1] == 1001);
        CHECK(val[2] == 1002);
    }
}

TEST_CASE("decode_traits std::string")
{
    SECTION("is_string")
    {
        CHECK(ext_traits::is_string<std::string>::value);
    }
    SECTION("string")
    {
        std::string input = R"("Hello World")";

        json_string_cursor cursor(input);
        auto result = reflect::decode_traits<std::string>::try_decode(make_alloc_set(), cursor);
        REQUIRE(result);

        CHECK((*result == "Hello World"));
    }
}

TEST_CASE("decode_traits std::pair")
{
    SECTION("std::pair<std::string,std::string>")
    {
        using value_type = std::pair<std::string,std::string>;

        std::string input = R"(["first","second"])";

        json_string_cursor cursor(input);
        auto result = reflect::decode_traits<value_type>::try_decode(make_alloc_set(), cursor);
        REQUIRE(result);

        value_type& val(*result);
        CHECK((val == value_type("first","second")));
    }
    SECTION("vector of std::pair<std::string,std::string>")
    {
        std::string input = R"([["first","second"],["one","two"]])";
        using value_type = std::vector<std::pair<std::string,std::string>>;

        json_string_cursor cursor(input);
        auto result = reflect::decode_traits<value_type>::try_decode(make_alloc_set(), cursor);
        REQUIRE(result);

        value_type& val(*result);

        REQUIRE(2 == val.size());
        CHECK((val[0] == value_type::value_type("first","second")));
        CHECK((val[1] == value_type::value_type("one","two")));
    }
    SECTION("map of std::string-std::pair<int,double>")
    {
        std::string input = R"({"foo": [100,1.5],"bar" : [200,2.5]})";
        using value_type = std::map<std::string,std::pair<int,double>>;

        json_string_cursor cursor(input);
        auto result = reflect::decode_traits<value_type>::try_decode(make_alloc_set(), cursor);
        REQUIRE(result);

        value_type& val(*result);

        REQUIRE(2 == val.size());
        REQUIRE(val.count("foo") > 0);
        REQUIRE(val.count("bar") > 0);
        CHECK(val["foo"].first == 100);
        CHECK(val["foo"].second == 1.5);
        CHECK(val["bar"].first == 200);
        CHECK(val["bar"].second == 2.5);
    }
    SECTION("Conversion error")
    {
        std::string input = R"({"foo": [100,1.5,30],"bar" : [200,2.5]])";
        using value_type = std::map<std::string,std::pair<int,double>>;

        json_string_cursor cursor(input);
        auto result = reflect::decode_traits<value_type>::try_decode(make_alloc_set(), cursor);
        REQUIRE_FALSE(result);
        CHECK(conv_errc::not_pair == result.error().code());
    }
}

TEST_CASE("decode_traits deserialization errors")
{
    SECTION("Expected comma or right brace")
    {
        std::string input = R"({"foo": [100,1.5],"bar" : [200,2.5]])";
        using value_type = std::map<std::string,std::pair<int,double>>;

        json_string_cursor cursor(input);
        auto result = reflect::decode_traits<value_type>::try_decode(make_alloc_set(), cursor);
        REQUIRE_FALSE(result);
        CHECK(json_errc::expected_comma_or_rbrace == result.error().code());
    }
}

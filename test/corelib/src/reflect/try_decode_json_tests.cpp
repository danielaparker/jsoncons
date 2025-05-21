// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/reflect/decode_json.hpp>
#include <jsoncons/reflect/json_conv_traits.hpp>
#include <jsoncons/json_traits_macros.hpp>

#include <map>
#include <vector>

#include <catch/catch.hpp>

namespace {
namespace ns {

    struct book1a
    {
        std::string author;
        std::string title;
        double price;
    };

} // namespace ns
} // namespace 

//JSONCONS_ALL_MEMBER_TRAITS(ns::book1a, author, title, price)

TEST_CASE("reflect decode json")
{
    SECTION("decode array")
    {
        std::string s = R"([false,"foo"])";
        auto result = jsoncons::reflect::try_decode_json<jsoncons::json>(s);

        REQUIRE(result);
        REQUIRE(result.has_value());
        REQUIRE(result.value().is_array());
        REQUIRE(result.value().size() == 2);
        CHECK(result.value()[0] == false);
        CHECK(result.value()[1] == "foo");
    }
    SECTION("decode object")
    {
        std::string s = R"({"a" : 1, "b" : 2})";
        auto result = jsoncons::reflect::try_decode_json<jsoncons::json>(s);

        REQUIRE(result);
        REQUIRE(result.has_value());
        REQUIRE(result.value().is_object());
        REQUIRE(result.value().size() == 2);
        CHECK(result.value().at("a") == 1);
        CHECK(result.value().at("b") == 2);
    }
}

TEST_CASE("reflect decode json with error")
{
    SECTION("decode array")
    {
        std::string s = R"([false,"foo")";
        auto result = jsoncons::reflect::try_decode_json<jsoncons::json>(s);

        REQUIRE(!result);
    }
    SECTION("decode object")
    {
        std::string s = R"({"a" : 1, "b : 2})";
        auto result = jsoncons::reflect::try_decode_json<jsoncons::json>(s);

        REQUIRE(!result);
    }
}

TEST_CASE("reflect decode tuple")
{
    SECTION("decode array to tuple")
    {
        std::string s = R"([false, 1, "foo"])"; 
        auto result = jsoncons::reflect::try_decode_json<std::tuple<bool,int,std::string>>(s);

        REQUIRE(result);
        CHECK(get<0>(result.value()) == false);
    }
}

TEST_CASE("cursor to basic_json")
{
    SECTION("single")
    {
        std::string s = R"("foo")"; 
        jsoncons::json_string_cursor cursor(s);
        std::error_code ec;
        auto j = jsoncons::reflect::to_basic_json(cursor, ec);
        REQUIRE_FALSE(ec);
        std::cout << j << "\n";
    }
    SECTION("array")
    {
        std::string s = R"([false, 1, "foo"])";
        jsoncons::json_string_cursor cursor(s);
        std::error_code ec;
        auto j = jsoncons::reflect::to_basic_json(cursor, ec);
        REQUIRE_FALSE(ec);
        REQUIRE(j.is_array());
        //std::cout << j << "\n";
    }
    SECTION("array of arrays")
    {
        std::string s = R"([[null, false, true], [1.5, 123456]])";
        jsoncons::json_string_cursor cursor(s);
        std::error_code ec;
        auto j = jsoncons::reflect::to_basic_json(cursor, ec);
        REQUIRE_FALSE(ec);
        REQUIRE(j.is_array());
        //std::cout << j << "\n";
    }

    SECTION("object")
    {
        std::string s = R"(
{
    "short" : "bar",
    "long" : "string to long for short string"
}
    )";
        jsoncons::json_string_cursor cursor(s);
        std::error_code ec;
        auto j = jsoncons::reflect::to_basic_json(cursor, ec);
        REQUIRE_FALSE(ec);
        REQUIRE(j.is_object());
        std::cout << j << "\n";
    }
    SECTION("object with nested containers")
    {
        std::string s = R"(
{"foo" : [{"short" : "bar",
          "long" : "string to long for short string", 
          "false" : false, 
          "true" : true,
          "null" : null,
          "integer" : 10,
          "double" : 1000.1}]
}
    )";
        jsoncons::json_string_cursor cursor(s);
        std::error_code ec;
        auto j = jsoncons::reflect::to_basic_json(cursor, ec);
        REQUIRE_FALSE(ec);
        REQUIRE(j.is_object());
        std::cout << j << "\n";
    }
}

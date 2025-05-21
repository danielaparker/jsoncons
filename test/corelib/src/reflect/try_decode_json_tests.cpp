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
    std::string s = R"([false, 1, "foo"])"; 
    auto result = jsoncons::reflect::try_decode_json<std::tuple<bool,int,std::string>>(s);

    REQUIRE(result);
    std::cout << get<0>(result.value()) << "\n";
}

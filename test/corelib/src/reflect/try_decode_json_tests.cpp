// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/reflect/decode_json.hpp>
#include <jsoncons/reflect/json_conv_traits.hpp>
#include <jsoncons/json_traits_macros.hpp>

#include <map>
#include <vector>
#include <tuple>

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

JSONCONS_ALL_MEMBER_TRAITS(ns::book1a, author, title, price)
TEST_CASE("default to json_type_traits tests")
{
    std::string an_author = "Haruki Murakami";
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    ns::book1a book{an_author, a_title, a_price};

    CHECK(jsoncons::is_json_type_traits_declared<ns::book1a>::value);
    //test_is_json_type_traits_declared(is_json_type_traits_declared<ns::book1a>());

    SECTION("book1a")
    {
        std::string s;

        jsoncons::encode_json(book, s);

        auto result = jsoncons::reflect::try_decode_json<jsoncons::json>(s);

        REQUIRE(result);
        REQUIRE(result.value().is<ns::book1a>() == true);

        CHECK(result.value()["author"].as<std::string>() == an_author);
        CHECK(result.value()["title"].as<std::string>() == a_title);
        CHECK(result.value()["price"].as<double>() == Approx(a_price).epsilon(0.001));

        jsoncons::json j2(book);

        CHECK(result.value() == j2);

        ns::book1a val = result.value().as<ns::book1a>();

        CHECK(val.author == book.author);
        CHECK(val.title == book.title);
        CHECK(val.price == Approx(book.price).epsilon(0.001));
    }
}

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
        
        REQUIRE_NOTHROW(jsoncons::reflect::decode_json<jsoncons::json>(s));
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

        REQUIRE_NOTHROW(jsoncons::reflect::decode_json<jsoncons::json>(s));
    }
}

TEST_CASE("reflect decode json with error")
{
    SECTION("decode array")
    {
        std::string s = R"([false,"foo")";
        auto result = jsoncons::reflect::try_decode_json<jsoncons::json>(s);

        REQUIRE(!result);
        REQUIRE_THROWS(jsoncons::reflect::decode_json<jsoncons::json>(s));
    }
    SECTION("decode object")
    {
        std::string s = R"({"a" : 1, "b : 2})";
        auto result = jsoncons::reflect::try_decode_json<jsoncons::json>(s);

        REQUIRE(!result);
        REQUIRE_THROWS(jsoncons::reflect::decode_json<jsoncons::json>(s));
    }
    SECTION("decode array to tuple")
    {
        std::string s = R"([false, 1, "foo"])"; 
        auto result = jsoncons::reflect::try_decode_json<std::tuple<bool,int,std::string>>(s);

        REQUIRE(result);
        CHECK(std::get<0>(result.value()) == false);
        REQUIRE_NOTHROW(jsoncons::reflect::decode_json<jsoncons::json>(s));
    }
    SECTION("decode array to pair")
    {
        std::string s = R"([1.5, "foo"])"; 
        auto result = jsoncons::reflect::try_decode_json<std::pair<double,std::string>>(s);

        REQUIRE(result);
        CHECK(1.5 == std::get<0>(result.value()));
        CHECK("foo" == std::get<1>(result.value()));
        REQUIRE_NOTHROW(jsoncons::reflect::decode_json<jsoncons::json>(s));
    }
}

TEST_CASE("cursor try_to_json")
{
    SECTION("single")
    {
        std::string s = R"("foo")"; 
        jsoncons::json_string_cursor cursor(s);
        std::error_code ec;
        auto j = jsoncons::try_to_json<jsoncons::json>(cursor, ec);
        REQUIRE_FALSE(ec);
        std::cout << j << "\n";
    }
    SECTION("array")
    {
        std::string s = R"([false, 1, "foo"])";
        jsoncons::json_string_cursor cursor(s);
        std::error_code ec;
        auto j = jsoncons::try_to_json<jsoncons::ojson>(cursor, ec);
        REQUIRE_FALSE(ec);
        REQUIRE(j.is_array());
        //std::cout << j << "\n";
    }
    SECTION("array of arrays")
    {
        std::string s = R"([[null, false, true], [1.5, 123456]])";
        jsoncons::json_string_cursor cursor(s);
        std::error_code ec;
        auto j = jsoncons::try_to_json<jsoncons::json>(cursor, ec);
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
        auto j = jsoncons::try_to_json<jsoncons::ojson>(cursor, ec);
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
        auto j = jsoncons::try_to_json<jsoncons::json>(cursor, ec);
        REQUIRE_FALSE(ec);
        REQUIRE(j.is_object());
        std::cout << j << "\n";
    }
}

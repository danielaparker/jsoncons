// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/decode_json.hpp>
#include <jsoncons/reflect/json_conv_traits.hpp>
#include <jsoncons/reflect/reflect_traits_gen.hpp>

#include <map>
#include <vector>
#include <tuple>

#include <catch/catch.hpp>

namespace {
namespace ns {

    struct book_all_m
    {
        std::string author;
        std::string title;
        double price;
    };

} // namespace ns
} // namespace 

JSONCONS_ALL_MEMBER_TRAITS(ns::book_all_m, author, title, price)
TEST_CASE("default to json_type_traits tests")
{
    std::string an_author = "Haruki Murakami";
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    ns::book_all_m book{an_author, a_title, a_price};

    CHECK(jsoncons::is_json_type_traits_declared<ns::book_all_m>::value);
    //test_is_json_type_traits_declared(is_json_type_traits_declared<ns::book_all_m>());

    SECTION("book_all_m")
    {
        std::string s;

        jsoncons::encode_json(book, s);

        auto result = jsoncons::try_decode_json<jsoncons::json>(s);

        REQUIRE(result);
        REQUIRE(result.value().is<ns::book_all_m>() == true);

        CHECK(result.value()["author"].as<std::string>() == an_author);
        CHECK(result.value()["title"].as<std::string>() == a_title);
        CHECK(result.value()["price"].as<double>() == Approx(a_price).epsilon(0.001));

        jsoncons::json j2(book);

        CHECK(result.value() == j2);

        ns::book_all_m val = result.value().as<ns::book_all_m>();

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
        auto result = jsoncons::try_decode_json<jsoncons::json>(s);

        REQUIRE(result);
        REQUIRE(result.has_value());
        REQUIRE(result.value().is_array());
        REQUIRE(result.value().size() == 2);
        CHECK(result.value()[0] == false);
        CHECK(result.value()[1] == "foo");
        
        REQUIRE_NOTHROW(jsoncons::decode_json<jsoncons::json>(s));
    }
    SECTION("decode object")
    {
        std::string s = R"({"a" : 1, "b" : 2})";
        auto result = jsoncons::try_decode_json<jsoncons::json>(s);

        REQUIRE(result);
        REQUIRE(result.has_value());
        REQUIRE(result.value().is_object());
        REQUIRE(result.value().size() == 2);
        CHECK(result.value().at("a") == 1);
        CHECK(result.value().at("b") == 2);

        REQUIRE_NOTHROW(jsoncons::decode_json<jsoncons::json>(s));
    }
}

TEST_CASE("reflect decode json with error")
{
    SECTION("decode array")
    {
        std::string s = R"([false,"foo")";
        auto result = jsoncons::try_decode_json<jsoncons::json>(s);

        REQUIRE(!result);
        REQUIRE_THROWS(jsoncons::decode_json<jsoncons::json>(s));
    }
    SECTION("decode object")
    {
        std::string s = R"({"a" : 1, "b : 2})";
        auto result = jsoncons::try_decode_json<jsoncons::json>(s);

        REQUIRE(!result);
        REQUIRE_THROWS(jsoncons::decode_json<jsoncons::json>(s));
    }
    SECTION("decode array to tuple")
    {
        std::string s = R"([false, 1, "foo"])"; 
        auto result = jsoncons::try_decode_json<std::tuple<bool,int,std::string>>(s);

        REQUIRE(result);
        CHECK(std::get<0>(result.value()) == false);
        REQUIRE_NOTHROW(jsoncons::decode_json<jsoncons::json>(s));
    }
    SECTION("decode array to pair")
    {
        std::string s = R"([1.5, "foo"])"; 
        auto result = jsoncons::try_decode_json<std::pair<double,std::string>>(s);

        REQUIRE(result);
        CHECK(1.5 == std::get<0>(result.value()));
        CHECK("foo" == std::get<1>(result.value()));
        REQUIRE_NOTHROW(jsoncons::decode_json<jsoncons::json>(s));
    }
}


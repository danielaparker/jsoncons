// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/allocator_set.hpp>
#include <jsoncons/decode_json.hpp>
#include <jsoncons/encode_json.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstdint>
#include <catch/catch.hpp>

namespace {
namespace ns {

    struct book
    {
        std::string author;
        std::string title;
        double price{0};
    };
    
} // namespace ns
} // namespace

JSONCONS_ALL_MEMBER_TRAITS(ns::book,author,title,price)

TEST_CASE("json_conv_traits single error tests")
{   
    SECTION("double")
    {
        auto j = jsoncons::json::parse(R"("foo")");
        REQUIRE(j.is_string());
    
        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,double>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE_FALSE(result);
        REQUIRE(jsoncons::conv_errc::not_double == result.error().code());
        //std::cout << result.error() << "\n\n";
    }
    SECTION("int64_t")
    {
        auto j = jsoncons::json::parse(R"("foo")");
        REQUIRE(j.is_string());

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,int64_t>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE_FALSE(result);
        REQUIRE(jsoncons::conv_errc::not_integer == result.error().code());
        //std::cout << result.error() << "\n\n";
    }
    SECTION("string_view")
    {
        auto j = jsoncons::json::parse(R"(100)");
        REQUIRE(j.is_number());

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,jsoncons::string_view>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE_FALSE(result);
        REQUIRE(jsoncons::conv_errc::not_string == result.error().code());
        //std::cout << result.error() << "\n\n";
    }
}

TEST_CASE("json_conv_traits as std::vector<T> tests")
{   
    SECTION("JSON is not an array")
    {
       const std::string s = R"(
{
    "author" : "Haruki Murakami",
    "title" : "Kafka on the Shore",
    "price" : 25.17
}
        )";

        auto j = jsoncons::json::parse(s);

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::vector<ns::book>>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE_FALSE(result);
        CHECK(jsoncons::conv_errc::not_vector == result.error().code());
    }
    SECTION("Invalid price")
    {
       const std::string s = R"(
[
    {
        "author" : "Haruki Murakami",
        "title" : "Kafka on the Shore",
        "price" : 25.17
    },
    {
        "author" : "Charles Bukowski",
        "title" : "Pulp",
        "price" : "foo"
    }
]
        )";

        auto j = jsoncons::json::parse(s);
        REQUIRE(j.is_array());
        REQUIRE(2 == j.size());

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::vector<ns::book>>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE_FALSE(result);
        CHECK(jsoncons::conv_errc::conversion_failed == result.error().code());
        CHECK("ns::book: price" == result.error().message_arg());
    }
}

TEST_CASE("json_conv_traits as std::map<string,T> tests")
{   
    SECTION("JSON is not an object")
    {
       const std::string s = R"(
["Haruki Murakami","Kafka on the Shore",25.17]
        )";

        auto j = jsoncons::json::parse(s);

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::map<std::string,ns::book>>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE_FALSE(result);
        CHECK(jsoncons::conv_errc::not_map == result.error().code());
    }
    SECTION("Invalid price")
    {
        const std::string s = R"(
 {
     "First prize" : {
         "author" : "Haruki Murakami",
         "title" : "Kafka on the Shore",
         "price" : 25.17
     },
     "Second prize" : {
         "author" : "Charles Bukowski",
         "title" : "Pulp",
         "price" : "foo"
     }
 }
        )";

        auto j = jsoncons::json::parse(s);
        REQUIRE(j.is_object());
        REQUIRE(2 == j.size());

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::map<std::string,ns::book>>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE_FALSE(result);
        CHECK(jsoncons::conv_errc::conversion_failed == result.error().code());
        CHECK("ns::book: price" == result.error().message_arg());
    }
}

TEST_CASE("json_conv_traits as std::map<int,T> tests")
{   
    SECTION("success")
    {
        const std::string s = R"(
 {
     "1" : {
         "author" : "Haruki Murakami",
         "title" : "Kafka on the Shore",
         "price" : 25.17
     },
     "2" : {
         "author" : "Charles Bukowski",
         "title" : "Pulp",
         "price" : 27
     }
 }
        )";

        auto j = jsoncons::json::parse(s);
        REQUIRE(j.is_object());
        REQUIRE(2 == j.size());

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::map<int,ns::book>>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE(result);
    }
    SECTION("invalid key")
    {
        const std::string s = R"(
 {
     "1" : {
         "author" : "Haruki Murakami",
         "title" : "Kafka on the Shore",
         "price" : 25.17
     },
     "foo" : {
         "author" : "Charles Bukowski",
         "title" : "Pulp",
         "price" : 27
     }
 }
        )";

        auto j = jsoncons::json::parse(s);
        REQUIRE(j.is_object());
        REQUIRE(2 == j.size());

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::map<int,ns::book>>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE_FALSE(result);
        CHECK(jsoncons::conv_errc::not_integer == result.error().code());
    }
}

using qualifying_result = std::tuple<std::size_t,std::string,std::string,std::string,std::chrono::milliseconds>;

TEST_CASE("json_conv_traits as std::tuple tests")
{
    SECTION("success")
    {
        std::string str = R"(
[
    [
        1,
        "Lewis Hamilton",
        "Mercedes",
        "1'24.303",
        0
    ],
    [
        2,
        "Valtteri Bottas",
        "Mercedes",
        "1'24.616",
        313
    ],
    [
        3,
        "Max Verstappen",
        "Red Bull",
        "1'25.325",
        1022
    ]
]        
        )";

        auto j = jsoncons::json::parse(str);
        REQUIRE(j.is_array());
        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::vector<qualifying_result>>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE(result);
    }

    SECTION("invalid epoch")
    {
        std::string str = R"(
[
    [
        1,
        "Lewis Hamilton",
        "Mercedes",
        "1'24.303",
        "foo"
    ],
    [
        2,
        "Valtteri Bottas",
        "Mercedes",
        "1'24.616",
        313
    ],
    [
        3,
        "Max Verstappen",
        "Red Bull",
        "1'25.325",
        "foo"
    ]
]        
        )";

        auto j = jsoncons::json::parse(str);
        REQUIRE(j.is_array());
        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::vector<qualifying_result>>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE_FALSE(result);
    }
}

TEST_CASE("json_conv_traits as std::pair tests")
{
    SECTION("not array")
    {
        auto j = jsoncons::json::parse(R"("foo")");

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::pair<int,int>>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE_FALSE(result);
        REQUIRE(jsoncons::conv_errc::not_pair == result.error().code());
        //std::cout << result.error() << "\n\n";
    }
    SECTION("not array of size 2")
    {
        auto j = jsoncons::json::parse(R"(["foo"])");

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::pair<int,int>>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE_FALSE(result);
        REQUIRE(jsoncons::conv_errc::not_pair == result.error().code());
        //std::cout << result.error() << "\n\n";
    }
    SECTION("invalid number")
    {
        auto j = jsoncons::json::parse(R"([10,"foo"])");
        REQUIRE(j.is_array());

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::pair<int,int>>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE_FALSE(result);
        CHECK(jsoncons::conv_errc::not_integer == result.error().code());
        //std::cout << result.error() << "\n\n";
    }
}

TEST_CASE("json_conv_traits as jsoncons::byte_string")
{
    SECTION("success")
    {
        jsoncons::json j(jsoncons::byte_string_arg, std::string("Hello World"), jsoncons::semantic_tag::none);

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,jsoncons::byte_string>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE(result);
        CHECK((jsoncons::byte_string{'H','e','l','l','o',' ','W','o','r','l','d'} == *result));
        //std::cout << result.error() << "\n\n";
    }
    SECTION("error")
    {
        jsoncons::json j(100);

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json, jsoncons::byte_string>::try_as(jsoncons::make_alloc_set(), j);
        REQUIRE(!result);
        CHECK(jsoncons::conv_errc::not_byte_string == result.error().code());
        //std::cout << result.error() << "\n\n";
    }
}

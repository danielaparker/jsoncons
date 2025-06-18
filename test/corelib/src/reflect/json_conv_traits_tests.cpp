// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
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

#if 0
TEST_CASE("json_conv_traits tests")
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
       "price" : 22.48
    }
]
    )";
    
    SECTION("test 1")
    {
        auto j = jsoncons::json::parse(s);
        REQUIRE(j.is_array());
        REQUIRE(2 == j.size());
    
        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,ns::book>::try_as(j[0]);
        //ns::book& item{result.value()};

        //std::cout << item.author << ", " 
        //          << item.title << ", " 
        //          << item.price << "\n";
    }
    
    SECTION("test 2")
    {
        auto j = jsoncons::json::parse(s);
        REQUIRE(j.is_array());
    
        std::cout << "is_json_conv_traits_declared: " << jsoncons::reflect::is_json_conv_traits_declared<std::vector<ns::book>>::value << "\n";

        std::cout << "is_compatible_array_type: " << jsoncons::reflect::is_json_conv_traits_declared<std::vector<ns::book>>::value << "\n";
        
        std::vector<ns::book> book_list = jsoncons::decode_json<std::vector<ns::book>>(s);

        ///auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::vector<ns::book>>::try_as(j);

        /*auto book_list = result.value();
    
        std::cout << "(1)\n";
        for (const auto& item : book_list)
        {
            std::cout << item.author << ", " 
                      << item.title << ", " 
                      << item.price << "\n";
        }
    
        std::cout << "\n(2)\n";
        jsoncons::encode_json(book_list, std::cout, jsoncons::indenting::indent);
        std::cout << "\n\n";*/
    }
}

TEST_CASE("json_conv_traits error tests")
{   
    SECTION("double")
    {
        auto j = jsoncons::json::parse(R"("foo")");
        REQUIRE(j.is_string());
    
        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,double>::try_as(j);
        REQUIRE_FALSE(result);
        REQUIRE(jsoncons::conv_errc::not_double == result.error().code());
        //std::cout << result.error() << "\n\n";
    }
    SECTION("int64_t")
    {
        auto j = jsoncons::json::parse(R"("foo")");
        REQUIRE(j.is_string());

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,int64_t>::try_as(j);
        REQUIRE_FALSE(result);
        REQUIRE(jsoncons::conv_errc::not_integer == result.error().code());
        //std::cout << result.error() << "\n\n";
    }
}
#endif

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

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::vector<ns::book>>::try_as(j);
        REQUIRE(!result);
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

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::vector<ns::book>>::try_as(j);
        REQUIRE(!result);
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

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::map<std::string,ns::book>>::try_as(j);
        REQUIRE(!result);
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

        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::map<std::string,ns::book>>::try_as(j);
        REQUIRE(!result);
        CHECK(jsoncons::conv_errc::conversion_failed == result.error().code());
        CHECK("ns::book: price" == result.error().message_arg());
    }
}



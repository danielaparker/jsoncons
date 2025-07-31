// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license 

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/decode_json.hpp>
#include <jsoncons/encode_json.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstdint>

using namespace jsoncons;

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
 
JSONCONS_ALL_MEMBER_TRAITS(ns::book_all_m,author,title,price)

void test_is_json_type_traits_declared(std::true_type)
{
}

TEST_CASE("JSONCONS_ALL_MEMBER_TRAITS using allocator tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    ns::book_all_m book{an_author, a_title, a_price};

    CHECK(is_json_type_traits_declared<ns::book_all_m>::value);
    test_is_json_type_traits_declared(is_json_type_traits_declared<ns::book_all_m>());

    SECTION("success")
    {
        std::string s;

        encode_json(book, s);

        json j = decode_json<json>(s);

        REQUIRE(j.is<ns::book_all_m>() == true);

        CHECK(an_author == j["author"].as<std::string>());
        CHECK(a_title == j["title"].as<std::string>()       );
        CHECK(Approx(a_price).epsilon(0.001) == j["price"].as<double>()                              );

        json j2(book);

        CHECK(j == j2);

        ns::book_all_m val = j.as<ns::book_all_m>();

        CHECK(val.author == book.author);
        CHECK(val.title == book.title);
        CHECK(val.price == Approx(book.price).epsilon(0.001));
    }
}

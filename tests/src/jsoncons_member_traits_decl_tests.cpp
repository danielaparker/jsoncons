// Copyright 2013 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstdint>

using namespace jsoncons;

namespace jsoncons_member_traits_decl_tests {

    struct book
    {
        std::string author;
        std::string title;
        double price;
    };
    struct book2
    {
        std::string author;
        std::string title;
        double price;
        std::string isbn;
    };

} // namespace jsoncons_member_traits_decl_tests
 
namespace ns = jsoncons_member_traits_decl_tests;

JSONCONS_MEMBER_TRAITS_DECL(ns::book,author,title,price)
JSONCONS_MEMBER_TRAITS_DECL(ns::book2,author,title,price,isbn)

TEST_CASE("JSONCONS_MEMBER_TRAITS_DECL tests")
{
    std::string author = "Haruki Murakami"; 
    std::string title = "Kafka on the Shore";
    double price = 25.17;

    ns::book book{author, title, price};

    SECTION("book")
    {
        std::string s;

        encode_json(book, s);

        json j = decode_json<json>(s);

        REQUIRE(j.is<ns::book>() == true);
        REQUIRE(j.is<ns::book2>() == false);

        CHECK(j["author"].as<std::string>() == author);
        CHECK(j["title"].as<std::string>() == title);
        CHECK(j["price"].as<double>() == Approx(price).epsilon(0.001));

        json j2(book);

        CHECK(j == j2);

        ns::book val = j.as<ns::book>();

        CHECK(val.author == book.author);
        CHECK(val.title == book.title);
        CHECK(val.price == Approx(book.price).epsilon(0.001));
    }
}


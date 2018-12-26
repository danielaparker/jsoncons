// Copyright 2013 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <catch/catch.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>

using namespace jsoncons;
using namespace jsoncons::jsonpath;

TEST_CASE("jsonpath function tests")
{
    const json store = json::parse(R"(
    {
        "store": {
            "book": [
                {
                    "category": "reference",
                    "author": "Nigel Rees",
                    "title": "Sayings of the Century",
                    "price": 8.95
                },
                {
                    "category": "fiction",
                    "author": "Evelyn Waugh",
                    "title": "Sword of Honour",
                    "price": 12.99
                },
                {
                    "category": "fiction",
                    "author": "Herman Melville",
                    "title": "Moby Dick",
                    "isbn": "0-553-21311-3",
                    "price": 8.99
                },
                {
                    "category": "fiction",
                    "author": "J. R. R. Tolkien",
                    "title": "The Lord of the Rings",
                    "isbn": "0-395-19395-8",
                    "price": 22.99
                }
            ],
            "bicycle": {
                "color": "red",
                "price": 19.95
            }
        }
    }
    )");

    SECTION("sum")
    {
        json result = json_query(store,"count($.store.book[*])");

        size_t expected = 4;

        REQUIRE(result.size() == 1);
        CHECK(result[0].as<size_t>() == expected);
    }

    SECTION("sum")
    {
        json result = json_query(store,"sum($.store.book[*].price)");
        double expected = 53.92;
        REQUIRE(result.size() == 1);
        CHECK(result[0].as<double>() == Approx(expected).epsilon(0.000001));

        json result2 = json_query(store,"$.store.book[?(@.price > sum($.store.book[*].price) / count($.store.book[*]))].title");
        std::cout << "result2: " << result2 << "\n";
        std::string expected2 = "The Lord of the Rings";
        REQUIRE(result2.size() == 1);
        CHECK(result2[0].as<std::string>() == expected2);
    }

    SECTION("avg")
    {
        json result = json_query(store,"avg($.store.book[*].price)");

        double expected = 13.48;

        REQUIRE(result.size() == 1);
        CHECK(result[0].as<double>() == Approx(expected).epsilon(0.000001));
    }

    SECTION("mult")
    {
        json result = json_query(store,"mult($.store.book[*].price)");

        double expected = 24028.731766049998;

        REQUIRE(result.size() == 1);
        CHECK(result[0].as<double>() == Approx(expected).epsilon(0.000001));
    }

    SECTION("min")
    {
        json result = json_query(store,"min($.store.book[*].price)");

        double expected = 8.95;

        REQUIRE(result.size() == 1);
        CHECK(result[0].as<double>() == Approx(expected).epsilon(0.000001));
    }

    SECTION("max")
    {
        json result = json_query(store,"max($.store.book[*].price)");

        double expected = 22.99;

        REQUIRE(result.size() == 1);
        CHECK(result[0].as<double>() == Approx(expected).epsilon(0.000001));
    }

    SECTION("max in filter")
    {
        std::string path = "$.store.book[?(@.price < max($.store.book[*].price))].title";

        json expected = json::parse(R"(
    ["Sayings of the Century","Sword of Honour","Moby Dick"]
        )");

        json result = json_query(store,path);

        CHECK(result == expected);
    }
}



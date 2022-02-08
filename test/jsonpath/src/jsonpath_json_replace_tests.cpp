// Copyright 2020 Daniel Parker
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
#include <unordered_set> // std::unordered_set
#include <fstream>

using namespace jsoncons;

TEST_CASE("test replace tests")
{
    json j;
    JSONCONS_TRY
    {
        j = json::parse(R"(
{ "store": {
    "book": [ 
      { "category": "reference",
        "author": "Nigel Rees",
        "title": "Sayings of the Century",
        "price": 8.95
      },
      { "category": "fiction",
        "author": "Evelyn Waugh",
        "title": "Sword of Honour",
        "price": 12.99
      },
      { "category": "fiction",
        "author": "Herman Melville",
        "title": "Moby Dick",
        "isbn": "0-553-21311-3",
        "price": 8.99
      }
    ]
  }
}
)");
    }
    JSONCONS_CATCH (const ser_error& e)
    {
        std::cout << e.what() << std::endl;
    }

    SECTION("test 1")
    {
        jsonpath::json_replace(j,"$..book[?(@.price==12.99)].price", 30.9);

        CHECK(30.9 == Approx(j["store"]["book"][1]["price"].as<double>()).epsilon(0.001));
    }

    SECTION("test 2")
    {
        std::string expr = "$.store.book[*].price";

        // make a discount on all books
        jsonpath::json_replace(j, expr,
            [](const std::string&,json& price) { price = std::round(price.as<double>() - 1.0); });

        CHECK(8.0 == Approx(j["store"]["book"][0]["price"].as<double>()).epsilon(0.001));
        CHECK(12.0 == Approx(j["store"]["book"][1]["price"].as<double>()).epsilon(0.001));
        CHECK(8.0 == Approx(j["store"]["book"][2]["price"].as<double>()).epsilon(0.001));
    }

    SECTION("legacy test")
    {
        std::string expr = "$.store.book[*].price";

        // make a discount on all books
        jsonpath::json_replace(j, expr,
            [](const json& price) { return std::round(price.as<double>() - 1.0); });

        CHECK(8.0 == Approx(j["store"]["book"][0]["price"].as<double>()).epsilon(0.001));
        CHECK(12.0 == Approx(j["store"]["book"][1]["price"].as<double>()).epsilon(0.001));
        CHECK(8.0 == Approx(j["store"]["book"][2]["price"].as<double>()).epsilon(0.001));
    }
}


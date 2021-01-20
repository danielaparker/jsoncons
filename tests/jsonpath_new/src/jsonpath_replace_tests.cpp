// Copyright 2013 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath_new/json_query.hpp>
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
{"store":
{"book": [
{"category": "reference",
"author": "Margaret Weis",
"title": "Dragonlance Series",
"price": 31.96}, {"category": "reference",
"author": "Brent Weeks",
"title": "Night Angel Trilogy",
"price": 14.70
}]}}
)");
    }
    JSONCONS_CATCH (const ser_error& e)
    {
        std::cout << e.what() << std::endl;
    }

    //std::cout << "!!!test_replace" << std::endl;
    //std::cout << ("1\n") << pretty_print(j) << std::endl;

    SECTION("test 1")
    {
        CHECK(31.96 == Approx(j["store"]["book"][0]["price"].as<double>()).epsilon(0.001));

        jsonpath_new::json_replace(j,"$..book[?(@.price==31.96)].price", 30.9);

        //CHECK(30.9 == Approx(j["store"]["book"][0]["price"].as<double>()).epsilon(0.001));
    }

    //std::cout << ("2\n") << pretty_print(j) << std::endl;
}


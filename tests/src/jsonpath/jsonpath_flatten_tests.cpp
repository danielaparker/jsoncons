// Copyright 2017 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/flatten.hpp>
#include <catch/catch.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>

using namespace jsoncons;

TEST_CASE("jsonpath flatten test")
{
    json input = json::parse(R"(
    {
       "application": "hiking",
       "reputons": [
           {
               "rater": "HikingAsylum",
               "assertion": "advanced",
               "rated": "Marilyn C",
               "rating": 0.90
            },
            {
               "rater": "HikingAsylum",
               "assertion": "intermediate",
               "rated": "Hongmin",
               "rating": 0.75
            }    
        ]
    }
    )");

    SECTION("flatten")
    {
        json result = jsonpath::flatten(input);

        REQUIRE(result.is_object());
        REQUIRE(result.size() == 9);

        //std::cout << pretty_print(result) << "\n";

        CHECK(result["$['application']"].as<std::string>() == std::string("hiking"));
        CHECK(result["$['reputons'][0]['assertion']"].as<std::string>() == std::string("advanced"));
        CHECK(result["$['reputons'][0]['rated']"].as<std::string>() == std::string("Marilyn C"));
        CHECK(result["$['reputons'][0]['rater']"].as<std::string>() == std::string("HikingAsylum"));
        CHECK(result["$['reputons'][0]['rating']"].as<double>() == Approx(0.9).epsilon(0.0000001));
        CHECK(result["$['reputons'][1]['assertion']"].as<std::string>() == std::string("intermediate"));
        CHECK(result["$['reputons'][1]['rated']"].as<std::string>() == std::string("Hongmin"));
        CHECK(result["$['reputons'][1]['rater']"].as<std::string>() == std::string("HikingAsylum"));
        CHECK(result["$['reputons'][1]['rating']"].as<double>() == Approx(0.75).epsilon(0.0000001));

        //std::cout << pretty_print(result) << "\n";
    }

    SECTION("unflatten")
    {
        json result = jsonpath::flatten(input);
        //std::cout << pretty_print(result) << "\n";

        json original = jsonpath::unflatten(result);
        //std::cout << pretty_print(original) << "\n";
        CHECK(original == input);
    }
}

TEST_CASE("jsonpath flatten array test")
{
    json input = json::parse(R"([1,2,3,"4\u0027s"])");

    SECTION("flatten array and unflatten")
    {
        json result = jsonpath::flatten(input);
        //std::cout << pretty_print(result) << "\n";

        json original = jsonpath::unflatten(result);
        //std::cout << pretty_print(original) << "\n";
        CHECK(original == input);
    }

}

TEST_CASE("jsonpath flatten with single quote test")
{
    json input = json::parse(R"(
    {
       "like'd": "pizza"
    }
    )");

    SECTION("flatten array and unflatten")
    {
        json result = jsonpath::flatten(input);

        json original = jsonpath::unflatten(result);
        //std::cout << pretty_print(original) << "\n";
        CHECK(original == input);
    }

}

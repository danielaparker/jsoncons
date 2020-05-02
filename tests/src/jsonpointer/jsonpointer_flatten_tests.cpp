// Copyright 2017 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <catch/catch.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>

using namespace jsoncons;
TEST_CASE("jsonpointer unflatten tests 1")
{
    SECTION("test 1")
    {
        json input = json::parse(R"(
        {
            "discards": {
                "1000": "Record does not exist",
                "1004": "Queue limit exceeded",
                "1010": "Discarding timed-out partial msg"
            },
            "warnings": {
                "0": "Phone number missing country code",
                "1": "State code missing",
                "2": "Zip code missing"
            }
        }
        )");

        json flattened = jsonpointer::flatten(input);

        json unflattened1 = jsonpointer::unflatten(flattened);
        //std::cout << "(1)\n" << pretty_print(unflattened1) << "\n";

        json unflattened2 = jsonpointer::unflatten(flattened,
            jsonpointer::unflatten_options::assume_object);
        //std::cout << "(2)\n" << pretty_print(unflattened2) << "\n";
    }
}

TEST_CASE("jsonpointer unflatten tests 2")
{
    json input = json::parse(R"(
        {
            "0": {
                "1000": "Record does not exist",
                "1004": "Queue limit exceeded",
                "1010": "Discarding timed-out partial msg"
            },
            "1": {
                "0": "Phone number missing country code",
                "1": "State code missing",
                "2": "Zip code missing"
            }
        }
        )");

    json flattened = jsonpointer::flatten(input);

    SECTION("default test")
    {
        json expected = json::parse(R"(
        [
            {
                "1000": "Record does not exist",
                "1004": "Queue limit exceeded",
                "1010": "Discarding timed-out partial msg"
            },
            ["Phone number missing country code", "State code missing", "Zip code missing"]
        ]        
        )");

        json unflattened = jsonpointer::unflatten(flattened);
        CHECK(unflattened == expected);
        //std::cout << "(1)\n" << pretty_print(unflattened) << "\n";
    }
    SECTION("object test")
    {
        json expected = json::parse(R"(
        {
            "0": {
                "1000": "Record does not exist",
                "1004": "Queue limit exceeded",
                "1010": "Discarding timed-out partial msg"
            },
            "1": {
                "0": "Phone number missing country code",
                "1": "State code missing",
                "2": "Zip code missing"
            }
        }        
        )");

        json unflattened = jsonpointer::unflatten(flattened, jsonpointer::unflatten_options::assume_object);
        CHECK(unflattened == expected);
        //std::cout << "(2)\n" << pretty_print(unflattened) << "\n";
    }
}

TEST_CASE("flatten test")
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
        json result = jsonpointer::flatten(input);

        REQUIRE(result.is_object());
        REQUIRE(result.size() == 9);

        CHECK(result["/application"].as<std::string>() == std::string("hiking"));
        CHECK(result["/reputons/0/assertion"].as<std::string>() == std::string("advanced"));
        CHECK(result["/reputons/0/rated"].as<std::string>() == std::string("Marilyn C"));
        CHECK(result["/reputons/0/rater"].as<std::string>() == std::string("HikingAsylum"));
        CHECK(result["/reputons/0/rating"].as<double>() == Approx(0.9).epsilon(0.0000001));
        CHECK(result["/reputons/1/assertion"].as<std::string>() == std::string("intermediate"));
        CHECK(result["/reputons/1/rated"].as<std::string>() == std::string("Hongmin"));
        CHECK(result["/reputons/1/rater"].as<std::string>() == std::string("HikingAsylum"));
        CHECK(result["/reputons/1/rating"].as<double>() == Approx(0.75).epsilon(0.0000001));

        //std::cout << pretty_print(result) << "\n";
        json unflattened = jsonpointer::unflatten(result);
        CHECK(unflattened == input);
        //std::cout << pretty_print(unflattened) << "\n";

    }
}


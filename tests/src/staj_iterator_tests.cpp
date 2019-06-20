// Copyright 2018 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_cursor.hpp>
#include <jsoncons/json_decoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("array_iterator test")
{
    std::string s = R"(
    [
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55              
        },
        {
            "enrollmentNo" : 101,
            "firstName" : "Catherine",
            "lastName" : "Smith",
            "mark" : 95
        },
        {
            "enrollmentNo" : 102,
            "firstName" : "William",
            "lastName" : "Skeleton",
            "mark" : 60              
        }
    ]
    )";

    json_cursor reader(s);

    staj_array_iterator<json> it(reader);
    staj_array_iterator<json> end;

    const auto& j1 = *it;
    CHECK(j1["firstName"].as<std::string>() == std::string("Tom"));
    ++it;
    const auto& j2 = *it;
    CHECK(j2["firstName"].as<std::string>() == std::string("Catherine"));
    ++it;
    const auto& j3 = *it;
    CHECK(j3["firstName"].as<std::string>() == std::string("William"));
    ++it;
    CHECK((it == end));
}

TEST_CASE("object_iterator test")
{
    std::string s = R"(
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55              
        }
    )";

    std::istringstream is(s);
    json_cursor reader(is);
    staj_object_iterator<json> it(reader);
    staj_object_iterator<json> end;

    const auto& p1 = *it;
    CHECK(p1.second.as<int>() == 100);
    ++it;
    const auto& p2 = *it;
    CHECK(p2.second.as<std::string>() == std::string("Tom"));
    ++it;
    const auto& p3 = *it;
    CHECK(p3.second.as<std::string>() == std::string("Cochrane"));
    ++it;
    const auto& p4 = *it;
    CHECK(p4.second.as<int>() == 55);
    ++it;
    CHECK((it == end));
}





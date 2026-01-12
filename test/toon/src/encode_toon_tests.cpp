// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/toon/encode_toon.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <iostream>

using namespace jsoncons;

TEST_CASE("toon is_number")
{
    SECTION("test1")
    {
        CHECK_FALSE(toon::detail::is_number("-"));
        CHECK(toon::detail::is_number("-0"));
        CHECK(toon::detail::is_number("-1"));
        CHECK_FALSE(toon::detail::is_number("-0a"));
        CHECK_FALSE(toon::detail::is_number("00"));
        CHECK_FALSE(toon::detail::is_number("-00"));
        CHECK_FALSE(toon::detail::is_number("0-0"));
        CHECK(toon::detail::is_number("-0.0"));
        CHECK(toon::detail::is_number("-1.1"));
        CHECK_FALSE(toon::detail::is_number("-0.0a"));
        CHECK(toon::detail::is_number("0"));
        CHECK(toon::detail::is_number("1"));
        CHECK(toon::detail::is_number("123456789"));
        CHECK_FALSE(toon::detail::is_number("123456789."));
        CHECK(toon::detail::is_number("123456789.0"));
        CHECK_FALSE(toon::detail::is_number("foo"));
        CHECK_FALSE(toon::detail::is_number("-foo"));
    }
}

TEST_CASE("toon array")
{
    SECTION("empty array")
    {
        std::string expected = R"([0]:)";
        std::string buffer;
        ojson j{json_array_arg};
        toon::encode_toon(j, buffer);
        CHECK(expected == buffer);
    }
    SECTION("array of empty arrays")
    {
        std::string expected = R"([2]:
  - [0]:
  - [0]:)";
        std::string buffer;
        auto j = ojson::parse(R"([[],[]])");
        toon::encode_toon(j, buffer);
        CHECK(expected == buffer);
    }
    SECTION("array of arrays of primitives")
    {
        std::string str = R"([["Foo","Bar"],[1,2,3]])";
        auto j = ojson::parse(str);

        std::string expected = R"([2]:
  - [2]: Foo,Bar
  - [3]: 1,2,3)";
        std::string buffer;
        toon::encode_toon(j, buffer);
        CHECK(expected == buffer);
    }
}
TEST_CASE("toon object")
{
    SECTION("object")
    {
        std::string str = R"({"foo":{}})";
        auto j = ojson::parse(str);

        std::string expected = R"(foo:)";
        std::string buffer;
        toon::encode_toon(j, buffer);
        CHECK(expected == buffer);
    }
    SECTION("array of objects of primitives")
    {
        std::string str = R"([{"foo":1,"bar":2},{"foo":3,"bar":4}])";
        auto j = ojson::parse(str);

        std::string expected = R"([2]{foo,bar}:
  1,2
  3,4)";
        std::string buffer;
        toon::encode_toon(j, buffer);
        CHECK(expected == buffer);
    }
    SECTION("mixed array as list items")
    {
        std::string str = R"([{"foo":1,"bar":2},{"foo":3,"bar":4,"baz":5}])";
        auto j = ojson::parse(str);

        std::string expected = R"([2]:
  - foo: 1
    bar: 2
  - foo: 3
    bar: 4
    baz: 5)";
        std::string buffer;
        toon::encode_toon(j, buffer);
        CHECK(expected == buffer);
    }
    SECTION("nested object")
    {
        std::string str = R"({
  "foo": {
    "bar": 1,
    "baz": 2
  },
  "quux": [1, 2, 3]
})";
        auto j = ojson::parse(str);

        std::string expected = R"(foo:
  bar: 1
  baz: 2
quux[3]: 1,2,3)";

        std::string buffer;
        toon::encode_toon(j, buffer);
        CHECK(expected == buffer);

    }
}

TEST_CASE("toon examples")
{
    SECTION("Example 1")
    {
        std::string str = R"({
  "context": {
    "task": "Our favorite hikes together",
    "location": "Boulder",
    "season": "spring_2025"
  },
  "friends": ["ana", "luis", "sam"],
  "hikes": [
    {
      "id": 1,
      "name": "Blue Lake Trail",
      "distanceKm": 7.5,
      "elevationGain": 320,
      "companion": "ana",
      "wasSunny": true
    },
    {
      "id": 2,
      "name": "Ridge Overlook",
      "distanceKm": 9.2,
      "elevationGain": 540,
      "companion": "luis",
      "wasSunny": false
    },
    {
      "id": 3,
      "name": "Wildflower Loop",
      "distanceKm": 5.1,
      "elevationGain": 180,
      "companion": "sam",
      "wasSunny": true
    }
  ]
}
        )";
        auto j = ojson::parse(str);

        std::string expected = R"(context:
  task: Our favorite hikes together
  location: Boulder
  season: spring_2025
friends[3]: ana,luis,sam
hikes[3]{id,name,distanceKm,elevationGain,companion,wasSunny}:
  1,Blue Lake Trail,7.5,320,ana,true
  2,Ridge Overlook,9.2,540,luis,false
  3,Wildflower Loop,5.1,180,sam,true)";

        std::string buffer;
        toon::encode_toon(j, buffer);
        CHECK(expected == buffer);
    }
}
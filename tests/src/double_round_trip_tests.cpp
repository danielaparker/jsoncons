// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>

using namespace jsoncons;

TEST_CASE("test_precision")
{
    std::string s = "42.229999999999997";
    json val = json::parse(s);
    CHECK(17 == val.precision());

    s = "0.42229999999999997";
    val = json::parse(s);
    REQUIRE(structure_tag_type::double_tag == val.structure_tag());
    CHECK(17 == val.precision()); // max is std::numeric_limits<double>::max_digits10

    val = json::parse("1.2345e+30");
    CHECK(5 == val.precision());

    val = json::parse("0.12345e+30");
    CHECK(6 == val.precision());

    val = json::parse("1.2345E+30");
    CHECK(5 == val.precision());

    val = json::parse("0.12345E+30");
    CHECK(6 == val.precision());

    val = json::parse("1.2345e-30");
    CHECK(5 == val.precision());

    val = json::parse("0.12345e-30");
    CHECK(6 == val.precision());

    val = json::parse("1.2345E-30");
    CHECK(5 == val.precision());

    val = json::parse("0.12345E-30");
    CHECK(6 == val.precision());

    val = json::parse("1.2345e30");
    CHECK(5 == val.precision());

    val = json::parse("0.12345e30");
    CHECK(6 == val.precision());

    val = json::parse("1.2345E30");
    CHECK(5 == val.precision());

    val = json::parse("0.12345E30");
    CHECK(6 == val.precision());

    val = json::parse("0E30");
    CHECK(1 == val.precision());

    val = json::parse("1E30");
    CHECK(1 == val.precision());

    val = json::parse("12E30");
    CHECK(2 == val.precision());
}

TEST_CASE("test_round_trip")
{
    std::string input = "42.229999999999997";
    CHECK(input == json::parse(input).as<std::string>());

    input = "9.0099999999999998";
    CHECK(input == json::parse(input).as<std::string>());

    input = "13.449999999999999";
    CHECK(input == json::parse(input).as<std::string>());

    input = "0.000071";
    CHECK(input == json::parse(input).as<std::string>());
}


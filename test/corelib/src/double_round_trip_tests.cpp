// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <ctime>
#include <sstream>
#include <utility>
#include <vector>

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>

#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("test_round_trip")
{
    {
        std::ostringstream os;
        double d = 42.229999999999997;
        json j = d;
        os << j;
        CHECK(json::parse(os.str()).as<double>() == d);
    }
    {
        std::ostringstream os;
        double d = 9.0099999999999998;
        json j = d;
        os << j;
        CHECK(json::parse(os.str()).as<double>() == d);
    }
    {
        std::ostringstream os;
        double d = 13.449999999999999;
        json j = d;
        os << j;
        CHECK(json::parse(os.str()).as<double>() == d);
    }
    {
        std::ostringstream os;
        double d = 0.000071;
        json j = d;
        os << j;
        CHECK(json::parse(os.str()).as<double>() == d);
    }
}


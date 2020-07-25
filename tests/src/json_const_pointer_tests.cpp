// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>
#include <iterator>

using namespace jsoncons;

TEST_CASE("json_const_pointer array tests")
{
    json j = json::parse(R"( ["one", "two", "three"] )");

    SECTION("size()")
    {
        json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_array());
        CHECK(v.size() == 3);
    }
}

TEST_CASE("json_const_pointer object tests")
{
    json j = json::parse(R"( {"one" : 1, "two" : 2, "three" : 3} )");

    SECTION("size()")
    {
        json v(json_const_pointer_arg, &j);
        REQUIRE(v.is_object());
        CHECK(v.size() == 3);
    }
}


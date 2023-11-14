// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json_location.hpp>
#include <catch/catch.hpp>
#include <iostream>

using namespace jsoncons;

TEST_CASE("test json_location")
{
    SECTION("test 1")
    {
        json_location loc;
        loc /= "foo";
        loc /= 1;

        CHECK(loc.size() == 2);
        CHECK(loc[0].element_kind() == path_element_kind::name);
        CHECK(loc[0].name() == "foo");
        CHECK(loc[1].element_kind() == path_element_kind::index);
        CHECK(loc[1].index() == 1);
    }
}


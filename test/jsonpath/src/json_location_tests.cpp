// Copyright 2021 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons_ext/jsonpath/json_location.hpp>
#include <catch/catch.hpp>
#include <iostream>

using json_location_node = jsoncons::jsonpath::json_location_node<std::string>;
using json_location = jsoncons::jsonpath::json_location<std::string>;

TEST_CASE("test json_location equals")
{
    json_location_node component1('$');
    json_location_node component2(&component1,"foo");
    json_location_node component3(&component2,"bar");
    json_location_node component4(&component3,0);

    json_location_node component11('$');
    json_location_node component12(&component11,"foo");
    json_location_node component13(&component12,"bar");
    json_location_node component14(&component13,0);

    json_location path1(component4);
    json_location path2(component14);

    CHECK(path1 == path2);
}

TEST_CASE("test json_location to_string")
{
    json_location_node component1('$');
    json_location_node component2(&component1,"foo");
    json_location_node component3(&component2,"bar");
    json_location_node component4(&component3,0);

    json_location path1(component4);

    CHECK(path1.to_string() == std::string("$['foo']['bar'][0]"));
}

TEST_CASE("test json_location with solidus to_string")
{
    json_location_node component1('$');
    json_location_node component2(&component1,"foo's");
    json_location_node component3(&component2,"bar");
    json_location_node component4(&component3,0);

    json_location path1(component4);

    CHECK(path1.to_string() == std::string(R"($['foo\'s']['bar'][0])"));
}



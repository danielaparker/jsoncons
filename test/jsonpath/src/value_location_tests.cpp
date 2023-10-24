// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons_ext/jsonpath/value_location.hpp>
#include <catch/catch.hpp>
#include <iostream>

using jsonpath_node = jsoncons::jsonpath::jsonpath_node<std::string>;
using value_location = jsoncons::jsonpath::value_location<std::string>;

TEST_CASE("test value_location equals")
{
    jsonpath_node component1("$");
    jsonpath_node component2(&component1,"foo");
    jsonpath_node component3(&component2,"bar");
    jsonpath_node component4(&component3,0);

    jsonpath_node component11("$");
    jsonpath_node component12(&component11,"foo");
    jsonpath_node component13(&component12,"bar");
    jsonpath_node component14(&component13,0);

    value_location path1(component4);
    value_location path2(component14);

    CHECK(path1 == path2);
}

TEST_CASE("test value_location to_string")
{
    jsonpath_node component1("$");
    jsonpath_node component2(&component1,"foo");
    jsonpath_node component3(&component2,"bar");
    jsonpath_node component4(&component3,0);

    value_location path1(component4);

    CHECK(path1.to_string() == std::string("$['foo']['bar'][0]"));
}

TEST_CASE("test value_location with solidus to_string")
{
    jsonpath_node component1("$");
    jsonpath_node component2(&component1,"foo's");
    jsonpath_node component3(&component2,"bar");
    jsonpath_node component4(&component3,0);

    value_location path1(component4);

    CHECK(path1.to_string() == std::string(R"($['foo\'s']['bar'][0])"));
}



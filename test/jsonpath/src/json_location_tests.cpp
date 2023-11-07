// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_location.hpp>
#include <catch/catch.hpp>
#include <iostream>

using path_node = jsoncons::jsonpath::path_node<jsoncons::json>;
using json_location = jsoncons::jsonpath::json_location<jsoncons::json>;

TEST_CASE("test json_location equals")
{
    path_node component1('$');
    path_node component2(&component1,"foo");
    path_node component3(&component2,"bar");
    path_node component4(&component3,0);

    path_node component11('$');
    path_node component12(&component11,"foo");
    path_node component13(&component12,"bar");
    path_node component14(&component13,0);

    json_location path1(component4);
    json_location path2(component14);

    CHECK((path1 == path2));
}

TEST_CASE("test json_location to_string")
{
    path_node component1('$');
    path_node component2(&component1,"foo");
    path_node component3(&component2,"bar");
    path_node component4(&component3,0);

    json_location path1(component4);

    CHECK(path1.to_string() == std::string("$['foo']['bar'][0]"));
}

TEST_CASE("test json_location with solidus to_string")
{
    path_node component1('$');
    path_node component2(&component1,"foo's");
    path_node component3(&component2,"bar");
    path_node component4(&component3,0);

    json_location path1(component4);

    CHECK(path1.to_string() == std::string(R"($['foo\'s']['bar'][0])"));
}


// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_location.hpp>
#include <catch/catch.hpp>
#include <iostream>

using basic_path_node = jsoncons::jsonpath::basic_path_node<char>;
using jsoncons::jsonpath::json_location;

TEST_CASE("test json_location equals")
{
    basic_path_node component1('$');
    basic_path_node component2(&component1,"foo");
    basic_path_node component3(&component2,"bar");
    basic_path_node component4(&component3,0);

    basic_path_node component11('$');
    basic_path_node component12(&component11,"foo");
    basic_path_node component13(&component12,"bar");
    basic_path_node component14(&component13,0);

    json_location path1(component4);
    json_location path2(component14);

    CHECK((path1 == path2));
}

TEST_CASE("test json_location to_string")
{
    basic_path_node component1('$');
    basic_path_node component2(&component1,"foo");
    basic_path_node component3(&component2,"bar");
    basic_path_node component4(&component3,0);

    json_location path1(component4);

    CHECK(path1.to_string() == std::string("$['foo']['bar'][0]"));
}

TEST_CASE("test json_location with solidus to_string")
{
    basic_path_node component1('$');
    basic_path_node component2(&component1,"foo's");
    basic_path_node component3(&component2,"bar");
    basic_path_node component4(&component3,0);

    json_location path1(component4);

    CHECK(path1.to_string() == std::string(R"($['foo\'s']['bar'][0])"));
}


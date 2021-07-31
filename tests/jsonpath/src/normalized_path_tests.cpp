// Copyright 2021 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons_ext/jsonpath/normalized_path.hpp>
#include <catch/catch.hpp>
#include <iostream>

using normalized_path_node = jsoncons::jsonpath::normalized_path_node<char>;
using normalized_path = jsoncons::jsonpath::normalized_path<char>;

TEST_CASE("test normalized_path equals")
{
    normalized_path_node component1('$');
    normalized_path_node component2(&component1,"foo");
    normalized_path_node component3(&component2,"bar");
    normalized_path_node component4(&component3,0);

    normalized_path_node component11('$');
    normalized_path_node component12(&component11,"foo");
    normalized_path_node component13(&component12,"bar");
    normalized_path_node component14(&component13,0);

    normalized_path path1(component4);
    normalized_path path2(component14);

    CHECK(path1 == path2);
}

TEST_CASE("test normalized_path to_string")
{
    normalized_path_node component1('$');
    normalized_path_node component2(&component1,"foo");
    normalized_path_node component3(&component2,"bar");
    normalized_path_node component4(&component3,0);

    normalized_path path1(component4);

    CHECK(path1.to_string() == std::string("$['foo']['bar'][0]"));
}

TEST_CASE("test normalized_path with solidus to_string")
{
    normalized_path_node component1('$');
    normalized_path_node component2(&component1,"foo's");
    normalized_path_node component3(&component2,"bar");
    normalized_path_node component4(&component3,0);

    normalized_path path1(component4);

    CHECK(path1.to_string() == std::string(R"($['foo\'s']['bar'][0])"));
}



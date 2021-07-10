// Copyright 2021 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons_ext/jsonpath/normalized_path.hpp>
#include <catch/catch.hpp>
#include <iostream>

using path_component = jsoncons::jsonpath::path_component<char>;
using normalized_path = jsoncons::jsonpath::normalized_path<char>;

TEST_CASE("test normalized_path equals")
{
    path_component component1('$');
    path_component component2(&component1,"foo");
    path_component component3(&component2,"bar");
    path_component component4(&component3,0);

    path_component component11('$');
    path_component component12(&component11,"foo");
    path_component component13(&component12,"bar");
    path_component component14(&component13,0);

    normalized_path path1(component4);
    normalized_path path2(component14);

    CHECK(path1 == path2);
}

TEST_CASE("test normalized_path to_string")
{
    path_component component1('$');
    path_component component2(&component1,"foo");
    path_component component3(&component2,"bar");
    path_component component4(&component3,0);

    normalized_path path1(component4);

    CHECK(path1.to_string() == std::string("$['foo']['bar'][0]"));
}

TEST_CASE("test normalized_path with solidus to_string")
{
    path_component component1('$');
    path_component component2(&component1,"foo's");
    path_component component3(&component2,"bar");
    path_component component4(&component3,0);

    normalized_path path1(component4);

    CHECK(path1.to_string() == std::string(R"($['foo\'s']['bar'][0])"));
}



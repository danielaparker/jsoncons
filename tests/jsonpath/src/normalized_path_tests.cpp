// Copyright 2021 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons_ext/jsonpath/normalized_path.hpp>
#include <catch/catch.hpp>
#include <iostream>

using path_node = jsoncons::jsonpath::detail::path_node<char>;
using normalized_path = jsoncons::jsonpath::detail::normalized_path<char>;

TEST_CASE("test normalized_path equals")
{
    path_node node1('$');
    path_node node2(&node1,"foo");
    path_node node3(&node2,"bar");
    path_node node4(&node3,0);

    path_node node11('$');
    path_node node12(&node11,"foo");
    path_node node13(&node12,"bar");
    path_node node14(&node13,0);

    normalized_path path1(node4);
    normalized_path path2(node14);

    CHECK(path1 == path2);
}

TEST_CASE("test normalized_path to_string")
{
    path_node node1('$');
    path_node node2(&node1,"foo");
    path_node node3(&node2,"bar");
    path_node node4(&node3,0);

    normalized_path path1(node4);

    CHECK(path1.to_string() == std::string("$['foo']['bar'][0]"));
}

TEST_CASE("test normalized_path with solidus to_string")
{
    path_node node1('$');
    path_node node2(&node1,"foo's");
    path_node node3(&node2,"bar");
    path_node node4(&node3,0);

    normalized_path path1(node4);

    CHECK(path1.to_string() == std::string(R"($['foo\'s']['bar'][0])"));
}



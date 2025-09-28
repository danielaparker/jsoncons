// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <iostream>
#include <jsoncons/flat_hash_map.hpp>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("ujson tests")
{
    SECTION("test1")
    {
    }
    SECTION("test2")
    {       	
        jsoncons::flat_hash_map<std::string, json> my_hash_map;
        my_hash_map.emplace("key1", 10);
        my_hash_map["key2"] = json(20);

        auto it = my_hash_map.find("key1");
        std::cout << it->key() << ", " << it->value() << "\n";

        std::cout << my_hash_map["key2"] << "\n";
    }
}


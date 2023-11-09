// Copyright 2013-2023 Daniel Parker
// Distributed under Boost license

#include <unordered_map>
#include <jsoncons/detail/string_view.hpp>
#include <catch/catch.hpp>

TEST_CASE("string_view tests")
{
    SECTION("test 1")
    {
        std::unordered_map<jsoncons::detail::string_view,int> map;

        std::string key1{"Foo"};
        std::string key2{"Bar"};

        map.emplace(key1, 1);
        map.emplace(key2, 2);

        CHECK(map.find(key1) != map.end());
        CHECK(map[key1] == 1);
        CHECK(map.find(key2) != map.end());
        CHECK(map[key2] == 2);
    }
    SECTION("test 2")
    {
        std::unordered_map<jsoncons::detail::wstring_view,int> map;

        std::wstring key1{L"Foo"};
        std::wstring key2{L"Bar"};

        map.emplace(key1, 1);
        map.emplace(key2, 2);

        CHECK(map.find(key1) != map.end());
        CHECK(map[key1] == 1);
        CHECK(map.find(key2) != map.end());
        CHECK(map[key2] == 2);
    }
}


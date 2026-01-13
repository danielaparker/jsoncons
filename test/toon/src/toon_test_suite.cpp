// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif

#include <jsoncons_ext/toon/encode_toon.hpp>
#include <jsoncons/json.hpp>

#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>
#include <unordered_set> // std::unordered_set
#include <fstream>
#include <catch/catch.hpp>

using namespace jsoncons;

void toon_tests(const std::string& fpath)
{
    std::cout << "Test " << fpath << '\n';

    std::fstream is(fpath);
    if (!is)
    {
        std::cerr << "Cannot open " << fpath << "\n";
        exit(1);
    }

    auto suite = ojson::parse(is);
    const auto& tests = suite.at("tests"); 
    for (const auto& test_group : tests.array_range())
    {
        const auto& j = test_group["input"];
        auto expected = test_group["expected"].as_string_view();
        std::string buffer;
        toon::encode_toon(j, buffer);
        if (expected != buffer)
        {
            std::cout << pretty_print(j) << "\n";
        }
        CHECK(expected == buffer);
    }
}

TEST_CASE("toon-tests")
{
    SECTION("compliance")
    {
        toon_tests("./toon/input/encode/arrays-nested.json");
        toon_tests("./toon/input/encode/arrays-objects.json");
        toon_tests("./toon/input/encode/arrays-primitive.json");
    }
}


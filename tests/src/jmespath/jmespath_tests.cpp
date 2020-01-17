// Copyright 2020 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>
#include <catch/catch.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <new>
#include <unordered_set> // std::unordered_set
#include <fstream>

using namespace jsoncons;

TEST_CASE("jmespath-tests")
{
    std::string fpath = "./input/jmespath/jmespath-tests.json";
    std::fstream is(fpath);
    REQUIRE(is);

    json tests = json::parse(is);
    for (const auto& test : tests.array_range())
    {
        std::string input = test["input"].as<std::string>();
        std::string path = test["expression"].as<std::string>();
        if (test.contains("expected"))
        {
            std::string expected = test["expected"].as<std::string>();

            ojson root = ojson::parse(input);
            ojson result = jmespath::search(root, path);
            ojson expected_result = ojson::parse(expected);
            if (result != expected_result)
            {
                std::cout << "\n" << test["annotation"] << "\n";
                std::cout << root << "\n";
                std::cout << path << "\n\n";
                std::cout << "actual: " << result << "\n\n";
                std::cout << "expected: " << expected << "\n\n";
            }
            CHECK(result == expected_result);
        }
        else
        {
            std::string error = test["error"].as<std::string>();
            ojson root = ojson::parse(input);
            REQUIRE_THROWS_WITH(jmespath::search(root, path), error);
        }
    }
}


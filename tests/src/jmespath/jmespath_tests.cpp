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

void jmespath_tests(const std::string& fpath)
{
    std::fstream is(fpath);
    REQUIRE(is);

    json tests = json::parse(is);
    for (const auto& test : tests.array_range())
    {
        const json& root = test["given"];

        for (const auto& item : test["cases"].array_range())
        {
            std::string path = item["expression"].as<std::string>();
            if (item.contains("result"))
            {
                const json& expected = item["result"];

                std::error_code ec;
                try
                {
                    json result = jmespath::search(root, path);
                    if (result != expected)
                    {
                        if (item.contains("annotation"))
                        {
                            std::cout << "\n" << item["annotation"] << "\n";
                        }
                        std::cout << "input\n" << pretty_print(root) << "\n";
                        std::cout << path << "\n\n";
                        std::cout << "actual\n: " << pretty_print(result) << "\n\n";
                        std::cout << "expected: " << pretty_print(expected) << "\n\n";
                    }
                    CHECK(result == expected);
                }
                catch (const std::exception& e)
                {
                    std::cout << e.what() << "\n";
                    if (item.contains("annotation"))
                    {
                        std::cout << "\n" << item["annotation"] << "\n";
                    }
                    std::cout << "input\n" << pretty_print(root) << "\n";
                    std::cout << "expression\n" << path << "\n";
                    std::cout << "expected: " << expected << "\n\n";
                    CHECK(false);
                }
            }
            else
            {
                std::string error = item["error"].as<std::string>();
                //REQUIRE_THROWS_WITH(jmespath::search(root, path), error);
            }
        }
    }
}

TEST_CASE("jmespath-tests")
{
    SECTION("Examples and tutorials")
    {
        //jmespath_tests("./input/jmespath/examples/jmespath-tests.json");

    }
    SECTION("compliance")
    {
        jmespath_tests("./input/jmespath/compliance-tests/basic.json"); // OK
        //jmespath_tests("./input/jmespath/compliance-tests/filters.json");
        jmespath_tests("./input/jmespath/compliance-tests/boolean.json"); // OK
        jmespath_tests("./input/jmespath/compliance-tests/wildcard.json"); // OK
    }
}


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
            std::string expr = item["expression"].as<std::string>();
            try
            {
                json actual = jmespath::search(root, expr);
                if (item.contains("result"))
                {
                    const json& expected = item["result"];
                    if (actual != expected)
                    {
                        if (item.contains("comment"))
                        {
                            std::cout << "\n" << item["comment"] << "\n";
                        }
                        std::cout << "Input:\n" << pretty_print(root) << "\n\n";
                        std::cout << "Expression: " << expr << "\n\n";
                        std::cout << "Actual: " << pretty_print(actual) << "\n\n";
                        std::cout << "Expected: " << pretty_print(expected) << "\n\n";
                    }
                    CHECK(actual == expected);
                }
                else if (item.contains("error"))
                {
                    if (item.contains("comment"))
                    {
                        std::cout << "Comment: " << item["comment"] << "\n";
                    }
                    std::cout << "Error: " << item["error"] << "\n\n";
                    std::cout << "Input:\n" << pretty_print(root) << "\n\n";
                    std::cout << "Expression: " << expr << "\n\n";
                    std::cout << "Actual: " << pretty_print(actual) << "\n\n";
                    CHECK(false);
                }

            }
            catch (const std::exception& e)
            {
                if (item.contains("result"))
                {
                    const json& expected = item["result"];
                    std::cout << e.what() << "\n";
                    if (item.contains("comment"))
                    {
                        std::cout << "Comment: " << item["comment"] << "\n\n";
                    }
                    std::cout << "Input\n" << pretty_print(root) << "\n\n";
                    std::cout << "Expression: " << expr << "\n\n";
                    std::cout << "Expected: " << expected << "\n\n";
                    CHECK(false);
                }
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
        //jmespath_tests("./input/jmespath/compliance-tests/syntax.json"); //

        jmespath_tests("./input/jmespath/compliance-tests/basic.json"); // OK
        jmespath_tests("./input/jmespath/compliance-tests/boolean.json"); // OK
        jmespath_tests("./input/jmespath/compliance-tests/current.json"); // OK
        jmespath_tests("./input/jmespath/compliance-tests/escape.json"); // OK
        jmespath_tests("./input/jmespath/compliance-tests/filters.json"); // OK
        jmespath_tests("./input/jmespath/compliance-tests/identifiers.json"); // OK
        jmespath_tests("./input/jmespath/compliance-tests/indices.json");  // OK
        jmespath_tests("./input/jmespath/compliance-tests/literal.json"); // OK
        jmespath_tests("./input/jmespath/compliance-tests/multiselect.json"); // OK 
        jmespath_tests("./input/jmespath/compliance-tests/pipe.json");
        jmespath_tests("./input/jmespath/compliance-tests/slice.json"); // OK
        jmespath_tests("./input/jmespath/compliance-tests/unicode.json"); // OK
        jmespath_tests("./input/jmespath/compliance-tests/wildcard.json"); // OK
        jmespath_tests("./input/jmespath/compliance-tests/benchmarks.json"); 

        jmespath_tests("./input/jmespath/compliance-tests/functions.json"); 

        //jmespath_tests("./input/jmespath/compliance-tests/test.json"); 
    }
}


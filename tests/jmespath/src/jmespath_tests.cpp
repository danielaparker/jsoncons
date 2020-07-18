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
        //jmespath_tests("./jmespath/input/examples/jmespath-tests.json");

    }
    SECTION("compliance")
    {
        // 4 syntax tests don't pass, appear to require extra grammatical checks 
        jmespath_tests("./jmespath/input/compliance/syntax.json"); 

        jmespath_tests("./jmespath/input/compliance/basic.json"); // OK
        jmespath_tests("./jmespath/input/compliance/boolean.json"); // OK
        jmespath_tests("./jmespath/input/compliance/current.json"); // OK
        jmespath_tests("./jmespath/input/compliance/escape.json"); // OK
        jmespath_tests("./jmespath/input/compliance/filters.json"); // OK
        jmespath_tests("./jmespath/input/compliance/identifiers.json"); // OK
        jmespath_tests("./jmespath/input/compliance/indices.json");  // OK
        jmespath_tests("./jmespath/input/compliance/literal.json"); // OK
        jmespath_tests("./jmespath/input/compliance/multiselect.json"); // OK 
        jmespath_tests("./jmespath/input/compliance/pipe.json"); // OK
        jmespath_tests("./jmespath/input/compliance/slice.json"); // OK
        jmespath_tests("./jmespath/input/compliance/unicode.json"); // OK
        jmespath_tests("./jmespath/input/compliance/wildcard.json"); // OK
        jmespath_tests("./jmespath/input/compliance/benchmarks.json"); // OK
        jmespath_tests("./jmespath/input/compliance/functions.json"); // OK

        //jmespath_tests("./jmespath/input/compliance/test.json"); 

        jmespath_tests("./jmespath/input/examples/jmespath-examples.json"); 
    }
}


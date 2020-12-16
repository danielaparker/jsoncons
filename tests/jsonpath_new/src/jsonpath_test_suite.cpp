// Copyright 2020 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath_new/jsonpath.hpp>
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

void jsonpath_tests(const std::string& fpath)
{
    std::fstream is(fpath);
    if (!is)
    {
        std::cerr << "Cannot open " << fpath << "\n";
        exit(1);
    }

    ojson tests = ojson::parse(is);
    for (const auto& test_group : tests.array_range())
    {
        const ojson& instance = test_group["given"];

        for (const auto& test_case : test_group["cases"].array_range())
        {
            std::string expr = test_case["expression"].as<std::string>();
            try
            {
                auto expression = jsoncons::jsonpath_new::make_expression<ojson>(jsoncons::string_view(expr));
                ojson actual = expression.evaluate(instance);
                if (test_case.contains("result"))
                {
                    const ojson& expected = test_case["result"];
                    //std::cout << "actual\n:" << actual << "\n";
                    if (actual != expected)
                    {
                        if (test_case.contains("comment"))
                        {
                            std::cout << "\n" << test_case["comment"] << "\n";
                        }
                        std::cout << "Input:\n" << pretty_print(instance) << "\n\n";
                        std::cout << "Expression: " << expr << "\n\n";
                        std::cout << "Actual: " << pretty_print(actual) << "\n\n";
                        std::cout << "Expected: " << pretty_print(expected) << "\n\n";
                    }
                    CHECK(actual == expected);
                }
                else if (test_case.contains("error"))
                {
                    if (test_case.contains("comment"))
                    {
                        std::cout << "Comment: " << test_case["comment"] << "\n";
                    }
                    std::cout << "Error: " << test_case["error"] << "\n\n";
                    std::cout << "Input:\n" << pretty_print(instance) << "\n\n";
                    std::cout << "Expression: " << expr << "\n\n";
                    std::cout << "Actual: " << pretty_print(actual) << "\n\n";
                    CHECK(false);
                }

            }
            catch (const std::exception& e)
            {
                if (test_case.contains("result"))
                {
                    const ojson& expected = test_case["result"];
                    std::cout << e.what() << "\n";
                    if (test_case.contains("comment"))
                    {
                        std::cout << "Comment: " << test_case["comment"] << "\n\n";
                    }
                    std::cout << "Input\n" << pretty_print(instance) << "\n\n";
                    std::cout << "Expression: " << expr << "\n\n";
                    std::cout << "Expected: " << expected << "\n\n";
                    CHECK(false);
                }
            }
        }
    }
}

TEST_CASE("jsonpath-tests")
{
    SECTION("Examples and tutorials")
    {
        //jsonpath_tests("./jsonpath/input/examples/jsonpath-tests.json");

    }
    SECTION("compliance")
    {
#if defined(JSONCONS_HAS_STD_REGEX)
        //jsonpath_tests("./jsonpath_new/input/compliance/jsonpath-regex-tests.json");
#endif
        //jsonpath_tests("./jsonpath_new/input/compliance/jsonpath-tests.json");

        //jsonpath_tests("./jsonpath_new/input/compliance/basic.json"); 
        //jsonpath_tests("./jsonpath_new/input/compliance/slice.json");
        //jsonpath_tests("./jsonpath_new/input/compliance/identifiers.json");
        //jsonpath_tests("./jsonpath_new/input/compliance/indices.json");
        //jsonpath_tests("./jsonpath_new/input/compliance/wildcard.json");
        //jsonpath_tests("./jsonpath_new/input/compliance/union.json");

        jsonpath_tests("./jsonpath_new/input/compliance/recursive-descent.json"); 
    }
}


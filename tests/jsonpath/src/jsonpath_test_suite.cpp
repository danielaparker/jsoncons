// Copyright 2020 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>
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

    json tests = json::parse(is);
    for (const auto& test_group : tests.array_range())
    {
        const json& instance = test_group["given"];

        for (const auto& test_case : test_group["cases"].array_range())
        {
            std::string expr = test_case["expression"].as<std::string>();
            try
            {
                jsonpath::result_options flags = jsonpath::result_options();
                if (test_case.contains("nodups") && test_case.at("nodups").as<bool>())
                {
                    flags |= jsonpath::result_options::nodups;
                }
                if (test_case.contains("sort") && test_case.at("sort").as<bool>())
                {
                    flags |= jsonpath::result_options::sort;
                }
                auto expression = jsoncons::jsonpath::make_expression<json>(expr);
                if (test_case.contains("result"))
                {
                    jsonpath::result_options rflags = flags | jsonpath::result_options::value;
                    json actual = expression.evaluate(instance, rflags);
                    const json& expected = test_case["result"];
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
                if (test_case.contains("path"))
                {
                    jsonpath::result_options pflags = flags | jsonpath::result_options::path;
                    json actual = expression.evaluate(instance, pflags);
                    const json& expected = test_case["path"];
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
                if (test_case.contains("error"))
                {
                    json actual = expression.evaluate(instance);
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
                    std::cout << e.what() << "\n";
                    const json& expected = test_case["result"];
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
    SECTION("compliance")
    {
#if defined(JSONCONS_HAS_STD_REGEX)
        jsonpath_tests("./jsonpath/input/compliance/regex.json");
#endif
        jsonpath_tests("./jsonpath/input/compliance/identifiers.json");
        jsonpath_tests("./jsonpath/input/compliance/dot-notation.json"); 
        jsonpath_tests("./jsonpath/input/compliance/slice.json");
        jsonpath_tests("./jsonpath/input/compliance/indices.json");
        jsonpath_tests("./jsonpath/input/compliance/wildcard.json");
        jsonpath_tests("./jsonpath/input/compliance/recursive-descent.json"); 
        jsonpath_tests("./jsonpath/input/compliance/union.json");       
        jsonpath_tests("./jsonpath/input/compliance/filters.json");
        jsonpath_tests("./jsonpath/input/compliance/functions.json");
        jsonpath_tests("./jsonpath/input/compliance/expressions.json");
        jsonpath_tests("./jsonpath/input/compliance/syntax.json");
        jsonpath_tests("./jsonpath/input/compliance/functions.json");

        //jsonpath_tests("./jsonpath/input/test.json");
    }
}


// Copyright 2020 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
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

    ojson tests = ojson::parse(is);
    for (const auto& test : tests.array_range())
    {
        const ojson& root = test["given"];

        for (const auto& item : test["cases"].array_range())
        {
            std::string expr = item["expression"].as<std::string>();
            try
            {
                ojson actual = jsonpath::json_query(root, expr);
                if (item.contains("result"))
                {
                    const ojson& expected = item["result"];
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
                    const ojson& expected = item["result"];
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

TEST_CASE("jsonpath-tests")
{
    SECTION("Examples and tutorials")
    {
        //jsonpath_tests("./jsonpath/input/examples/jsonpath-tests.json");

    }
    SECTION("compliance")
    {
    #if !(defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 9))
    // GCC 4.8 has broken regex support: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631
        jsonpath_tests("./jsonpath/input/compliance/jsonpath-regex-tests.json");
    #endif
        jsonpath_tests("./jsonpath/input/compliance/basic.json"); 
        jsonpath_tests("./jsonpath/input/compliance/slice.json");
        jsonpath_tests("./jsonpath/input/compliance/jsonpath-tests.json");
    }
}


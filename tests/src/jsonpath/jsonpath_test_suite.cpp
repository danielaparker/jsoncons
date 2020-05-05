// Copyright 2013 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
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

namespace {

    void run_jsonpath_test_suite(std::string fpath)
    {
        std::fstream is(fpath);
        REQUIRE(is);

        ojson tests = ojson::parse(is);
        for (const auto& test : tests.array_range())
        {
            const ojson& root = test["given"];

            for (const auto& item : test["cases"].array_range())
            {
                std::string path = item["expression"].as<std::string>();
                if (item.contains("result"))
                {
                    const ojson& expected = item["result"];

                    std::error_code ec;
                    try
                    {
                        ojson result = jsonpath::json_query(root, path);
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
                    }
                }
                else
                {
                    std::string error = item["error"].as<std::string>();
                    REQUIRE_THROWS_WITH(jsonpath::json_query(root, path), error);
                }
            }
        }
    }
}

TEST_CASE("jsonpath-test-suite")
{
    run_jsonpath_test_suite("./input/jsonpath/jsonpath-tests.json");

    #if !(defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 9))
    // GCC 4.8 has broken regex support: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631
    run_jsonpath_test_suite("./input/jsonpath/jsonpath-regex-tests.json");
    #endif
}


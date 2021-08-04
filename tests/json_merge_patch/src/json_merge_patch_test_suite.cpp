// Copyright 2020 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/json_merge_patch/json_merge_patch.hpp>
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

void json_merge_patch_tests(const std::string& fpath)
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
        for (const auto& test_case : test_group["cases"].array_range())
        {
            const json& patch = test_case["patch"];
            if (test_case.contains("result"))
            {
                json target = test_group.at("given");
                json_merge_patch::apply_merge_patch(target, patch);
                const json& expected = test_case["result"];
                if (target == expected)
                {
                    json target2 = test_group.at("given");
                    json patch2 = json_merge_patch::from_diff(target2, target);
                    json_merge_patch::apply_merge_patch(target2, patch2);
                    if (target2 != target)
                    {
                        if (test_case.contains("comment"))
                        {
                            std::cout << "\n" << test_case["comment"] << "\n";
                        }
                        std::cout << "Source: " << pretty_print(test_group.at("given")) << "\n\n";
                        std::cout << "Target: " << pretty_print(target) << "\n\n";
                        std::cout << "Diff: " << pretty_print(patch2) << "\n\n";
                        std::cout << "Result: " << pretty_print(target2) << "\n\n";
                    }
                    CHECK(target2 == target);
                }
                else 
                {
                    if (test_case.contains("comment"))
                    {
                        std::cout << "\n" << test_case["comment"] << "\n";
                    }
                    std::cout << "Input: " << pretty_print(test_group.at("given")) << "\n\n";
                    std::cout << "Patch: " << pretty_print(patch) << "\n\n";
                    std::cout << "Target: " << pretty_print(target) << "\n\n";
                    std::cout << "Expected: " << pretty_print(expected) << "\n\n";
                }
                CHECK(target == expected); //-V521
            }
        }
    }
}

TEST_CASE("json_merge_patch tests")
{
    SECTION("compliance")
    {
        json_merge_patch_tests("./json_merge_patch/input/compliance/rfc7396-test-cases.json");
        //json_merge_patch_tests("./json_merge_patch/input/compliance/test.json");
    }
}


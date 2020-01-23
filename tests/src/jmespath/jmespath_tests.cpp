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
        jmespath_tests("./input/jmespath/examples/jmespath-tests.json");
    }
    SECTION("basics")
    {
        //jmespath_tests("./input/jmespath/compliance-tests/basic.json");
    }
}

using jmespath::detail::jmespath_evaluator;

/*
TEST_CASE("jmespath expressions")
{
    std::string input = R"(
    {"reservations": [{"instances": [{"state": "running"},{"state": "stopped"}]},{"instances": [{"state": "terminated"},{"state": "runnning"}]}]}
    )";

    json root = json::parse(input);
    jmespath_evaluator<json,const json&>::jmespath_context context;
    std::error_code ec;

    SECTION("reservations[*].instances[*].state")
    {
        jmespath_evaluator<json,const json&>::identifier_selector reservations("reservations");
        auto& result = reservations.select(context, root, ec);
        std::cout << pretty_print(result) << "\n";

        jmespath_evaluator<json, const json&>::expression_selector expr{};
        expr.add_selector(std::make_unique<jmespath_evaluator<json, const json&>::identifier_selector>(reservations));

        jmespath_evaluator<json, const json&>::list_projection proj1{};
        jmespath_evaluator<json, const json&>::identifier_selector instances("instances");
        proj1.add_selector(std::make_unique<jmespath_evaluator<json, const json&>::identifier_selector>(instances));
        jmespath_evaluator<json, const json&>::list_projection proj2{};
        jmespath_evaluator<json, const json&>::identifier_selector state("state");
        proj2.add_selector(std::make_unique<jmespath_evaluator<json, const json&>::identifier_selector>(state));
        proj1.add_selector(std::make_unique<jmespath_evaluator<json, const json&>::list_projection>(std::move(proj2)));
        expr.add_selector(std::make_unique<jmespath_evaluator<json, const json&>::list_projection>(std::move(proj1)));
        auto& result3 = expr.select(context, root, ec);
        std::cout << pretty_print(result3) << "\n";
    }
    SECTION("reservations[*].instances[*].state 2")
    {
        std::vector<std::unique_ptr<jmespath_evaluator<json, const json&>::selector_base>> expr_stack;
        std::vector<size_t> offset_stack;

        offset_stack.push_back(expr_stack.size()); // 0
        expr_stack.push_back(std::make_unique<jmespath_evaluator<json, const json&>::expression_selector>());
        expr_stack.push_back(std::make_unique<jmespath_evaluator<json, const json&>::identifier_selector>("reservations"));
        offset_stack.push_back(expr_stack.size()); // 2
        expr_stack.push_back(std::make_unique<jmespath_evaluator<json, const json&>::list_projection>());
        expr_stack.push_back(std::make_unique<jmespath_evaluator<json, const json&>::identifier_selector>("instances"));
        offset_stack.push_back(expr_stack.size()); // 5
        expr_stack.push_back(std::make_unique<jmespath_evaluator<json, const json&>::list_projection>());
        expr_stack.push_back(std::make_unique<jmespath_evaluator<json, const json&>::identifier_selector>("state"));

        while (!offset_stack.empty())
        {
            const size_t pos = offset_stack.back();
            for (size_t i = pos + 1; i < expr_stack.size(); ++i)
            {
                expr_stack[pos]->add_selector(std::move(expr_stack[i]));
            }
            expr_stack.erase(expr_stack.begin()+pos+1,expr_stack.end());
            offset_stack.pop_back();
        }
        auto& result = expr_stack[0]->select(context, root, ec);
        std::cout << pretty_print(result) << "\n";
    }
}
*/

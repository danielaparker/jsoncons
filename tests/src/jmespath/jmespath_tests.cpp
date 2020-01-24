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
        //jmespath_tests("./input/jmespath/examples/jmespath-tests.json");
    }
    SECTION("basics")
    {
        //jmespath_tests("./input/jmespath/compliance-tests/basic.json");
    }
}

using jmespath::detail::jmespath_evaluator;


TEST_CASE("jmespath expressions")
{
    std::string input = R"(
    {"reservations": [{"instances": [{"state": "running"},{"state": "stopped"}]},{"instances": [{"state": "terminated"},{"state": "runnning"}]}]}
    )";

    json root = json::parse(input);
    jmespath_evaluator<json,const json&>::jmespath_context context;
    std::error_code ec;
/*
    SECTION("reservations[*].instances[*].state")
    {
        auto reservations = std::make_unique<jmespath_evaluator<json,const json&>::identifier_selector>("reservations");
        auto instances = std::make_unique<jmespath_evaluator<json,const json&>::identifier_selector>("instances");
        auto state = std::make_unique<jmespath_evaluator<json, const json&>::identifier_selector>("state");

        auto expr = std::make_unique<jmespath_evaluator<json, const json&>::expression_selector>();
        expr->add_selector(std::move(reservations));

        auto expr2 = std::make_unique<jmespath_evaluator<json, const json&>::list_projection>(std::move(expr)); 
        expr2->add_selector(std::move(instances));

        auto expr3 = std::make_unique<jmespath_evaluator<json, const json&>::list_projection>(std::move(expr2));
        expr3->add_selector(std::move(state));

        std::cout << "\n" << expr3->to_string() << "\n";

        auto& result3 = expr3->evaluate(context, root, ec);
        std::cout << pretty_print(result3) << "\n";
    }
*/
    SECTION("reservations[].instances[].state2")
    {
        auto reservations = std::make_unique<jmespath_evaluator<json,const json&>::identifier_selector>("reservations");
        auto instances = std::make_unique<jmespath_evaluator<json,const json&>::identifier_selector>("instances");
        auto state = std::make_unique<jmespath_evaluator<json, const json&>::identifier_selector>("state");

        auto sub_expr1 = std::make_unique<jmespath_evaluator<json, const json&>::sub_expression>(std::move(reservations));
        auto lp_instances = std::make_unique<jmespath_evaluator<json, const json&>::flatten_projection>();

        // list projection lp_state must receive as input [[{"state": "running"},{"state": "stopped"}],[{"state": "terminated"},{"state": "runnning"}]]
        auto lp_state = std::make_unique<jmespath_evaluator<json, const json&>::flatten_projection>();
        lp_state->add_selector(std::move(state));

        auto sub_expr2 = std::make_unique<jmespath_evaluator<json, const json&>::sub_expression>(std::move(instances));
        sub_expr2->add_selector(std::move(lp_state));

        lp_instances->add_selector(std::move(sub_expr2));


        sub_expr1->add_selector(std::move(lp_instances));

        std::cout << "\n" << sub_expr1->to_string() << "\n\n";

        auto& result1 = sub_expr1->evaluate(context, root, ec);
        std::cout << pretty_print(result1) << "\n";
    }
}


// Copyright 2020 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>
#include <catch/catch.hpp>
#include <iostream>

using jsoncons::json;
namespace jmespath = jsoncons::jmespath;

TEST_CASE("jmespath_expression tests")
{
    SECTION("Test 1")
    {
        std::string jtext = R"(
            {
              "people": [
                {
                  "age": 20,
                  "other": "foo",
                  "name": "Bob"
                },
                {
                  "age": 25,
                  "other": "bar",
                  "name": "Fred"
                },
                {
                  "age": 30,
                  "other": "baz",
                  "name": "George"
                }
              ]
            }
        )";

        auto expr = jmespath::jmespath_expression<json>::compile("sum(people[].age)");

        json doc = json::parse(jtext);

        json result = expr.evaluate(doc);
        std::cout << pretty_print(result) << "\n\n";
    }    
    SECTION("Test 2")
    {
        std::string jtext = R"(
{
    "group": {
      "value": 1
    },
    "array": [
      {"value": 2}
    ]
}
        )";

        json doc = json::parse(jtext);

        std::unique_ptr<jmespath::jmespath_expression<json>> expr1;
        expr1 = jsoncons::make_unique<jmespath::jmespath_expression<json >>(jmespath::jmespath_expression<json>::compile("group.value"));
        json result1 = expr1->evaluate(doc);
        std::cout << pretty_print(result1) << "\n\n";

        std::unique_ptr<jmespath::jmespath_expression<json>> expr2;
        expr2 = jsoncons::make_unique<jmespath::jmespath_expression<json >>(jmespath::jmespath_expression<json>::compile("array[0].value"));
        json result2 = expr2->evaluate(doc);
        std::cout << pretty_print(result2) << "\n\n";

        std::unique_ptr<jmespath::jmespath_expression<json>> expr3;
        expr3 = jsoncons::make_unique<jmespath::jmespath_expression<json >>(jmespath::jmespath_expression<json>::compile("nullable.value"));
        json result3 = expr3->evaluate(doc);
        std::cout << pretty_print(result3) << "\n\n";
    }    
    SECTION("Test 3")
    {
        std::string jtext = R"(
{
    "group": {
      "value": 1
    },
    "array": [
      {"value": 2}
    ]
}
        )";

        json doc = json::parse(jtext);

        std::unique_ptr<jmespath::jmespath_expression<json>> expr1;
        {
            expr1 = jsoncons::make_unique<jmespath::jmespath_expression<json >>(jmespath::jmespath_expression<json>::compile("group.value"));
        }
        json result1 = expr1->evaluate(doc);
        std::cout << pretty_print(result1) << "\n\n";

        std::unique_ptr<jmespath::jmespath_expression<json>> expr2;
        {
            expr2 = jsoncons::make_unique<jmespath::jmespath_expression<json >>(jmespath::jmespath_expression<json>::compile("array[0].value"));
        }
        json result2 = expr2->evaluate(doc);
        std::cout << pretty_print(result2) << "\n\n";

        std::unique_ptr<jmespath::jmespath_expression<json>> expr3;
        {
            expr3 = jsoncons::make_unique<jmespath::jmespath_expression<json >>(jmespath::jmespath_expression<json>::compile("nullable.value"));
        }
        json result3 = expr3->evaluate(doc);
        std::cout << pretty_print(result3) << "\n\n";
    }    
}


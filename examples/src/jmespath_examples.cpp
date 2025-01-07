// Copyright 2013-2024 Daniel Parker
// Distributed under Boost license

#include <jsoncons_ext/jmespath/jmespath.hpp>
#include <jsoncons/json.hpp>

#include <string>
#include <fstream>
#include <cassert>

// for brevity
using jsoncons::json; 
namespace jmespath = jsoncons::jmespath;

void search_example() 
{
    std::string jtext = R"(
    {
      "locations": [
        {"name": "Seattle", "state": "WA"},
        {"name": "New York", "state": "NY"},
        {"name": "Bellevue", "state": "WA"},
        {"name": "Olympia", "state": "WA"}
      ]
    }        
    )";

    std::string expr = "locations[?state == 'WA'].name | sort(@) | {WashingtonCities: join(', ', @)}";

    json doc = json::parse(jtext);

    json result = jmespath::search(doc, expr);

    std::cout << pretty_print(result) << "\n\n";
}

void jmespath_expression_example()
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

    auto expr = jmespath::jmespath_expression<json>::compile("people[?age > `20`].[name, age]");

    json doc = json::parse(jtext);

    json result = expr.evaluate(doc);

    std::cout << pretty_print(result) << "\n\n";
}

#if defined(_MSC_VER)

#include <execution>
#include <concurrent_vector.h> // microsoft PPL library

void query_json_lines_in_parallel() 
{

    std::vector<std::string> lines = { {
        R"({"name": "Seattle", "state" : "WA"})",
        R"({ "name": "New York", "state" : "NY" })",
        R"({ "name": "Bellevue", "state" : "WA" })",
        R"({ "name": "Olympia", "state" : "WA" })"
} };

    auto expr = jsoncons::jmespath::make_expression<jsoncons::json>(
        R"([@][?state=='WA'].name)");

    concurrency::concurrent_vector<std::string> result;

    auto f = [&](const std::string& line)
        {
            const auto j = jsoncons::json::parse(line);
            const auto r = expr.evaluate(j);
            if (!r.empty())
                result.push_back(r.at(0).as<std::string>());
        };

    std::for_each(std::execution::par, lines.begin(), lines.end(), f);

    for (const auto& s : result)
    {
        std::cout << s << "\n";
    }
}

#endif

int main()
{
    std::cout << "\nJMESPath examples\n\n";
    search_example();
    jmespath_expression_example();

#if defined(_MSC_VER)
    query_json_lines_in_parallel();
#endif

    std::cout << "\n";
}


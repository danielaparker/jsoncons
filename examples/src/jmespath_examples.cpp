// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <fstream>
#include <cassert>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>

// for brevity
using jsoncons::json; 
namespace jmespath = jsoncons::jmespath;

namespace {

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

} // namespace

void jmespath_examples()
{
    std::cout << "\nJMESPath examples\n\n";
    search_example();
    jmespath_expression_example();

    std::cout << "\n";
}


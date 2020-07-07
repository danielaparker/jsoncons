### jmespath extension

The jmespath extension implements [JMESPath](https://jmespath.org/). JMESPath is a query language 
for transforming JSON documents into other JSON documents.
It's supported in both the AWS and Azure CLI and has libraries available in a number of languages.

### Compliance level

Fully compliant. The jsoncons implementation passes all [compliance tests](https://github.com/jmespath/jmespath.test).

### Classes
<table border="0">
  <tr>
    <td><a href="jmespath_expression.md">jmespath_expression</a></td>
    <td>Represents the compiled form of a JMESPath expression.</td> 
  </tr>
</table>

### Functions

<table border="0">
  <tr>
    <td><a href="search.md">search</a></td>
    <td>Searches for all values that match a JMESPath expression</td> 
  </tr>
</table>
    
### Examples

The examples below are from the [JMESPath front page](https://jmespath.org/) and the [JMESPath Tutorial](https://jmespath.org/tutorial.html).

#### search function

[jsoncons::jmespath::search](search.md) takes two arguments, a [basic_json](../basic_json.md) 
and a JMESPath expression string, and returns a `basic_json` result. This is the simplest way to
compile and evaluate a JMESPath expression.

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>

// for brevity
using jsoncons::json; 
namespace jmespath = jsoncons::jmespath;

int main() 
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
```
Output:
```
{
    "WashingtonCities": "Bellevue, Olympia, Seattle"
}
```

#### jmespath_expression

A [jsoncons::jmespath::jmespath_expression](jmespath_expression.md) 
represents the compiled form of a JMESPath expression. It allows you to 
evaluate a single compiled expression on multiple JSON documents.
A `jmespath_expression` is immutable and thread-safe. 

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>

// for brevity
using jsoncons::json; 
namespace jmespath = jsoncons::jmespath;

int main()
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
```
Output:
```
[
    ["Fred", 25],
    ["George", 30]
]
```


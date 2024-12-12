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
    <td>Represents the compiled form of a JMESPath string.</td> 
  </tr>
</table>

### Functions

<table border="0">
  <tr>
    <td><a href="search.md">search</a></td>
    <td>Searches for all values that match a JMESPath expression</td> 
  </tr>
  <tr>
    <td><a href="make_expression.md">make_expression</a></td>
    <td>Returns a compiled JMESPath expression for later evaluation. (since 0.159.0)</td> 
  </tr>
</table>
    
### Examples

[search function](#eg1)  
[jmespath_expression](#eg2)  
[custom_functions (since 1.0.0)](#eg3)  

 <div id="eg1"/>

#### search function

[jsoncons::jmespath::search](search.md) takes two arguments, a [basic_json](../basic_json.md) 
and a JMESPath expression string, and returns a `basic_json` result. This is the simplest way to
compile and evaluate a JMESPath expression.

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>

// for brevity
using jsoncons::json; 
namespace jmespath = jsoncons::jmespath;

int main() 
{
    // This examples is from the JMESPath front page
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

Credit to [JMESPath](https://jmespath.org/) for this example

 <div id="eg2"/>

#### jmespath_expression

A [jsoncons::jmespath::jmespath_expression](jmespath_expression.md) 
represents the compiled form of a JMESPath string. It allows you to 
evaluate a single compiled expression on multiple JSON documents.
A `jmespath_expression` is immutable and thread-safe. 

```cpp
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

 // auto expr = jmespath::jmespath_expression<json>::compile("people[?age > `20`].[name, age]"); // until 0.159.0
    auto expr = jmespath::make_expression<json>("people[?age > `20`].[name, age]");              // since 0.159.0

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

Credit to [JMESPath Tutorial](https://jmespath.org/tutorial.html) for this Example

 <div id="eg3"/>

#### custom_functions (since 1.0.0)

```cpp
#include <chrono>
#include <thread>
#include <string>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath.hpp>

// for brevity
namespace jmespath = jsoncons::jmespath;

// When adding custom functions, they are generally placed in their own project's source code and namespace.
namespace myspace {

template <typename Json>
class my_custom_functions : public jmespath::custom_functions<Json>
{
    using reference = const Json&;
    using pointer = const Json*;

    static thread_local size_t current_index;
public:
    my_custom_functions()
    {
        this->register_function("current_index", // function name
            0,                                   // number of arguments   
            [](const jsoncons::span<const jmespath::parameter<Json>> params,
                jmespath::dynamic_resources<Json>& resources,
                std::error_code& ec) -> reference
            {
                auto result = resources.create_json(current_index);
                return *result;
            }
        );
        this->register_function("generate_array", // function name
            4,                                    // number of arguments   
            [](const jsoncons::span<const jmespath::parameter<Json>> params,
                jmespath::dynamic_resources<Json>& resources,
                std::error_code& ec) -> reference
            {
                JSONCONS_ASSERT(4 == params.size());

                if (!(params[0].is_value() && params[2].is_expression()))
                {
                    ec = jmespath::jmespath_errc::invalid_argument;
                    return resources.null_value();
                }

                reference context = params[0].value();
                reference countValue = get_value(context, resources, params[1]);
                const auto& expr = params[2].expression();
                auto argDefault = params[3];

                if (!countValue.is_number())
                {
                    ec = jmespath::jmespath_errc::invalid_argument;
                    return resources.null_value();
                }

                auto result = resources.create_json(jsoncons::json_array_arg);
                int count = countValue.template as<int>();
                for (size_t i = 0; i < count; i++)
                {
                    current_index = i;
                    std::error_code ec2;

                    reference ele = expr.evaluate(context, resources, ec2); 

                    if (ele.is_null())
                    {
                        auto defaultVal = get_value(context, resources, argDefault);
                        result->emplace_back(defaultVal);
                    }
                    else
                    {
                        result->emplace_back(ele); 
                    }
                }
                current_index = 0;

                return *result;
            }
        );
        this->register_function("add", // function name
            2,                         // number of arguments   
            [](jsoncons::span<const jmespath::parameter<Json>> params,
                jmespath::dynamic_resources<Json>& resources,
                std::error_code& ec) -> reference
            {
                JSONCONS_ASSERT(2 == params.size());

                if (!(params[0].is_value() && params[1].is_value()))
                {
                    ec = jmespath::jmespath_errc::invalid_argument;
                    return resources.null_value();
                }

                reference arg0 = params[0].value();
                reference arg1 = params[1].value();
                if (!(arg0.is_number() && arg1.is_number()))
                {
                    ec = jmespath::jmespath_errc::invalid_argument;
                    return resources.null_value();
                }

                if (arg0.is<int64_t>() && arg1.is<int64_t>())
                {
                    int64_t v = arg0.template as<int64_t>() + arg1.template as<int64_t>();
                    return *resources.create_json(v);
                }
                else
                {
                    double v = arg0.template as<double>() + arg1.template as<double>();
                    return *resources.create_json(v);
                }
            }
        );
    }

    static reference get_value(reference context, jmespath::dynamic_resources<Json>& resources,
        const jmespath::parameter<Json>& param)
    {
        if (param.is_expression())
        {
            const auto& expr = param.expression();
            std::error_code ec;
            reference value = expr.evaluate(context, resources, ec);
            return value;
        }
        else
        {
            reference value = param.value();
            return value;
        }
    }
};

template <typename Json>
thread_local size_t my_custom_functions<Json>::current_index = 0;

} // namespace myspace

// for brevity
using json = jsoncons::json;
   
int main()
{
    std::string jtext = R"(
          {
            "devices": [
              {
                "position": 1,
                "id": "id-xxx",
                "state": 1
              },
              {
                "position": 5,
                "id": "id-yyy",
                "state": 1
              },
              {
                "position": 9,
                "id": "id-mmm",
                "state": 2
              }
            ]
          }        
    )";
  
    auto expr = jmespath::make_expression<json>("generate_array(devices, `16`, &[?position==add(current_index(), `1`)] | [0], &{id: '', state: `0`, position: add(current_index(), `1`)})",
        myspace::my_custom_functions<json>{});
  
    auto doc = json::parse(jtext);
  
    auto result = expr.evaluate(doc);
  
    std::cout << pretty_print(result) << "\n\n";
}
```

Credit to [PR #560](https://github.com/danielaparker/jsoncons/pull/560) for this example

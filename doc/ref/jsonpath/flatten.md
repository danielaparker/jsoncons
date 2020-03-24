### jsoncons::jsonpath::flatten

```c++
#include <jsoncons_ext/jsonpath/filter.hpp>

template<class Json>
Json flatten(const Json& value)

template<class Json>
Json unflatten(const Json& value)
```
Flattens a json object or array into a single depth object of JSONPath-value pairs.

#### Return value

A flattened json object of JSONPath-value pairs

### Examples

#### Flatten and unflatten

```c++
#include <iostream>
#include <cassert>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

// for brevity
using jsoncons::json; 
namespace jpath = jsoncons::jsonpath;

int main()
{
    json input = json::parse(R"(
    {
       "application": "hiking",
       "reputons": [
           {
               "rater": "HikingAsylum",
               "assertion": "advanced",
               "rated": "Marilyn C",
               "rating": 0.90
            },
            {
               "rater": "HikingAsylum",
               "assertion": "intermediate",
               "rated": "Hongmin",
               "rating": 0.75
            }    
        ]
    }
    )");

    json result = jpath::flatten(input);

    std::cout << pretty_print(result) << "\n";

    json original = jpath::unflatten(result);
    assert(original == input);
}
```
Output:
```
{
    "$['application']": "hiking",
    "$['reputons'][0]['assertion']": "advanced",
    "$['reputons'][0]['rated']": "Marilyn C",
    "$['reputons'][0]['rater']": "HikingAsylum",
    "$['reputons'][0]['rating']": 0.9,
    "$['reputons'][1]['assertion']": "intermediate",
    "$['reputons'][1]['rated']": "Hongmin",
    "$['reputons'][1]['rater']": "HikingAsylum",
    "$['reputons'][1]['rating']": 0.75
}
```


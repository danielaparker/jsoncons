### jsoncons::jsonpointer::flatten

```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
Json flatten(const Json& value); // (1)

template<class Json>
Json unflatten(const Json& value); // (2) (since v0.150.0)
```

Flattens a json object or array into a single depth object of JSONPointer-value pairs.
The keys in the flattened object are JSONPointer's.
The values are primitive (string, number, boolean, or null). Empty objects or arrays become null.

#### Return value

(1) A flattened json object of JSONPointer-value pairs

(2) An unflattened json object

#### Remarks

There is no unique solution for unflattening a single depth object of JSONPointer-value pairs. 
An integer appearing in a path could be an index or it could be an object key.
The unflatten function assumes it is an object key.

### Examples

#### Flatten and unflatten

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

// for brevity
using jsoncons::json; 
namespace jsonpointer = jsoncons::jsonpointer;

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

    json flattened = jsonpointer::flatten(input);

    std::cout << "(1)\n" << pretty_print(flattened) << "\n\n";

    json unflattened = jsonpointer::unflatten(flattened);

    std::cout << "(2)\n" << pretty_print(unflattened) << "\n";
}
```
Output:
```
(1)
{
    "/application": "hiking",
    "/reputons/0/assertion": "advanced",
    "/reputons/0/rated": "Marilyn C",
    "/reputons/0/rater": "HikingAsylum",
    "/reputons/0/rating": 0.9,
    "/reputons/1/assertion": "intermediate",
    "/reputons/1/rated": "Hongmin",
    "/reputons/1/rater": "HikingAsylum",
    "/reputons/1/rating": 0.75
}

(2)
{
    "application": "hiking",
    "reputons": {
        "0": {
            "assertion": "advanced",
            "rated": "Marilyn C",
            "rater": "HikingAsylum",
            "rating": 0.9
        },
        "1": {
            "assertion": "intermediate",
            "rated": "Hongmin",
            "rater": "HikingAsylum",
            "rating": 0.75
        }
    }
}
```
### See also

[jsoncons::jsonpath::flatten](../jsonpath/flatten.md)

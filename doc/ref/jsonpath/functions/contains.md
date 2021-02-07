### contains

```
boolean contains(array|string subject, any search)
```

If subject is an array, returns true if the array contains an item that is equal to 
the provided search value, false otherwise.

If subject is a string, returns true if the string contains a substring that is equal to
the provided search value, false otherwise.

Returns null if 

- the provided subject is not an array or string, or
- the provided subject is a string but the provided search value is not a string.

### Examples

```c++
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

// for brevity
using jsoncons::json; 
namespace jsonpath = jsoncons::jsonpath;

int main() 
{
    std::string data = R"(
{
    "books":
    [
        {
            "title" : "A Wild Sheep Chase",
            "author" : "Haruki Murakami",
            "price" : 22.72
        },
        {
            "title" : "The Night Watch",
            "author" : "Sergei Lukyanenko",
            "price" : 23.58
        },
        {
            "title" : "The Comedians",
            "author" : "Graham Greene",
            "price" : 21.99
        },
        {
            "title" : "The Night Watch",
            "author" : "Phillips, David Atlee"
        }
    ]
}
    )";

    json j = json::parse(data);

    // All books that don't have a price
    std::string expr = "$.books[?(!contains(keys(@),'price'))]";

    json result = jsonpath::json_query(j, expr);
    std::cout << result << "\n\n";
}
```
Output:
```
[{"author":"Phillips, David Atlee","title":"The Night Watch"}]
```


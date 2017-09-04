### jsoncons::jsonpointer::select, jsoncons::jsonpointer::try_select

Selects a `json` value.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
Json select(const Json& root, 
            typename Json::string_view_type path); // (1)

template<class Json>
std::tuple<Json,jsonpointer_errc> try_select(const Json& root, 
                                             typename Json::string_view_type path);  // (2)

```

#### Exceptions

(1) On error, a [parse_error](../parse_error.md) exception that has an associated [jsonpointer_errc](jsonpointer_errc.md) error code.

(2) On success, returns the selected Json value and a value-initialized [jsonpointer_errc](jsonpointer_errc.md)
On error, returns a null Json value and a [jsonpointer_errc](jsonpointer_errc.md) error code 

### Examples

#### Select author from second book

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
json root = json::parse(R"(
[
  { "category": "reference",
    "author": "Nigel Rees",
    "title": "Sayings of the Century",
    "price": 8.95
  },
  { "category": "fiction",
    "author": "Evelyn Waugh",
    "title": "Sword of Honour",
    "price": 12.99
  }
]
)");

    json result = jsonpointer::select(root, "/1/author");

    std::cout << result << std::endl;
}
```
Output:
```json
"Evelyn Waugh"
```

#### Select author from second book

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
json root = json::parse(R"(
[
  { "category": "reference",
    "author": "Nigel Rees",
    "title": "Sayings of the Century",
    "price": 8.95
  },
  { "category": "fiction",
    "author": "Evelyn Waugh",
    "title": "Sword of Honour",
    "price": 12.99
  }
]
)");

    json result = jsonpointer::select(root, "/1/author");

    std::cout << result << std::endl;
}
```
Output:
```json
"Evelyn Waugh"
```



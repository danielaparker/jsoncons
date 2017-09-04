### jsoncons::jsonpointer::select

Selects a `json` value.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
std::tuple<Json,jsonpointer_errc> try_select(const Json& root, typename Json::string_view_type path)
```

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



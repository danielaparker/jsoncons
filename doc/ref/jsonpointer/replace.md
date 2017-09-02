### jsoncons::jsonpointer::replace

Replace a `json` element or member.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
Json replace(const Json& root, typename Json::string_view_type path, const Json& value)
```

### Examples

#### Replace a value

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json example = json::parse(R"(
        {
          "baz": "qux",
          "foo": "bar"
        }
    )");

    jsonpointer::replace(example, "/baz", json("boo"));

    std::cout << pretty_print(example) << std::endl;
}
```
Output:
```json
{
  "baz": "boo",
  "foo": "bar"
}
```



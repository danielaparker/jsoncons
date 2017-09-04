### jsoncons::jsonpointer::replace

Replace a `json` element or member.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
void replace(const Json& root, typename Json::string_view_type path, const Json& value)
```

#### Exceptions

On error, a [parse_error](../parse_error.md) exception that has an associated [jsonpointer_errc](jsonpointer_errc.md) error code.

### Examples

#### Replace a value

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json target = json::parse(R"(
        {
          "baz": "qux",
          "foo": "bar"
        }
    )");

    try
    {
        jsonpointer::replace(target, "/baz", json("boo"));
        std::cout << pretty_print(target) << std::endl;
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}
```
Output:
```json
{
    "baz": "boo",
    "foo": "bar"
}
```



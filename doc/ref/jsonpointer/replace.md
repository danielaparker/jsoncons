### jsoncons::jsonpointer::replace, jsoncons::jsonpointer::try_replace

Replace a `json` element or member.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
void replace(const Json& root, typename Json::string_view_type path, const Json& value); // (1)

template<class Json>
jsonpointer_errc replace(const Json& root, typename Json::string_view_type path, const Json& value); // (2)
```

#### Exceptions

(1) On error, a [parse_error](../parse_error.md) exception that has an associated [jsonpointer_errc](jsonpointer_errc.md) error code.

#### Return value

(1) None

(2) On success, returns the selected Json value and a value-initialized [jsonpointer_errc](jsonpointer_errc.md)
On error, returns a null Json value and a [jsonpointer_errc](jsonpointer_errc.md) error code 

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

    auto ec = jsonpointer::try_replace(target, "/baz", json("boo"));
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << pretty_print(target) << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
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



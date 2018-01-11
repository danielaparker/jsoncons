### jsoncons::jsonpointer::replace

Replace a `json` element or member.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
void replace(Json& target, const typename Json::string_view_type& path, const Json& value); 

template<class Json>
void replace(Json& target, const typename Json::string_view_type& path, const Json& value, std::error_code& ec); 
```

Replaces the value at the location specified by `path` with a new value. 

#### Return value

On success, a value-initialized [jsonpointer_errc](jsonpointer_errc.md). 

On error, a [jsonpointer_errc](jsonpointer_errc.md) error code 

### Examples

#### Replace an object value

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

    auto ec = jsonpointer::replace(target, "/baz", json("boo"));
    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << target << std::endl;
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

#### Replace an array value

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    auto ec = jsonpointer::replace(target, "/foo/1", json("qux"));
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
    "foo": ["bar","qux"]
}
```



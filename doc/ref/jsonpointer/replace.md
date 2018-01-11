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

### Exceptions

(1) Throws a [jsonpointer_error](jsonpointer_error.md) if get fails.
 
(2) Sets the `std::error_code&' to the [jsonpointer_error_category](jsonpointer_errc.md) if get fails. 

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

    std::error_code ec;
    jsonpointer::replace(target, "/baz", json("boo"), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << target << std::endl;
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

    std::error_code ec;
    jsonpointer::replace(target, "/foo/1", json("qux"), ec);
    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << pretty_print(target) << std::endl;
    }
}
```
Output:
```json
{
    "foo": ["bar","qux"]
}
```



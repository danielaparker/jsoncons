### jsoncons::jsonpointer::remove

Removes a `json` element.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
jsonpointer_errc remove(Json& target, typename Json::string_view_type path); 
```

Removes the value at the location specifed by `path`.

#### Return value

On success, a value-initialized [jsonpointer_errc](jsonpointer_errc.md). 

On error, a [jsonpointer_errc](jsonpointer_errc.md) error code 

### Examples

#### Remove an object member

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json target = json::parse(R"(
        { "foo": "bar", "baz" : "qux"}
    )");

    auto ec = jsonpointer::remove(target, "/baz");
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
{"foo":"bar"}
```

#### Remove an array element

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "qux", "baz" ] }
    )");

    auto ec = jsonpointer::remove(target, "/foo/1");
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
{"foo":["bar","baz"]}
```



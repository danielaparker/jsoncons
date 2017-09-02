### jsoncons::jsonpointer::add

Adds a `json` value.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
Json add(const Json& root, typename Json::string_view_type path, const Json& value)
```

### Examples

#### Add a member to an object

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json example = json::parse(R"(
    { "foo": "bar"}
    )");

    jsonpointer::add(example, "/baz", json("qux"));

    std::cout << example << std::endl;
}
```
Output:
```json
{ "foo": "bar", "baz" : "qux"}
```

#### Add an element to an array

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json example = json::parse(R"(
    { "foo": [ "bar", "baz" ] }
    )");

    jsonpointer::add(example, "/foo/1", json("qux"));

    std::cout << example << std::endl;
}
```
Output:
```json
{ "foo": [ "bar", "qux", "baz" ] }
```



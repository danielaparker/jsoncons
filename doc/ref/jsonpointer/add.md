### jsoncons::jsonpointer::add

Adds a `json` value.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
void add(const Json& root, typename Json::string_view_type path, const Json& value)
```

#### Exceptions

[parse_error](../parse_error.md)

### Examples

#### Add a member to an object

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json target = json::parse(R"(
    { "foo": "bar"}
    )");

    try
    {
        jsonpointer::add(target, "/baz", json("qux"));
        std::cout << target << std::endl;
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}
```
Output:
```json
{"baz":"qux","foo":"bar"}
```

#### Add an element to the second position in an array

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json target = json::parse(R"(
    { "foo": [ "bar", "baz" ] }
    )");

    try
    {
        jsonpointer::add(target, "/foo/1", json("qux"));
        std::cout << target << std::endl;
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}
```
Output:
```json
{"foo":["bar","qux","baz"]}
```

#### Add an element to the end of an array

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json target = json::parse(R"(
    { "foo": [ "bar", "baz" ] }
    )");

    try
    {
        jsonpointer::add(target, "/foo/-", json("qux"));
        std::cout << target << std::endl;
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}
```
Output:
```json
{"foo":["bar","baz","qux"]}
```

#### The specified index must not exceed the size of the array

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json target = json::parse(R"(
    { "foo": [ "bar", "baz" ] }
    )");

    try
    {
        jsonpointer::add(target, "/foo/3", json("qux"));
        std::cout << target << std::endl;
    }
    catch (const parse_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}
```
Output:
```
Index exceeds array size at line 1 and column 7
```



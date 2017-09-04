### jsoncons::jsonpointer::add, jsoncons::jsonpointer::try_add

Adds a `json` value.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
void add(const Json& root, typename Json::string_view_type path, const Json& value); // (1) 

template<class Json>
jsonpointer_errc add(const Json& root, typename Json::string_view_type path, const Json& value); // (2)
```

#### Exceptions

(1) On error, a [parse_error](../parse_error.md) exception that has an associated [jsonpointer_errc](jsonpointer_errc.md) error code.

#### Return value

(1) None

(2) On success, returns the selected Json value and a value-initialized [jsonpointer_errc](jsonpointer_errc.md)
On error, returns a null Json value and a [jsonpointer_errc](jsonpointer_errc.md) error code 

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

    auto ec = jsonpointer::try_add(target, "/foo/1", json("qux"));
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

    auto ec = jsonpointer::try_add(target, "/foo/3", json("qux"));
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
```
Index exceeds array size
```


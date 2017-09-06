### jsoncons::jsonpointer::insert

Adds a `json` value.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
jsonpointer_errc insert(Json& target, typename Json::string_view_type path, const Json& value); 
```

Inserts a value into the target at the specified path, if the path doesn't specify an object member that already has the same key.

- If `path` specifies an array index, a new value is inserted into the array at the specified index.

- If `path` specifies an object member that does not already exist, a new member is added to the object.

#### Return value

On success, a value-initialized [jsonpointer_errc](jsonpointer_errc.md). 

On error, a [jsonpointer_errc](jsonpointer_errc.md) error code 

### Examples

#### Add a member to a target location that does not already exist

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json target = json::parse(R"(
        { "foo": "bar"}
    )");

    auto ec = jsonpointer::insert_or_assign(target, "/baz", json("qux"));
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

    auto ec = jsonpointer::insert_or_assign(target, "/foo/1", json("qux"));
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

#### Add a value to the end of an array

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    auto ec = jsonpointer::insert_or_assign(target, "/foo/-", json("qux"));
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
{"foo":["bar","baz","qux"]}
```

#### Add an object member to a location that already exists

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json target = json::parse(R"(
        { "foo": "bar", "baz" : "abc"}
    )");

    auto ec = jsonpointer::insert_or_assign(target, "/baz", json("qux"));
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
{"baz":"qux","foo":"bar"}
```

#### Add a value to a location in an array that exceeds the size of the array

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json target = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    auto ec = jsonpointer::insert_or_assign(target, "/foo/3", json("qux"));
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


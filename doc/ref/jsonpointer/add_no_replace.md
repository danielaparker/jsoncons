### jsoncons::jsonpointer::add_no_replace

Adds a value to an object or inserts it into an array at the target location,
if a value does not already exist at that location.

```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json, class T>
void add_no_replace(Json& target, const Json::string_view_type& location, 
                    T&& value, bool create_if_missing = false);              (1) 

template<class Json, class T>
void add_no_replace(Json& target, const Json::string_view_type& location, 
                    T&& value, std::error_code& ec);                         (2) 

template<class Json, class T>
void add_no_replace(Json& target, const Json::string_view_type& location, 
                    T&& value, bool create_if_missing, std::error_code& ec); (3) 
```

Inserts a value into the target at the specified location, if the location doesn't specify an object member that already has the same key.

- If `location` specifies an array index, a new value is inserted into the array at the specified index.

- If `location` specifies an object member that does not already exist, a new member is added to the object.

#### Parameters
<table>
  <tr>
    <td>target</td>
    <td>JSON value</td> 
  </tr>
  <tr>
    <td>location</td>
    <td>JSON Pointer</td> 
  </tr>
  <tr>
    <td>value</td>
    <td>New value</td> 
  </tr>
  <tr>
    <td><code>create_if_missing</code> (since 0.162.0)</td>
    <td>Create key-object pairs when object key is missing</td> 
  </tr>
  <tr>
    <td><code>ec</code></td>
    <td>out-parameter for reporting errors in the non-throwing overload</td> 
  </tr>
</table>

#### Return value

None

### Exceptions

(1) Throws a [jsonpointer_error](jsonpointer_error.md) if `add_no_replace` fails.
 
(2) Sets the out-parameter `ec` to the [jsonpointer_error_category](jsonpointer_errc.md) if `add_no_replace` fails. 

### Examples

#### Add a member to a target location that does not already exist

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

// for brevity
using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    auto target = json::parse(R"(
        { "foo": "bar"}
    )");

    std::error_code ec;
    jsonpointer::add_no_replace(target, "/baz", json("qux"), ec);
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
{"baz":"qux","foo":"bar"}
```

#### Insert an element to the second position in an array

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    auto target = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    std::error_code ec;
    jsonpointer::add_no_replace(target, "/foo/1", json("qux"), ec);
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
{"foo":["bar","qux","baz"]}
```

#### Insert a value at the end of an array

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    auto target = json::parse(R"(
        { "foo": [ "bar", "baz" ] }
    )");

    std::error_code ec;
    jsonpointer::add_no_replace(target, "/foo/-", json("qux"), ec);
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
{"foo":["bar","baz","qux"]}
```

#### Attempt to add_no_replace object member at a location that already exists

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    auto target = json::parse(R"(
        { "foo": "bar", "baz" : "abc"}
    )");

    std::error_code ec;
    jsonpointer::add_no_replace(target, "/baz", json("qux"), ec);
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
```
Key already exists
```

#### Attempt to add_no_replace value to a location in an array that exceeds the size of the array

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    auto target = json::parse(R"(
    { "foo": [ "bar", "baz" ] }
    )");

    std::error_code ec;
    jsonpointer::add(target, "/foo/3", json("qux"), ec);
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
```
Index exceeds array size
```

#### add_no_replace with create if missing

```c++
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    std::vector<std::string> keys = { "foo","bar","baz" };

    jsonpointer::json_pointer ptr;
    for (const auto& key : keys)
    {
        ptr /= key;
    }

    json doc;
    jsonpointer::add_no_replace(doc, ptr, "str", true);

    std::cout << pretty_print(doc) << "\n\n";
}
```
Output:
```json
{
    "foo": {
        "bar": {
            "baz": "str"
        }
    }
}
```


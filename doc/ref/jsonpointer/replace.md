### jsoncons::jsonpointer::replace

Replace a `json` element or member.

```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json, class T>
void replace(Json& target, const Json::string_view_type& location, 
             T&& value, bool create_if_missing = false);              (1)

template<class Json, class T>
void replace(Json& target, const Json::string_view_type& location, 
             T&& value, std::error_code& ec);                         (2)

template<class Json, class T>
void replace(Json& target, const Json::string_view_type& location, 
             T&& value, bool create_if_missing, std::error_code& ec); (3)
```

Replaces the value at the location specified by `location` with a new value. 

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
    <td>Replacement value</td> 
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

#### Exceptions

(1) Throws a [jsonpointer_error](jsonpointer_error.md) if `replace` fails.
 
(2) Sets the out-parameter `ec` to the [jsonpointer_error_category](jsonpointer_errc.md) if `replace` fails. 

### Examples

#### Replace an object value

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    auto target = json::parse(R"(
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

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    auto target = json::parse(R"(
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

#### Replace a value at a location after creating objects when missing object keys

```c++
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    std::vector<std::string> keys = {"foo","bar","baz"};

    jsonpointer::json_pointer location;
    for (const auto& key : keys)
    {
        location /= key;
    }

    json doc;
    jsonpointer::replace(doc, location, "str", true);

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


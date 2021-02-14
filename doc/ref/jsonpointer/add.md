### jsoncons::jsonpointer::add

Adds a value to an object or inserts it into an array at the target location.
If a value already exists at the target location, that value is replaced.

```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json, class T>
void add(Json& target, const Json::string_view_type& location, 
         T&& value, bool create_if_missing = false);              (1)

template<class Json, class T>
void add(Json& target, const Json::string_view_type& location, 
         T&& value, std::error_code& ec);                         (2)

template<class Json, class T>
void add(Json& target, const Json::string_view_type& location, 
         T&& value, bool create_if_missing, std::error_code& ec); (3) (since 0.162.0)
```

Inserts a value into the target at the specified location, or if the location specifies an object member that already has the same key, assigns the new value to that member

- If `location` specifies an array index, a new value is inserted into the array at the specified index.

- If `location` specifies an object member that does not already exist, a new member is added to the object.

- If `location` specifies an object member that does exist, that member's value is replaced.

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
    <td>New or replacement value</td> 
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

(1) Throws a [jsonpointer_error](jsonpointer_error.md) if `add` fails.
 
(2)-(3) Sets the out-parameter `ec` to the [jsonpointer_error_category](jsonpointer_errc.md) if `add` fails. 
 
### Examples

#### Insert or assign an object member at a location that already exists

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
    jsonpointer::add(target, "/baz", json("qux"), ec);
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

#### Add a value to a location after creating objects when missing object keys

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
    jsonpointer::add(doc, location, "str", true);

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



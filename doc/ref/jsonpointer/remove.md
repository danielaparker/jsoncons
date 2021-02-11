### jsoncons::jsonpointer::remove

Removes a `json` element.

```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
void remove(Json& target, const Json::string_view_type& location); (1)

template<class Json>
void remove(Json& target, const Json::string_view_type& location, 
            std::error_code& ec);                              (2)
```

Removes the value at the location specifed by `location`.

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
    <td><code>ec</code></td>
    <td>out-parameter for reporting errors in the non-throwing overload</td> 
  </tr>
</table>

#### Return value

None

### Exceptions

(1) Throws a [jsonpointer_error](jsonpointer_error.md) if `remove` fails.
 
(2) Sets the out-parameter `ec` to the [jsonpointer_error_category](jsonpointer_errc.md) if `remove` fails. 

### Examples

#### Remove an object member

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    auto target = json::parse(R"(
        { "foo": "bar", "baz" : "qux"}
    )");

    std::error_code ec;
    jsonpointer::remove(target, "/baz", ec);
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
{"foo":"bar"}
```

#### Remove an array element

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    auto target = json::parse(R"(
        { "foo": [ "bar", "qux", "baz" ] }
    )");

    std::error_code ec;
    jsonpointer::remove(target, "/foo/1", ec);
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
{"foo":["bar","baz"]}
```



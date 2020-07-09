### jsoncons::jsonpointer::add

```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class J>
void add(J& target, const typename J::string_view_type& path, const J& value); // (1)

template<class J>
void add(J& target, const typename J::string_view_type& path, const J& value, std::error_code& ec); // (2)
```

Inserts a value into the target at the specified path, or if the path specifies an object member that already has the same key, assigns the new value to that member

- If `path` specifies an array index, a new value is inserted into the array at the specified index.

- If `path` specifies an object member that does not already exist, a new member is added to the object.

- If `path` specifies an object member that does exist, that member's value is replaced.

#### Return value

None

### Exceptions

(1) Throws a [jsonpointer_error](jsonpointer_error.md) if `add` fails.
 
(2) Sets the out-parameter `ec` to the [jsonpointer_error_category](jsonpointer_errc.md) if `add` fails. 

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



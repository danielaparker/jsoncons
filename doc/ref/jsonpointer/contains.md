### jsoncons::jsonpointer::contains

Returns `true` if the json doc contains the given json pointer, otherwise `false'

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
bool contains(const Json& doc, const typename Json::string_view_type& path);

```

#### Return value

Returns `true` if the json doc contains the given json pointer, otherwise `false'

### Examples

#### Examples from [RFC6901](https://tools.ietf.org/html/rfc6901)

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    // Example from RFC 6901
    const json example = json::parse(R"(
       {
          "foo": ["bar", "baz"],
          "": 0,
          "a/b": 1,
          "c%d": 2,
          "e^f": 3,
          "g|h": 4,
          "i\\j": 5,
          "k\"l": 6,
          " ": 7,
          "m~n": 8
       }
    )");

    std::cout << "(1) " << jsonpointer::contains(example, "/foo/0") << std::endl;
    std::cout << "(2) " << jsonpointer::contains(example, "e^g") << std::endl;
}
```
Output:
```json
(1) true
(2) false
```


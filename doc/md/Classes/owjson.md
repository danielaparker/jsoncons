```c++
jsoncons::owjson

typedef basic_json<wchar_t,
                   JsonTraits = ojson_traits<wchar_t>,
                   Allocator = std::allocator<wchar_t>> owjson
```
The `owjson` class is an instantiation of the `basic_json` class template that uses `wchar_t` as the character type. The supplied `JsonTraits` template parameter keeps object members in their original order. 

The `jsoncons` library will always rebind the supplied allocator from the template parameter to internal data structures.

### Header
```c++
#include <jsoncons/json.hpp>
```
### Interface

The interface is the same as [wjson](wjson), substituting wide character instantiations of classes - `std::wstring`, `std::wistream`, etc. - for narrow character ones.

- In `owjson`, `object_allocator` is a random access iterator, so object members can be accessed by position, e.g.

```
owjson o = owjson::parse(R"(
{
    "street_number" : "100",
    "street_name" : "Queen St W"
}
)");

owjson::member_type member = o.object_range().begin()[1];
std::cout << member.name() << "=" << member.value() << std::endl;
```
- `owjson`, like `wjson`, supports object member `set` methods that take an `object_iterator` as the first parameter. But while with `wjson` that parameter is just a hint that allows optimization, with `owjson` it is the actual location where to insert the member.

- In `owjson`, the `set` members that just take a name and a value always insert the member at the end.

### See also

- [wjson](wjson) constructs a wide character json value that sorts name-value members alphabetically

- [json](json) constructs a narrow character json value that sorts name-value members alphabetically

- [ojson](ojson) constructs a narrow character json value that retains the original name-value insertion order


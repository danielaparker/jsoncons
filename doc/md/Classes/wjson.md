```c++
jsoncons::wjson

typedef basic_json<wchar_t,
                   JsonTraits = json_traits<wchar_t>,
                   Allocator = std::allocator<wchar_t>> wjson
```
The `wjson` class is an instantiation of the `basic_json` class template that uses `wchar_t` as the character type. The supplied `JsonTraits` template parameter keeps object members in alphabetically sorted order. If you prefer to retain the original insertion order, use [owjson](owjson) instead.

The `jsoncons` library will always rebind the supplied allocator from the template parameter to internal data structures.

### Header
```c++
#include <jsoncons/json.hpp>
```
### Interface

The interface is the same as [json](json), substituting wide character instantiations of classes - `std::wstring`, `std::wistream`, etc. - for narrow character ones.

### See also

- [owjson](owjson) constructs a wide character json value that retains the original name-value insertion order

- [json](json) constructs a narrow character json value that sorts name-value members alphabetically

- [ojson](ojson) constructs a narrow character json value that retains the original name-value insertion order


```c++
jsoncons::wojson

typedef basic_json<wchar_t,
                   JsonTraits = o_json_traits<wchar_t>,
                   Allocator = std::allocator<wchar_t>> wojson
```
The `wojson` class is an instantiation of the `basic_json` class template that uses `wchar_t` as the character type. The original insertion order of an object's name/value pairs is preserved. 

The `jsoncons` library will always rebind the supplied allocator from the template parameter to internal data structures.

### Header
```c++
#include <jsoncons/json.hpp>
```
### Interface

The interface is the same as [wjson](wjson), substituting wide character instantiations of classes - `std::wstring`, `std::wistream`, etc. - for utf8 character ones.

- `wojson`, like `wjson`, supports object member `set` methods that take an `object_iterator` as the first parameter. But while with `wjson` that parameter is just a hint that allows optimization, with `wojson` it is the actual location where to insert the member.

- In `wojson`, the `set` members that just take a name and a value always insert the member at the end.

### See also

- [wjson](wjson) constructs a wide character json value that sorts name-value members alphabetically

- [json](json) constructs a utf8 character json value that sorts name-value members alphabetically

- [ojson](ojson) constructs a utf8 character json value that preserves the original insertion order of an object's name/value pairs


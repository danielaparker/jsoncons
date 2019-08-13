### jsoncons::wjson_options

```c++
#include <jsoncons/json_options.hpp>

typedef basic_json_options<wchar_t> wjson_options
```
The wjson_options class is an instantiation of the basic_json_options class template that uses `wchar_t` as the character type.

#### Interface

The interface is the same as [basic_json_options](basic_json_options.md), substituting wide character instantiations of classes - `std::wstring`, etc. - for utf8 character ones.

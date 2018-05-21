### jsoncons::wjson_serializing_options

```c++
typedef basic_json_serializing_options<wchar_t> wjson_serializing_options
```
The wjson_serializing_options class is an instantiation of the basic_json_serializing_options class template that uses `wchar_t` as the character type.

#### Header

```c++
#include <jsoncons/json_serializing_options.hpp>
```

#### Interface

The interface is the same as [json_serializing_options](json_serializing_options.md), substituting wide character instantiations of classes - `std::wstring`, etc. - for utf8 character ones.

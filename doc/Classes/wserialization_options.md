### jsoncons::wserialization_options

```c++
typedef basic_serialization_options<wchar_t> wserialization_options
```
The wserialization_options class is an instantiation of the basic_serialization_options class template that uses `wchar_t` as the character type.

#### Header

```c++
#include <jsoncons/serialization_options.hpp>
```

#### Interface

The interface is the same as [serialization_options](serialization_options.md), substituting wide character instantiations of classes - `std::wstring`, etc. - for utf8 character ones.

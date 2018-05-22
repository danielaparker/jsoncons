### jsoncons::wjson_serializer

```c++
typedef basic_json_serializer<wchar_t> wjson_serializer
```

The `wjson_serializer` class is an instantiation of the `basic_json_serializer` class template that uses `wchar_t` as the character type. It implements [wjson_content_handler](basic_json_content_handler.md) and supports pretty print serialization.

#### Header

    #include <jsoncons/json_serializer.hpp>

#### Interface

The interface is the same as [json_serializer](json_serializer.md), substituting wide character instantiations of classes - `std::wstring`, etc. - for utf8 character ones.

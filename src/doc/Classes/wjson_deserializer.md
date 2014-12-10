    jsoncons::wjson_deserializer

    typedef basic_json_deserializer<wchar_t,std::allocator<void>> wjson_deserializer

The `wjson_deserializer` class is an instantiation of the `basic_json_deserializer` class template that uses `wchar_t` as the character type.

### Header

    #include "jsoncons/json_deserializer.hpp"

### Interface

The interface is the same as [json_deserializer](json_deserializer), substituting wide character instantiations of classes - `std::wstring`, etc. - for narrow character ones.



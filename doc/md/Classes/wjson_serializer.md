    jsoncons::wjson_serializer

    typedef basic_json_serializer<wchar_t> wjson_serializer

The `wjson_serializer` class is an instantiation of the `basic_json_serializer` class template that uses `wchar_t` as the character type. It implements [wjson_output_handler](basic_json_output_handler) and supports pretty print serialization.

### Header

    #include "jsoncons/json_serializer.hpp"

### Interface

The interface is the same as [json_serializer](json_serializer), substituting wide character instantiations of classes - `std::wstring`, etc. - for narrow character ones.

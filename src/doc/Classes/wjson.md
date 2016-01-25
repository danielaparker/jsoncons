    jsoncons::wjson

    typedef basic_json<wchar_t,std::allocator<char>> wjson

The `wjson` class is an instantiation of the `basic_json` class template that uses `wchar_t` as the character type
and `std::allocator<wchar_t>` as the allocator type. The `jsoncons` library will rebind the supplied allocator to the types it allocates, `char`, `json::array` and `json::object`, and the internal containers`std::basic_string` and `std::vector` will rebind as well.

### Header

    #include "jsoncons/json.hpp"

### Interface

The interface is the same as [json](json), substituting wide character instantiations of classes - `std::wstring`, `std::wistream`, etc. - for narrow character ones.

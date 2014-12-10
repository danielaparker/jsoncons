    jsoncons::wjson

    typedef basic_json<wchar_t,std::allocator<void>> wjson

The `wjson` class is an instantiation of the `basic_json` class template that uses `wchar_t` as the character type
and `std::allocator<void>` as the allocator type. The allocator type is used to supply an allocator for dynamically allocated, 
fixed size small objects in the `wjson` container, the `wjson` container will rebind it as necessary. The allocator type
is not used for structures including vectors and strings that use large or variable amounts of memory, 
these always use the default allocators.

### Header

    #include "jsoncons/json.hpp"

### Interface

The interface is the same as [json](json), substituting wide character instantiations of classes - `std::wstring`, `std::wistream`, etc. - for narrow character ones.

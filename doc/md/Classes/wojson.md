    jsoncons::wojson

    typedef basic_json<wchar_t,
                       JsonTraits = ojson_traits<wchar_t>,
                       Allocator = std::allocator<wchar_t>> wojson

The `wojson` class is an instantiation of the `basic_json` class template that uses `wchar_t` as the character type. The supplied `JsonTraits` template parameter keeps object members in their original order. The `jsoncons` library will always rebind the supplied allocator from the template parameter to internal data structures.

### Header

    #include "jsoncons/json.hpp"

### Interface

The interface is the same as [ojson](ojson), substituting wide character instantiations of classes - `std::wstring`, `std::wistream`, etc. - for narrow character ones.

    jsoncons::wjson

    typedef basic_json<wchar_t,storage<wchar_t>> wjson

The `wjson` class is an instantiation of the `basic_json` class template that uses `wchar_t` as the character type.

### Header

    #include "jsoncons/json.hpp"

### Interface

The interface is the same as [json](json), substituting wide character instantiations of classes - `std::wstring`, `std::wistream`, etc. - for narrow character ones.

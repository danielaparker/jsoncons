```c++
    jsoncons::ojson

    typedef basic_json<char,
                       JsonTraits = ojson_traits<char>,
                       Allocator = std::allocator<char>> json
```
The `ojson` class is an instantiation of the `basic_json` class template that uses `char` as the character type. The supplied `JsonTraits` template parameter keeps object members in their original order. The `jsoncons` library will always rebind the supplied allocator from the template parameter to internal data structures.

### Header

    #include "jsoncons/json.hpp"



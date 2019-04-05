### jsoncons::ubjson::decode_ubjson

Decodes a [Universal Binary JSON Specification](http://ubjson.org/) data format to a json value.

#### Header
```c++
#include <jsoncons_ext/ubjson/ubjson.hpp>

template<class T>
T decode_ubjson(const std::vector<uint8_t>& v); // (1)

template<class T>
T decode_ubjson(std::istream>& is); // (2)
```

#### Exceptions

Throws [ser_error](../serialization_error.md) if parsing fails.

#### See also

- [encode_ubjson](encode_ubjson.md) encodes a json value to the [Universal Binary JSON Specification](http://ubjson.org/) data format.



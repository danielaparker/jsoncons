### jsoncons::ubjson::decode_ubjson

Decodes a [Universal Binary JSON Specification](http://ubjson.org/) data format to a json value.

#### Header
```c++
#include <jsoncons_ext/ubjson/ubjson.hpp>

template<class Json>
Json decode_ubjson(const std::vector<uint8_t>& v); // (1)

template<class Json>
Json decode_ubjson(std::basic_istream<typename Json::char_type>& is); // (2)
```

#### Exceptions

Throws [serialization_error](../serialization_error.md) if parsing fails.

#### See also

- [encode_ubjson](encode_ubjson.md) encodes a json value to the [Universal Binary JSON Specification](http://ubjson.org/) data format.



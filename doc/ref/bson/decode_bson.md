### jsoncons::bson::decode_bson

Decodes a [Bin­ary JSON](http://bsonspec.org/) binary serialization format to a json value.

#### Header
```c++
#include <jsoncons_ext/bson/bson.hpp>

template<class Json>
Json decode_bson(const std::vector<uint8_t>& v)
```

#### Exceptions

Throws [parse_error](../parse_error.md) if parsing fails.

#### See also

- [encode_bson](encode_bson.md) encodes a json value to the [Bin­ary JSON](http://bsonspec.org/) binary serialization format.



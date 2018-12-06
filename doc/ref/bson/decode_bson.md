### jsoncons::bson::decode_bson

Decodes a [bson](http://bsonspec.org/) binary serialization format to a json value.

#### Header
```c++
#include <jsoncons_ext/bson/bson.hpp>

template<class Json>
Json decode_bson(const std::vector<uint8_t>& v)
```

#### See also

- [encode_bson](encode_bson.md) encodes a json value to the [bson](http://bsonspec.org/) binary serialization format.



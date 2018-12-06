### jsoncons::bson::encode_bson

Encodes a json value to the [bson](http://bsonspec.org/) binary serialization format.

#### Header
```c++
#include <jsoncons_ext/bson/bson.hpp>

template<class Json>
void encode_bson(const Json& jval, std::vector<uint8_t>& v)
```

#### See also

- [decode_bson](decode_bson) decodes a [bson](http://bsonspec.org/) binary serialization format to a json value.


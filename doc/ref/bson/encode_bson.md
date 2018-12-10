### jsoncons::bson::encode_bson

Encodes a json value to the [Bin­ary JSON](http://bsonspec.org/) data format.

#### Header
```c++
#include <jsoncons_ext/bson/bson.hpp>

template<class Json>
void encode_bson(const Json& jval, std::vector<uint8_t>& v)
```

#### See also

- [decode_bson](decode_bson) decodes a [Bin­ary JSON](http://bsonspec.org/) data format to a json value.


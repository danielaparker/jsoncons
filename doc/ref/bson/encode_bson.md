### jsoncons::bson::encode_bson

Encodes a json value to the [Bin­ary JSON](http://bsonspec.org/) data format.

#### Header
```c++
#include <jsoncons_ext/bson/bson.hpp>

template<class T>
void encode_bson(const T& jval, std::vector<uint8_t>& v); // (1)

template<class T>
void encode_bson(const T& jval, std::ostream& os); // (2)
```

#### See also

- [decode_bson](decode_bson) decodes a [Bin­ary JSON](http://bsonspec.org/) data format to a json value.


### jsoncons::ubjson::encode_ubjson

Encodes a json value to the [Bin­ary JSON](http://ubjsonspec.org/) data format.

#### Header
```c++
#include <jsoncons_ext/ubjson/ubjson.hpp>

template<class Json>
void encode_ubjson(const Json& jval, std::vector<uint8_t>& v)
```

#### See also

- [decode_ubjson](decode_ubjson) decodes a [Binary JSON](http://ubjsonspec.org/) data format to a json value.


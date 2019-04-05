### jsoncons::ubjson::encode_ubjson

Encodes a json value to the [Bin­ary JSON](http://ubjsonspec.org/) data format.

#### Header
```c++
#include <jsoncons_ext/ubjson/ubjson.hpp>

template<class T>
void encode_ubjson(const T& jval, std::vector<uint8_t>& v); // (1)

template<class T>
void encode_ubjson(const T& jval, std::ostream& os); // (2)
```

#### See also

- [decode_ubjson](decode_ubjson) decodes a [Binary JSON](http://ubjsonspec.org/) data format to a json value.


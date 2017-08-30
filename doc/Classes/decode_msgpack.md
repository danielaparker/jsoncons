### jsoncons::msgpack::decode_msgpack

Decodes a [MessagePack](http://msgpack.org/index.html) binary serialization format to a json value.

#### Header
```c++
#include <jsoncons_ext/msgpack/msgpack.hpp>

template<class Json>
Json decode_msgpack(const std::vector<uint8_t>& v)
```

#### See also

- [encode_msgpack](encode_msgpack.md) encodes a json value to the [MessagePack](http://msgpack.org/index.html) binary serialization format.



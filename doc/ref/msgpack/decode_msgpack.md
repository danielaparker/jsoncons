### jsoncons::msgpack::decode_msgpack

Decodes a [MessagePack](http://msgpack.org/index.html) data format to a json value.

#### Header
```c++
#include <jsoncons_ext/msgpack/msgpack.hpp>

template<class T>
T decode_msgpack(const msgpack_view& v); // (1)

template<class T>
T decode_msgpack(std::istream& is); // (2)
```

#### Exceptions

Throws [ser_error](../serialization_error.md) if parsing fails.

#### See also

- [encode_msgpack](encode_msgpack.md) encodes a json value to the [MessagePack](http://msgpack.org/index.html) data format.



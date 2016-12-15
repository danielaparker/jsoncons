```c++
jsoncons::binary::decode_message_pack
```
Decodes a [MessagePack](http://msgpack.org/index.html) binary serialization format to a json value.

### Header
```c++
#include <jsoncons/binary/message_pack.hpp>

template<class Json>
Json decode_message_pack(const std::vector<uint8_t>& v)
```

### See also

- [encode_message_pack](encode_message_pack) encodes a json value to the [MessagePack](http://msgpack.org/index.html) binary serialization format.



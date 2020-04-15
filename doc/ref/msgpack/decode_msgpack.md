### jsoncons::msgpack::decode_msgpack

```c++
#include <jsoncons_ext/msgpack/msgpack.hpp>

template<class T>
T decode_msgpack(const std::vector<uint8_t>& v,
                 const msgpack_decode_options& options = msgpack_decode_options()); // (1)

template<class T>
T decode_msgpack(std::istream& is,
                 const msgpack_decode_options& options = msgpack_decode_options()); // (2)

template<class T,class TempAllocator>
T decode_msgpack(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                 const std::vector<uint8_t>& v,
                 const msgpack_decode_options& options = msgpack_decode_options()); // (3)

template<class T,class TempAllocator>
T decode_msgpack(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                 std::istream& is,
                 const msgpack_decode_options& options = msgpack_decode_options()); // (4)
```

Decodes a [MessagePack](http://msgpack.org/index.html) data format into a C++ data structure.

(1) Reads a MessagePack bytes buffer into a type T, using the specified (or defaulted) [options](msgpack_options.md). 
Type T must be an instantiation of [basic_json](../basic_json.md) 
or support [json_type_traits](../json_type_traits.md).

(2) Reads a MessagePack binary stream into a type T, using the specified (or defaulted) [options](msgpack_options.md). 
Type T must be an instantiation of [basic_json](../basic_json.md) 
or support [json_type_traits](../json_type_traits.md).

#### Exceptions

Throws [ser_error](../ser_error.md) if parsing fails.

### See also

- [encode_msgpack](encode_msgpack.md) encodes a json value to the [MessagePack](http://msgpack.org/index.html) data format.



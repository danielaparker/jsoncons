### jsoncons::ubjson::decode_ubjson

Decodes a [Universal Binary JSON Specification (JSON)](http://ubjson.org/) data format into a C++ data structure.

```c++
#include <jsoncons_ext/ubjson/ubjson.hpp>

template<class T>
T decode_ubjson(const std::vector<uint8_t>& v,
                const bson_decode_options& options = bson_decode_options()); // (1)

template<class T>
T decode_ubjson(std::istream>& is,
                const bson_decode_options& options = bson_decode_options()); // (2)

template<class T,class TempAllocator>
T decode_ubjson(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const std::vector<uint8_t>& v,
                const bson_decode_options& options = bson_decode_options()); // (3)

template<class T,class TempAllocator>
T decode_ubjson(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                std::istream>& is,
                const bson_decode_options& options = bson_decode_options()); // (4)
```

(1) Reads a UBJSON bytes buffer into a type T, using the specified (or defaulted) [options](ubjson_options.md). 
Type T must be an instantiation of [basic_json](../basic_json.md) 
or support [json_type_traits](../json_type_traits.md).  

(2) Reads a UBJSON binary stream into a type T, using the specified (or defaulted) [options](ubjson_options.md). 
Type T must be an instantiation of [basic_json](../basic_json.md) 
or support [json_type_traits](../json_type_traits.md).

#### Exceptions

Throws [ser_error](../ser_error.md) if parsing fails.

### See also

- [encode_ubjson](encode_ubjson.md) encodes a json value to the [Universal Binary JSON Specification](http://ubjson.org/) data format.



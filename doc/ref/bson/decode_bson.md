### jsoncons::bson::decode_bson

Decodes a [Binary JSON (BSON)](http://bsonspec.org/) data format into a C++ data structure.

```c++
#include <jsoncons_ext/bson/bson.hpp>

template<class T>
T decode_bson(const std::vector<uint8_t>& source,
              const bson_decode_options& options = bson_decode_options()); // (1) (until v0.152.0)

template<class T, class Source>
T decode_bson(const Source& source, 
              const bson_decode_options& options = bson_decode_options()); // (1) (since v0.152.0)

template<class T>
T decode_bson(std::istream& is,
              const bson_decode_options& options = bson_decode_options()); // (2)

template<class T,class TempAllocator>
T decode_bson(temp_allocator_arg_t, const TempAllocator& temp_alloc,
              const std::vector<uint8_t>& source,
              const bson_decode_options& options = bson_decode_options()); // (3) (until v0.152.0)

template<class T, class Source, class TempAllocator>
T decode_bson(temp_allocator_arg_t, const TempAllocator& temp_alloc,
              const Source& source,
              const bson_decode_options& options = bson_decode_options()); // (3) (since v0.152.0)

template<class T,class TempAllocator>
T decode_bson(temp_allocator_arg_t, const TempAllocator& temp_alloc,
              std::istream& is,
              const bson_decode_options& options = bson_decode_options()); // (4)
```

(1) Reads a BSON contiguous byte sequence provided by `source` into a type T, using the specified (or defaulted) [options](bson_options.md). 
Type `Source` must be a container that has member functions `data()` and `size()`, 
and member type `value_type` an integral type of size exactly 8 bits (since v0.152.0.)
Type T must be an instantiation of [basic_json](../basic_json.md) 
or support [json_type_traits](../json_type_traits.md). 

(2) Reads a BSON binary stream into a type T, using the specified (or defaulted) [options](bson_options.md). 
Type T must be an instantiation of [basic_json](../basic_json.md) 
or support [json_type_traits](../json_type_traits.md). 

#### Exceptions

Throws [ser_error](../ser_error.md) if parsing fails.

### See also

- [encode_bson](encode_bson.md) encodes a json value to the [Bin­ary JSON](http://bsonspec.org/) data format.



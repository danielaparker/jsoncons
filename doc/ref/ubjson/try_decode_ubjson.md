### jsoncons::ubjson::try_decode_ubjson

Decodes a [Universal Binary JSON Specification (JSON)](http://ubjson.org/) data format into a C++ data structure.

```cpp
#include <jsoncons_ext/ubjson/ubjson.hpp>

template <typename T,typename BytesLike>
read_result<T> try_decode_ubjson(const BytesLike& source,
    const bson_decode_options& options = bson_decode_options());               (1) (since 1.4.0)

template <typename T>
read_result<T> try_decode_ubjson(std::istream>& is,
    const bson_decode_options& options = bson_decode_options());               (2) (since 1.4.0)

template <typename T,typename BytesLike,typename Allocator,typename TempAllocator>
read_result<T> try_decode_ubjson(const allocator_set<Allocator,TempAllocator>& alloc_set,
    const BytesLike& source,
    const ubjson_decode_options& options = ubjson_decode_options());           (3) (since 1.4.0)

template <typename T,typename Allocator,typename TempAllocator>
read_result<T> try_decode_ubjson(const allocator_set<Allocator,TempAllocator>& alloc_set,
    std::istream& is,
    const ubjson_decode_options& options = ubjson_decode_options());           (4) (since 1.4.0)

template <typename T,typename InputIt>
read_result<T> try_decode_ubjson(InputIt first, InputIt last,
    const bson_decode_options& options = bson_decode_options());               (5) (since 1.4.0)
```

(1) Reads UBJSON data from a contiguous byte sequence provided by `source` into a type T, using the specified (or defaulted) [options](ubjson_options.md). 
Type `BytesLike` must be a container that has member functions `data()` and `size()`, 
and member type `value_type` with size exactly 8 bits (since 0.152.0.)
Any of the values types `int8_t`, `uint8_t`, `char`, `unsigned char` and `std::byte` (since C++17) are allowed.
Type 'T' must be an instantiation of [basic_json](../basic_json.md) or support jsoncons reflection traits.

(2) Reads UBJSON data from a binary stream into a type T, using the specified (or defaulted) [options](ubjson_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) or support jsoncons reflection traits.

(3)-(4) are identical to (1)-(2) except an [allocator_set](allocator_set.md) is passed as an additional argument and
provides allocators for result data and temporary allocations.

(5) Reads UBJSON data from the range [`first`,`last`) into a type T, using the specified (or defaulted) [options](ubjson_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) or support jsoncons reflection traits.

#### Return value

[read_result<T>](read_result.md)

#### Exceptions

Any overload may throw std::bad_alloc if memory allocation fails.

### See also

[encode_ubjson](encode_ubjson.md) encodes a json value to the [Universal Binary JSON Specification](http://ubjson.org/) data format.



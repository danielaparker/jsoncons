### jsoncons::ubjson::decode_ubjson

Decodes a [Universal Binary JSON Specification (JSON)](http://ubjson.org/) data format into a C++ data structure.

```c++
#include <jsoncons_ext/ubjson/ubjson.hpp>

template<class T>
T decode_ubjson(const std::vector<uint8_t>& source,
                const bson_decode_options& options = bson_decode_options()); (1) (until 0.152.0)

template<class T, class Source>
T decode_ubjson(const Source& source,
                const bson_decode_options& options = bson_decode_options()); (1) (since 0.152.0)

template<class T>
T decode_ubjson(std::istream>& is,
                const bson_decode_options& options = bson_decode_options()); (2)

template<class T, class InputIt>
T decode_ubjson(InputIt first, InputIt last,
                const bson_decode_options& options = bson_decode_options()); (3)
```

(1) Reads UBJSON data from a contiguous byte sequence provided by `source` into a type T, using the specified (or defaulted) [options](ubjson_options.md). 
Type `Source` must be a container that has member functions `data()` and `size()`, 
and member type `value_type` with size exactly 8 bits (since 0.152.0.)
Any of the values types `int8_t`, `uint8_t`, `char`, `unsigned char` and `std::byte` (since C++17) are allowed.
Type 'T' must be an instantiation of [basic_json](../basic_json.md) 
or support [json_type_traits](../json_type_traits.md).  

(2) Reads UBJSON data from a binary stream into a type T, using the specified (or defaulted) [options](ubjson_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) 
or support [json_type_traits](../json_type_traits.md).

(3) Reads UBJSON data from the range [`first`,`last`) into a type T, using the specified (or defaulted) [options](ubjson_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) 
or support [json_type_traits](../json_type_traits.md).

#### Exceptions

Throws a [ser_error](../ser_error.md) if parsing fails, and a [conv_error](conv_error.md) if type conversion fails.

### See also

[encode_ubjson](encode_ubjson.md) encodes a json value to the [Universal Binary JSON Specification](http://ubjson.org/) data format.



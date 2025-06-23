### jsoncons::bson::decode_bson

Decodes a [Binary JSON (BSON)](http://bsonspec.org/) data format into a C++ data structure.

```cpp
#include <jsoncons_ext/bson/bson.hpp>

template <typename T,typename BytesLike>
T decode_bson(const BytesLike& source, 
    const bson_decode_options& options = bson_decode_options());        (1) 

template <typename T>
T decode_bson(std::istream& is,
    const bson_decode_options& options = bson_decode_options());        (2)

template <typename T,typename BytesLike,typename Alloc,typename TempAlloc>
T decode_bson(const allocator_set<Alloc,TempAlloc>& alloc_set,
    const BytesLike& source,
    const bson_decode_options& options = bson_decode_options());        (3) (since 0.171.0)

template <typename T,typename Alloc,typename TempAlloc>
T decode_bson(const allocator_set<Alloc,TempAlloc>& alloc_set,
    std::istream& is,
    const bson_decode_options& options = bson_decode_options());        (4) (since 0.171.0)

template <typename T,typename InputIt>
T decode_bson(InputIt first, InputIt last,
    const bson_decode_options& options = bson_decode_options());        (5) (since 0.153.0)

template <typename T,typename BytesLike>
read_result<T> try_decode_bson(const BytesLike& source, 
    const bson_decode_options& options = bson_decode_options());        (6) (since 1.4.0)

template <typename T>
read_result<T> try_decode_bson(std::istream& is,
    const bson_decode_options& options = bson_decode_options());        (7) (since 1.4.0)

template <typename T,typename BytesLike,typename Alloc,typename TempAlloc>
read_result<T> try_decode_bson(const allocator_set<Alloc,TempAlloc>& alloc_set,
    const BytesLike& source,
    const bson_decode_options& options = bson_decode_options());        (8) (since 1.4.0)

template <typename T,typename Alloc,typename TempAlloc>
read_result<T> try_decode_bson(const allocator_set<Alloc,TempAlloc>& alloc_set,
    std::istream& is,
    const bson_decode_options& options = bson_decode_options());        (9) (since 1.4.0)

template <typename T,typename InputIt>
read_result<T> try_decode_bson(InputIt first, InputIt last,
    const bson_decode_options& options = bson_decode_options());        (10) (since 1.4.0)
```

(1) Reads BSON data from a contiguous byte sequence provided by `source` into a type T, using the specified (or defaulted) [options](bson_options.md). 
Type `BytesLike` must be a container that has member functions `data()` and `size()`, 
and member type `value_type` with size exactly 8 bits (since 0.152.0.)
Any of the values types `int8_t`, `uint8_t`, `char`, `unsigned char` and `std::byte` (since C++17) are allowed.
Type 'T' must be an instantiation of [basic_json](../corelib/basic_json.md) 
or support jsoncons reflection traits. 

(2) Reads BSON data from a binary stream into a type T, using the specified (or defaulted) [options](bson_options.md). 
Type 'T' must be an instantiation of [basic_json](../corelib/basic_json.md) 
or support jsoncons reflection traits. 

(3)-(4) are identical to (1)-(2) except an [allocator_set](allocator_set.md) is passed as an additional argument and
provides allocators for result data and temporary allocations.

(5) Reads BSON data from the range [`first`,`last`) into a type T, using the specified (or defaulted) [options](bson_options.md). 
Type 'T' must be an instantiation of [basic_json](../corelib/basic_json.md) 
or support jsoncons reflection traits. 

(6)-(10) Non-throwing versions of (1)-(5)

#### Return value

(1)-(5) Deserialized value

(6)-(10) [read_result<T>](../corelib/read_result.md)

#### Exceptions

(1)-(5) Throw [ser_error](../corelib/ser_error.md) if read fails.

Any overload may throw `std::bad_alloc` if memory allocation fails.

### Examples

#### Throwing overload

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/bson/bson.hpp>
#include <iostream>

int main()
{
    std::vector<uint8_t> input = { 0x13,0x00,0x00,0x00, // Document has 19 bytes
                                  0x05, // Binary data
                                  0x70,0x44,0x00, // "pD"
                                  0x05,0x00,0x00,0x00, // Length is 5
                                  0x80, // Subtype is 128
                                  0x48,0x65,0x6c,0x6c,0x6f, // 'H','e','l','l','o'
                                  0x00 // terminating null
    };

    json j = bson::decode_bson<json>(input);
    std::cout << "JSON:\n" << pretty_print(j) << "\n\n";

    std::cout << "tag: " << j["pD"].tag() << "\n";
    std::cout << "ext_tag: " << j["pD"].ext_tag() << "\n";
    auto bytes = j["pD"].as<std::vector<uint8_t>>();
    std::cout << "binary data: " << byte_string_view{ bytes } << "\n";
}
```
Output:
```
JSON:
{
    "pD": "SGVsbG8"
}

tag: ext
ext_tag: 128
binary data: 48,65,6c,6c,6f
```
Note that printing a json value by default encodes byte strings as base64url strings, but the json value holds the actual bytes.

#### Non-throwing overload

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/bson/bson.hpp>
#include <iostream>

namespace bson = jsoncons::bson;

int main()
{
    std::vector<uint8_t> input = {0x13, 0x00, 0x00, 0x00, // Document has 19 bytes
        0x05, // Binary data
        0x70, 0x44, 0x00, // "pD"
        0x05, 0x00, 0x00, 0x00, // Length is 5
        0x80, // Subtype is 128
        0x48, 0x65, 0x6c, 0x6c, 0x6f, // 'H','e','l','l','o'
        0x00 // terminating null
    };

    auto result = bson::try_decode_bson<jsoncons::json>(input);
    if (!result)
    {
        std::cout << result.error() << "\n";
        exit(1);
    }
    jsoncons::json& j(*result);
    std::cout << pretty_print(j) << "\n\n";
    std::cout << "tag: " << j["pD"].tag() << "\n";
    std::cout << "ext_tag: " << j["pD"].ext_tag() << "\n";
    auto bytes = j["pD"].as<std::vector<uint8_t>>();
    std::cout << "binary data: " << jsoncons::byte_string_view{bytes} << "\n";
}

```
Output:
```

{
    "pD": "SGVsbG8"
}

tag: ext
ext_tag: 128
binary data: 48,65,6c,6c,6f
```

### See also

[encode_bson](encode_bson.md) encodes a json value to the [Bin­ary JSON](http://bsonspec.org/) data format.



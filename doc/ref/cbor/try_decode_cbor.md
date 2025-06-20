### jsoncons::cbor::try_decode_cbor

```cpp
#include <jsoncons_ext/cbor/cbor.hpp>
```

<br>

Decodes a [Concise Binary Object Representation](http://cbor.io/) data format into a C++ data structure.

```cpp
template <typename T,typename BytesLike>
read_result<T> try_decode_cbor(const BytesLike& source,
    const cbor_decode_options& options = cbor_decode_options());        (1) (since 1.4.0)

template <typename T>
read_result<T> try_decode_cbor(std::istream& is,
    const cbor_decode_options& options = cbor_decode_options());        (2) (since 1.4.0)

template <typename T,typename BytesLike,typename Allocator,typename TempAllocator>
read_result<T> try_decode_cbor(const allocator_set<Allocator,TempAllocator>& alloc_set,
    const BytesLike& source,
    const cbor_decode_options& options = cbor_decode_options());        (3) (since 1.4.0)

template <typename T,typename Allocator,typename TempAllocator>
read_result<T> try_decode_cbor(const allocator_set<Allocator,TempAllocator>& alloc_set,
    std::istream& is,
    const cbor_decode_options& options = cbor_decode_options());        (4) (since 1.4.0)

template <typename T,typename InputIt>
read_result<T> try_decode_cbor(InputIt first, InputIt last,
    const cbor_decode_options& options = cbor_decode_options());        (5) (since 1.4.0)
```

(1) Reads CBOR data from a contiguous byte sequence provided by `source` into a type T, using the specified (or defaulted) [options](cbor_options.md). 
Type `BytesLike` must be a container that has member functions `data()` and `size()`, 
and member type `value_type` with size exactly 8 bits (since 0.152.0.)
Any of the values types `int8_t`, `uint8_t`, `char`, `unsigned char` and `std::byte` (since C++17) are allowed.
Type 'T' must be an instantiation of [basic_json](../basic_json.md) or support jsoncons reflection traits.

(2) Reads CBOR data from a binary stream into a type T, using the specified (or defaulted) [options](cbor_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) or support jsoncons reflection traits.

(3)-(4) are identical to (1)-(2) except an [allocator_set](allocator_set.md) is passed as an additional argument and
provides allocators for result data and temporary allocations.

(5) Reads CBOR data from the range [`first`,`last`) into a type T, using the specified (or defaulted) [options](cbor_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) or support jsoncons reflection traits.

#### Return value

[read_result<T>](read_result.md)

#### Exceptions

Any overload may throw `std::bad_alloc` if memory allocation fails.

### Examples

#### Round trip (JSON to CBOR bytes back to JSON)

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>

namespace cbor = jsoncons::cbor;

int main()
{
    auto j1 = jsoncons::ojson::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum",
           "assertion": "advanced",
           "rated": "Marilyn C",
           "rating": 0.90
         }
       ]
    }
    )");

    std::vector<uint8_t> v;
    cbor::encode_cbor(j1, v);

    auto result = cbor::try_decode_cbor<jsoncons::ojson>(v);
    if (!result)
    {
        std::cout << result.error() << "\n";
        exit(1);
    }
    jsoncons::ojson& j2(*result);
    std::cout << pretty_print(j2) << '\n';
}
```
Output:
```json
{
    "application": "hiking",
    "reputons": [
        {
            "rater": "HikingAsylum",
            "assertion": "advanced",
            "rated": "Marilyn C",
            "rating": 0.9
        }
    ]
}
```

### See also

[byte_string_view](../byte_string_view.md)  

[encode_cbor](encode_cbor.md) encodes a json value to the [Concise Binary Object Representation](http://cbor.io/) data format.  



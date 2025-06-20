### jsoncons::csv::try_decode_csv

Decodes a [comma-separated variables (CSV)](https://en.wikipedia.org/wiki/Comma-separated_values) data format into a C++ data structure.

```cpp
#include <jsoncons_ext/csv/csv.hpp>

template <typename T,typename CharsLike>
read_result<T> try_decode_csv(const CharsLike& s, 
    const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>()));     (1) (since 1.4.0)

template <typename T,typename CharT>
read_result<T> try_decode_csv(std::basic_istream<CharT>& is, 
    const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>()));     (2) (since 1.4.0)

template <typename T,typename CharsLike,typename Allocator,typename TempAllocator>
read_result<T> try_decode_csv(allocator_set<Allocator,TempAllocator> alloc_set,
    const CharsLike& s,                                                                       (3) (since 1.4.0)
    const basic_csv_decode_options<CharsLike::value_type>& options = 
        basic_csv_decode_options<CharsLike::value_type>()); 

template <typename T,typename CharT,typename Allocator,typename TempAllocator>
read_result<T> try_decode_csv(allocator_set<Allocator,TempAllocator> alloc_set,
    std::basic_istream<CharT>& is,
    const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>());      (4) (since 1.4.0)

template <typename T,typename InputIt>
read_result<T> try_decode_csv(InputIt first, InputIt last,
    const basic_csv_decode_options<CharT>& options = basic_csv_decode_options<CharT>()));     (5) (since 1.4.0)
```

(1) Reads CSV data from a contiguous character sequence into a type T, using the specified (or defaulted) [options](basic_csv_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) or support jsoncons reflection traits.

(2) Reads CSV data from an input stream into a type T, using the specified (or defaulted) [options](basic_csv_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) or support jsoncons reflection traits.

Functions (3)-(4) are identical to (1)-(2) except an [allocator_set](../allocator_set.md) is passed as an additional argument and
provides allocators for result data and temporary allocations.

(5) Reads CSV data from the range [`first`,`last`) into a type T, using the specified (or defaulted) [options](basic_csv_options.md). 
Type 'T' must be an instantiation of [basic_json](../basic_json.md) or support jsoncons reflection traits.

#### Return value

Returns a value of type `T`.

#### Return value

[read_result<T>](read_result.md)

#### Exceptions

Any overload may throw `std::bad_alloc` if memory allocation fails.

### Examples



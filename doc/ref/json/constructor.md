### `jsoncons::basic_json::basic_json`

```c++
constexpr basic_json(); // (1)

basic_json(const basic_json& other); // (2)

basic_json(const basic_json& other, const allocator_type& alloc); // (3)

basic_json(basic_json&& other) noexcept; // (4)

basic_json(basic_json&& other, const allocator_type& alloc) noexcept; // (5)

template <class T>
basic_json(const T& val); // (6)

template <class Unsigned>
constexpr basic_json(Unsigned val, semantic_tag tag); // (7)

template <class Signed>
constexpr basic_json(Signed val, semantic_tag tag); // (8)

constexpr basic_json(half_arg_t, uint16_t value, 
                     semantic_tag tag = semantic_tag::none); // (9)

constexpr basic_json(double val, semantic_tag tag); // (10)

explicit basic_json(json_object_arg_t, 
                    semantic_tag tag = semantic_tag::none, 
                    const Allocator& alloc = Allocator()); // (11) 

template<class InputIt>
basic_json(json_object_arg_t, 
           InputIt first, InputIt last, 
           semantic_tag tag = semantic_tag::none,
           const Allocator& alloc = Allocator()); // (12) 

basic_json(json_object_arg_t, 
           std::initializer_list<std::pair<std::basic_string<char_type>,basic_json>> init, 
           semantic_tag tag = semantic_tag::none, 
           const Allocator& alloc = Allocator()); // (13)

explicit basic_json(json_array_arg_t, 
                    semantic_tag tag = semantic_tag::none, 
                    const Allocator& alloc = Allocator()); // (14)

template<class InputIt>
basic_json(json_array_arg_t, 
           InputIt first, InputIt last, 
           semantic_tag tag = semantic_tag::none, 
           const Allocator& alloc = Allocator()); // (15) 

basic_json(json_array_arg_t, 
           std::initializer_list<basic_json> init, 
           semantic_tag tag = semantic_tag::none, 
           const Allocator& alloc = Allocator()); // (16)

template <class T>
basic_json(const T& val, const allocator_type& alloc); // (17)

basic_json(const char_type* val); // (18)

basic_json(const char_type* val, const allocator_type& alloc); // (19)

basic_json(const byte_string_view& bytes, 
           semantic_tag tag, 
           const Allocator& alloc = Allocator()); // (20)

basic_json(byte_string_arg_t, const jsoncons::span<const uint8_t>& bytes, 
           semantic_tag tag = semantic_tag::none,
           const Allocator& alloc = Allocator()); // (21) (until v0.152)

template <class Source>
basic_json(byte_string_arg_t, const Source& source, 
           semantic_tag tag = semantic_tag::none,
           const Allocator& alloc = Allocator()); // (21) (since v0.152)

template <class Source>
basic_json(byte_string_arg_t, const Source& source, 
           uint64_t ext_tag,
           const Allocator& alloc = Allocator()); // (22) (since v0.152)
```

(1) Constructs an empty json object. 

(2) Constructs a copy of val

(3) Copy with allocator

(4) Acquires the contents of val, leaving val a `null` value

(5) Move with allocator

(6) Constructs a `basic_json` value for types supported in [json_type_traits](json_type_traits.md).

(7) Constructs a `basic_json` value from an unsigned integer and a [semantic_tag](../semantic_tag.md). This overload only participates in overload resolution if `Unsigned` is an unsigned integral type.

(8) Constructs a `basic_json` value from a signed integer and a [semantic_tag](../semantic_tag.md). This overload only participates in overload resolution if `Signed` is a signed integral type.

(9) Constructs a `basic_json` value for a half precision floating point number.
Uses [half_arg_t](../half_arg_t.md) as first argument to disambiguate overloads that construct half precision floating point numbers.

(10) Constructs a `basic_json` value from a double and a [semantic_tag](../semantic_tag.md).

(11)-(12) use [json_object_arg_t](../json_object_arg_t.md) as first argument to disambiguate overloads that construct json objects.

(11) Constructs a json object with the provided [semantic_tag](../semantic_tag.md) and allocator.

(12) Constructs a json object with the contents of the range `[first,last]`.

(13) Constructs a json object with the contents of the initializer list `init`.

(14)-(16) use [json_array_arg_t](../json_aray_arg_t.md) as first argument to disambiguate overloads that construct json objects.

(14) Constructs a json array with the provided [semantic_tag](../semantic_tag.md) and allocator.

(15) Constructs a json array with the contents of the range `[first,last]`.
`std::iterator_traits<InputIt>::value_type` must be convertible to `basic_json`. 

(16) Constructs a json array with the contents of the initializer list `init`.

(17) Constructs a `basic_json` value for types supported in [json_type_traits](json_type_traits.md) with allocator.

(18) Constructs a `basic_json` value from a text string.

(19) Constructs a `basic_json` value from a text string with supplied allocator.

(20) Constructs a `basic_json` value from a [byte_string_view](../byte_string_view.md).

(21) Constructs a `basic_json` value for a byte string from a contiguous byte sequence provided by `source`
with a generic tag.
Type `Source` must be a contiguous container that has member functions `data()` and `size()`, and member type `value_type` 
with width of exactly 8 bits (since v0.152.0.)
Any of the values types `int8_t`, `uint8_t`, `char`, `unsigned char` and `std::byte` (since C++17) are allowed.

Uses [byte_string_arg_t](../byte_string_arg_t.md) as first argument to disambiguate overloads that construct byte strings.

(22) Constructs a `basic_json` value for a byte string from a contiguous byte sequence provided by `source`
with a format specific tag.
Type `Source` must be a contiguous container that has member functions `data()` and `size()`, and member type `value_type` 
with width of exactly 8 bits (since v0.152.0.)
Any of the values types `int8_t`, `uint8_t`, `char`, `unsigned char` and `std::byte` (since C++17) are allowed.

Uses [byte_string_arg_t](../byte_string_arg_t.md) as first argument to disambiguate overloads that construct byte strings.

### Helpers

Helper                |Definition
--------------------|------------------------------
[json_object_arg][../json_object_arg.md] |    
[json_object_arg_t][../json_object_arg_t.md] | json object construction tag
[json_array_arg][../json_array_arg.md] |
[json_array_arg_t][../json_array_arg_t.md] | json array construction tag
[byte_string_arg][../byte_string_arg.md] |
[byte_string_arg_t][../byte_string_arg_t.md] | byte string construction tag
[half_arg][../half_arg.md] |
[half_arg_t][../half_arg_t.md] | half precision floating point number construction tag

### Examples

```c++
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <jsoncons/json.hpp>

using namespace jsoncons;
int main()
{
    json j1; // An empty object
    std::cout << "(1) " << j1 << std::endl;

    json j2(json_object_arg, {{"baz", "qux"}, {"foo", "bar"}}); // An object 
    std::cout << "(2) " << j2 << std::endl;

    json j3(json_array_arg, {"bar", "baz"}); // An array 
    std::cout << "(3) " << j3 << std::endl;
  
    json j4(json::null()); // A null value
    std::cout << "(4) " << j4 << std::endl;
    
    json j5(true); // A boolean value
    std::cout << "(5) " << j5 << std::endl;

    double x = 1.0/7.0;

    json j6(x); // A double value
    std::cout << "(6) " << j6 << std::endl;

    json j8("Hello"); // A text string
    std::cout << "(8) " << j8 << std::endl;

    std::vector<int> v = {10,20,30};
    json j9 = v; // From a sequence container
    std::cout << "(9) " << j9 << std::endl;

    std::map<std::string, int> m{ {"one", 1}, {"two", 2}, {"three", 3} };
    json j10 = m; // From an associative container
    std::cout << "(10) " << j10 << std::endl;

    std::vector<uint8_t> bytes = {'H','e','l','l','o'};
    json j11(byte_string_arg, bytes); // A byte string
    std::cout << "(11) " << j11 << std::endl;

    json j12(half_arg, 0x3bff);
    std::cout << "(12) " << j12.as_double() << std::endl;
}
```

```
(1) {}
(2) {"baz":"qux","foo":"bar"}
(3) ["bar","baz"]
(4) null
(5) true
(6) 0.14285714285714285
(8) "Hello"
(9) [10,20,30]
(10) {"one":1,"three":3,"two":2}
(11) "SGVsbG8"
(12) 0.999512
```

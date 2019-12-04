### `jsoncons::basic_json::basic_json`

```c++
basic_json(); // (1)

basic_json(const basic_json& other); // (2)

basic_json(const basic_json& other, const allocator_type& alloc); // (3)

basic_json(basic_json&& other) noexcept; // (4)

basic_json(basic_json&& other, const allocator_type& alloc) noexcept; // (5)

explicit basic_json(json_object_arg_t, 
                    semantic_tag tag = semantic_tag::none, 
                    const Allocator& alloc = Allocator()); // (6) 

template<class InputIt>
basic_json(json_object_arg_t, 
           InputIt first, InputIt last, 
           semantic_tag tag = semantic_tag::none,
           const Allocator& alloc = Allocator()); // (7) 

basic_json(json_object_arg_t, 
           std::initializer_list<std::pair<std::basic_string<char_type>,basic_json>> init, 
           semantic_tag tag = semantic_tag::none, 
           const Allocator& alloc = Allocator()); // (8)

explicit basic_json(json_array_arg_t, 
                    semantic_tag tag = semantic_tag::none, 
                    const Allocator& alloc = Allocator()); // (9)

template<class InputIt>
basic_json(json_array_arg_t, 
           InputIt first, InputIt last, 
           semantic_tag tag = semantic_tag::none, 
           const Allocator& alloc = Allocator()); // (10) 

basic_json(json_array_arg_t, 
           std::initializer_list<basic_json> init, 
           semantic_tag tag = semantic_tag::none, 
           const Allocator& alloc = Allocator()); // (11)

template <class T>
basic_json(const T& val); // (12)

template <class T>
basic_json(const T& val, const allocator_type& alloc); // (13)

basic_json(const char_type* val); // (14)

basic_json(const char_type* val, const allocator_type& alloc); // (15)

basic_json(const byte_string_view& bytes, 
           semantic_tag tag = semantic_tag::none, 
           const Allocator& alloc = Allocator()); // (16)

basic_json(bstr_arg_t, const jsoncons::span<const uint8_t>& bytes, 
           semantic_tag tag = semantic_tag::none,
           const Allocator& alloc = Allocator()); // (17)

basic_json(half_arg_t, uint16_t value, semantic_tag tag = semantic_tag::none); // (18)
```

(1) Constructs an empty json object. 

(2) Constructs a copy of val

(3) Copy with allocator

(4) Acquires the contents of val, leaving val a `null` value

(5) Move with allocator

(6) Constructs a json object with the provided [semantic_tag](../semantic_tag.md) and allocator.

(7) Constructs a json object with the contents of the range `[first,last]`.

(8) Constructs a json object with the contents of the initializer list `init`.

(6)-(8) use [json_object_arg_t](../json_object_arg_t.md) as first argument to disambiguate overloads that construct json objects.

(9) Constructs a json array with the provided [semantic_tag](../semantic_tag.md) and allocator.

(9)-(11) use [json_array_arg_t](../json_aray_arg_t.md) as first argument to disambiguate overloads that construct json objects.

(10) Constructs a json array with the contents of the range `[first,last]`.
`std::iterator_traits<InputIt>::value_type` must be convertible to `basic_json`. 

(11) Constructs a json array with the contents of the initializer list `init`.

(12) Constructs a `basic_json` value for types supported in [json_type_traits](json_type_traits.md).

(13) Constructs a `basic_json` value for types supported in [json_type_traits](json_type_traits.md) with allocator.

(14) Constructs a `basic_json` value for a text string.

(15) Constructs a `basic_json` value for a text string with supplied allocator.

(16) Constructs a `basic_json` value for a [byte_string](../byte_string.md).

(17) Constructs a `basic_json` value for a byte string from any container of `utf8_t` values.
Uses [bstr_arg_t](../bstr_arg_t.md) as first argument to disambiguate overloads that construct byte strings.

(18) Constructs a `basic_json` value for a half precision floating point number.
Uses [half_arg_t](../half_arg_t.md) as first argument to disambiguate overloads that construct half precision floating point numbers.

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
    json j11(bstr_arg, bytes); // A byte string
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

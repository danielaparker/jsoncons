### `jsoncons::basic_json::basic_json`

```c++
basic_json(); // (1)

basic_json(const basic_json& other); // (2)

basic_json(const basic_json& other, const allocator_type& allocator); // (3)

basic_json(basic_json&& other) noexcept; // (4)

basic_json(basic_json&& other, const allocator_type& allocator) noexcept; // (5)

explicit basic_json(json_object_arg_t, 
                    semantic_tag tag = semantic_tag::none, 
                    const Allocator& alloc = Allocator()); // (6) 

basic_json(json_object_arg_t, 
           std::initializer_list<array> init, 
           semantic_tag tag = semantic_tag::none, 
           const Allocator& alloc = Allocator()); // (7)

explicit basic_json(json_array_arg_t, 
                    semantic_tag tag = semantic_tag::none, 
                    const Allocator& alloc = Allocator()); // (8)

basic_json(json_array_arg_t, 
           std::initializer_list<basic_json> init, 
           semantic_tag tag = semantic_tag::none, 
           const Allocator& alloc = Allocator()); // (9)

template <class T>
basic_json(const T& val); // (10)

template <class T>
basic_json(const T& val, const allocator_type& allocator); // (11)

basic_json(const char_type* val); // (12)

basic_json(const char_type* val, const allocator_type& allocator); // (13)

basic_json(const byte_string_view& bs,
           semantic_tag tag = semantic_tag::none); // (14)

basic_json(const byte_string_view& bs, 
           semantic_tag tag = semantic_tag::none, 
           const allocator_type& allocator); // (15)

basic_json(half_arg_t, uint16_t value, semantic_tag tag = semantic_tag::none); // (16)
```

(1) Constructs a `basic_json` value that holds an empty basic_json object. 

(2) Constructs a copy of val

(3) Copy with allocator

(4) Acquires the contents of val, leaving val a `null` value

(5) Move with allocator

(10) Constructs a `basic_json` value for types supported in [json_type_traits](json_type_traits.md).

(11) Constructs a `basic_json` value for types supported in [json_type_traits](json_type_traits.md) with allocator.

(12) Constructs a `basic_json` value for a text string.

(13) Constructs a `basic_json` value for a text string with supplied allocator.

(14) Constructs a `basic_json` value for a [byte_string](../byte_string.md).

(15) Constructs a `basic_json` value for a [byte_string](../byte_string.md) with supplied allocator.

(16) Constructs a `basic_json` value for a half precision floating point number.

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

    json j2 = json(json_object_arg, {{"baz", "qux"}, {"foo", "bar"}}); // An object 
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

    json j7(x,4); // A double value with specified precision
    std::cout << "(7) " << j7 << std::endl;

    json j8("Hello"); // A text string
    std::cout << "(8) " << j8 << std::endl;

    std::vector<uint8_t> bs = {'H','e','l','l','o'};
    json j9(byte_string_view(bs.data(), bs.size())); // A byte string
    std::cout << "(9) " << j9 << std::endl;

    std::vector<int> v = {10,20,30};
    json j10 = v; // From a sequence container
    std::cout << "(10) " << j10 << std::endl;

    std::map<std::string, int> m{ {"one", 1}, {"two", 2}, {"three", 3} };
    json j11 = m; // From an associative container
    std::cout << "(11) " << j11 << std::endl;
}
```

```
(1) {}
(2) {"baz":"qux","foo":"bar"}
(3) ["bar","baz"]
(4) null
(5) true
(6) 0.142857142857143
(7) 0.1429
(8) "Hello"
(9) "SGVsbG8_"
(10) [10,20,30]
(11) {"one":1,"three":3,"two":2}
```

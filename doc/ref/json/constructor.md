### `jsoncons::basic_json::basic_json`

```c++
basic_json(); // (1)

basic_json(const allocator_type& allocator); // (2)

basic_json(const basic_json& val); // (3)

basic_json(const basic_json& val, const allocator_type& allocator); // (4)

basic_json(basic_json&& val) noexcept; // (5)

basic_json(basic_json&& val, const allocator_type& allocator) noexcept; // (6)

basic_json(const array& val); // (7)

basic_json(array&& val) noexcept; // (8)

basic_json(const object& val); // (9)

basic_json(object&& val) noexcept; // (10)

template <class T>
basic_json(const T& val); // (11)

template <class T>
basic_json(const T& val, const allocator_type& allocator); // (12)

basic_json(const char_type* val); // (13)

basic_json(const char_type* val, const allocator_type& allocator); // (14)

basic_json(const byte_string_view& bs,
           semantic_tag tag = semantic_tag::none); // (15)

basic_json(const byte_string_view& bs, 
           semantic_tag tag = semantic_tag::none, 
           const allocator_type& allocator); // (16)

basic_json(const bignum& n); // (17)

basic_json(const bignum& n, const allocator_type& allocator); // (18)
```

(1) Constructs a `basic_json` value that holds an empty basic_json object. 

(2) Constructs a `basic_json` value that holds a basic_json object. 

(3) Constructs a copy of val

(4) Copy with allocator

(5) Acquires the contents of val, leaving val a `null` value

(6) Move with allocator

(7) Constructs a `basic_json` value from a basic_json array

(8) Acquires the contents of a basic_json array

(9) Constructs a `basic_json` value from a basic_json object

(10) Acquires the contents of a basic_json object

(11) Constructs a `basic_json` value for types supported in [json_type_traits](json_type_traits.md).

(12) Constructs a `basic_json` value for types supported in [json_type_traits](json_type_traits.md) with allocator.

(13) Constructs a `basic_json` value for a text string.

(14) Constructs a `basic_json` value for a text string with supplied allocator.

(15) Constructs a `basic_json` value for a [byte_string](../byte_string.md).

(16) Constructs a `basic_json` value for a [byte_string](../byte_string.md) with supplied allocator.

(17) Constructs a `basic_json` value for a [bignum](../bignum.md).

(18) Constructs a `basic_json` value for a [bignum](../bignum.md) with supplied allocator.

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

    json j2 = json::object({{"baz", "qux"}, {"foo", "bar"}}); // An object 
    std::cout << "(2) " << j2 << std::endl;

    json j3 = json::array({"bar", "baz"}); // An array 
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

    const uint8_t bs[] = {'H','e','l','l','o'};
    json j9(byte_string("Hello")); // A byte string
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

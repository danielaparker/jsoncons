### jsoncons::jsonpointer::get

Selects a `json` value.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class J>
typename return_type<J,J&>::type get(J& root, const typename J::string_view_type& path); // (1)

template<class J>
typename return_type<J,const J&>::type get(const J& root, const typename J::string_view_type& path); // (2)

template<class J>
typename return_type<J,J&>::type get(J& root, const typename J::string_view_type& path, std::error_code& ec); // (3)

template<class J>
typename return_type<J,const J&>::type get(const J& root, const typename J::string_view_type& path, std::error_code& ec); // (4)

```

#### Return value

(1) On success, returns the selected item. `return_type<J,J&>::type` resolves to `J&` if `J`'s `at` functions return references,
    and `J` if `J`'s `at` functions return values.

(2) On success, returns the selected item, otherwise an undefined item. `return_type<J,const J&>::type` resolves to `const J&` if `J`'s `at` functions return references,
    and `J` if `J`'s `at` functions return values.

(3) On success, returns the selected item. `return_type<J,J&>::type` resolves to `J&` if `J`'s `at` functions return references,
    and `J` if `J`'s `at` functions return values.

(4) On success, returns the selected item, otherwise an undefined item. `return_type<J,const J&>::type` resolves to `const J&` if `J`'s `at` functions return references,
    and `J` if `J`'s `at` functions return values.

### Exceptions

(1) Throws a [jsonpointer_error](jsonpointer_error.md) if get fails.
 
(2) Sets the `std::error_code&` to the [jsonpointer_error_category](jsonpointer_errc.md) if get fails. 

#### Requirements

The type J satisfies the requirements for `jsonpointer::get` if it defines the following types

name              |type                  |notes
------------------|----------------------|---------------
`value_type`      |`J::value_type`       |
`reference`       |`J::reference`        |
`const_reference` |`J::const_reference`  |
`pointer`         |`J::pointer`          |
`const_pointer`   |`J::const_pointer`    |
`string_type`     |`J::string_type`      |
`string_view_type`|`J::string_view_type` |

and given 

- a value `index` of type `size_t`
- a value `key` of type `string_view_type` 
- an rvalue expression `j` of type `J`

the following expressions are valid

expression     |return type                |effects
---------------|---------------------------|---------------
is_array()     |`bool`                     |
is_object()    |`bool`                     |
size()         |`size_t`                   |
has_key(key)   |`bool`                     |
at(index)      |`reference` or `value_type`|
at(key)        |`reference` or `value_type`|

### Examples

#### Examples from [RFC6901](https://tools.ietf.org/html/rfc6901)

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    const json example = json::parse(R"(
        {
          "foo": ["bar", "baz"],
          "": 0,
          "a/b": 1,
          "c%d": 2,
          "e^f": 3,
          "g|h": 4,
          "i\\j": 5,
          "k\"l": 6,
          " ": 7,
          "m~n": 8
       }
    )");
   
    try
    {
        const json& result1 = jsonpointer::get(example, "");
        std::cout << "(1) " << result1 << std::endl;
        const json& result2 = jsonpointer::get(example, "/foo");
        std::cout << "(2) " << result2 << std::endl;
        const json& result3 = jsonpointer::get(example, "/foo/0");
        std::cout << "(3) " << result3 << std::endl;
        const json& result4 = jsonpointer::get(example, "/");
        std::cout << "(4) " << result4 << std::endl;
        const json& result5 = jsonpointer::get(example, "/a~1b");
        std::cout << "(5) " << result5 << std::endl;
        const json& result6 = jsonpointer::get(example, "/c%d");
        std::cout << "(6) " << result6 << std::endl;
        const json& result7 = jsonpointer::get(example, "/e^f");
        std::cout << "(7) " << result7 << std::endl;
        const json& result8 = jsonpointer::get(example, "/g|h");
        std::cout << "(8) " << result8 << std::endl;
        const json& result9 = jsonpointer::get(example, "/i\\j");
        std::cout << "(9) " << result9 << std::endl;
        const json& result10 = jsonpointer::get(example, "/k\"l");
        std::cout << "(10) " << result10 << std::endl;
        const json& result11 = jsonpointer::get(example, "/ ");
        std::cout << "(11) " << result11 << std::endl;
        const json& result12 = jsonpointer::get(example, "/m~0n");
        std::cout << "(12) " << result12 << std::endl;
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
```
Output:
```json
(1) {"":0," ":7,"a/b":1,"c%d":2,"e^f":3,"foo":["bar","baz"],"g|h":4,"i\\j":5,"k\"l":6,"m~n":8}
(2) ["bar","baz"]
(3) "bar"
(4) 0
(5) 1
(6) 2
(7) 3
(8) 4
(9) 5
(10) 6
(11) 7
(12) 8
```

#### Select author from second book

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json doc = json::parse(R"(
    [
      { "category": "reference",
        "author": "Nigel Rees",
        "title": "Sayings of the Century",
        "price": 8.95
      },
      { "category": "fiction",
        "author": "Evelyn Waugh",
        "title": "Sword of Honour",
        "price": 12.99
      }
    ]
    )");

    // Using exceptions to report errors
    try
    {
        json result = jsonpointer::get(doc, "/1/author");
        std::cout << "(1) " << result << std::endl;
    }
    catch (const jsonpointer::jsonpointer_error& e)
    {
        std::cout << e.what() << std::endl;
    }

    // Using error codes to report errors
    std::error_code ec;
    const json& result = jsonpointer::get(doc, "/0/title", ec);

    if (ec)
    {
        std::cout << ec.message() << std::endl;
    }
    else
    {
        std::cout << "(2) " << result << std::endl;
    }
}
```
Output:
```json
(1) "Evelyn Waugh"
(2) "Sayings of the Century"
```

#### Select values from `cbor_view` object

A [cbor_view](../cbor/cbor_view.md) satisfies the requirements for `jsonpointer::get`.

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;

int main()
{
    ojson j = ojson::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum.example.com",
           "assertion": "is-good",
           "rated": "sk",
           "rating": 0.90
         }
       ]
    }
    )");

    std::vector<uint8_t> data;
    cbor::encode_cbor(j, data);

    std::error_code ec;
    cbor::cbor_view rated = jsonpointer::get(cbor::cbor_view(data), "/reputons/0/rated", ec);
    cbor::cbor_view rating = jsonpointer::get(cbor::cbor_view(data), "/reputons/0/rating", ec);

    std::cout << rated.as_string() << ", " << rating.as_double() << std::endl;
}
```

Output:

```
sk, 0.9
```

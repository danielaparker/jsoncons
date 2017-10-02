### jsoncons::jsonpointer::get

Selects a `json` value.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class J>
std::tuple<J,jsonpointer_errc> get(const J& root, typename J::string_view_type path);

```

#### Return value

On success, returns the selected J value and a value-initialized [jsonpointer_errc](jsonpointer_errc.md). 

On error, returns a null J value and a [jsonpointer_errc](jsonpointer_errc.md) error code 

#### Requirements

The type J satisfies the requirements for `jsonpointer::get` if it defines the following types

name            |type               |notes
----------------|-------------------|---------------
char_type       |J::char_type       |
string_type     |J::string_type     |
string_view_type|J::string_view_type|

and given 

- a value `index` of type `size_t`
- a value `key` of type `string_type` 
- an rvalue expression `j` of type `J`

the following expressions are valid

expression     |return type|effects
---------------|-----------|---------------
is_array()     |bool       |
is_object()    |bool       |
size()         |size_t     |
has_key(key)   |bool       |
at(index)      |const J&   |
at(key)        |const J&   |

### Examples

#### Examples from [RFC6901](https://tools.ietf.org/html/rfc6901)

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    json example = json::parse(R"(
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
   
    json result;
    jsonpointer::jsonpointer_errc ec;

    std::tie(result,ec) = jsonpointer::get(example, "");
    std::cout << "(1) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/foo");
    std::cout << "(2) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/foo/0");
    std::cout << "(3) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/");
    std::cout << "(4) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/a~1b");
    std::cout << "(5) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/c%d");
    std::cout << "(6) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/e^f");
    std::cout << "(7) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/g|h");
    std::cout << "(8) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/i\\j");
    std::cout << "(9) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/k\"l");
    std::cout << "(10) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/ ");
    std::cout << "(11) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::get(example, "/m~0n");
    std::cout << "(12) " << result << std::endl;
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
    json root = json::parse(R"(
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

    json result;
    jsonpointer::jsonpointer_errc ec;
    std::tie(result,ec) = jsonpointer::get(root, "/1/author");

    if (ec == jsonpointer::jsonpointer_errc())
    {
        std::cout << result << std::endl;
    }
    else
    {
        std::cout << make_error_code(ec).message() << std::endl;
    }
}
```
Output:
```json
"Evelyn Waugh"
```



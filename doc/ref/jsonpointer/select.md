### jsoncons::jsonpointer::select

Selects a `json` value.

#### Header
```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
std::tuple<Json,jsonpointer_errc> select(const Json& root, typename Json::string_view_type path);

```

#### Return value

On success, returns the selected Json value and a value-initialized [jsonpointer_errc](jsonpointer_errc.md). 

On error, returns a null Json value and a [jsonpointer_errc](jsonpointer_errc.md) error code 

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

    std::tie(result,ec) = jsonpointer::select(example, "");
    std::cout << "(1) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/foo");
    std::cout << "(2) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/foo/0");
    std::cout << "(3) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/");
    std::cout << "(4) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/a~1b");
    std::cout << "(5) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/c%d");
    std::cout << "(6) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/e^f");
    std::cout << "(7) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/g|h");
    std::cout << "(8) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/i\\j");
    std::cout << "(9) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/k\"l");
    std::cout << "(10) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/ ");
    std::cout << "(11) " << result << std::endl;
    std::tie(result,ec) = jsonpointer::select(example, "/m~0n");
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
    std::tie(result,ec) = jsonpointer::select(root, "/1/author");

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



### jsoncons::jsonpointer::get

Selects a value.

```c++
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

template<class Json>
Json& get(Json& root, const Json::string_view_type& location,
          bool create_if_missing = false);                               (1)

template<class Json>
const Json& get(const Json& root, const Json::string_view_type& location);   (2)

template<class Json>
Json& get(Json& root, const Json::string_view_type& location, 
          std::error_code& ec);                                          (3)

template<class Json>
const Json& get(const Json& root, const Json::string_view_type& location, 
                std::error_code& ec);                                    (4)

template<class Json>
Json& get(Json& root, const Json::string_view_type& location, 
          bool create_if_missing, std::error_code& ec);                  (5)
```

#### Parameters
<table>
  <tr>
    <td>root</td>
    <td>JSON value</td> 
  </tr>
  <tr>
    <td>location</td>
    <td>JSON Pointer</td> 
  </tr>
  <tr>
    <td><code>create_if_missing</code> (since 0.162.0)</td>
    <td>Create key-object pairs when object key is missing</td> 
  </tr>
  <tr>
    <td><code>ec</code></td>
    <td>out-parameter for reporting errors in the non-throwing overload</td> 
  </tr>
</table>

#### Return value

(1), (3) and (5) return the selected item by reference. 

    json j(json_array_arg, {"baz","foo"});
    json& item = jsonpointer::get(j,"/0"); // "baz"

(2) and (4) return the selected item by const reference.  

    const json j(json_array_arg, {"baz","foo"});
    const json& item = jsonpointer::get(j,"/1"); // "foo"

### Exceptions

(1)-(2) throw a [jsonpointer_error](jsonpointer_error.md) if get fails.
 
(3)-(5) set the out-parameter `ec` to the [jsonpointer_error_category](jsonpointer_errc.md) if get fails. 

### Examples

#### Select author from second book

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    auto j = json::parse(R"(
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
        json result = jsonpointer::get(j, "/1/author");
        std::cout << "(1) " << result << std::endl;
    }
    catch (const jsonpointer::jsonpointer_error& e)
    {
        std::cout << e.what() << std::endl;
    }

    // Using error codes to report errors
    std::error_code ec;
    const json& result = jsonpointer::get(j, "/0/title", ec);

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

#### Using jsonpointer::json_pointer with jsonpointer::get 

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    auto j = json::parse(R"(
       {
          "a/b": ["bar", "baz"],
          "m~n": ["foo", "qux"]
       }
    )");

    jsonpointer::json_pointer ptr;
    ptr /= "m~n";
    ptr /= "1";

    std::cout << "(1) " << ptr << "\n\n";

    std::cout << "(2)\n";
    for (const auto& item : ptr)
    {
        std::cout << item << "\n";
    }
    std::cout << "\n";

    json item = jsonpointer::get(j, ptr);
    std::cout << "(3) " << item << "\n";
}
```
Output:
```
(1) /m~0n/1

(2)
m~n
1

(3) "qux"
```

#### Examples from [RFC6901](https://tools.ietf.org/html/rfc6901)

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    auto j = json::parse(R"(
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
        const json& result1 = jsonpointer::get(j, "");
        std::cout << "(1) " << result1 << std::endl;
        const json& result2 = jsonpointer::get(j, "/foo");
        std::cout << "(2) " << result2 << std::endl;
        const json& result3 = jsonpointer::get(j, "/foo/0");
        std::cout << "(3) " << result3 << std::endl;
        const json& result4 = jsonpointer::get(j, "/");
        std::cout << "(4) " << result4 << std::endl;
        const json& result5 = jsonpointer::get(j, "/a~1b");
        std::cout << "(5) " << result5 << std::endl;
        const json& result6 = jsonpointer::get(j, "/c%d");
        std::cout << "(6) " << result6 << std::endl;
        const json& result7 = jsonpointer::get(j, "/e^f");
        std::cout << "(7) " << result7 << std::endl;
        const json& result8 = jsonpointer::get(j, "/g|h");
        std::cout << "(8) " << result8 << std::endl;
        const json& result9 = jsonpointer::get(j, "/i\\j");
        std::cout << "(9) " << result9 << std::endl;
        const json& result10 = jsonpointer::get(j, "/k\"l");
        std::cout << "(10) " << result10 << std::endl;
        const json& result11 = jsonpointer::get(j, "/ ");
        std::cout << "(11) " << result11 << std::endl;
        const json& result12 = jsonpointer::get(j, "/m~0n");
        std::cout << "(12) " << result12 << std::endl;
    }
    catch (const jsonpointer::jsonpointer_error& e)
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

#### Get a value at a location after creating objects when missing object keys

```c++
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using jsoncons::json;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    std::vector<std::string> keys = {"foo","bar","baz"};

    jsonpointer::json_pointer location;
    for (const auto& key : keys)
    {
        location /= key;
    }

    json doc;
    json result = jsonpointer::get(doc, location, true);

    std::cout << pretty_print(doc) << "\n\n";
}
```
Output:
```json
{
    "foo": {
        "bar": {
            "baz": {}
        }
    }
}
```

### See also

[basic_json_pointer](basic_json_pointer.md)


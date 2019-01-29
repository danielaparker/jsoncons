### jsonpointer extension

The jsonpointer extension implements the IETF standard [JavaScript Object Notation (JSON) Pointer](https://tools.ietf.org/html/rfc6901)

### Classes
<table border="0">
  <tr>
    <td><a href="path.md">basic_path</a></td>
    <td>Objects of type `basic_path` represent JSON Pointer paths.</td> 
  </tr>
</table>

### Non-member functions

<table border="0">
  <tr>
    <td><a href="contains.md">contains</a></td>
    <td>Returns `true` if the json document contains the given json pointer</td> 
  </tr>
  <tr>
    <td><a href="get.md">get</a></td>
    <td>Get a value from a JSON document using JSON Pointer path notation.</td> 
  </tr>
  <tr>
    <td><a href="insert.md">insert</a></td>
    <td>Inserts a value in a JSON document using JSON Pointer path notation, if the path doesn't specify an object member that already has the same key.</td> 
  </tr>
  <tr>
    <td><a href="insert_or_assign.md">insert_or_assign</a></td>
    <td>Inserts a value in a JSON document using JSON Pointer path notation, or if the path specifies an object member that already has the same key, assigns the new value to that member.</td> 
  </tr>
  <tr>
    <td><a href="remove.md">remove</a></td>
    <td>Removes a value from a JSON document using JSON Pointer path notation.</td> 
  </tr>
  <tr>
    <td><a href="replace.md">replace</a></td>
    <td>Replaces a value in a JSON document using JSON Pointer path notation.</td> 
  </tr>
</table>

### Examples

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
    json result = jsonpointer::get(doc, "/0/title", ec);

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

#### Using `jsonpointer::path` to build JSON Pointer 

```c++
int main()
{
    json example = json::parse(R"(
       {
          "a/b": ["bar", "baz"],
          "m~n": ["foo", "qux"]
       }
    )");

    jsonpointer::path p;
    p.append("m~n");
    p.append("1");

    std::cout << "(1) " << p << "\n\n";

    std::cout << "(2)\n";
    for (const auto& item : p)
    {
        std::cout << item << "\n";
    }
    std::cout << "\n";

    json j = jsonpointer::get(example, p);
    std::cout << "(3) " << j << "\n";
}
```
Output
```
(1) /m~0n/1

(2)
m~n
1

(3) "qux"
```

### jsonpointer extension

The jsonpointer extension implements the IETF standard [JavaScript Object Notation (JSON) Pointer](https://tools.ietf.org/html/rfc6901)

<table border="0">
  <tr>
    <td><a href="contains.md">contains</a></td>
    <td>Returns `true` if the json document contains the given json pointer</td> 
  </tr>
  <tr>
    <td><a href="get.md">get</a></td>
    <td>Get a value from a JSON document using Json Pointer path notation.</td> 
  </tr>
  <tr>
    <td><a href="insert.md">insert</a></td>
    <td>Inserts a value in a JSON document using Json Pointer path notation, if the path doesn't specify an object member that already has the same key.</td> 
  </tr>
  <tr>
    <td><a href="insert_or_assign.md">insert_or_assign</a></td>
    <td>Inserts a value in a JSON document using Json Pointer path notation, or if the path specifies an object member that already has the same key, assigns the new value to that member.</td> 
  </tr>
  <tr>
    <td><a href="remove.md">remove</a></td>
    <td>Removes a value from a JSON document using Json Pointer path notation.</td> 
  </tr>
  <tr>
    <td><a href="replace.md">replace</a></td>
    <td>Replaces a value in a JSON document using Json Pointer path notation.</td> 
  </tr>
</table>

### Examples

Example. Select author from second book

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

[jsonpointer::get](get.md) may also be used to query the nested data items of a packed CBOR value.



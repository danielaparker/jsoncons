### jsoncons::jsonpath::json_query

Returns a `json` array of values or normalized path expressions selected from a root `json` structure.

#### Header
```c++
#include <jsoncons/jsonpath/json_query.hpp>

enum class result_type {value,path};

template<Json>
Json json_query(const Json& root, 
                const typename Json::string_view_type& path,
                result_type result_t = result_type::value);
```
#### Parameters

<table>
  <tr>
    <td>root</td>
    <td>JSON value</td> 
  </tr>
  <tr>
    <td>path</td>
    <td>JSONPath expression string</td> 
  </tr>
  <tr>
    <td>result_t</td>
    <td>Indicates whether results are matching values (the default) or normalized path expressions</td> 
  </tr>
</table>

#### Return value

Returns a `json` array containing either values or normalized path expressions matching the input path expression. 
Returns an empty array if there is no match.

#### Exceptions

Throws [jsonpath_error](jsonpath_error.md) if JSONPath evaluation fails.

#### Note
Stefan Goessner's javascript implemention returns `false` in case of no match, but in a note he suggests an alternative is to return an empty array. The `jsoncons` implementation takes that alternative and returns an empty array in case of no match.
####

### Store examples

The examples below use the JSON text from [Stefan Goessner's JSONPath](http://goessner.net/articles/JsonPath/) (booklist.json).

```json
{ "store": {
    "book": [ 
      { "category": "reference",
        "author": "Nigel Rees",
        "title": "Sayings of the Century",
        "price": 8.95
      },
      { "category": "fiction",
        "author": "Evelyn Waugh",
        "title": "Sword of Honour",
        "price": 12.99
      },
      { "category": "fiction",
        "author": "Herman Melville",
        "title": "Moby Dick",
        "isbn": "0-553-21311-3",
        "price": 8.99
      },
      { "category": "fiction",
        "author": "J. R. R. Tolkien",
        "title": "The Lord of the Rings",
        "isbn": "0-395-19395-8",
        "price": 22.99
      }
    ],
    "bicycle": {
      "color": "red",
      "price": 19.95
    }
  }
}
```

#### Return values

```c++    
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <fstream>

namespace jc = jsoncons;
namespace jp = jsoncons::jsonpath;

int main()
{
    std::ifstream is("./input/booklist.json");
    jc::json booklist;
    is >> booklist;

    // The authors of books that are cheaper than $10
    jc::json result1 = jp::json_query(booklist, "$.store.book[?(@.price < 10)].author");
    std::cout << "(1) " << result1 << "\n";

    // The number of books
    jc::json result2 = jp::json_query(booklist, "$..book.length");
    std::cout << "(2) " << result2 << "\n";

    // The third book
    jc::json result3 = jp::json_query(booklist, "$..book[2]");
    std::cout << "(3)\n" << pretty_print(result3) << "\n";

    // All books whose author's name starts with Evelyn
    jc::json result4 = jp::json_query(booklist, "$.store.book[?(@.author =~ /Evelyn.*?/)]");
    std::cout << "(4)\n" << pretty_print(result4) << "\n";

    // The titles of all books that have isbn number
    jc::json result5 = jp::json_query(booklist, "$..book[?(@.isbn)].title");
    std::cout << "(5) " << result5 << "\n";

    // All authors and titles of books
    jc::json result6 = jp::json_query(booklist, "$['store']['book']..['author','title']");
    std::cout << "(6)\n" << pretty_print(result6) << "\n";

    // Union of a subset of book titles identified by index
    jc::json result7 = jp::json_query(booklist, "$.store[book[0].title,book[1].title,book[3].title]");
    std::cout << "(7)\n" << pretty_print(result7) << "\n";

    // Union of third book title and all book titles with price > 10
    jc::json result8 = jp::json_query(booklist, "$.store[book[3].title,book[?(@.price > 10)].title]");
    std::cout << "(8)\n" << pretty_print(result8) << "\n";

    // Normalized path expressions
    jc::json result9 = jp::json_query(booklist, "$.store.book[?(@.author =~ /Evelyn.*?/)]", jp::result_type::path);
    std::cout << "(9)\n" << pretty_print(result9) << "\n";

    // All titles whose author's second name is 'Waugh'
    jc::json result10 = jp::json_query(booklist,"$.store.book[?(tokenize(@.author,'\\\\s+')[1] == 'Waugh')].title");
    std::cout << "(10)\n" << result10 << "\n";

    // All keys in the second book
    jc::json result11 = jp::json_query(booklist,"keys($.store.book[1])[*]");
    std::cout << "(11)\n" << result11 << "\n";
}
```
Output:
```
(1) ["Nigel Rees","Herman Melville"]
(2) [4]
(3)
[
    {
        "author": "Herman Melville",
        "category": "fiction",
        "isbn": "0-553-21311-3",
        "price": 8.99,
        "title": "Moby Dick"
    }
]
(4)
[
    {
        "author": "Evelyn Waugh",
        "category": "fiction",
        "price": 12.99,
        "title": "Sword of Honour"
    }
]
(5) ["Moby Dick","The Lord of the Rings"]
(6)
[
    "Evelyn Waugh",
    "Herman Melville",
    "J. R. R. Tolkien",
    "Moby Dick",
    "Nigel Rees",
    "Sayings of the Century",
    "Sword of Honour",
    "The Lord of the Rings"
]
(7)
[
    "Sayings of the Century",
    "Sword of Honour",
    "The Lord of the Rings"
]
(8)
[
    "Sword of Honour",
    "The Lord of the Rings"
]
(9)
[
    "$['store']['book'][1]"
]
(10)
["Sword of Honour"]
(11)
["author","category","price","title"]

#### Return normalized path expressions

```c++
using namespace jsoncons;
using namespace jsoncons::jsonpath;

int main()
{
    std::string path = "$.store.book[?(@.price < 10)].title";
    json result = json_query(store,path,result_type::path);

    std::cout << pretty_print(result) << std::endl;
}
```
Output:
```json
[
    "$['store']['book'][0]['title']",
    "$['store']['book'][2]['title']"
]
```

### More examples

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>

using namespace jsoncons;
using namespace jsoncons::jsonpath;

int main()
{
    const json j = json::parse(R"(
    [
      {
        "root": {
          "id" : 10,
          "second": [
            {
                 "names": [
                   2
              ],
              "complex": [
                {
                  "names": [
                    1
                  ],
                  "panels": [
                    {
                      "result": [
                        1
                      ]
                    },
                    {
                      "result": [
                        1,
                        2,
                        3,
                        4
                      ]
                    },
                    {
                      "result": [
                        1
                      ]
                    }
                  ]
                }
              ]
            }
          ]
        }
      },
      {
        "root": {
          "id" : 20,
          "second": [
            {
              "names": [
                2
              ],
              "complex": [
                {
                  "names": [
                    1
                  ],
                  "panels": [
                    {
                      "result": [
                        1
                      ]
                    },
                    {
                      "result": [
                        3,
                        4,
                        5,
                        6
                      ]
                    },
                    {
                      "result": [
                        1
                      ]
                    }
                  ]
                }
              ]
            }
          ]
        }
      }
    ]
    )");

    // Find all arrays of elements where result.length is 4
    json result1 = json_query(j,"$..[?(@.result.length == 4)].result");
    std::cout << "(1) " << result1 << std::endl;

    // Find array of elements that has id 10 and result.length is 4
    json result2 = json_query(j,"$..[?(@.id == 10)]..[?(@.result.length == 4)].result");
    std::cout << "(2) " << result2 << std::endl;

    // Find all arrays of elements where result.length is 4 and that have value 3 
    json result3 = json_query(j,"$..[?(@.result.length == 4 && (@.result[0] == 3 || @.result[1] == 3 || @.result[2] == 3 || @.result[3] == 3))].result");
    std::cout << "(3) " << result3 << std::endl;
}
```
Output:

```
(1) [[1,2,3,4],[3,4,5,6]]
(2) [[1,2,3,4]]
(3) [[1,2,3,4],[3,4,5,6]]
```


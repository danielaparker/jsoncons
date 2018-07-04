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

using namespace jsoncons;
using namespace jsoncons::jsonpath;

int main()
{
    std::ifstream is("./input/booklist.json");
    json booklist = json::parse(is);

    // all authors whose books are cheaper than $10
    json result = json_query(booklist,"$.store.book[?(@.price < 10)].author");
    std::cout  << "(1)\n" << pretty_print(result) << std::endl;

    // All books whose author's name starts with Evelyn
    json result1 = json_query(booklist, "$.store.book[?(@.author =~ /Evelyn.*?/)]");
    std::cout << "(2)\n" << pretty_print(result1) << std::endl;
}
```
Output:
```
(1)
["Nigel Rees","Herman Melville"]
(2)
[
    {
        "author": "Evelyn Waugh",
        "category": "fiction",
        "price": 12.99,
        "title": "Sword of Honour"
    }
]
```

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


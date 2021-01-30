### jsoncons::jsonpath::json_replace

```c++
#include <jsoncons_ext/jsonpath/json_query.hpp>

template<class Json, class Source, class T>
void json_replace(Json& root, const Source& path, T&& new_value); (1)

template<class Json, class T>
void json_replace(Json& root, const typename Json::char_type* path, T&& new_value); (2)

template<class Json, class Source, class Op>
void json_replace(Json& root, const Source& path, T Op); (3)

template<class Json, class Op>
void json_replace(Json& root, const typename Json::char_type* path, T Op); (4)
```

(1) Searches for all values that match the JSONPath expression `expr` and replaces them with the specified value
The JSONPath expression `expr` is provided as a sequential container 
or view of characters, such as a `std::basic_string` or `std::basic_string_view`.

(2) Searches for all values that match the JSONPath expression `expr` and replaces them with the specified value
The JSONPath expression `expr` is provided as a null terminated string.

(3) Searches for all values that match a JSONPath expression `expr` and replaces them with the result of the given function
The JSONPath expression `expr` is provided as a sequential container 
or view of characters, such as a `std::basic_string` or `std::basic_string_view`.

(4) Searches for all values that match a JSONPath expression `expr` and replaces them with the result of the given function
The JSONPath expression `expr` is provided as a null terminated string.

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
    <td>(1) new_value</td>
    <td>The value to use as replacement</td> 
  </tr>
  <tr>
    <td>(2) op</td>
    <td>Callback function to rewrite values that matched the expression</td> 
  </tr>
</table>

#### Exceptions

Throws a [jsonpath_error](jsonpath_error.md) if JSONPath evaluation fails.

### Examples

#### Change the price of a book

Input JSON file `booklist.json`:

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
      }
    ]
  }
}
```
```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

using namespace jsoncons;
using namespace jsoncons::jsonpath;

int main()
{
    std::ifstream is("input/booklist.json");
    json booklist;
    is >> booklist;

    // Change the price of "Moby Dick"
    json_replace(booklist,"$.store.book[?(@.isbn == '0-553-21311-3')].price",10.0);
    std::cout << pretty_print(booklist) << std::endl;

}
```
Output:
```json
{
    "store": {
        "book": [
            {
                "author": "Nigel Rees",
                "category": "reference",
                "price": 8.95,
                "title": "Sayings of the Century"
            },
            {
                "author": "Evelyn Waugh",
                "category": "fiction",
                "price": 12.99,
                "title": "Sword of Honour"
            },
            {
                "author": "Herman Melville",
                "category": "fiction",
                "isbn": "0-553-21311-3",
                "price": 10.0,
                "title": "Moby Dick"
            }
        ]
    }
}
```

#### Change the prices of all books

Input JSON file `booklist.json`:

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
      }
    ]
  }
}
```
```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>
#include <cmath>

using namespace jsoncons;
using namespace jsoncons::jsonpath;

int main()
{
    std::ifstream is("input/booklist.json");
    json booklist;
    is >> booklist;

    // make a discount on all books
	jsonpath::json_replace(booklist, "$.store.book[*].price",
			[](const json& price) { return std::round(price.as<double>() - 1.0); });
    std::cout << pretty_print(booklist) << std::endl;

}
```
Output:
```json
{
    "store": {
        "book": [
            {
                "author": "Nigel Rees", 
                "category": "reference", 
                "price": 8.0, 
                "title": "Sayings of the Century"
            }, 
            {
                "author": "Evelyn Waugh", 
                "category": "fiction", 
                "price": 12.0, 
                "title": "Sword of Honour"
            }, 
            {
                "author": "Herman Melville", 
                "category": "fiction", 
                "isbn": "0-553-21311-3", 
                "price": 8.0, 
                "title": "Moby Dick"
            } 
        ]
    }
}
```


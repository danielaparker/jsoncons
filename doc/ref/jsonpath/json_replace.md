### jsoncons::jsonpath::json_replace

```c++
#include <jsoncons_ext/jsonpath/json_query.hpp>

```c++
template<class Json, class Source, class T>
void json_replace(Json& root, const Source& path, T&& new_value); 

template<class Json, class T>
void json_replace(Json& root, const typename Json::char_type* path, T&& new_value);           (1)
```
```c++
template<class Json, class Source, class UnaryCallback>
void json_replace(Json& root, const Source& path, UnaryCallback callback1); 

template<class Json, class UnaryCallback>
void json_replace(Json& root, const typename Json::char_type* path, UnaryCallback callback1);        (2) (until 0.161.0)
```
```c++
template<class Json, class Source, class BinaryCallback>
void json_replace(Json& root, const Source& path, BinaryCallback callback2); 

template<class Json, class BinaryCallback>
void json_replace(Json& root, const typename Json::char_type* path, BinaryCallback callback2); (3) (since 0.161.0)
```

(1) Searches for all values that match the JSONPath expression `path` and replaces them with the specified value
The JSONPath expression `path` is provided as a sequential container 
or view of characters, or as a null terminated string.

(2) Searches for all values that match a JSONPath expression `path` and replaces them with the result of the given function
The JSONPath expression `path` is provided as a sequential container 
or view of characters, or as a null terminated string.

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
    <td>new_value</td>
    <td>The value to use as replacement</td> 
  </tr>
  <tr>
    <td><code>callback1</code></td>
    <td>A function object that accepts a const reference to a Json value
    and returns a Json value. 
It must have function call signature equivalent to
<br/><br/><code>
Json fun(const Json& val);
</code><br/><br/>
  </tr>
  <tr>
    <td><code>callback2</code></td>
    <td>A function object that accepts a path and a reference to a Json value. 
It must have function call signature equivalent to
<br/><br/><code>
void fun(const Json::string_type& path, Json& val);
</code><br/><br/>
  </tr>
</table>

#### Exceptions

Throws a [jsonpath_error](jsonpath_error.md) if JSONPath evaluation fails.

### Examples

#### Change the price of a book

Input JSON file `store.json`:

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
    std::ifstream is("input/store.json");
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

Input JSON file `store.json`:

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
    std::ifstream is("input/store.json");
    json booklist;
    is >> booklist;

    // make a discount on all books
	jsonpath::json_replace(booklist, "$.store.book[*].price",
			[](const json& price) {return std::round(price.as<double>() - 1.0);});
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


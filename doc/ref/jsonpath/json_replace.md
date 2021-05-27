### jsoncons::jsonpath::json_replace

```c++
#include <jsoncons_ext/jsonpath/json_query.hpp>
```
```c++
template<class Json, class T>
void json_replace(Json& root, const Json::string_view_type& expr, T&& new_value, 
                  result_options options = result_options::nodups);                              (until 0.164.0)

template<class Json, class T>
void json_replace(Json& root, const Json::string_view_type& expr, T&& new_value,             (1)
                  result_options options = result_options::nodups,
                  const custom_functions<Json>& funcs = custom_functions<Json>());               (since 0.164.0)
```

```c++
template<class Json, class BinaryCallback>
void json_replace(Json& root, const Json::string_view_type& expr, BinaryCallback callback, 
                  result_options options = result_options::nodups);                              (until 0.164.0)

template<class Json, class BinaryCallback>                                                   (2)
void json_replace(Json& root, const Json::string_view_type& expr, BinaryCallback callback, 
                  result_options options = result_options::nodups,
                  const custom_functions<Json>& funcs = custom_functions<Json>());               (since 0.164.0)
```

(1) Searches for all values that match the JSONPath expression `expr` and replaces them with the specified value

(2) Searches for all values that match a JSONPath expression `expr` and, for each result, 
calls a callback provided by the user with a path and mutable reference to the value.

#### Parameters

<table>
  <tr>
    <td><code>root</code></td>
    <td>JSON value</td> 
  </tr>
  <tr>
    <td><code>expr</code></td>
    <td>JSONPath expression string</td> 
  </tr>
  <tr>
    <td><code>new_value</code></td>
    <td>The value to use as replacement</td> 
  </tr>
  <tr>
    <td><code>callback</code> (until 0.161.0)</td>
    <td>A function object that accepts a const reference to a Json value
    and returns a Json value. 
It must have function call signature equivalent to
<br/><br/><code>
Json fun(const Json& val);
</code><br/><br/>
  </tr>
  <tr>
    <td><code>callback</code> (since 0.161.0)</td>
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

The examples use the sample data file `books.json`, 

```json
{
    "books":
    [
        {
            "category": "fiction",
            "title" : "A Wild Sheep Chase",
            "author" : "Haruki Murakami",
            "price" : 22.72
        },
        {
            "category": "fiction",
            "title" : "The Night Watch",
            "author" : "Sergei Lukyanenko",
            "price" : 23.58
        },
        {
            "category": "fiction",
            "title" : "The Comedians",
            "author" : "Graham Greene",
            "price" : 21.99
        },
        {
            "category": "memoir",
            "title" : "The Night Watch",
            "author" : "Phillips, David Atlee"
        }
    ]
}
```

#### Change the price of A Wild Sheep Chase

```c++
#include <fstream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

using namespace jsoncons;
using namespace jsoncons::jsonpath;

int main()
{
    std::ifstream is("./input/books.json");
    json data = json::parse(is);

    jsonpath::json_replace(data,"$.books[?(@.title == 'A Wild Sheep Chase')].price",20.0);
    std::cout << pretty_print(data) << "\n\n";
}
```
Output:
```json
{
    "books": [
        {
            "author": "Haruki Murakami",
            "category": "fiction",
            "price": 20.0,
            "title": "A Wild Sheep Chase"
        },
        {
            "author": "Sergei Lukyanenko",
            "category": "fiction",
            "price": 23.58,
            "title": "The Night Watch"
        },
        {
            "author": "Graham Greene",
            "category": "fiction",
            "price": 21.99,
            "title": "The Comedians"
        },
        {
            "author": "Phillips, David Atlee",
            "category": "memoir",
            "title": "The Night Watch"
        }
    ]
}
```

#### Make a discount on all books

```c++
#include <cmath>
#include <fstream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

using namespace jsoncons;
using namespace jsoncons::jsonpath;

int main()
{
    std::ifstream is("./input/books.json");
    json data = json::parse(is);

    auto f = [](const std::string& /*path*/, json& price) 
    {
        price = std::round(price.as<double>() - 1.0);
    };

    // make a discount on all books
    jsonpath::json_replace(data, "$.books[*].price", f);
    std::cout << pretty_print(data);
}
```
Output:
```json
{
    "books": [
        {
            "author": "Haruki Murakami",
            "category": "fiction",
            "price": 22.0,
            "title": "A Wild Sheep Chase"
        },
        {
            "author": "Sergei Lukyanenko",
            "category": "fiction",
            "price": 23.0,
            "title": "The Night Watch"
        },
        {
            "author": "Graham Greene",
            "category": "fiction",
            "price": 21.0,
            "title": "The Comedians"
        },
        {
            "author": "Phillips, David Atlee",
            "category": "memoir",
            "title": "The Night Watch"
        }
    ]
}
```

#### Add a missing price

```c++
#include <cmath>
#include <fstream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

using namespace jsoncons;
using namespace jsoncons::jsonpath;

int main()
{
    std::ifstream is("./input/books.json");
    json data = json::parse(is);

    auto f = [](const std::string& /*path*/, json& book) 
    {
        if (book.at("category") == "memoir" && !book.contains("price"))
        {
            book.try_emplace("price",140.0);
        }
    };

    jsonpath::json_replace(data, "$.books[*]", f);
    std::cout << pretty_print(data);
}
```
Output:
```json
{
    "books": [
        {
            "author": "Haruki Murakami",
            "category": "fiction",
            "price": 22.72,
            "title": "A Wild Sheep Chase"
        },
        {
            "author": "Sergei Lukyanenko",
            "category": "fiction",
            "price": 23.58,
            "title": "The Night Watch"
        },
        {
            "author": "Graham Greene",
            "category": "fiction",
            "price": 21.99,
            "title": "The Comedians"
        },
        {
            "author": "Phillips, David Atlee",
            "category": "memoir",
            "price": 140.0,
            "title": "The Night Watch"
        }
    ]
}
```


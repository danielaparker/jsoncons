### jsoncons::jsonpath::make_expression

```c++
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

template <class Json,class Source>
jsonpath_expression<Json> make_expression(const Source& expr); 

template <class Json>
jsonpath_expression<Json> make_expression(const Json::string_view_type& expr); (1)
```
```c++
template <class Json,class Source>
jsonpath_expression<Json> make_expression(const Source& expr,
                                        std::error_code& ec); 

template <class Json>
jsonpath_expression<Json> make_expression(const Json::string_view_type& expr,
                                        std::error_code& ec); (2)
```

(1) Makes a [jsonpath_expression](jsonpath_expression.md) from the JSONPath expression `expr`.

(2) Makes a [jsonpath_expression](jsonpath_expression.md) from the JSONPath expression `expr`.

#### Parameters

<table>
  <tr>
    <td><code>expr</code></td>
    <td>JSONPath expression string</td> 
  </tr>
  <tr>
    <td><code>ec</code></td>
    <td>out-parameter for reporting errors in the non-throwing overload</td> 
  </tr>
</table>

#### Return value

Returns a `jsonpath_expression` object that represents the JSONPath expression.

#### Exceptions

(1) throws a [jsonpath_error](jsonpath_error.md) if JSONPath compilation fails.

(2) sets the out-parameter `ec` to the [jsonpath_error_category](jsonpath_errc.md) if JSONPath compilation fails. 

### Examples

The examples below uses the sample data file `books.json`, 

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

#### Return copies

```c++
int main()
{
    auto expr = jsonpath::make_expression<json>("$.books[?(@.price > avg($.books[*].price))].title");

    std::ifstream is("./input/books.json");
    json data = json::parse(is);

    json result = expr.evaluate(data);
    std::cout << pretty_print(result) << "\n\n";
}
```
Output:
```
[
    "The Night Watch"
]
```

#### Access path and reference to original value

```c++
int main()
{
    auto expr = jsonpath::make_expression<json>("$.books[?(@.price >= 22.0)]");

    std::ifstream is("./input/books.json");
    json data = json::parse(is);

    auto callback = [](const std::string& path, const json& val)
    {
        std::cout << path << ": " << val << "\n";
    };
    expr.evaluate(data, callback, jsonpath::result_options::path);
}
```
Output:
```
$['books'][0]: {"author":"Haruki Murakami","category":"fiction","price":22.72,"title":"A Wild Sheep Chase"}
$['books'][1]: {"author":"Sergei Lukyanenko","category":"fiction","price":23.58,"title":"The Night Watch"}
```

### jsoncons::jsonpath::jsonpath_expression::select

```cpp
template <class BinaryOp>
void select(const_reference root_value, BinaryOp op, 
    result_options options = result_options());                                   (1) (since 0.172.0)
```

(1) Evaluates the root value against the compiled JSONPath expression and calls a provided
callback repeatedly with the results.

#### Parameters

<table>
  <tr>
    <td>root_value</td>
    <td>JSON value</td> 
  </tr>
  <tr>
    <td><code>op</code></td>
    <td>A function object that accepts a path and a reference to a Json value. 
It must have function call signature equivalent to
<br/><br/><code>void fun(const basic_path_node<Json::char_type>& path, const Json& val);</code><br/><br/>
  </tr>
  <tr>
    <td>result_options</td>
    <td>Result options, a bitmask of type <a href="result_options.md">result_options</></td> 
  </tr>
</table>

### Examples

#### Return locations of selected values (since 0.172.0)

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

using json = jsoncons::json;
namespace jsonpath = jsoncons::jsonpath;

int main()
{
    auto expr = jsoncons::jsonpath::make_expression<json>("$.books[*]");

    std::ifstream is("./input/books.json");
    json doc = json::parse(is);

    auto op = [&](const jsonpath::path_node& path, const json& value)
    {
        if (value.at("category") == "memoir" && !value.contains("price"))
        {
            std::cout << jsonpath::to_string(path) << ": " << value << "\n";
        }
    };

    expr.select(root_value, op);
}
```
Output:
```
$['books'][3]: {"author":"Phillips, David Atlee","category":"memoir","title":"The Night Watch"}
```


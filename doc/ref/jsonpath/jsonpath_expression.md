### jsoncons::jsonpath::jsonpath_expression

```c++
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

template <class Json,class JsonReference>
class jsonpath_expression
```

#### Member functions
```c++
Json evaluate(reference root_value, result_options options = result_options()); (1)
```
```c++
template <class BinaryCallback>
void evaluate(reference root_value, BinaryCallback callback, 
              result_options options = result_options());  (2)
```

(1) Evaluates the root value against the compiled JSONPath expression and returns an array of values or 
normalized path expressions. 

(2) Evaluates the root value against the compiled JSONPath expression and calls a provided
callback repeatedly with the results.

#### Parameters

<table>
  <tr>
    <td>root_value</td>
    <td>JSON value</td> 
  </tr>
  <tr>
    <td><code>callback</code></td>
    <td>A function object that accepts a path and a reference to a Json value. 
It must have function call signature equivalent to
<br/><br/><code>
void fun(const Json::string_type& path, const Json& val);
</code><br/><br/>
  </tr>
  <tr>
    <td>result_options</td>
    <td>Result options, a bitmask of type <a href="result_options.md">result_options</></td> 
  </tr>
</table>

#### Static functions

    static jsonpath_expression compile(const string_view_type& expr);    (1)

    static jsonpath_expression compile(const string_view_type& expr,
                                       std::error_code& ec);             (2)

Compiles the JSONPath expression for later evaluation. Returns a `jsonpath_expression` object 
that represents the JSONPath expression.

#### Parameters

<table>
  <tr>
    <td>expr</td>
    <td>JSONPath expression</td> 
  </tr>
  <tr>
    <td>ec</td>
    <td>out-parameter for reporting errors in the non-throwing overload</td> 
  </tr>
</table>

#### Exceptions

(1) Throws a [jsonpath_error](jsonpath_error.md) if JSONPath compilation fails.

(2) Sets the out-parameter `ec` to the [jsonpath_error_category](jsonpath_errc.md) if JSONPath compilation fails. 

### Examples

#### Custom functions

```c++
int main()
{
    jsonpath::custom_functions<json> funcs;
    funcs.register_function("divide", // function name
         2,                           // number of arguments   
         [](jsoncons::span<const jsonpath::parameter<json>> params, std::error_code& ec) -> json 
         {
           if (!(params[0].value().is_number() && params[1].value().is_number())) 
           {
               ec = jsonpath::jsonpath_errc::invalid_type; 
               return json::null();
           }
           return json(params[0].value().as<double>() / params[1].value().as<double>());
         }
    );

    json root = json::parse(R"([{"foo": 60, "bar": 10},{"foo": 60, "bar": 5}])");

    auto expr = jsonpath::make_expression<json>("$[?(divide(@.foo, @.bar) == 6)]", funcs);
    json result = expr.evaluate(root);

    std::cout << result << "\n\n";
}
```
Output:
```
[{"bar": 10,"foo": 60}]
```

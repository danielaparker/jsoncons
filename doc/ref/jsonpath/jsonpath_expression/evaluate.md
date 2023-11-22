### jsoncons::jsonpath::jsonpath_expression::evaluate

```cpp
Json evaluate(reference root_value, result_options options = result_options()); (1)
```
```cpp
template <class BinaryOp>
void evaluate(reference root_value, BinaryOp op, 
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
    <td><code>op</code></td>
    <td>A function object that accepts a path and a reference to a Json value. 
It must have function call signature equivalent to
<br/><br/><code>
void fun(const Json::string_view_type& path, const Json& val);
</code><br/><br/>
  </tr>
  <tr>
    <td>result_options</td>
    <td>Result options, a bitmask of type <a href="result_options.md">result_options</></td> 
  </tr>
</table>

#### Exceptions

(1) Throws a [jsonpath_error](jsonpath_error.md) if JSONPath compilation fails.

(2) Sets the out-parameter `ec` to the [jsonpath_error_category](jsonpath_errc.md) if JSONPath compilation fails. 


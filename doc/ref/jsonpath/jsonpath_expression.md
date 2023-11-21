### jsoncons::jsonpath::jsonpath_expression

```cpp
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

template <class Json>
class jsonpath_expression
```

#### Member functions
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
<br/><br/><code>
void fun(const basic_path_node<Json::char_type>& path, const Json& val);
</code><br/><br/>
  </tr>
  <tr>
    <td>result_options</td>
    <td>Result options, a bitmask of type <a href="result_options.md">result_options</></td> 
  </tr>
</table>

```cpp
template <class BinaryOp>
void update(const_reference root_value, BinaryOp op);                                   (1) (since 0.172.0)
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
<br/><br/><code>
void fun(const basic_path_node<Json::char_type>& path, Json& val);
</code><br/><br/>
  </tr>
</table>

#### Static functions

    static jsonpath_expression compile(const string_view_type& expr);           (1)

    static jsonpath_expression compile(const string_view_type& expr,
                                       const custom_functions<Json>& funcs);    (2) (since 0.164.0)

    static jsonpath_expression compile(const string_view_type& expr,
                                       std::error_code& ec);                    (3)

    static jsonpath_expression compile(const string_view_type& expr,
                                       const custom_functions<Json>& funcs,
                                       std::error_code& ec);                    (4) (since 0.164.0) 

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


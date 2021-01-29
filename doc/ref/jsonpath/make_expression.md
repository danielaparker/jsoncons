### jsoncons::jsonpath::make_expression

```c++
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

template <class Json>
jsonpath_expression<Json> make_expression(const typename Json::string_view_type& expr); (1)

template <class Json>
jsonpath_expression<Json> make_expression(const typename Json::string_view_type& expr,
                                          std::error_code& ec); (2)
```

Returns a compiled JSONPath expression for later evaluation.

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

#### Return value

Returns a `jsonpath_expression` object that represents the JSONPath expression.

#### Exceptions

(1) Throws a [jsonpath_error](jsonpath_error.md) if JSONPath compilation fails.

(2) Sets the out-parameter `ec` to the [jsonpath_error_category](jsonpath_errc.md) if JSONPath compilation fails. 


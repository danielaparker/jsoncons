### jsoncons::jsonpath::make_expression

```c++
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

template <class Json,class Source>
jsonpath_expression<Json> make_expression(const Source& expr); (1)

template <class Json>
jsonpath_expression<Json> make_expression(const typename Json::char_type* expr); (2)


template <class Json,class Source>
jsonpath_expression<Json> make_expression(const Source& expr,
                                          std::error_code& ec); (3)

template <class Json>
jsonpath_expression<Json> make_expression(const typename Json::char_type* expr,
                                          std::error_code& ec); (4)
```

(1) Makes a [jsonpath_expression](jsonpath_expression.md) from the JSONPath expression `expr`.
The JSONPath expression `expr` is provided as a sequential container or view of characters, 
such as a `std::basic_string` or `std::basic_string_view`.

(2) Makes a [jsonpath_expression](jsonpath_expression.md) from the JSONPath expression `expr`.
The JSONPath expression `expr` is provided as a null terminated string.

(3) Makes a [jsonpath_expression](jsonpath_expression.md) from the JSONPath expression `expr`.
The JSONPath expression `expr` is provided as a sequential container or view of characters, 
such as a `std::basic_string` or `std::basic_string_view`.

(4) Makes a [jsonpath_expression](jsonpath_expression.md) from the JSONPath expression `expr`.
The JSONPath expression `expr` is provided as a null terminated string.

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

(1) and (3) throw a [jsonpath_error](jsonpath_error.md) if JSONPath compilation fails.

(2) and (4) set the out-parameter `ec` to the [jsonpath_error_category](jsonpath_errc.md) if JSONPath compilation fails. 


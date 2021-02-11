### jsoncons::jmespath::make_expression

```c++
#include <jsoncons_ext/jmespath/jmespath.hpp>

template <class Json>
jmespath_expression<Json> make_expression(const json::string_view_type& expr); (1)

template <class Json>
jmespath_expression<Json> make_expression(const json::string_view_type& expr,
                                          std::error_code& ec); (2)
```

Returns a compiled JMESPath expression for later evaluation.

#### Parameters

<table>
  <tr>
    <td>expr</td>
    <td>JMESPath expression</td> 
  </tr>
  <tr>
    <td>ec</td>
    <td>out-parameter for reporting errors in the non-throwing overload</td> 
  </tr>
</table>

#### Return value

Returns a `jmespath_expression` object that represents the JMESPath expression.

#### Exceptions

(1) Throws a [jmespath_error](jmespath_error.md) if JMESPath compilation fails.

(2) Sets the out-parameter `ec` to the [jmespath_error_category](jmespath_errc.md) if JMESPath compilation fails. 


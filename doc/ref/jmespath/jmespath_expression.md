### jsoncons::jmespath::jmespath_expression

```cpp
#include <jsoncons_ext/jmespath/jmespath.hpp>

template <class Json>
class jmespath_expression
```

#### Member functions

    Json evaluate(reference doc); (1)

    Json evaluate(reference doc, std::error_code& ec); (2)

#### Parameters

<table>
  <tr>
    <td>doc</td>
    <td>Json value</td> 
  </tr>
  <tr>
    <td>ec</td>
    <td>out-parameter for reporting errors in the non-throwing overload</td> 
  </tr>
</table>

#### Exceptions

(1) Throws a [jmespath_error](jmespath_error.md) if JMESPath evaluation fails.

(2) Sets the out-parameter `ec` to the [jmespath_error_category](jmespath_errc.md) if JMESPath evaluation fails. 

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

#### Exceptions

(1) Throws a [jmespath_error](jmespath_error.md) if JMESPath compilation fails.

(2) Sets the out-parameter `ec` to the [jmespath_error_category](jmespath_errc.md) if JMESPath compilation fails. 


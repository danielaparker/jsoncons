### jsoncons::jmespath::search

```c++
#include <jsoncons_ext/jmespath/jmespath.hpp>

template<Json>
Json search(const Json& root, 
            const typename Json::string_view_type& path); // (1)

template<Json>
Json search(const Json& root, 
            const typename Json::string_view_type& expr,
            std::error_code& ec); // (2)
```

Returns a Json value.

#### Parameters

<table>
  <tr>
    <td>root</td>
    <td>Json value</td> 
  </tr>
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

Returns a Json value.

#### Exceptions

(1) Sets the `std::error_code&` to the [jmespath_error_category](jmespath_errc.md) if JMESPath evaluation fails. 

(2) Throws [jmespath_error](jmespath_error.md) if JMESPath evaluation fails.



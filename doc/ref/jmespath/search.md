### jsoncons::jmespath::search

```c++
#include <jsoncons_ext/jmespath/jmespath.hpp>

enum class result_type {value,path};

template<Json>
Json search(const Json& root, 
            const typename Json::string_view_type& path); // (1)

template<Json>
Json search(const Json& root, 
            const typename Json::string_view_type& path,
            std::error_code& ec); // (2)
```

Returns a `json` array of values or normalized path expressions selected from a root `json` structure.

#### Parameters

<table>
  <tr>
    <td>root</td>
    <td>JSON value</td> 
  </tr>
  <tr>
    <td>path</td>
    <td>JMESPath expression string</td> 
  </tr>
  <tr>
    <td>result_t</td>
    <td>Indicates whether results are matching values (the default) or normalized path expressions</td> 
  </tr>
</table>

#### Return value

Returns a `json` array containing either values or normalized path expressions matching the input path expression. 
Returns an empty array if there is no match.

#### Exceptions

Throws [jsonpath_error](jsonpath_error.md) if JSONPath evaluation fails.



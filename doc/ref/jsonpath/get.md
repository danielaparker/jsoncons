### jsoncons::jsonpath::get

```cpp
#include <jsoncons_ext/jsonpath/jsonpath.hpp>

template<class Json>
Json* get(Json& root, const basic_json_location<Json::char_type>& location); 
```

Returns a pointer to a JSON value in a JSON document at a specified location.

#### Parameters
<table>
  <tr>
    <td>root</td>
    <td>Root JSON value</td> 
  </tr>
  <tr>
    <td>location</td>
    <td>A <a href="basic_json_location.md">basic_json_location</a></td> 
  </tr>
</table>

#### Return value

Returns a pointer to the selected item, or null if not found. 

### Exceptions

None.



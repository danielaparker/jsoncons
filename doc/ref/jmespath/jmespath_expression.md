### jsoncons::jmespath::jmespath_expression

```cpp
#include <jsoncons_ext/jmespath/jmespath.hpp>

template <typename Json>
class jmespath_expression
```

#### Member types

Type                       |Definition
---------------------------|------------------------------
char_type                  |Json::char_type
string_type                |std::basic_json<char_type>

#### Member functions

    Json evaluate(reference doc) const;                                    (1)

    Json evaluate(reference doc, 
        const std::vector<std::pair<std::string,Json>>& params) const;           (until 1.3.1)  
                                                                           (2)
    Json evaluate(reference doc, 
        const std::map<string_type,Json>& params) const;                         (since 1.3.1) 
                                                            
    Json evaluate(reference doc, std::error_code& ec) const;               (3)
                                                            
    Json evaluate(reference doc, 
        const std::vector<std::pair<std::string,Json>>& params,                  (until 1.3.1)
        std::error_code& ec) const;    
                                                                           (4)
    Json evaluate(reference doc, 
        const std::map<string_type,Json>& params,                                (since 1.3.1)
        std::error_code& ec) const;    

#### Parameters

<table>
  <tr>
    <td>doc</td>
    <td>Json value</td> 
  </tr>
  <tr>
    <td>params</td>
    <td>List of parameters to be provided to an initial (global) scope when the query is evaluated</td> 
  </tr>
  <tr>
    <td>ec</td>
    <td>out-parameter for reporting errors in the non-throwing overload</td> 
  </tr>
</table>

#### Exceptions

(1),(3) Throws a [jmespath_error](jmespath_error.md) if JMESPath evaluation fails.

(2),(4) Sets the out-parameter `ec` to the [jmespath_error_category](jmespath_errc.md) if JMESPath evaluation fails. 


### jsoncons::jsonschema::make_json_schema

```cpp
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

template <class Json>
json_schema<Json> make_json_schema(const Json& sch, 
    evaluation_options options = evaluation_options{});                     (1)

template <class Json,class URIResolver>
json_schema<Json> make_json_schema(const Json& sch, 
    const URIResolver& resolver,                                            (2)
    evaluation_options options = evaluation_options{});                     

template <class Json,class URIResolver>
json_schema<Json> make_json_schema(const Json& sch, 
    const std::string& retrieval_uri,                                       (3)
    evaluation_options options = evaluation_options{});                      

template <class Json>
json_schema<Json> make_json_schema(const Json& sch, 
    const std::string& retrieval_uri,                                       (4)
    const URIResolver& resolver, 
    evaluation_options options = evaluation_options{});                      
```

Returns a `json_schema<Json>` that represents a compiled JSON Schema document.

#### Parameters

<table>
  <tr>
    <td>schema</td>
    <td>JSON Schema</td> 
  </tr>
  <tr>
    <td>resolver</td>
    <td>A function object with the signature of <code>resolver</code> being equivalent to 
    <pre>
    Json fun(const jsoncons::uri& uri)   
    </pre></td>   
  </tr>
  <tr>
    <td><a href="evaluation_options.md">options</a></td>
    <td>Evaluation options</td> 
  </tr>
  <tr>
    <td>retrieval_uri</td>
    <td>Optional retrieval URI</td> 
  </tr>
</table>

#### Return value

Returns a `json_schema<Json>` that represents a compiled JSON Schema document.

#### Exceptions

(1)-(4) Throws a [schema_error](schema_error.md) if JSON Schema loading fails.



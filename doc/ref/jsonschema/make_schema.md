### jsoncons::jsonschema::make_schema

```cpp
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

template <class Json>
std::shared_ptr<json_schema<Json>> make_schema(const Json& schema);  (1)

template <class Json>
std::shared_ptr<json_schema<Json>> make_schema(const Json& schema,
    const std::string& retrieval_uri);                               (2) (since 0.173.0)

template <class Json,class URIResolver>
std::shared_ptr<json_schema<Json>> make_schema(const Json& schema,
    const std::string& retrieval_uri, const URIResolver& resolver);  (3) (since 0.173.0)

template <class Json,class URIResolver>
std::shared_ptr<json_schema<Json>> make_schema(const Json& schema, 
    const URIResolver& resolver);                                    (4)
```

Returns a `shared_ptr` to a `json_schema<Json>`.

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
</table>

#### Return value

Returns a `shared_ptr` to a `json_schema<Json>`.

#### Exceptions

(1)-(2) Throws a [schema_error](schema_error.md) if JSON Schema loading fails.



### jsoncons::jsonschema::validation_output

```cpp
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>
```

<br>

`jsoncons::jsonschema::validation_output` defines an event type for reporting failures in jsonschema operations.

#### Member functions

    const std::string& instance_location() const;                       (until 0.174.0)
    const jsonpointer::json_pointer& instance_location() const;         (since 0.174.0)
The location of the JSON value within the instance being validated,
expressed as a URI fragment-encoded JSON Pointer.

    const std::string& message() const;
An error message that is produced by the validation.

    const jsonpointer::json_pointer& eval_path() const;                 (since 0.174.0)
The relative location of the validating keyword that follows 
the validation path, expressed as a JSONPointer. 

    const std::string& schema_path() const;                             (until 0.174.0)
    const jsoncons::uri& schema_path() const;                           (since 0.174.0)
The absolute, dereferenced location of the validating keyword,
expressed as an absolute URI using the canonical URI of the 
relevant schema.

    const std::vector<validation_output>& nested_errors() const
Returns a list of nested errors


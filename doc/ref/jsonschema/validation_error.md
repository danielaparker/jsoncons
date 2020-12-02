### jsoncons::jsonschema::validation_error

```c++
#include <jsoncons_ext/jsonschema/jsonvalidation_error.hpp>
```

<br>

`jsoncons::jsonschema::validation_error` defines an exception type for reporting failures in jsonschema operations.

![validation_error](./diagrams/validation_error.png)

#### Constructors

    validation_error(const std::string& pointer_to_violation,
                     const std::string& message,
                     const std::string& keyword);

#### Member functions

    const char* what() const noexcept;
Returns an error message

    const std::string& keyword() const;
Returns the JSON Schema keyword that occurred in the schema and was violated by the input JSON.



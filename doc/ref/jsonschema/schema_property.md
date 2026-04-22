### jsoncons::jsonschema::schema_property

```cpp
#include <jsoncons_ext/jsonschema/validator.hpp>

template <typename Json>
schema_property;
```

#### Destructor

    ~schema_property() = default;

#### Member functions

    virtual const std::string& keyword() const = 0;
Returns a property keyword.

    virtual const Json& constraint() const = 0;
Returns a subschema or an array of subschemas representing a constraint.

    virtual const jsoncons::uri& schema_location() const = 0;
Returns the location of the constraint in the JSON Schema.



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

    virtual const Json& subschemas() const = 0;
Returns the subschemas for this property

    virtual const jsoncons::uri& schema_location() const = 0;
Returns the location of the constraint in the JSON Schema.



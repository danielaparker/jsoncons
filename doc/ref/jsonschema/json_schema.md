### jsoncons::jsonschema::json_schema

```cpp
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

template <class Json>
class json_schema
```

#### Member functions

    bool is_valid(const Json& instance) const;  (1)

    void validate(const Json& instance) const;  (2)

    void validate(const Json& instance, Json& patch) const;  (3)

    template <class Reporter>
    void validate(const Json& instance, const Reporter& reporter) const;  (4)

    template <class Reporter>
    void validate(const Json& instance, const Reporter& reporter, Json& patch) const;  (5)

    void validate(const Json& instance, json_visitor& visitor) const;  (6)

(1) Validates input JSON against a JSON Schema and returns false upon the 
first schema violation.

(2) Validates input JSON against a JSON Schema with a default error reporter
that throws upon the first schema violation.

(3) Validates input JSON against a JSON Schema with a default error reporter
that throws upon the first schema violation. Writes a JSONPatch document to the output
parameter.

(4) Validates input JSON against a JSON Schema with a provided error reporter
that is called for each schema violation. 

(5) Validates input JSON against a JSON Schema with a provided error reporter
that is called for each schema violation. Writes a JSONPatch document to the output
parameter.

(6) Validates input JSON against a JSON Schema and writes the validation messages
to a [json_visitor](../corelib/basic_json_visitor.md).

#### Parameters

<table>
  <tr>
    <td>instance</td>
    <td>Input Json</td> 
  </tr>
  <tr>
    <td>reporter</td>
    <td>A function object with signature equivalent to 
    <pre>
           void fun(const validation_output& o)</pre>
which accepts an argument of type <a href="validation_output.md">validation_output</a>.</td> 
  </tr>
  <tr>
    <td>patch</td>
    <td>A JSONPatch document that may be applied to the input JSON
to fill in missing properties that have "default" values in the
schema.</td> 
  </tr>
  <tr>
    <td>visitor</td>
    <td>A [json_visitor](../corelib/basic_json_visitor.md) that receives JSON events 
    corresponding to an array of validation messages.</td> 
  </tr>
</table>

#### Return value
 
(1) `true` if the instance is valid, otherwise `false` 

(2) - (5) None.

#### Exceptions

(2) - (3) Throws a [validation_error](validation_error.md) for the first schema violation.

(4) - (5) `reporter` is called for each schema violation


### jsoncons::jsonschema::json_validator

```c++
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

template <class Json>
class json_validator
```

#### Constructor

    json_validator(std::shared_ptr<json_schema<Json>> schema);

#### Member functions

    Json validate(const Json& instance) const; // (1)

    template <class Reporter>
    Json validate(const Json& instance, const Reporter& reporter) const; // (2)

(1) Validates input JSON against a JSON Schema with a default error reporter
that throws upon the first schema violation.

(2) Validates input JSON against a JSON Schema with a provided error reporter
that is called for each schema violation.

#### Parameters

<table>
  <tr>
    <td>instance</td>
    <td>Input Json</td> 
  </tr>
  <tr>
    <td>reporter</td>
    <td>A function object with the signature of <code>reporter</code> being equivelent to 
    <pre>
           void fun(const validation_event& e)</pre></td>
  </tr>
</table>

#### Return value

Returns a JSONPatch document that may be applied to the input JSON
to fill in missing properties that have "default" values in the
schema.

#### Exceptions

(1) Throws a [std::invalid_argument](https://en.cppreference.com/w/cpp/error/invalid_argument) for the first schema violation.

(2) `reporter` is called for each schema violation


### jsoncons::jsonschema::json_schema::validate

```cpp
void validate(const Json& instance) const;  (1)

void validate(const Json& instance, Json& patch) const;  (2)

template <class MsgReporter>
void validate(const Json& instance, const MsgReporter& reporter) const;  (3)

template <class MsgReporter>
void validate(const Json& instance, const MsgReporter& reporter, Json& patch) const;  (4)

void validate(const Json& instance, json_visitor<Json>& visitor) const;  (5)
```

(1) Validates input JSON against a JSON Schema with a default error reporter
that throws upon the first schema violation.

(2) Validates input JSON against a JSON Schema with a default error reporter
that throws upon the first schema violation. Writes a JSONPatch document to the output
parameter.

(3) Validates input JSON against a JSON Schema with a provided error reporter
that is called for each schema violation. 

(4) Validates input JSON against a JSON Schema with a provided error reporter
that is called for each schema violation. Writes a JSONPatch document to the output
parameter.

(5) Validates input JSON against a JSON Schema and writes the validation messages
to a [json_visitor](../../corelib/basic_json_visitor.md).

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
           void fun(const validation_message& o)</pre>
which accepts an argument of type <a href="../validation_message.md">validation_message</a>.</td> 
  </tr>
  <tr>
    <td>patch</td>
    <td>A JSONPatch document that may be applied to the input JSON
to fill in missing properties that have "default" values in the
schema.</td> 
  </tr>
  <tr>
    <td>visitor</td>
    <td>A [json_visitor](../../corelib/basic_json_visitor.md) that receives JSON events 
    corresponding to an array of validation messages.</td> 
  </tr>
</table>

#### Return value
 
None.

#### Exceptions

(1) - (2) Throws a [validation_error](../validation_error.md) for the first schema violation.


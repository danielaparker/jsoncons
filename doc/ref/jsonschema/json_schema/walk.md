### jsoncons::jsonschema::json_schema::walk

```cpp
template <class WalkReporter>
walk_result walk(const Json& instance, const WalkReporter& reporter) const; (since 0.175.0)
```

Walks through a JSON schema.

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
           walk_result fun(const std::string& keyword,
            const Json& schema, const uri& schema_location,
            const Json& instance, const jsonpointer::json_pointer& instance_location)</pre>
</td> 
  </tr>
</table>

#### Return value
 
[walk_result](../walk_result.md)

#### Exceptions

None

### Examples

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <iostream>
#include <cassert>

using jsoncons::ojson;
namespace jsonschema = jsoncons::jsonschema;

int main()
{
    std::string schema_string = R"(
{
  "$id": "https://example.com/arrays.schema.json",
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "description": "A representation of a person, company, organization, or place",
  "type": "object",
  "properties": {
    "fruits": {
      "type": "array",
      "items": {
        "type": "string"
      }
    },
    "vegetables": {
      "type": "array",
      "items": {
        "$ref": "#/$defs/veggie"
      }
    }
  },
  "$defs": {
    "veggie": {
      "type": "object",
      "required": [
        "veggieName",
        "veggieLike"
      ],
      "properties": {
        "veggieName": {
          "type": "string",
          "description": "The name of the vegetable."
        },
        "veggieLike": {
          "type": "boolean",
          "description": "Do I like this vegetable?"
        }
      }
    }
  }
}
    )";

    ojson schema = ojson::parse(schema_string);
    jsonschema::json_schema<ojson> compiled = jsonschema::make_json_schema(std::move(schema));

    std::string data_string = R"(
{
  "fruits": [
    "apple",
    "orange",
    "pear"
  ],
  "vegetables": [
    {
      "veggieName": "potato",
      "veggieLike": true
    },
    {
      "veggieName": "broccoli",
      "veggieLike": false
    }
  ]
}
    )";

    // Data
    ojson data = ojson::parse(data_string);

    auto reporter = [](const std::string& keyword,
        const ojson& subschema, 
        const jsoncons::uri& /*schema_location*/,
        const ojson& /*instance*/, 
        const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_result
        {
            if (keyword == "type")
            {
                assert(subschema.is_object());
                auto it = subschema.find("type");
                if (it != subschema.object_range().end())
                {
                    std::cout << instance_location.string() << ": " << it->value() << "\n";
                }
            }
            return jsonschema::walk_result::advance;
        };
    compiled.walk(data, reporter);
}
```
Output:
```
/fruits/0: "string"
/fruits/1: "string"
/fruits/2: "string"
/fruits: "array"
/vegetables/0/veggieName: "string"
/vegetables/0/veggieLike: "boolean"
/vegetables/0: "object"
/vegetables/1/veggieName: "string"
/vegetables/1/veggieLike: "boolean"
/vegetables/1: "object"
/vegetables: "array"
: "object"
```

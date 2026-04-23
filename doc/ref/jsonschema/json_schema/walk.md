### jsoncons::jsonschema::json_schema<Json>::walk

```cpp
template <typename Reporter>
void walk(const Json& instance, const Reporter& reporter) const; (since 0.175.0)

template <typename Reporter>
void walk(const Json& instance, const Reporter& reporter, jsoncons::optional<Json>& patch) const; (since 0.175.0)
```

Walks through a JSON schema to collect information.

#### Parameters

<table>
  <tr>
    <td>instance</td>
    <td>Input Json</td> 
  </tr>
  <tr>
    <td>reporter</td>
    <td>A function object which returns a <a href="../walk_result.md">walk_result</a> (<a href="../walk_state.md">walk_state</a> since 1.7.0)
    to indicates whether to keep walking through the schema or stop. The signature of the function should be equivalent to 
    <pre>
<a href="../walk_result.md">walk_result</a> fun(const std::string& keyword,
    const Json& schema, 
    const uri& schema_location,
    const Json& instance, 
    const jsonpointer::json_pointer& instance_location)</pre>
or
    <pre>
<a href="../walk_state.md">walk_state</a> fun(const std::string& keyword,
    const uri& schema_location,
    const Json& instance,                                    (since 1.7.0)
    const jsonpointer::json_pointer& instance_location,      
    jsoncons::optional<Json>& patch)</pre>
</td> 
  </tr>
</table>

#### Return value
 
None

#### Exceptions

None

### Examples

#### Construct a type tree based on a JSON Schema and an instance

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <iostream>
#include <cassert>

namespace jsonschema = jsoncons::jsonschema;

int main()
{
    std::string schema_str = R"(
{
  "$id": "https://example.com/arrays.schema.json",
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "description": "Arrays of strings and objects",
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

    auto schema = jsoncons::ojson::parse(schema_str);
    auto compiled = jsonschema::make_json_schema(std::move(schema));

    std::string data_str = R"(
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
    auto data = jsoncons::ojson::parse(data_str);

    auto reporter = [](const std::string& keyword,
        const jsoncons::ojson& schema,
        const jsoncons::uri& /*schema_location*/,
        const jsoncons::ojson& /*instance*/,
        const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_result
        {
            if (keyword == "type")
            {
                assert(schema.is_object());
                auto it = schema.find("type");
                if (it != schema.object_range().end())
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

The type tree shows the allowable types for the data values as specifed in the schema.
No validation of the data is performed during its construction.

#### Construct a type tree referencing the original JSON Schema and an instance (since 1.7.0)

```cpp
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons/json.hpp>

#include <iostream>
#include <string>
#include <assert>

namespace jsonschema = jsoncons::jsonschema;
namespace jsonpointer = jsoncons::jsonpointer;

int main()
{
    std::string schema_str = R"(
{
  "$id": "https://example.com/arrays.schema.json",
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "description": "Arrays of strings and objects",
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

    auto schema = jsoncons::ojson::parse(schema_str);
    auto compiled = jsonschema::make_json_schema(std::move(schema));

    std::string data_str = R"(
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
    auto data = jsoncons::ojson::parse(data_str);

    auto reporter = [&schema](const std::string& keyword,
        const jsoncons::uri& schema_location,
        const ojson& /*instance*/, 
        const jsoncons::jsonpointer::json_pointer& instance_location, 
        jsoncons::optional<ojson>& /*patch*/) -> jsonschema::walk_state
    {
        //std::cout << "keyword: " << keyword << "\n";

        if (keyword == "type")
        {
            auto location = schema_location.fragment();
            const auto& subschema = jsonpointer::get(schema, location); 
            auto it = subschema.find("type");
            if (it != subschema.object_range().end())
            {
                std::cout << instance_location.string() << ": " << it->value() << "\n";
            }
        }
        return jsonschema::walk_state::advance;
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

The type tree shows the allowable types for the data values as specifed in the schema.
No validation of the data is performed during its construction.

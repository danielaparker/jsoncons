### jsoncons::jsonschema::json_schema<Json>::validate

```cpp
void validate(const Json& instance) const;                                            (1)

void validate(const Json& instance, Json& patch) const;                               (2)

template <typename CustomReporter>
void validate(const Json& instance, CustomReporter&& reporter) const;                 (3)

template <typename CustomReporter>
void validate(const Json& instance, CustomReporter&& reporter, Json& patch) const;    (4)

void validate(const Json& instance, json_visitor& visitor) const;                     (5)
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
to a [json_visitor](../corelib/basic_json_visitor.md).

#### Parameters

<table>
  <tr>
    <td>instance</td>
    <td>Input Json</td> 
  </tr>
  <tr>
    <td>reporter</td>
    <td>A function object which returns a <a href="../walk_result.md">walk_result</a> (<a href="../walk_state.md">walk_state</a> since 1.7.0)
    to indicates whether to keep validating or stop. 
    The signature of the function should be equivalent to 
    <pre>
       <a href="../walk_result.md">walk_result</a> fun(const <a href="../validation_message.md">validation_message</a>& msg);</pre>
     or 
    <pre>
       <a href="../walk_state.md">walk_state</a> fun(const <a href="../validation_message.md">validation_message</a>& msg,    (since 1.7.0)
           jsoncons::optional&lt;Json&gt;& patch);</pre>
</td> 
  </tr>
  <tr>
    <td>patch</td>
    <td>A JSONPatch document that may be applied to the input JSON
to fill in missing properties that have "default" values in the
schema.</td> 
  </tr>
  <tr>
    <td>visitor</td>
    <td>A <a href="../corelib/basic_json_visitor.md">json_visitor</a> that receives JSON events 
    produced from an array of validation messages.</td> 
  </tr>
</table>

#### Return value
 
None.

#### Exceptions

(1) - (2) Throws a [validation_error](validation_error.md) for the first schema violation.

### Examples

#### Validate using a custom reporter

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <iostream>

namespace jsonschema = jsoncons::jsonschema;

int main()
{
    std::string schema_str = R"(
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
      "items": { "$ref": "#/$defs/veggie" }
    }
  },
  "$defs": {
    "veggie": {
      "type": "object",
      "required": [ "veggieName", "veggieLike" ],
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

    std::string data_str = R"(
{
  "fruits": [ "apple", "orange", "pear" ],
  "vegetables": [
    {
      "veggieName": "potato",
      "veggieLike": true
    },
    {
      "veggieName": "broccoli",
      "veggieLike": "false"
    },
    {
      "veggieName": "carrot",
      "veggieLike": false
    },
    {
      "veggieName": "Swiss Chard"
    }
  ]
}
    )";

    auto schema = jsoncons::ojson::parse(schema_str);
    auto compiled = jsonschema::make_json_schema(schema);
    auto data = jsoncons::ojson::parse(data_str);

    std::cout << "\n(1) Validate using exceptions\n";
    try
    {
        compiled.validate(data);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << "\n";
    }

    std::cout << "\n(2) Validate using reporter callback\n";
    auto reporter = [](const jsonschema::validation_message& msg) -> jsonschema::walk_result
        {
            std::cout << msg.instance_location().string() << ": " << msg.message() << "\n";
            return jsonschema::walk_result::advance;
        };
    compiled.validate(data, reporter);

    std::cout << "\n(3) Validate outputting to a json decoder\n";
    jsoncons::json_decoder<jsoncons::ojson> decoder;
    compiled.validate(data, decoder);
    auto output = decoder.get_result();
    std::cout << pretty_print(output) << "\n";
}
```
Output:
```
(1) Validate using exceptions
/vegetables/1/veggieLike: Expected boolean, found string

(2) Validate using reporter callback
/vegetables/1/veggieLike: Expected boolean, found string
/vegetables/3: Required property 'veggieLike' not found.

(3) Validate outputting to a json decoder
[
    {
        "valid": false,
        "evaluationPath": "/properties/vegetables/items/$ref/properties/veggieLike/type",
        "schemaLocation": "https://example.com/arrays.schema.json#/$defs/veggie/properties/veggieLike",
        "instanceLocation": "/vegetables/1/veggieLike",
        "error": "Expected boolean, found string"
    },
    {
        "valid": false,
        "evaluationPath": "/properties/vegetables/items/$ref/required",
        "schemaLocation": "https://example.com/arrays.schema.json#/$defs/veggie/required",
        "instanceLocation": "/vegetables/3",
        "error": "Required property 'veggieLike' not found."
    }
]
```

#### Validate using a custom reporter that produces a patch to fix document being validated (since 1.7.0)

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>
#include <iostream>
#include <cassert>

namespace jsonschema = jsoncons::jsonschema;
namespace jsonpatch = jsoncons::jsonpatch;

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
      "items": { "$ref": "#/$defs/veggie" }
    }
  },
  "$defs": {
    "veggie": {
      "type": "object",
      "required": [ "veggieName", "veggieLike" ],
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

    std::string data_str = R"(
{
  "fruits": [ "apple", "orange", "pear" ],
  "vegetables": [
    {
      "veggieName": "potato",
      "veggieLike": true
    },
    {
      "veggieName": "carrot",
      "veggieLike": false
    },
    {
      "veggieName": "Swiss Chard"
    }
  ]
}
    )";

    auto schema = jsoncons::json::parse(schema_str);
    auto compiled = jsonschema::make_json_schema(schema);
    auto data = jsoncons::json::parse(data_str);
    jsoncons::json patch{jsoncons::json_array_arg};

    // reporter that patching
    auto reporter = [](const jsonschema::validation_message& msg,
        jsoncons::optional<jsoncons::json>& patch) -> jsonschema::walk_state
        {
            if (patch)
            {
                if (msg.message().find("Required property") != std::string::npos && msg.message().find("not found") != std::string::npos)
                {
                    jsoncons::json j;
                    j.try_emplace("op", "add");
                    j.try_emplace("path", msg.instance_location().string() + "/veggieLike");
                    j.try_emplace("value", false);
                    patch->push_back(std::move(j));

                    //there could be "return jsonschema::walk_state::advance_no_error;" for saying to validator "its not error, go forward!"
                }
            }
            std::cout << msg.instance_location().string() << ": " << msg.message() << "\n";
            return jsonschema::walk_state::advance;
        };

    std::cout << "\n(1) Validate and create patch\n\n";

    compiled.validate(data, reporter, patch);

    std::cout << "\nPATCH:\n" << pretty_print(patch) << "\n";

    std::cout << "\n(2) Apply patch\n";

    jsonpatch::apply_patch<jsoncons::json>(data, patch);

    std::cout << "\n\nPATCHED DATA:\n" << pretty_print(data) << "\n";

    std::cout << "\n(3) Re-validate\n\n";
    assert(compiled.is_valid(data));
}
```
Output:

```
(1) Validate and create patch

/vegetables/2: Required property 'veggieLike' not found.

PATCH:
[
    {
        "op": "add",
        "path": "/vegetables/2/veggieLike",
        "value": false
    }
]

(2) Apply patch


PATCHED DATA:
{
    "fruits": [
        "apple",
        "orange",
        "pear"
    ],
    "vegetables": [
        {
            "veggieLike": true,
            "veggieName": "potato"
        },
        {
            "veggieLike": false,
            "veggieName": "carrot"
        },
        {
            "veggieLike": false,
            "veggieName": "Swiss Chard"
        }
    ]
}

(3) Re-validate
```

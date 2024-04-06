### jsonschema extension

The jsonschema extension implements Drafts 4, 6, 7, 2019-9 and 2020-12 of the [JSON Schema Specification](https://json-schema.org/specification)
(since 0.174.0).

### Compliance 

#### Keywords

The jsoncons implementation passes all required tests in the [JSON Schema Test Suite](https://github.com/json-schema-org/JSON-Schema-Test-Suite/) for the keywords below.

| Keyword                    | Draft 4   | Draft 6   | Draft 7   | Draft 2019-09 | Draft 2020-12 |
|:--------------------------:|:---------:|:---------:|:---------:|:---------:|:---------:|
| $anchor                    |           |           |           | &#x1F7E2; | &#x1F7E2; |
| $defs                      |           |           |           | &#x1F7E2; | &#x1F7E2; |
| $dynamicAnchor             |           |           |           |           | &#x1F7E2; |
| $dynamicRef                |           |           |           |           | &#x1F7E2; |
| $id                        | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| $recursiveAnchor           |           |           |           | &#x1F7E2; |           |
| $recursiveRef              |           |           |           | &#x1F7E2; |           |
| $ref                       | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| $vocabulary                |           |           |           | &#x1F7E2; | &#x1F7E2; |
| additionalItems            | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| additionalProperties       | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| allOf                      | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| anyOf                      | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| const                      |           | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| contains                   |           | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| contentEncoding            |           |           | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| contentMediaType           |           |           | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| definitions                | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |           |           |
| dependencies               | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |           |           |
| dependentRequired          |           |           |           | &#x1F7E2; | &#x1F7E2; |
| dependentSchemas           |           |           |           | &#x1F7E2; | &#x1F7E2; |
| enum                       | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| exclusiveMaximum (boolean) | &#x1F7E2; |           |           |           |           |
| exclusiveMaximum (numeric) |           | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| exclusiveMinimum (boolean) | &#x1F7E2; |           |           |           |           |
| exclusiveMinimum (numeric) |           | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| if-then-else               |           |           | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| items                      | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| maxContains                |           |           |           | &#x1F7E2; | &#x1F7E2; |
| minContains                |           |           |           | &#x1F7E2; | &#x1F7E2; |
| maximum                    | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| maxItems                   | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| maxLength                  | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| maxProperties              | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| minimum                    | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| minItems                   | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| minLength                  | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| minProperties              | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| multipleOf                 | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| not                        | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| oneOf                      | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| pattern                    | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| patternProperties          | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| prefixItems                |           |           |           |           | &#x1F7E2; |
| properties                 | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| propertyNames              |           | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| required                   | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| type                       | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| unevaluatedItems           |           |           |           | &#x1F7E2; | &#x1F7E2; |
| unevaluatedProperties      |           |           |           | &#x1F7E2; | &#x1F7E2; |
| uniqueItems                | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |

#### Format

The implementation understands the following [formats](https://json-schema.org/understanding-json-schema/reference/string.html#format):

| Format        | Draft 4   | Draft 6   | Draft 7   | Draft 2019-09 | Draft 2020-12 |
|:-------------:|:---------:|:---------:|:---------:|:---------:|:---------:|
| date          |           | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| date-time     | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| email         | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| hostname      | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| ipv4          | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| ipv6          | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| json-pointer  |           | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| regex         | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |
| time          |           | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; | &#x1F7E2; |

Any other format type is ignored.

By default, since Draft 2019-09, format is no longer an assertion. It can be configured to be an assertion 
by setting the evaluation option `require_format_validation` to `true` 

### Classes
<table border="0">
  <tr>
    <td><a href="json_schema.md">json_schema</a></td>
    <td>A <code>json_schema</code> represents the compiled form of a JSON Schema document.</td> 
  </tr>
  <tr>
    <td><a href="evaluation_options.md">evaluation_options</a></td>
    <td>Specifies options for evaluating JSON Schema documents.</td> 
  </tr>
  <tr>
    <td><a href="validation_message.md"></a></td>
    <td>Allows configuration of JSON Schema evaluation.</td> 
  </tr>
</table>

### Functions

<table border="0">
  <tr>
    <td><a href="make_json_schema.md">make_json_schema</a></td>
    <td>Processes a JSON Schema document and returns the compiled form as a <code>json_schema</code>. 
  </tr>
</table>
  
### Default values
  
The JSON Schema Specification includes the ["default" keyword](https://json-schema.org/understanding-json-schema/reference/generic.html)  
for specifying a default value, but doesn't prescribe how implementations should use it during validation.
Some implementations ignore the default keyword, others support updating the input JSON to fill in a default value 
for a missing key/value pair. This implementation outputs a JSONPatch document that may be further applied to the input JSON to add the
missing key/value pairs.
  
### Examples

The example schemas are from [JSON Schema Miscellaneous Examples](https://json-schema.org/learn/miscellaneous-examples.html),
the [JSON Schema Test Suite](https://github.com/json-schema-org/JSON-Schema-Test-Suite), and user contributions.

[Arrays of things](#eg1)  
[Using a URIResolver to resolve references to schemas defined in external files](#eg2)  
[Validate before decoding JSON into C++ class objects](#eg3)  
[Default values](#eg4)  

 <div id="eg1"/>

#### Arrays of things

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

// for brevity
using jsoncons::json;
namespace jsonschema = jsoncons::jsonschema; 

int main() 
{
    // JSON Schema
    json schema = json::parse(R"(
{
  "$id": "https://example.com/arrays.schema.json",
  "$schema": "http://json-schema.org/draft-07/schema#",
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
      "items": { "$ref": "#/definitions/veggie" }
    }
  },
  "definitions": {
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
    )");

    // Data
    json data = json::parse(R"(
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
   )");

    try
    {
        // Throws schema_error if JSON Schema loading fails
        json_schema<json> compiled = jsonschema::make_json_schema(schema);

        std::size_t error_count = 0;
        auto reporter = [&error_count](const jsonschema::validation_message& msg)
        {
            ++error_count;
            std::cout << msg.instance_location().string() << ": " << msg.message() << "\n";
        };

        // Will call reporter for each schema violation
        compiled.validate(data, reporter);

        std::cout << "\nError count: " << error_count << "\n\n";
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
}
```

Output:
```
#/vegetables/1/veggieLike: Expected boolean, found string
#/vegetables/3: Required key "veggieLike" not found

Error count: 2
```

<div id="eg2"/>

#### Using a URIResolver to resolve references to schemas defined in external files

In this example, the main schema defines a reference using the `$ref` property to a
second schema, defined in an external file `name.json`,

```json
{
    "definitions": {
        "orNull": {
            "anyOf": [
                {
                    "type": "null"
                },
                {
                    "$ref": "#"
                }
            ]
        }
    },
    "type": "string"
}
```

jsoncons needs to know how to turn a URI reference to `name.json` into a JSON Schema document,
and for that it needs you to provide a `URIResolver`.

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <fstream>

// for brevity
using jsoncons::json;
namespace jsonschema = jsoncons::jsonschema; 

json resolver(const jsoncons::uri& uri)
{
    std::cout << "uri: " << uri.string() << ", path: " << uri.path() << "\n\n";

    std::string pathname = /* path_to_directory */;
    pathname += std::string(uri.path());

    std::fstream is(pathname.c_str());
    if (!is)
        throw jsonschema::schema_error("Could not open " + std::string(uri.base()) + " for schema loading\n");

    return json::parse(is);        
}

int main()
{ 
    // JSON Schema
    json schema = json::parse(R"(
{
    "$id": "http://localhost:1234/object",
    "type": "object",
    "properties": {
        "name": {"$ref": "name.json#/definitions/orNull"}
    }
}
    )");

    // Data
    json data = json::parse(R"(
{
    "name": {
        "name": null
    }
}
    )");

    try
    {
       // Throws schema_error if JSON Schema loading fails
       json_schema<json> compiled = jsonschema::make_json_schema(schema, resolver);

       std::size_t error_count = 0;
       auto reporter = [&error_count](const jsonschema::validation_message& msg)
       {
           ++error_count;
            std::cout << msg.instance_location().string() << ": " << msg.message() << "\n";
       };

       // Will call reporter for each schema violation
       compiled.validate(data, reporter);

       std::cout << "\nError count: " << error_count << "\n\n";
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
}
```
Output:
```
#/name: No rule matched, but one of them is required to match

Error count: 1
```

<div id="eg3"/>

#### Validate before decoding JSON into C++ class objects 

This example illustrates decoding data that validates against "oneOf"
into a `std::variant`.

```cpp
#include <variant> // This example requires C++17
#include <iostream>
#include <cassert>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

// for brevity
using jsoncons::json;
namespace jsonschema = jsoncons::jsonschema; 

namespace ns {

    struct os_properties {
        std::string command;
    };

    struct db_properties {
        std::string query;
    };

    struct api_properties {
        std::string target;
    };

    struct job_properties {
        std::string name;
        std::variant<os_properties,db_properties,api_properties> run;
    };

} // namespace ns

JSONCONS_N_MEMBER_TRAITS(ns::os_properties, 1, command)
JSONCONS_N_MEMBER_TRAITS(ns::db_properties, 1, query)
JSONCONS_N_MEMBER_TRAITS(ns::api_properties, 1, target)
JSONCONS_N_MEMBER_TRAITS(ns::job_properties, 2, name, run)

std::string test_schema = R"(
{
  "title": "job",
  "description": "job properties json schema",
  "definitions": {
    "os_properties": {
      "type": "object",
      "properties": {
        "command": {
          "description": "this is the OS command to run",
          "type": "string",
          "minLength": 1
        }
      },
      "required": [ "command" ],
      "additionalProperties": false
    },
    "db_properties": {
      "type": "object",
      "properties": {
        "query": {
          "description": "this is db query to run",
          "type": "string",
          "minLength": 1
        }
      },
      "required": [ "query" ],
      "additionalProperties": false
    },

    "api_properties": {
      "type": "object",
      "properties": {
        "target": {
          "description": "this is api target to run",
          "type": "string",
          "minLength": 1
        }
      },
      "required": [ "target" ],
      "additionalProperties": false
    }
  },

  "type": "object",
  "properties": {
    "name": {
      "description": "name of the flow",
      "type": "string",
      "minLength": 1
    },
    "run": {
      "description": "job run properties",
      "type": "object",
      "oneOf": [

        { "$ref": "#/definitions/os_properties" },
        { "$ref": "#/definitions/db_properties" },
        { "$ref": "#/definitions/api_properties" }

      ]
    }
  },
  "required": [ "name", "run" ],
  "additionalProperties":  false
}
)";

std::string test_data = R"(
{
    "name": "testing flow", 
    "run" : {
            "command": "some command"    
            }
}

)";

int main() 
{
    try
    {
        json schema = json::parse(test_schema);
        json data = json::parse(test_data);

        // Throws schema_error if JSON Schema loading fails
        json_schema<json> compiled = jsonschema::make_json_schema(schema);

        // Test that input is valid before attempting to decode
        if (compiled.is_valid(data))
        {
            const ns::job_properties v = data.as<ns::job_properties>(); 

            std::string output;
            jsoncons::encode_json(v, output, indenting::indent);
            std::cout << output << std::endl;

            // Verify that output is valid
            json test = json::parse(output);
            assert(compiled.is_valid(test));
        }
        else
        {
            std::cout << "Invalid input\n";
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
}
```
Output:
```
{
    "name": "testing flow",
    "run": {
        "command": "some command"
    }
}
```

<div id="eg4"/>

#### Default values

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>
#include <fstream>

// for brevity
using jsoncons::json;
namespace jsonschema = jsoncons::jsonschema; 
namespace jsonpatch = jsoncons::jsonpatch; 

int main() 
{
    // JSON Schema
    json schema = json::parse(R"(
{
    "properties": {
        "bar": {
            "type": "string",
            "minLength": 4,
            "default": "bad"
        }
    }
}
    )");

    // Data
    json data = json::parse("{}");

    try
    {
       // will throw schema_error if JSON Schema loading fails 
       json_schema<json> compiled = jsonschema::make_json_schema(schema); 

       // will throw a validation_error on first encountered schema violation 
       json patch;
       compiled.validate(data, patch); 

       std::cout << "Patch: " << patch << "\n";

       std::cout << "Original data: " << data << "\n";

       jsonpatch::apply_patch(data, patch);

       std::cout << "Patched data: " << data << "\n\n";
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
}
```
Output:
```
Patch: [{"op":"add","path":"/bar","value":"bad"}]
Original data: {}
Patched data: {"bar":"bad"}
```


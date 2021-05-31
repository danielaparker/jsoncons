### jsonschema extension

The jsonschema extension implements the JSON Schema [Draft 7](https://json-schema.org/specification-links.html#draft-7) specification for validating input JSON. (since 0.160.0)

### Compliance level

The jsoncons implementation passes all [draft 7 required tests](https://github.com/json-schema-org/JSON-Schema-Test-Suite/tree/master/tests/draft7).

In addition, the validator understands the following optional [format types](https://json-schema.org/understanding-json-schema/reference/string.html#format):

|                      | Draft 7            |
|----------------------|--------------------|
| date                 | <em>&#x2713;</em> |
| time                 | <em>&#x2713;</em> |
| date-time            | <em>&#x2713;</em> |
| email                | <em>&#x2713;</em> |
| hostname             | <em>&#x2713;</em> |
| ipv4                 | <em>&#x2713;</em> |
| ipv6                 | <em>&#x2713;</em> |
| regex                | <em>&#x2713;</em> |

Any other format type is ignored.

### Classes
<table border="0">
  <tr>
    <td><a href="json_validator.md">json_validator</a></td>
    <td>JSON Schema validator.</td> 
  </tr>
</table>

### Functions

<table border="0">
  <tr>
    <td><a href="make_schema.md">make_schema</a></td>
    <td>Loads a JSON Schema and returns a shared pointer to a <code>json_schema</code>. 
  </tr>
</table>
  
### Default values
  
The JSON Schema Specification includes the ["default" keyword](https://json-schema.org/understanding-json-schema/reference/generic.html)  
for specifying a default value, but doesn't prescribe how implementations should use it during validation.
Some implementations ignore the default keyword, others support updating the input JSON to fill in a default value 
for a missing key/value pair. This implementation returns a JSONPatch document that may be further applied to the input JSON to add the
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

```c++
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
        auto sch = jsonschema::make_schema(schema);

        std::size_t error_count = 0;
        auto reporter = [&error_count](const jsonschema::validation_output& o)
        {
            ++error_count;
            std::cout << o.instance_location() << ": " << o.message() << "\n";
        };

        jsonschema::json_validator<json> validator(sch);

        // Will call reporter for each schema violation
        validator.validate(data, reporter);

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

```c++
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
       auto sch = jsonschema::make_schema(schema, resolver);

       std::size_t error_count = 0;
       auto reporter = [&error_count](const jsonschema::validation_output& o)
       {
           ++error_count;
            std::cout << o.instance_location() << ": " << o.message() << "\n";
       };

       jsonschema::json_validator<json> validator(sch);

       // Will call reporter for each schema violation
       validator.validate(data, reporter);

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

```c++
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
        auto sch = jsonschema::make_schema(schema);

        jsonschema::json_validator<json> validator(sch);

        // Test that input is valid before attempting to decode
        if (validator.is_valid(data))
        {
            const ns::job_properties v = data.as<ns::job_properties>(); 

            std::string output;
            jsoncons::encode_json_pretty(v, output);
            std::cout << output << std::endl;

            // Verify that output is valid
            json test = json::parse(output);
            assert(validator.is_valid(test));
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

```c++
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
       auto sch = jsonschema::make_schema(schema, resolver); 

       jsonschema::json_validator<json> validator(sch); 

       // will throw a validation_error on first encountered schema violation 
       json patch = validator.validate(data); 

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


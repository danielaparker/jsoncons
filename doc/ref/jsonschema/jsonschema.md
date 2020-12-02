### jsonschema extension

The jsonschema extension implements the JSON Schema [Draft 7](https://json-schema.org/specification-links.html#draft-7) specification for validating input JSON. (since 0.160.0)

### Compliance level

The jsonschema extension supports JSON Schema draft 7.

The supported [formats](https://json-schema.org/understanding-json-schema/reference/string.html#format) 
in the JSON Schema specification are:

|                      | Draft 7            |
|----------------------|--------------------|
| date                 | :heavy_check_mark: |
| time                 | :heavy_check_mark: |
| date-time            | :heavy_check_mark: |
| email                | :heavy_check_mark: |
| hostname             | :heavy_check_mark: |
| ipv4                 | :heavy_check_mark: |
| ipv6                 | :heavy_check_mark: |
| regex                | :heavy_check_mark: |

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
    <td>Loads a JSON Schema and returns a <code>shared_ptr</code> to a <code>json_schema</code>. 
  </tr>
</table>
  
### Examples

The examples below are from [JSON Schema Miscellaneous Examples](https://json-schema.org/learn/miscellaneous-examples.html)
and the [JSON Schema Test Suite](https://github.com/json-schema-org/JSON-Schema-Test-Suite).

#### Arrays of things

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

using jsoncons::json;
namespace jsonschema = jsoncons::jsonschema; // for brevity

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

    auto sch = jsonschema::make_schema(schema);

    std::size_t error_count = 0;
    auto reporter = [&error_count](const jsonschema::validation_error& e)
    {
        ++error_count;
        std::cout << e.what() << "\n";
    };

    jsonschema::json_validator<json> validator(sch);
    validator.validate(data, reporter);

    std::cout << "\nError count: " << error_count << "\n";
}
```

Output:
```
/vegetables/1/veggieLike: Expected boolean, found string
/vegetables/3: Required key "veggieLike" not found

Error count: 2
```

#### Using a URIResolver to resolve references to schemas defined in external files

In this example, the main schema defines a reference using
the `$ref` property to a second schema defined in the file `name.json`,
shown below:

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
You need to provide a `URIResolver` to tell jsoncons how to resolve the reference into a 
JSON Schema document.

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <fstream>

using jsoncons::json;
namespace jsonschema = jsoncons::jsonschema; // for brevity

json resolver(const jsoncons::uri& uri)
{
    if (uri.path() == "/draft-07/schema")
    {
        return jsoncons::jsonschema::json_schema_draft7::get_schema();
    }
    else
    {
        std::cout << uri.string() << ", " << uri.path() << "\n\n";

        std::string pathname = /* path_to_directory */;
        pathname += std::string(uri.path());

        std::fstream is(pathname.c_str());
        if (!is)
            throw jsonschema::schema_error("Could not open " + std::string(uri.base()) + " for schema loading\n");

        return json::parse(is);
    }
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

    auto sch = jsonschema::make_schema(schema, resolver);

    std::size_t error_count = 0;
    auto reporter = [&error_count](const jsonschema::validation_error& e)
    {
        ++error_count;
        std::cout << e.what() << "\n";
    };

    jsonschema::json_validator<json> validator(sch);
    validator.validate(data, reporter);

    std::cout << "\nError count: " << error_count << "\n";
}
```
Output:
```
http://localhost:1234/name.json, /name.json

/name: No subschema matched, but one of them is required to match
```

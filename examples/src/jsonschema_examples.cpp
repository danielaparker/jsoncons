// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>
#include <fstream>

// for brevity
using jsoncons::json;
namespace jsonschema = jsoncons::jsonschema; 
namespace jsonpatch = jsoncons::jsonpatch; 

namespace {

    void reporter_example() 
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
            // Will throw schema_error if JSON Schema loading fails
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
            std::cout << e.what() << "\n";
        }
    }

    json resolver(const jsoncons::uri& uri)
    {
        std::cout << "uri: " << uri.string() << ", path: " << uri.path() << "\n\n";

        std::string pathname = "./input/jsonschema/";
        pathname += std::string(uri.path());

        std::fstream is(pathname.c_str());
        if (!is)
            throw jsonschema::schema_error("Could not open " + std::string(uri.base()) + " for schema loading\n");

        return json::parse(is);        
    }

    void uriresolver_example()
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
            // Will throw schema_error if JSON Schema loading fails
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

    void defaults_example() 
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

        try
        {
            // Data
            json data = json::parse("{}");

            // will throw schema_error if JSON Schema loading fails 
            auto sch = jsonschema::make_schema(schema, resolver); 

            jsonschema::json_validator<json> validator(sch); 

            // will throw a validation_error when a schema violation happens 
            json patch = validator.validate(data); 

            std::cout << "Patch: " << patch << "\n";

            std::cout << "Original data: " << data << "\n";

            jsonpatch::apply_patch(data, patch);

            std::cout << "Patched data: " << data << "\n\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }

} // namespace

void jsonschema_examples()
{
    std::cout << "\nJSON Schema Examples\n\n";

    reporter_example();
    uriresolver_example();
    defaults_example();

    std::cout << "\n";
}


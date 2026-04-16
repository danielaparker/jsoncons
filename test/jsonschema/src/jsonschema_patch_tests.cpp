// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/utility/byte_string.hpp>

#include <fstream>
#include <iostream>
#include <regex>
#include <catch/catch.hpp>
 
using jsoncons::json;
namespace jsonschema = jsoncons::jsonschema;
namespace jsonpatch = jsoncons::jsonpatch;

TEST_CASE("jsonschema patch tests")
{
    SECTION("oneOf")
    {
        json schema = json::parse(R"(
{
  "$id": "https://example.com/oneOf",
  "$schema": "http://json-schema.org/draft-07/schema#",
  "oneOf": [
    {
      "type": "object",
      "additionalProperties": false,
      "properties": {
        "foo": {
          "default": "foo-default"
        }
      }
    },
    {
      "type": "object",
      "additionalProperties": false,
      "properties": {
        "bar": {
          "default": "bar-default"
        }
      }
    }
  ]
}
        )");

        const json data = json::parse(R"(
{
  "bar": "bar-custom"
}
        )");

        const jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(std::move(schema)); 

        const json expectedPatch{jsoncons::json_array_arg};

        json resultPatch;
        compiled.validate(data, resultPatch);

        CHECK(expectedPatch == resultPatch);
    }
    SECTION("anyOf")
    {
        json schema = json::parse(R"(
{
  "$id": "https://example.com/oneOf",
  "$schema": "http://json-schema.org/draft-07/schema#",
  "anyOf": [
    {
      "type": "object",
      "additionalProperties": false,
      "properties": {
        "foo": {
          "default": "foo-default"
        }
      }
    },
    {
      "type": "object",
      "additionalProperties": false,
      "properties": {
        "bar": {
          "default": "bar-default"
        }
      }
    }
  ]
}
        )");

        const json data = json::parse(R"(
{
  "bar": "bar-custom"
}
        )");

        const jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(std::move(schema)); 

        const json expectedPatch{jsoncons::json_array_arg};

        json resultPatch;
        compiled.validate(data, resultPatch);

        CHECK(expectedPatch == resultPatch);
    }

    SECTION("patch empty object with no default")
    {
        std::string schemaString = R"(
{ 
  "$schema": "https://json-schema.org/draft/2020-12/schema", 
  "$id": "https://example.com/main-schema", "type": "object", 
  "properties": { "objectType": { "type": "string", "enum": [ "Table", "Chair", "Planner", "Apple", "Water" ] } }, "required": [ "objectType" ] 
}         
        )";

        jsoncons::ojson data, patch;
        auto schema = jsoncons::jsonschema::make_json_schema(jsoncons::ojson::parse(schemaString));

        schema.validate(data, patch);
        CHECK(patch.is_array());
        CHECK(patch.empty());
    }
}

TEST_CASE("jsonschema reporter patch tests")
{
    SECTION("custom reporter that can control json-patches")
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
      "veggieName": "carrot",
      "veggieLike": false
    },
    {
      "veggieName": "Swiss Chard"
    }
  ]
}
        )";

        json schema = json::parse(schema_str);
        jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(std::move(schema));
        json data = json::parse(data_str);
        json patch;

        // reporter that patching
        auto reporter = [](const jsonschema::validation_message& msg, 
                           jsoncons::optional<json>& patch) -> jsonschema::walk_result
        {
            if (patch)
            {
                if (msg.message().find("Required property") != std::string::npos && msg.message().find("not found") != std::string::npos)
                {
                    json j;
                    j.try_emplace("op", "add");
                    j.try_emplace("path", msg.instance_location().string() + "/veggieLike");
                    j.try_emplace("value", false);
                    patch->push_back(std::move(j));

                    //there could be "return jsonschema::walk_result::advance_no_error;" for saying to validator "its not error, go forward!"
                }
            }
            std::cout << msg.instance_location().string() << ": " << msg.message() << "\n";
            return jsonschema::walk_result::advance;
        };
        compiled.validate(data, reporter, patch);

        std::cout << "\n(3) Re-validate\n\n";

        std::cout << "PATCH:\n" << pretty_print(patch) << "\n";

        jsoncons::jsonpatch::apply_patch<json>(data, patch);

        std::cout << "PATCHED DATA:\n" << pretty_print(data) << "\n";

        CHECK(compiled.is_valid(data));
    }
}

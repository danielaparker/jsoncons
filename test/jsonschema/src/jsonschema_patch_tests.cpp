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


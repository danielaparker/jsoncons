// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons/byte_string.hpp>

#include <catch/catch.hpp>
#include <fstream>
#include <iostream>
#include <regex>

using jsoncons::json;
namespace jsonschema = jsoncons::jsonschema;

TEST_CASE("jsonschema version tests")
{
    json schema_04 = json::parse(R"(
{
    "$schema": "http://json-schema.org/draft-04/schema#",
    "description": "A product from Acme's catalog",
    "properties": {
      "id": {
        "description": "The unique identifier for a product",
        "type": "integer"
      },
      "name": {
        "description": "Name of the product",
        "type": "string"
      },
      "price": {
        "exclusiveMinimum": true,
        "minimum": 0,
        "type": "number"
      },
      "tags": {
        "items": {
          "type": "string"
        },
        "minItems": 1,
        "type": "array",
        "uniqueItems": true
      }
    },
    "required": ["id", "name", "price"],
    "title": "Product",
    "type": "object"
  }
      )");

    json schema_07 = json::parse(R"(
{
    "$schema": "http://json-schema.org/draft-07/schema#",
    "description": "A product from Acme's catalog",
    "properties": {
      "id": {
        "description": "The unique identifier for a product",
        "type": "integer"
      },
      "name": {
        "description": "Name of the product",
        "type": "string"
      },
      "price": {
        "exclusiveMinimum": true,
        "minimum": 0,
        "type": "number"
      },
      "tags": {
        "items": {
          "type": "string"
        },
        "minItems": 1,
        "type": "array",
        "uniqueItems": true
      }
    },
    "required": ["id", "name", "price"],
    "title": "Product",
    "type": "object"
  }
      )");

    SECTION("test 4")
    {
        REQUIRE_THROWS_WITH(jsonschema::make_schema(schema_04), "Unsupported schema version http://json-schema.org/draft-04/schema#");
    }

    SECTION("test 7")
    {
        REQUIRE_THROWS_WITH(jsonschema::make_schema(schema_07), "exclusiveMinimum must be a number value");
    }
}


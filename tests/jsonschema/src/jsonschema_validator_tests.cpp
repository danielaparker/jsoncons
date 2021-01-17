// Copyright 2020 Daniel Parker
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

TEST_CASE("jsonschema validator tests")
{
    SECTION("Basic")
    {
        json schema = json::parse(R"(
{
  "$id": "https://example.com/polygon",
  "$schema": "https://json-schema.org/draft/2019-09/schema",
  "$defs": {
    "point": {
      "type": "object",
      "properties": {
        "x": { "type": "number" },
        "y": { "type": "number" }
      },
      "additionalProperties": false,
      "required": [ "x", "y" ]
    }
  },
  "type": "array",
  "items": { "$ref": "#/$defs/point" },
  "minItems": 3,
  "maxItems": 1
}

        )");

        json instance = json::parse(R"(
[
  {
    "x": 2.5,
    "y": 1.3
  },
  {
    "x": 1,
    "z": 6.7
  }
]
        )");

        auto sch = jsonschema::make_schema(schema);
        jsonschema::json_validator<json> validator(sch);

        CHECK_FALSE(validator.is_valid(instance));
    }
}

/*
: Expected minimum item count: 3, found: 2
/1: Required key "y" not found
/1: Validation failed for additional property "z". False schema always fails
*/

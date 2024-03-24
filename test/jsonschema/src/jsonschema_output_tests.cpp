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

TEST_CASE("jsonschema output format tests")
{
    SECTION("Basic")
    {
        json schema = json::parse(R"(
{
  "$id": "https://example.com/polygon",
  "$schema": "http://json-schema.org/draft-07/schema#",
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

        jsonschema::json_schema<json> validator = jsonschema::make_json_schema(schema);

        auto reporter = [](const jsonschema::validation_output& o)
        {
            std::cout << o.keyword() << ", " << o.keyword_location() << ", " << o.absolute_keyword_location() << "\n";

            for (auto& item : o.nested_errors())
            {
                std::cout << "    " << item.keyword() << ", " << item.keyword_location()  << ", " << item.absolute_keyword_location() << "\n";
            }
/*
            if (o.keyword() == "minItems")
            {
                CHECK(o.schema_path() == std::string("https://example.com/polygon#/minItems"));
            }
            else if (o.keyword() == "maxItems")
            {
                CHECK(o.schema_path() == std::string("https://example.com/polygon#/maxItems"));
            }
            else if (o.keyword() == "required")
            {
                CHECK(o.schema_path() == std::string("https://example.com/polygon#/$defs/point/required"));
            }
            else if (o.keyword() == "additionalProperties")
            {
                CHECK(o.schema_path() == std::string("https://example.com/polygon#/$defs/point/additionalProperties/false"));
            }
            else
            {
                //std::cout << o.keyword() << ", " << o.instance_location() << ": " << o.message() << ", " << o.schema_path() << "\n";
                //for (const auto& nested : o.nested_errors())
                //{
                //    std::cout << "    " << nested.message() << "\n";
                //}
            }
 */
        };
        validator.validate(instance, reporter);

    }
}

/*
: Expected minimum item count: 3, found: 2
/1: Required key "y" not found
/1: Validation failed for additional property "z". False schema always fails
*/

// https://github.com/json-schema-org/json-schema-spec/issues/643

TEST_CASE("jsonschema output format tests 2")
{
    json schema = json::parse(R"(
{
  "$id":"http://schemarepo.org/schemas/user.json",
  "$schema":"http://json-schema.org/draft-07/schema#",
  "type":"object",
  "definitions":{
    "min18":{
      "type":"integer",
      "minimum":18
    },
    "username":{
      "type":"string",
      "minLength":8
    },
    "member":{
      "type":"object",
      "properties":{
        "age":{"$ref":"#/definitions/min18"},
        "username":{"$ref":"#/definitions/username"}
      }
    },
    "membershipTypes":{"enum":["admin","user"]}
  },
  "oneOf":[
    {
      "properties":{
        "member":{"$ref":"#/definitions/member"},
        "membershipType":{"$ref":"#/definitions/membershipTypes"}
      }
    },
    {
      "properties":{
        "membershipType":{"const":"guest"},
        "firstName":{"type":"string"},
        "lastName":{"type":"string"}
      },
      "additionalProperties":false
    }
  ]
}
    )");

    /*SECTION("With ref")
    {

        json instance = json::parse(R"(
{
  "member":{
      "age":5,  // doesn't meet minimum
      "username":"aName"  // doesn't meet minLength
  },
  "membershipType":"user"
}
)");

        jsonschema::json_schema<json> validator = jsonschema::make_json_schema(schema);

        auto reporter = [](const jsonschema::validation_output& o)
        {
            std::cout << o.keyword() << ", " << o.keyword_location() << ", " << o.absolute_keyword_location() << "\n";

            for (auto& item : o.nested_errors())
            {
                std::cout << "    " << item.keyword() << ", " << item.keyword_location()  << ", " << item.absolute_keyword_location() << "\n";
            }
        };
        validator.validate(instance, reporter);

    }*/
}

/*
{
  "valid":false,
  "errors":[
    {
      "keywordLocation":"#/oneOf",
      "instanceLocation":"/",
      "message":"the instance did not pass any of the subschema"
    },
    {
      "keywordLocation":"#/oneOf/0/properties/member/properties/age/$ref/minimum",
      "absoluteKeywordLocation":"http://schemarepo.org/schemas/user.json#/definitions/min18/minimum",
      "instanceLocation":"/member/age",
      "message":"value is too small"
    },
    {
      "keywordLocation":"#/oneOf/0/properties/member/properties/userName/$ref/minLength",
      "absoluteKeywordLocation":"http://schemarepo.org/schemas/user.json#/definitions/username/minLength",
      "instanceLocation":"/member/username",
      "message":"value is too short"
    },
    {
      "keywordLocation":"#/oneOf/1/membershipType",
      "instanceLocation":"/member/membershipType",
      "message":"value does not match the required value"
    },
    {
      "keywordLocation":"#/oneOf/1/additionalProperties",
      "instanceLocation":"/member/member",
      "message":"additional properties are not allowed"
    }
  ]
}

*/

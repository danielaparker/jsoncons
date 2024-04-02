// Copyright 2013-2024 Daniel Parker
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
using jsoncons::ojson;
namespace jsonschema = jsoncons::jsonschema;

TEST_CASE("jsonschema validation report tests")
{
    json schema = json::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "$id": "https://test.com/schema",
    "$defs": {
        "integer": {
            "type": "integer"
        },      
        "minimum": {
            "minimum": 5
        }      
    },
    "type" : "object",
    "properties" : {
        "passes" : true,
        "fails" : false,
        "refs" : {"$ref" : "#/$defs/integer"},
        "multi" : {
            "allOf" : [{"$ref" : "#/$defs/integer"},{"$ref" : "#/$defs/minimum"}]
        }
    }
}
    )");

    SECTION("Test 1")
    {
        ojson expected = ojson::parse(R"(
[
    {
        "valid": false,
        "evaluationPath": "/properties/fails",
        "schemaLocation": "https://test.com/schema#/properties/fails",
        "instanceLocation": "/fails",
        "error": "False schema always fails"
    }
]
        )");

        jsoncons::json_decoder<ojson> decoder;    
        jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(schema);
    
        json data = json::parse(R"({"fails":"value"})");
    
        compiled.validate(data, decoder);
        
        ojson output = decoder.get_result();
        CHECK(expected == output);
        //std::cout << pretty_print(output) << "\n";
    }
    SECTION("Test 2")
    {
        ojson expected = ojson::parse(R"(
[
    {
        "valid": false,
        "evaluationPath": "/properties/multi/allOf",
        "schemaLocation": "https://test.com/schema#/properties/multi/allOf",
        "instanceLocation": "/multi",
        "error": "No schema matched, but all of them are required to match",
        "details": [
            {
                "valid": false,
                "evaluationPath": "/properties/multi/allOf/0/$ref/type",
                "schemaLocation": "https://test.com/schema#/$defs/integer",
                "instanceLocation": "/multi",
                "error": "Expected integer, found number"
            },
            {
                "valid": false,
                "evaluationPath": "/properties/multi/allOf/1/$ref/minimum",
                "schemaLocation": "https://test.com/schema#/$defs/minimum/minimum",
                "instanceLocation": "/multi",
                "error": "3.5 is less than minimum 5"
            }
        ]
    }
]
        )");

        jsoncons::json_decoder<ojson> decoder;    
        jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(schema);

        json data = json::parse(R"({"multi":3.5})");

        compiled.validate(data, decoder);

        ojson output = decoder.get_result();
        CHECK(expected == output);
        //std::cout << pretty_print(output) << "\n";
    }
}

TEST_CASE("jsonschema prefixItems report tests")
{
    json schema = json::parse(R"(
{
  "type": "array",
  "prefixItems": [
    { "type": "number" },
    { "type": "string" },
    { "enum": ["Street", "Avenue", "Boulevard"] },
    { "enum": ["NW", "NE", "SW", "SE"] }
  ],
  "items": false
}
    )");

    SECTION("Test 1")
    {
        ojson expected = ojson::parse(R"(
[
    {
        "valid": false,
        "evaluationPath": "/prefixItems/4",
        "schemaLocation": "#/prefixItems",
        "instanceLocation": "/4",
        "error": "Extra item at index '4' but the schema does not allow extra items."
    }
]
        )");

        jsoncons::json_decoder<ojson> decoder;    
        jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(schema);
    
        json data = json::parse(R"(
            [1600, "Pennsylvania", "Avenue", "NW", "Washington"]
        )");
    
        compiled.validate(data, decoder);
        
        ojson output = decoder.get_result();
        CHECK(expected == output);
        //std::cout << pretty_print(output) << "\n";
    }
}

TEST_CASE("jsonschema additionalProperties output tests")
{
    json schema = json::parse(R"(
{
  "type": "object",
  "properties": {
    "number": { "type": "number" },
    "street_name": { "type": "string" },
    "street_type": { "enum": ["Street", "Avenue", "Boulevard"] }
  },
  "additionalProperties": false
}
    )");

    SECTION("Test 1")
    {
        ojson expected = ojson::parse(R"(
[
    {
        "valid": false,
        "evaluationPath": "/additionalProperties/direction",
        "schemaLocation": "#",
        "instanceLocation": "/direction",
        "error": "Additional property 'direction' but the schema does not allow additional properties."
    }
]
        )");

        jsoncons::json_decoder<ojson> decoder;    
        jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(schema);
    
        json data = json::parse(R"(
{ "number": 1600, "street_name": "Pennsylvania", "street_type": "Avenue", "direction": "NW" }
)");
    
        compiled.validate(data, decoder);
        
        ojson output = decoder.get_result();
        CHECK(expected == output);
        //std::cout << pretty_print(output) << "\n";
    }
}

TEST_CASE("jsonschema unevaluatedProperties output tests")
{
    json schema = json::parse(R"(
{
  "allOf": [
    {
      "type": "object",
      "properties": {
        "street_address": { "type": "string" },
        "city": { "type": "string" },
        "state": { "type": "string" }
      },
      "required": ["street_address", "city", "state"]
    }
  ],

  "properties": {
    "type": { "enum": ["residential", "business"] }
  },
  "required": ["type"],
  "unevaluatedProperties": false
}    
)");

    SECTION("Test 1")
    {
        ojson expected = ojson::parse(R"(
[
    {
        "valid": false,
        "evaluationPath": "/unevaluatedProperties/something that doesn't belong",
        "schemaLocation": "#",
        "instanceLocation": "/something that doesn't belong",
        "error": "Unevaluated property 'something that doesn't belong' but the schema does not allow unevaluated properties."
    }
]
        )");

        jsoncons::json_decoder<ojson> decoder;    
        jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(schema);
    
        json data = json::parse(R"(
{
  "street_address": "1600 Pennsylvania Avenue NW",
  "city": "Washington",
  "state": "DC",
  "type": "business",
  "something that doesn't belong": "hi!"
}
        )");
    
        compiled.validate(data, decoder);
        
        ojson output = decoder.get_result();
        CHECK(expected == output);
    }
}

TEST_CASE("jsonschema unevaluatedItems output tests")
{
    json schema = json::parse(R"(
{
  "prefixItems": [
    { "type": "string" }, { "type": "number" }
  ],
  "unevaluatedItems": false
}
)");

    SECTION("Test 1")
    {
        ojson expected = ojson::parse(R"(
[
    {
        "valid": false,
        "evaluationPath": "/unevaluatedProperties/2",
        "schemaLocation": "#",
        "instanceLocation": "/2",
        "error": "Unevaluated item at index '2' but the schema does not allow unevaluated items."
    }
]
        )");

        jsoncons::json_decoder<ojson> decoder;    
        jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(schema);
    
        json data = json::parse(R"(
["foo", 42, null]
        )");
    
        compiled.validate(data, decoder);
        
        ojson output = decoder.get_result();
        CHECK(expected == output);
        //std::cout << pretty_print(output) << "\n";
    }
}


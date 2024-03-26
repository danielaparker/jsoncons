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
        json expected = json::parse(R"(
{
    "details": [
        {
            "error": "False schema always fails",
            "evaluationPath": "/properties/fails",
            "instanceLocation": "/fails",
            "schemaLocation": "https://test.com/schema#/properties/fails",
            "valid": false
        }
    ],
    "valid": false
}
        )");

        jsoncons::json_decoder<json> decoder;    
        jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(schema);
    
        json data = json::parse(R"({"fails":"value"})");
    
        compiled.validate(data, decoder);
        
        json output = decoder.get_result();
        CHECK(expected == output);
        //std::cout << pretty_print(output) << "\n";
    }
}


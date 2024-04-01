// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch.hpp>
#include <jsoncons/byte_string.hpp>

#include <catch/catch.hpp>
#include <fstream>
#include <iostream>
#include <regex>

using jsoncons::json;
namespace jsonschema = jsoncons::jsonschema;
namespace jsonpatch = jsoncons::jsonpatch;

TEST_CASE("jsonschema indexing and storing schema tests")
{
    std::string schema_document_str = R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "$id": "https://example.com/schema",
    "$defs": {
        "foo": {
            "$id": "schema/foo",
            "additionalProperties": {
                "$ref": "otherSchema"
            }
        }
    },
    "properties": {
        "thing": {"$ref": "schema/foo"},
        "nope" : false
    }
}
    )";

    std::string other_schema_str = R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "$id": "https://example.com/otherSchema"
}
    )";

    json schema_document = json::parse(schema_document_str);

    json other_schema = json::parse(other_schema_str);
    
    auto resolver = [other_schema](const jsoncons::uri& uri)
        {
            //std::cout << "resolver: " << uri.string() << "\n";
            if (uri.string() == "https://example.com/schema/otherSchema")
            {
                return other_schema;
            }
            else
            {
                return json::null();
            }
        };
    jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(schema_document, resolver); 

    SECTION("test 1")
    {
    }
}


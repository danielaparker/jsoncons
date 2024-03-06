// Copyright 2013-2023 Daniel Parker
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

TEST_CASE("jsonschema $recursiveRef tests")
{
    std::string tree_schema_string = R"(
{
    "$schema": "https://json-schema.org/draft/2019-09/schema",
    "$id": "https://example.com/tree",
    "$recursiveAnchor": true,

    "type": "object",
    "properties": {
        "data": true,
        "children": {
            "type": "array",
            "items": {
                "$recursiveRef": "#"
            }
        }
    }
}
    )";

    std::string strict_tree_schema_string = R"(
{
    "$schema": "https://json-schema.org/draft/2019-09/schema",
    "$id": "https://example.com/strict-tree",
    "$recursiveAnchor": true,

    "$ref": "tree",
    "unevaluatedProperties": false
}
    )";

    json tree_schema = json::parse(tree_schema_string);

    json strict_tree_schema = json::parse(strict_tree_schema_string);
    
    auto resolver = [tree_schema](const jsoncons::uri& uri)
        {
            //std::cout << "resolver: " << uri.string() << "\n";
            if (uri.string() == "https://example.com/tree")
            {
                return tree_schema;
            }
            else
            {
                throw jsonschema::schema_error("Could not resolve " + uri.string() + "\n");
            }
        };
    auto sch = jsonschema::make_schema(strict_tree_schema, resolver); 

    SECTION("instance with misspelled field")
    {
        std::string data_string = R"(
{
    "children": [ { "daat": 1 } ]
}
        )";

        try
        {
            // will throw schema_error if JSON Schema loading fails 

            // Data
            json data = json::parse(data_string);

            jsonschema::json_validator<json> validator(sch); 

            std::size_t error_count = 0;
            auto reporter = [&](const jsonschema::validation_output& /*o*/)
            {
                //std::cout << "  Failed: " << "eval_path: " << o.eval_path().to_string() << ", schema_path: " << o.schema_path().string() << ", " << o.instance_location() << ": " << o.message() << "\n";
                //for (const auto& err : o.nested_errors())
                //{
                    //std::cout << "  Nested error: " << err.instance_location() << ": " << err.message() << "\n";
                //}
                ++error_count;
            };
            validator.validate(data, reporter);
            CHECK(error_count > 0);
            //std::cout << "error_count: " << error_count << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }

    }
}

TEST_CASE("jsonschema $dynamicRef tests")
{
    std::string tree_schema_string = R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "$id": "https://example.com/tree",
    "$dynamicAnchor": "node",

    "type": "object",
    "properties": {
        "data": true,
        "children": {
            "type": "array",
            "items": {
                "$dynamicRef": "#node"
            }
        }
    }
}
    )";

    std::string strict_tree_schema_string = R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "$id": "https://example.com/strict-tree",
    "$dynamicAnchor": "node",

    "$ref": "tree",
    "unevaluatedProperties": false
}
    )";

    json tree_schema = json::parse(tree_schema_string);

    json strict_tree_schema = json::parse(strict_tree_schema_string);
    
    auto resolver = [tree_schema](const jsoncons::uri& uri)
        {
            //std::cout << "resolver: " << uri.string() << "\n";
            if (uri.string() == "https://example.com/tree")
            {
                return tree_schema;
            }
            else
            {
                throw jsonschema::schema_error("Could not resolve " + uri.string() + "\n");
            }
        };
    auto sch = jsonschema::make_schema(strict_tree_schema, resolver); 

    SECTION("instance with misspelled field")
    {
        std::string data_string = R"(
{
    "children": [ { "daat": 1 } ]
}
        )";

        try
        {
            // will throw schema_error if JSON Schema loading fails 

            // Data
            json data = json::parse(data_string);

            jsonschema::json_validator<json> validator(sch); 

            std::size_t error_count = 0;
            auto reporter = [&](const jsonschema::validation_output& /*o*/)
            {
                //std::cout << "  Failed: " << "eval_path: " << o.eval_path().to_string() << ", schema_path: " << o.schema_path().string() << ", " << o.instance_location() << ": " << o.message() << "\n";
                //for (const auto& err : o.nested_errors())
                //{
                    //std::cout << "  Nested error: " << err.instance_location() << ": " << err.message() << "\n";
                //}
                ++error_count;
            };
            validator.validate(data, reporter);
            CHECK(error_count > 0);
            //std::cout << "error_count: " << error_count << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }

    }
}


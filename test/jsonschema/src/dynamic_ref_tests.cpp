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

TEST_CASE("jsonschema $recursiveRef tests")
{
    {

        std::cout << "**********************888\n";

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
    auto reporter = [](const jsonschema::validation_message& msg) -> jsonschema::walk_result
    {
        auto patchInside = msg.patch();
        if (patchInside.has_value())
        {
            try {
                json* patchPtr = std::any_cast<json*>(patchInside.value());

                if (msg.message().find("Required property") != std::string::npos && msg.message().find("not found") != std::string::npos)
                {
                    json j;
                    j.try_emplace("op", "add");
                    j.try_emplace("path", msg.instance_location().string() + "/veggieLike");
                    j.try_emplace("value", false);
                    patchPtr->push_back(std::move(j));

                    //there could be "return jsonschema::walk_result::advance_no_error;" for saying to validator "its not error, go forward!"
                }

            } catch (const std::bad_any_cast& e) {
                std::cout << "cant cast patch-any:" << e.what() << "\n";
            }
        }
        std::cout << msg.instance_location().string() << ": " << msg.message() << "\n";
        return jsonschema::walk_result::advance;
    };
    compiled.validate(data, reporter, patch);

    std::cout << "\n(3) Validate outputting to a json decoder\n";
    jsoncons::json_decoder<json> decoder;
    compiled.validate(data, decoder);
    json output = decoder.get_result();
    std::cout << pretty_print(output) << "\n";

    std::cout << "PATCH:" << pretty_print(patch) << "\n";

    jsoncons::jsonpatch::apply_patch<json>(data, patch);

    compiled.validate(data, decoder);

    CHECK(decoder.is_valid());
}





    std::string tree_schema_str = R"(
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

    std::string strict_tree_schema_str = R"(
{
    "$schema": "https://json-schema.org/draft/2019-09/schema",
    "$id": "https://example.com/strict-tree",
    "$recursiveAnchor": true,

    "$ref": "tree",
    "unevaluatedProperties": false
}
    )";

    json tree_schema = json::parse(tree_schema_str);

    json strict_tree_schema = json::parse(strict_tree_schema_str);
    
    auto resolver = [tree_schema](const jsoncons::uri& uri)
        {
            //std::cout << "resolve: " << uri.string() << "\n";
            if (uri.string() == "https://example.com/tree")
            {
                return tree_schema;
            }
            else
            {
                return json::null();
            }
        };
    jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(strict_tree_schema, resolver); 

    SECTION("instance with misspelled field")
    {
        std::string data_str = R"(
{
    "children": [ { "daat": 1 } ]
}
        )";

        try
        {
            // will throw schema_error if JSON Schema compilation fails 

            // Data
            json data = json::parse(data_str);

            std::size_t error_count = 0;
            auto reporter = [&](const jsonschema::validation_message& /*msg*/) -> jsonschema::walk_result
            {
                //std::cout << "  Failed: " << "eval_path: " << msg.eval_path().string() << ", schema_location: " << msg.schema_location().string() << ", " << msg.instance_location().string() << ": " << msg.message() << "\n";
                //for (const auto& err : msg.nested_errors())
                //{
                    //std::cout << "  Nested error: " << err.instance_location().string() << ": " << err.message() << "\n";
                //}
                ++error_count;
                return jsonschema::walk_result::advance;
            };
            compiled.validate(data, reporter);
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
    std::string tree_schema_str = R"(
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

    std::string strict_tree_schema_str = R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "$id": "https://example.com/strict-tree",
    "$dynamicAnchor": "node",

    "$ref": "tree",
    "unevaluatedProperties": false
}
    )";

    json tree_schema = json::parse(tree_schema_str);

    json strict_tree_schema = json::parse(strict_tree_schema_str);
    
    auto resolver = [tree_schema](const jsoncons::uri& uri)
        {
            //std::cout << "resolve: " << uri.string() << "\n";
            if (uri.string() == "https://example.com/tree")
            {
                return tree_schema;
            }
            else
            {
                return json::null();
            }
        };
    jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(strict_tree_schema, resolver); 

    SECTION("instance with misspelled field")
    {
        std::string data_str = R"(
{
    "children": [ { "daat": 1 } ]
}
        )";

        try
        {
            // will throw schema_error if JSON Schema compilation fails 

            // Data
            json data = json::parse(data_str);

            std::size_t error_count = 0;
            auto reporter = [&](const jsonschema::validation_message& /*msg*/) -> jsonschema::walk_result
                {
                    //std::cout << "  Failed: " << "eval_path: " << msg.eval_path().string() << ", schema_location: " << msg.schema_location().string() << ", " << msg.instance_location().string() << ": " << msg.message() << "\n";
                    //for (const auto& err : msg.nested_errors())
                    //{
                        //std::cout << "  Nested error: " << err.instance_location().string() << ": " << err.message() << "\n";
                    //}
                    ++error_count;
                    return jsonschema::walk_result::advance;
                };
            compiled.validate(data, reporter);
            CHECK(error_count > 0);
            //std::cout << "error_count: " << error_count << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }

    }
}

TEST_CASE("jsonschema $dynamicRef tests 2")
{
    std::string schema_str = R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "$id": "https://test.json-schema.org/dynamic-ref-leaving-dynamic-scope/main",
    "if": {
        "$id": "first_scope",
        "$defs": {
            "thingy": {
                "$comment": "this is first_scope#thingy",
                "$dynamicAnchor": "thingy",
                "type": "number"
            }
        }
    },
    "then": {
        "$id": "second_scope",
        "$ref": "start",
        "$defs": {
            "thingy": {
                "$comment": "this is second_scope#thingy, the final destination of the $dynamicRef",
                "$dynamicAnchor": "thingy",
                "type": "null"
            }
        }
    },
    "$defs": {
        "start": {
            "$comment": "this is the landing spot from $ref",
            "$id": "start",
            "$dynamicRef": "inner_scope#thingy"
        },
        "thingy": {
            "$comment": "this is the first stop for the $dynamicRef",
            "$id": "inner_scope",
            "$dynamicAnchor": "thingy",
            "type": "string"
        }
    }
}
    )";

    json schema = json::parse(schema_str);

    jsonschema::json_schema<json> compiled = jsonschema::make_json_schema(schema); 

    SECTION("/then/$defs/thingy is the final stop for the $dynamicRef")
    {
        try
        {
            // will throw schema_error if JSON Schema compilation fails 

            // Data
            json data(jsoncons::null_type{});

            std::size_t error_count = 0;
            auto reporter = [&](const jsonschema::validation_message& msg) -> jsonschema::walk_result
            {
                std::cout << "  Failed: " << "eval_path: " << msg.eval_path().string() << ", schema_location: " << msg.schema_location().string() << ", " << msg.instance_location().string() << ": " << msg.message() << "\n";
                for (const auto& err : msg.details())
                {
                    std::cout << "  Nested error: " << err.instance_location().string() << ": " << err.message() << "\n";
                }
                ++error_count;
                return jsonschema::walk_result::advance;
            };
            compiled.validate(data, reporter);
            CHECK(0 == error_count);
            //std::cout << "error_count: " << error_count << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
}


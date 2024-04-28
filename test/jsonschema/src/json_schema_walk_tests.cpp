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
using jsoncons::ojson;
namespace jsonschema = jsoncons::jsonschema;
#if 0
TEST_CASE("jsonschema walk tests")
{
    std::string schema_string = R"(
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
      "items": {
        "$ref": "#/$defs/veggie"
      }
    }
  },
  "$defs": {
    "veggie": {
      "type": "object",
      "required": [
        "veggieName",
        "veggieLike"
      ],
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

    ojson schema = ojson::parse(schema_string);
    jsonschema::json_schema<ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

    SECTION("walk")
    {
        std::string data_string = R"(
{
  "fruits": [
    "apple",
    "orange",
    "pear"
  ],
  "vegetables": [
    {
      "veggieName": "potato",
      "veggieLike": true
    },
    {
      "veggieName": "broccoli",
      "veggieLike": false
    }
  ]
}
        )";

        // Data
        ojson data = ojson::parse(data_string);
 
        ojson expected = ojson::parse(R"(      
{
    "/fruits/0": "string",
    "/fruits/1": "string",
    "/fruits/2": "string",
    "/fruits": "array",
    "/vegetables/0/veggieName": "string",
    "/vegetables/0/veggieLike": "boolean",
    "/vegetables/0": "object",
    "/vegetables/1/veggieName": "string",
    "/vegetables/1/veggieLike": "boolean",
    "/vegetables/1": "object",
    "/vegetables": "array",
    "": "object"
}
        )");

        ojson result(jsoncons::json_object_arg);
        auto reporter = [&](const std::string& keyword,
            const ojson& schema, const jsoncons::uri& /*schema_location*/,
            const ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location)
        {
            if (keyword == "type" && schema.is_object())
            {
                auto it = schema.find("type");
                if (it != schema.object_range().end())
                {
                    result.try_emplace(instance_location.string(), it->value());
                    //std::cout << instance_location.string() << ": " << it->value() << "\n";
                }
            }
        };
        compiled.walk(data, reporter);
        CHECK(expected == result);
        //std::cout << pretty_print(result) << "\n";
    }
} 
#endif
#if 0
TEST_CASE("jsonschema with $dynamicRef walk test")
{
    std::string schema_string = R"(
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

    ojson schema = ojson::parse(schema_string);
    jsonschema::json_schema<ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

    SECTION("walk")
    {
        std::string data_string = R"(null)";

        try
        {
            // Data
            ojson data = ojson::parse(data_string);

            auto reporter = [&](const std::string& keyword,
                const ojson& schema, const jsoncons::uri& /*schema_location*/,
                const ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location)
            {
                std::cout << keyword << "\n";
                if (keyword == "type" && schema.is_object())
                {
                    auto it = schema.find("type");
                    if (it != schema.object_range().end())
                    {
                        std::cout << instance_location.string() << ": " << it->value() << "\n";
                    }
                }
            };
            compiled.walk(data, reporter);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
}
#endif

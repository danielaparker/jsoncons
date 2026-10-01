// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons/json.hpp>

#include <jsoncons/utility/byte_string.hpp>

#include <catch/catch.hpp>
#include <fstream>
#include <iostream>
#include <regex>

using jsoncons::json;
using jsoncons::ojson;
namespace jsonschema = jsoncons::jsonschema;

TEST_CASE("jsonschema walk tests")
{
    std::string schema_str = R"(
{
  "$id": "https://example.com/arrays.schema.json",
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "description": "Arrays of strings and objects",
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

    jsoncons::ojson schema = jsoncons::ojson::parse(schema_str);
    jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

    SECTION("walk")
    {
        std::string data_str = R"(
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
        jsoncons::ojson data = jsoncons::ojson::parse(data_str);
 
        jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
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

        jsoncons::ojson result(jsoncons::json_object_arg);
        auto reporter = [&](const std::string& keyword,
            const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
            const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
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
            return jsonschema::walk_state::advance;
        };
        schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
        compiled.walk(data, reporter);
        CHECK(expected == result);
        //std::cout << pretty_print(result) << "\n";
    }
} 

TEST_CASE("jsonschema with $dynamicRef walk test")
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

    jsoncons::ojson schema = jsoncons::ojson::parse(schema_str);
    jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

    SECTION("walk")
    {
        std::string data_str = R"(null)";

        try
        {
            // Data
            jsoncons::ojson data = jsoncons::ojson::parse(data_str);

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{
    "" : "null"
}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const std::string& keyword,
                const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
                const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
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
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
}

TEST_CASE("jsonschema walk keyword test")
{
    SECTION("prefixItems")
    {
        try
        {
            jsoncons::ojson schema = jsoncons::ojson::parse(R"(
    {
        "$schema": "https://json-schema.org/draft/2020-12/schema",
        "prefixItems": [
            {"type": "integer"},
            {"type": "string"}
        ]
    }        
            )");
            jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

            jsoncons::ojson data = jsoncons::ojson::parse(R"(
[ 1, "foo" ]
            )");

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{
    "/0": "integer",
    "/1": "string"
}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const std::string& keyword,
                const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
                const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
            {
                if (keyword == "type")
                {
                    REQUIRE(schema.is_object());
                    auto it = schema.find("type");
                    if (it != schema.object_range().end())
                    {
                        result.try_emplace(instance_location.string(), it->value());
                    }
                }
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
    SECTION("dependentRequired")
    {
        try
        {
            jsoncons::ojson schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "dependentRequired": {"bar": ["foo"]}
}
            )");
            jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

            jsoncons::ojson data = jsoncons::ojson::parse(R"(
{"foo": 1, "bar": 2}
            )");

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{"":{"bar":["foo"]}}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const std::string& keyword,
                const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
                const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
            {
                //std::cout << "keyword: " << keyword << "\n";
                if (keyword == "dependentRequired")
                {
                    REQUIRE(schema.is_object());
                    auto it = schema.find("dependentRequired");
                    if (it != schema.object_range().end())
                    {
                        result.try_emplace(instance_location.string(), it->value());
                    }
                }
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
            //std::cout << result << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
    SECTION("dependentSchemas")
    {
        try
        {
            jsoncons::ojson schema = jsoncons::ojson::parse(R"(
{
            "$schema": "https://json-schema.org/draft/2020-12/schema",
            "dependentSchemas": {
                "bar": {
                    "properties": {
                        "foo": {"type": "integer"},
                        "bar": {"type": "integer"}
                    }
                }
            }
        }
            )");
            jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

            jsoncons::ojson data = jsoncons::ojson::parse(R"(
{"foo": 1, "bar": 2}
            )");

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{"/bar/foo":"integer","/bar/bar":"integer"}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const std::string& keyword,
                const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
                const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
            {
                //std::cout << "keyword: " << keyword << "\n";
                if (keyword == "type")
                {
                    REQUIRE(schema.is_object());
                    auto it = schema.find("type");
                    if (it != schema.object_range().end())
                    {
                        result.try_emplace(instance_location.string(), it->value());
                    }
                }
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
            //std::cout << result << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
    SECTION("propertyNames")
    {
        try
        {
            jsoncons::ojson schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "propertyNames": {"maxLength": 3}
}
            )");
            jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

            jsoncons::ojson data = jsoncons::ojson::parse(R"(
{
    "f": {},
    "foo": {}
}
            )");

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{"/f":3,"/foo":3}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const std::string& keyword,
                const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
                const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
            {
                //std::cout << "keyword: " << keyword << "\n";
                if (keyword == "maxLength")
                {
                    REQUIRE(schema.is_object());
                    auto it = schema.find("maxLength");
                    if (it != schema.object_range().end())
                    {
                        result.try_emplace(instance_location.string(), it->value());
                    }
                }
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
            //std::cout << result << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
    SECTION("contains")
    {
        try
        {
            jsoncons::ojson schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "contains": {"minimum": 5}
}
            )");
            jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

            jsoncons::ojson data = jsoncons::ojson::parse(R"(
[3, 4, 5]
            )");

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{"/0":5,"/1":5,"/2":5}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const std::string& keyword,
                const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
                const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
            {
                //std::cout << "keyword: " << keyword << "\n";
                if (keyword == "minimum")
                {
                    REQUIRE(schema.is_object());
                    auto it = schema.find("minimum");
                    if (it != schema.object_range().end())
                    {
                        result.try_emplace(instance_location.string(), it->value());
                    }
                }
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
            //std::cout << result << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
    SECTION("patternProperties")
    {
        try
        {
            jsoncons::ojson schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "patternProperties": {
        "f.*o": {"type": "integer"}
    }
}
            )");
            jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

            jsoncons::ojson data = jsoncons::ojson::parse(R"(
{"foo": 1, "foooooo" : 2}
            )");

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{"/foo":"integer","/foooooo":"integer"}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const std::string& keyword,
                const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
                const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
            {
                //std::cout << "keyword: " << keyword << "\n";
                if (keyword == "type")
                {
                    REQUIRE(schema.is_object());
                    auto it = schema.find("type");
                    if (it != schema.object_range().end())
                    {
                        result.try_emplace(instance_location.string(), it->value());
                    }
                }
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
            //std::cout << result << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
    SECTION("additionalProperties")
    {
        try
        {
            jsoncons::ojson schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "additionalProperties": {"type": "boolean"}
}
            )");
            jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

            jsoncons::ojson data = jsoncons::ojson::parse(R"(
{"foo" : true}
            )");

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{"/foo":"boolean"}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const std::string& keyword,
                const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
                const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
            {
                //std::cout << "keyword: " << keyword << "\n";
                if (keyword == "type")
                {
                    REQUIRE(schema.is_object());
                    auto it = schema.find("type");
                    if (it != schema.object_range().end())
                    {
                        result.try_emplace(instance_location.string(), it->value());
                    }
                }
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
            //std::cout << result << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
    SECTION("additionalItems")
    {
        try
        {
            jsoncons::ojson schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2019-09/schema",
    "items": [{}],
    "additionalItems": {"type": "integer"}
}
            )");
            jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

            jsoncons::ojson data = jsoncons::ojson::parse(R"(
[ null, 2, 3, 4 ]
            )");

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{"/1":"integer","/2":"integer","/3":"integer"}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const std::string& keyword,
                const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
                const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
            {
                //std::cout << "keyword: " << keyword << "\n";
                if (keyword == "type")
                {
                    REQUIRE(schema.is_object());
                    auto it = schema.find("type");
                    if (it != schema.object_range().end())
                    {
                        result.try_emplace(instance_location.string(), it->value());
                    }
                }
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
            //std::cout << result << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }    
    SECTION("oneOf")
    {
        try
        {
            jsoncons::ojson schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "oneOf": [
        {
            "type": "integer"
        },
        {
            "minimum": 2
        }
    ]
}
                    )");
            jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

            jsoncons::ojson data = jsoncons::ojson::parse(R"(
1
            )");

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{"":"integer"}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const std::string& keyword,
                const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
                const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
            {
                //std::cout << "keyword: " << keyword << "\n";
                if (keyword == "type")
                {
                    REQUIRE(schema.is_object());
                    auto it = schema.find("type");
                    if (it != schema.object_range().end())
                    {
                        result.try_emplace(instance_location.string(), it->value());
                    }
                }
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
            //std::cout << result << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
    SECTION("anyOf")
    {
        try
        {
            jsoncons::ojson schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "anyOf": [
        {
            "type": "integer"
        },
        {
            "minimum": 2
        }
    ]
}
                    )");
            jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

            jsoncons::ojson data = jsoncons::ojson::parse(R"(
1
            )");

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{"":"integer"}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const std::string& keyword,
                const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
                const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
            {
                //std::cout << "keyword: " << keyword << "\n";
                if (keyword == "type")
                {
                    REQUIRE(schema.is_object());
                    auto it = schema.find("type");
                    if (it != schema.object_range().end())
                    {
                        result.try_emplace(instance_location.string(), it->value());
                    }
                }
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
            //std::cout << result << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
    SECTION("allOf")
    {
        try
        {
            jsoncons::ojson schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "anyOf": [
        {
            "type": "integer"
        },
        {
            "minimum": 2
        }
    ]
}
                    )");
            jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

            jsoncons::ojson data = jsoncons::ojson::parse(R"(
1
            )");

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{"":"integer"}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const std::string& keyword,
                const jsoncons::ojson& schema, const jsoncons::uri& /*schema_location*/,
                const jsoncons::ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location) -> jsonschema::walk_state
            {
                //std::cout << "keyword: " << keyword << "\n";
                if (keyword == "type")
                {
                    REQUIRE(schema.is_object());
                    auto it = schema.find("type");
                    if (it != schema.object_range().end())
                    {
                        result.try_emplace(instance_location.string(), it->value());
                    }
                }
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
            //std::cout << result << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
    SECTION("allOf 2")
    {
        try
        {
            jsoncons::ojson schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "anyOf": [
        {
            "type": "integer"
        },
        {
            "minimum": 2
        }
    ]
}
                    )");
            jsonschema::json_schema<jsoncons::ojson> compiled = jsonschema::make_json_schema(std::move(schema)); 

            jsoncons::ojson data = jsoncons::ojson::parse(R"(
1
            )");

            jsoncons::ojson expected = jsoncons::ojson::parse(R"(      
{"":"integer"}
            )");

            jsoncons::ojson result(jsoncons::json_object_arg);
            auto reporter = [&](const jsonschema::schema_property<jsoncons::ojson>& property,
                const jsoncons::ojson& /*instance*/, 
                const jsoncons::jsonpointer::json_pointer& instance_location, 
                jsoncons::optional<jsoncons::ojson>& /*patch*/) -> jsonschema::walk_state
            {
                //std::cout << "keyword: " << keyword << "\n";
                if (property.keyword() == "type")
                {
                    REQUIRE(property.subschemas().is_object());
                    auto it = property.subschemas().find("type");
                    if (it != property.subschemas().object_range().end())
                    {
                        result.try_emplace(instance_location.string(), it->value());
                    }
                }
                return jsonschema::walk_state::advance;
            };
            schema = jsoncons::ojson::null(); // walk mustn't try to access memory in original schema
            compiled.walk(data, reporter);
            CHECK(expected == result);
            //std::cout << result << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }
    }
}


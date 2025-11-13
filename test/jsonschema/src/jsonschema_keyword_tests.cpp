// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/utility/byte_string.hpp>

#include <fstream>
#include <iostream>
#include <regex>
#include <catch/catch.hpp>

using jsoncons::json;
namespace jsonschema = jsoncons::jsonschema;

TEST_CASE("jsonschema keyword tests")
{
    SECTION("patternProperies")
    {
        auto main_schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "$id": "/schema.json",
    "type": "object",
    "properties": {
        "$schema": {
            "type": "string"
        },
        "version": {
            "description": "Version number of JSON content",
            "type": "string"
        },
        "features": {
            "description": "List of features supported by the device",
            "type": "object",
            "patternProperties": {
                "Firmware Update": {
                    "$ref": "/firmware_update.json#/$defs/firmware_update"
                }
            },
            "required": [
                "Firmware Update"
            ]
        }
    },
    "required": [
        "$schema",
        "version",
        "features"
    ],
    "additionalProperties": false
}
        )");

            auto firmware_update_schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "$id": "/firmware_update.json",
    "type": "object",
    "$defs": {
        "firmware_update": {
            "allOf": [
                {
                    "description": "Firmware Update Feature description",
                    "properties": {
                        "feature_type": {
                            "const": "update"
                        },
                        "version": {
                            "enum": [
                                "V1",
                                "V2"
                            ]
                        }
                    }
                },
                {
                    "$ref": "/basic_types.json#/$defs/feature"
                }
            ]
        }
    }
}
        )");

auto basic_types_schema = jsoncons::ojson::parse(R"(
{
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "$id": "/basic_types.json",
    "type": "object",
    "$defs": {
        "parameter": {
            "type": "object",
            "description": "A configuration parameter",
            "properties": {
                "description": {
                    "type": "string"
                },
                "name": {
                    "description": "The name of the parameter in the config provider",
                    "type": "string"
                }
            },
            "required": [
                "name"
            ],
            "additionalProperties": true
        },
        "const_parameter": {
            "type": "object",
            "description": "A configuration parameter with a constant value",
            "properties": {
                "description": {
                    "type": "string"
                },
                "from": {
                    "const": "const"
                },
                "value": {
                    "description": "The constant value of the parameter",
                    "type": [
                        "string",
                        "number",
                        "boolean",
                        "array"
                    ]
                }
            },
            "required": [
                "from",
                "value"
            ]
        },
        "parameter_list": {
            "patternProperties": {
                "[a-zA-Z_]{1,}": {
                    "oneOf": [
                        {
                            "$ref": "#/$defs/parameter"
                        },
                        {
                            "$ref": "#/$defs/const_parameter"
                        }
                    ]
                }
            }
        },
        "feature": {
            "description": "A generic feature description",
            "properties": {
                "feature_type": {
                    "type": "string"
                },
                "version": {
                    "type": "string"
                },
                "description": {
                    "type": "string"
                },
                "config": {
                    "$ref": "#/$defs/parameter_list"
                }
            },
            "required": [
                "feature_type",
                "version"
            ]
        }
    }
}
        )");

std::string json_str = R"(
{
          "$schema": "/schema.json",
          "version": "v0.1",
          "features": {
              "Firmware Update": {
                  "feature_type": "update",
                  "version": "V2",
                  "config": {
                      "firmware_update": {
                          "from": "const",
                          "value": "xyz"
                      }
                  }
              }
          }
}
        )";

        auto resolver = [&](const jsoncons::uri& p_uri)
          {
              std::string my_file_path = p_uri.path();
              if (my_file_path == "/basic_types.json")
              {
                  return basic_types_schema;
              }
              if (my_file_path == "/firmware_update.json")
              {
                  return firmware_update_schema;
              }
              return jsoncons::ojson::null();
          };

        auto compiled = jsoncons::jsonschema::make_json_schema(main_schema, resolver);
        CHECK(compiled.is_valid(jsoncons::ojson::parse(json_str)));
    }

    SECTION("additionalProperties")
    {
        std::vector<jsoncons::jsonschema::validation_message> result;

        auto reporter =
        [&result](const jsoncons::jsonschema::validation_message& message) -> jsoncons::jsonschema::walk_result
            {
                /*std::cout << "+++++++++++++++++" << std::endl;
                std::cout << "message: " << message.message() << std::endl;
                std::cout << "instance location: " << message.instance_location().string() << std::endl;
                std::cout << "schema location: " << message.schema_location().string() << std::endl;
                std::cout << "keyword: " << message.keyword() << std::endl;
                std::cout << "evaluation path: " << message.eval_path().string() << std::endl;
                std::cout << "+++++++++++++++++" << std::endl;
                std::cout << std::endl;*/

                result.push_back(message);
                return jsoncons::jsonschema::walk_result::advance;
            };

        std::string schemaString = R"(
            {
                "$schema" : "https://json-schema.org/draft/2020-12/schema",
                "type" : "object",
                "additionalProperties" : false,
                "required" : [ "s1" ],
                "properties" : {
                    "s1" : {"type" : "string"},
                    "n2" : {"type" : "integer"}
                }
            }
          )";

        std::string dataString = R"(
            {
                "s1": "1",
                "n2": 2,
                "x4": 4,
                "x5": 5
            }
          )";

        auto schema = jsoncons::jsonschema::make_json_schema(jsoncons::json::parse(schemaString));
        auto parsed = jsoncons::json::parse(dataString);

        schema.validate(parsed, reporter);
        REQUIRE(2U == result.size());
        CHECK(std::string("additionalProperties") == result[0].keyword());
        CHECK(std::string("/additionalProperties/x4") == result[0].eval_path().string());
        CHECK(std::string("/x4") == result[0].instance_location().string());
        CHECK(std::string("additionalProperties") == result[1].keyword());
        CHECK(std::string("/additionalProperties/x5") == result[1].eval_path().string());
        CHECK(std::string("/x5") == result[1].instance_location().string());
    }
}

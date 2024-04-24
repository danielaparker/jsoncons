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
    jsonschema::json_schema<ojson> compiled = jsonschema::make_json_schema(schema); 

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

        try
        {
            // will throw schema_error if JSON Schema compilation fails 

            // Data
            ojson data = ojson::parse(data_string);

            auto reporter = [&](const std::string& keyword,
                const ojson& schema, const jsoncons::uri& /*schema_location*/,
                const ojson& /*instance*/, const jsoncons::jsonpointer::json_pointer& instance_location)
            {
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


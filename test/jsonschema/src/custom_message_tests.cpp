// Copyright 2013-2025 Daniel Parker
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

TEST_CASE("jsonschema custom message tests")
{
    std::string schema_str = R"(
{
  "type": "object",
  "properties": {
    "foo": {
      "type": "array",
      "maxItems": 3,
      "items" : {
        "type" : "number"
      }/*,
      "message" : {
        "maxItems" : "Custom Message: Maximum 3 numbers can be given in 'foo'",
        "type" : "Custom Message: Only numbers are supported in 'foo'"
      }*/
    },
    "bar": {
      "type": "string"
    }
  }
}
    )";
    
    auto options = jsonschema::evaluation_options{}
        .error_message_keyword("message");

    auto schema = jsoncons::json::parse(schema_str);    
    auto compiled = jsonschema::make_json_schema<json>(schema, options); 
    
    SECTION("test 1")
    {
        std::string data_str = R"(
{
  "foo": [],
  "bar": "Bar 1"
}
        )";

        try
        {
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


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
     },
     "message" : {
       "maxItems" : "Custom Message: Maximum 3 numbers can be given in 'foo'",
       "type" : "Custom Message: Only numbers are supported in 'foo'"
     }
   },
   "bar": {
     "type": "string"
   }
 },
 "message": {
   "type" : "Custom Message: Invalid type provided"
 }
}
    )";
    
    auto options = jsonschema::evaluation_options{}
        .custom_message_keyword("message");

    auto schema = jsoncons::json::parse(schema_str);    
    auto compiled = jsonschema::make_json_schema<json>(schema, options); 
    
    SECTION("test 1")
    {
        std::string data_str = R"(
{
    "foo": [1, 2, 3],
    "bar": 123
}        
        )";

        json data = json::parse(data_str);

        std::vector<std::string> messages;
        auto reporter = [&](const jsonschema::validation_message& msg) -> jsonschema::walk_result
        {
            messages.push_back(msg.message());
            return jsonschema::walk_result::advance;
        };
        compiled.validate(data, reporter);

        REQUIRE(messages.size() == 1);
        CHECK("Custom Message: Invalid type provided" == messages[0]);
    }

    SECTION("test 2")
    {
        std::string data_str = R"(
{
    "foo": [1, 2, "text"],
    "bar": "Bar 1"
}        
        )";

        json data = json::parse(data_str);

        std::vector<std::string> messages;
        auto reporter = [&](const jsonschema::validation_message& msg) -> jsonschema::walk_result
        {
            messages.push_back(msg.message());
            return jsonschema::walk_result::advance;
        };
        compiled.validate(data, reporter);

        REQUIRE(messages.size() == 1);
        CHECK("Custom Message: Only numbers are supported in 'foo'" == messages[0]);
    }

    SECTION("test 3")
    {
        std::string data_str = R"(
{
    "foo": [1, 2, "text"],
    "bar": 123
}        
        )";

        json data = json::parse(data_str);

        std::vector<std::string> messages;
        auto reporter = [&](const jsonschema::validation_message& msg) -> jsonschema::walk_result
        {
            messages.push_back(msg.message());
            return jsonschema::walk_result::advance;
        };
        compiled.validate(data, reporter);
        
        REQUIRE(messages.size() == 2);
        CHECK("Custom Message: Invalid type provided" == messages[0]);
        CHECK("Custom Message: Only numbers are supported in 'foo'" == messages[1]);
    }

    SECTION("test 4")
    {
        std::string data_str = R"(
{
            "foo": [1, 2, "text", 3],
            "bar": 123
}        
        )";

        json data = json::parse(data_str);

        std::vector<std::string> messages;
        auto reporter = [&](const jsonschema::validation_message& msg) -> jsonschema::walk_result
        {
            messages.push_back(msg.message());
            return jsonschema::walk_result::advance;
        };
        compiled.validate(data, reporter);

        REQUIRE(messages.size() == 3);
        CHECK("Custom Message: Invalid type provided" == messages[0]);
        CHECK("Custom Message: Only numbers are supported in 'foo'" == messages[1]);
        CHECK("Custom Message: Maximum 3 numbers can be given in 'foo'" == messages[2]);
    }
}


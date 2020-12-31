// Copyright 2020 Daniel Parker
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

TEST_CASE("jsonschema defaults tests")
{
    SECTION("Basic")
    {
        json schema = json::parse(R"(
{
    "properties": {
        "bar": {
            "type": "string",
            "minLength": 4,
            "default": "bad"
        }
    }
}
    )");

        try
        {
            // Data
            json data = json::parse("{}");

            // will throw schema_error if JSON Schema loading fails 
            auto sch = jsonschema::make_schema(schema); 

            jsonschema::json_validator<json> validator(sch); 

            // will throw a validation_error when a schema violation happens 
            json patch = validator.validate(data); 

            jsonpatch::apply_patch(data, patch);

            json expected = json::parse(R"(
            {"bar":"bad"}
            )");

            CHECK(data == expected);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }

    }

}


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

TEST_CASE("jsonschema defaults tests")
{
    SECTION("Basic")
    {
        jsoncons::json schema = jsoncons::json::parse(R"(
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
            jsoncons::json data = jsoncons::json::parse("{}");

            // will throw schema_error if JSON Schema compilation fails 
            jsonschema::json_schema<jsoncons::json> compiled = jsonschema::make_json_schema(schema); 

            // will throw a validation_error when a schema violation happens 
            jsoncons::json patch;
            compiled.validate(data, patch); 

            jsonpatch::apply_patch(data, patch);

            jsoncons::json expected = jsoncons::json::parse(R"(
            {"bar":"bad"}
 )");

            CHECK(expected == data);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << "\n";
        }

    }
}


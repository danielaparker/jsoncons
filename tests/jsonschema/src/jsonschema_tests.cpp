// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#include <catch/catch.hpp>
#include <fstream>
#include <iostream>
#include <regex>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>
#include <jsoncons/byte_string.hpp>

using jsoncons::json;
namespace jsonschema = jsoncons::jsonschema;

namespace {

    json resolver(const jsoncons::uri& uri)
    {
        if (uri.path() == "/draft-07/schema") 
        {
            return jsoncons::jsonschema::json_schema_draft7<json>::get_schema();
        }
        else
        {
            //std::cout << uri.string() << ", " << uri.path() << "\n";
            std::string pathname = "./jsonschema/input/remotes";
            pathname += std::string(uri.path());

            std::fstream is(pathname.c_str());
            if (!is)
                throw jsonschema::schema_error("Could not open " + std::string(uri.base()) + " for schema loading\n");

            return json::parse(is);
        }
    }

    void jsonschema_tests(const std::string& fpath)
    {
        std::fstream is(fpath);
        REQUIRE(is);

        json tests = json::parse(is); 

        for (const auto& test_group : tests.array_range()) 
        {
            auto schema = jsonschema::make_schema(test_group.at("schema"), resolver);
            jsonschema::json_validator<json> validator(schema);

            for (const auto& test_case : test_group["tests"].array_range()) 
            {
                auto reporter = [&test_case](const jsonschema::validation_output& o)
                {
                    CHECK_FALSE(test_case["valid"].as<bool>());
                    if (test_case["valid"].as<bool>())
                    {
                        std::cout << "  Test case: " << test_case["description"] << "\n";
                        std::cout << "  Failed: " << o.instance_location() << ": " << o.message() << "\n";
                    }
                    else
                    {
                        //std::cout << o.what() << "\n";
                    }
                };
                validator.validate(test_case.at("data"), reporter);
            }
        }
    }
}
 
TEST_CASE("jsonschema-tests")
{
    SECTION("compliance")
    {
        jsonschema_tests("./jsonschema/input/compliance/draft7/additionalItems.json");
#ifdef JSONCONS_HAS_STD_REGEX
        jsonschema_tests("./jsonschema/input/compliance/draft7/additionalProperties.json");
#endif
        jsonschema_tests("./jsonschema/input/compliance/draft7/allOf.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/anyOf.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/boolean_schema.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/const.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/contains.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/default.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/definitions.json"); 

        jsonschema_tests("./jsonschema/input/compliance/draft7/dependencies.json");

        jsonschema_tests("./jsonschema/input/compliance/draft7/enum.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/exclusiveMaximum.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/exclusiveMinimum.json");

#ifdef JSONCONS_HAS_STD_REGEX
        jsonschema_tests("./jsonschema/input/compliance/draft7/format.json");
#endif
        jsonschema_tests("./jsonschema/input/compliance/draft7/if-then-else.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/items.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/maximum.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/maxItems.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/maxLength.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/maxProperties.json");

        jsonschema_tests("./jsonschema/input/compliance/draft7/minimum.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/minItems.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/minLength.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/minProperties.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/multipleOf.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/not.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/oneOf.json");
#ifdef JSONCONS_HAS_STD_REGEX
        jsonschema_tests("./jsonschema/input/compliance/draft7/pattern.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/patternProperties.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/properties.json");
#endif

        jsonschema_tests("./jsonschema/input/compliance/draft7/propertyNames.json");

        jsonschema_tests("./jsonschema/input/compliance/draft7/ref.json"); // *
        jsonschema_tests("./jsonschema/input/compliance/draft7/refRemote.json");

        jsonschema_tests("./jsonschema/input/compliance/draft7/required.json");

        jsonschema_tests("./jsonschema/input/compliance/draft7/type.json");

        jsonschema_tests("./jsonschema/input/compliance/draft7/uniqueItems.json"); 

        // format tests
        jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/date.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/date-time.json");
        //jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/ecmascript-regex.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/email.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/hostname.json");
        //jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/idn-email.json");
        //jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/idn-hostname.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/ipv4.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/ipv6.json");
        //jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/iri.json");
        //jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/iri-reference.json");
        //jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/json-pointer.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/regex.json");
        //jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/relative-json-pointer.json");
        jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/time.json");
        //jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/uri.json");
        //jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/uri-reference.json");
        //jsonschema_tests("./jsonschema/input/compliance/draft7/optional/format/uri-template.json");

        jsonschema_tests("./jsonschema/input/compliance/draft7/optional/content.json");
    }
}

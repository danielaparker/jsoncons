// Copyright 2013-2023 Daniel Parker
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
            return jsoncons::jsonschema::draft201909::schema_draft201909<json>::get_schema();
        }
        else
        {
            //std::cout << uri.string() << ", " << uri.path() << "\n";
            std::string pathname = "./jsonschema/JSON-Schema-Test-Suite/remotes";
            pathname += std::string(uri.path());

            std::fstream is(pathname.c_str());
            if (!is)
                throw jsonschema::schema_error("Could not open " + pathname + " for schema loading\n");

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
            try
            {
                auto schema = jsonschema::make_schema(test_group.at("schema"), resolver);
                jsonschema::json_validator<json> validator(schema);

                for (const auto& test_case : test_group["tests"].array_range()) 
                {
                    auto reporter = [&](const jsonschema::validation_output& o)
                    {
                        CHECK_FALSE(test_case["valid"].as<bool>());
                        if (test_case["valid"].as<bool>())
                        {
                            std::cout << "  File: " << fpath << "\n";
                            std::cout << "  Test case: " << test_case["description"] << "\n";
                            std::cout << "  Failed: " << o.instance_location() << ": " << o.message() << "\n";
                            for (const auto& err : o.nested_errors())
                            {
                                std::cout << "  Nested error: " << err.instance_location() << ": " << err.message() << "\n";
                            }
                        }
                        else
                        {
                            //std::cout << o.what() << "\n";
                        }
                    };
                    validator.validate(test_case.at("data"), reporter);
                }
            }
            catch (const std::exception& e)
            {
                std::cout << "  File: " << fpath << "\n";
                std::cout << e.what() << "\n\n";
                CHECK(false);
            }
        }
    }
}
 
TEST_CASE("jsonschema-tests")
{
    SECTION("issues")
    {
        //jsonschema_tests("./jsonschema/issues/draft2019-09/issue1.json");
        //jsonschema_tests("./jsonschema/issues/draft2019-09/issue2.json");
        //jsonschema_tests("./jsonschema/issues/draft2019-09/issue3.json");
        //jsonschema_tests("./jsonschema/issues/draft2019-09/issue4.json");
        //jsonschema_tests("./jsonschema/issues/draft2019-09/issue5.json");
        //jsonschema_tests("./jsonschema/issues/draft2019-09/issue6.json");
    }
    SECTION("tests")
    {
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/anchor.json");
#if 0
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/additionalItems.json");
#ifdef JSONCONS_HAS_STD_REGEX
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/additionalProperties.json");
#endif
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/allOf.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/anyOf.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/boolean_schema.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/const.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/contains.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/default.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/definitions.json"); 

        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/dependencies.json");

        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/enum.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/exclusiveMaximum.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/exclusiveMinimum.json");

#ifdef JSONCONS_HAS_STD_REGEX
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/format.json");
#endif
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/if-then-else.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/items.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/maximum.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/maxItems.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/maxLength.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/maxProperties.json");

        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/minimum.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/minItems.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/minLength.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/minProperties.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/multipleOf.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/not.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/oneOf.json");
#ifdef JSONCONS_HAS_STD_REGEX
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/pattern.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/patternProperties.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/properties.json");
#endif
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/propertyNames.json");

        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/ref.json"); // *

        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/refRemote.json");

        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/required.json");

        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/type.json");

        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/uniqueItems.json"); 
        // format tests
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/date.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/date-time.json");
        //jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/ecmascript-regex.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/email.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/hostname.json");
        //jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/idn-email.json");
        //jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/idn-hostname.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/ipv4.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/ipv6.json");
        //jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/iri.json");
        //jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/iri-reference.json");
        //jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/json-pointer.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/regex.json");
        //jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/relative-json-pointer.json");
        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/time.json");
 
        //jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/uri.json");
        //jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/uri-reference.json");
        //jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/format/uri-template.json");

        jsonschema_tests("./jsonschema/JSON-Schema-Test-Suite/tests/draft2019-09/optional/content.json");
#endif
    }
}

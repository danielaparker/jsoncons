// Copyright 2020 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons_ext/yaml/yaml_parser.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <fstream>

using namespace jsoncons;

TEST_CASE("yaml test")
{
    const std::string input = R"(
invoice: 34843
date   : !!str 2001-01-23
bill-to: &id001
    given  : Chris
    family : Dumars
    address:
        lines: |
            458 Walkman Dr.
            Suite #292
)";


    SECTION("test1")
    {
        jsoncons::json_decoder<json> decoder;
        yaml::yaml_parser parser;

        parser.reset();

        parser.update(input.data(),input.length());
        parser.parse_some(decoder);
        parser.finish_parse(decoder);
        CHECK(parser.done());

        json j = decoder.get_result();
        std::cout << pretty_print(j) << "\n\n";
    }
}


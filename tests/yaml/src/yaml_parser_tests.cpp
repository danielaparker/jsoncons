// Copyright 2013 Daniel Parker
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

TEST_CASE("test_parse_empty_object")
{
    jsoncons::json_decoder<json> decoder;
    yaml::yaml_parser parser;

    parser.reset();

    static std::string s("{}");

    parser.update(s.data(),s.length());
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());

    json j = decoder.get_result();
}

TEST_CASE("test_parse_array")
{
    jsoncons::json_decoder<json> decoder;
    yaml::yaml_parser parser;

    parser.reset();

    static std::string s("[]");

    parser.update(s.data(),s.length());
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());

    json j = decoder.get_result();
}

TEST_CASE("test_parse_string")
{
    jsoncons::json_decoder<json> decoder;
    yaml::yaml_parser parser;

    parser.reset();

    static std::string s("\"\"");

    parser.update(s.data(),s.length());
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());

    json j = decoder.get_result();
}

TEST_CASE("test_parse_integer")
{
    jsoncons::json_decoder<json> decoder;
    yaml::yaml_parser parser;

    parser.reset();

    static std::string s("10");

    parser.update(s.data(),s.length());
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());

    json j = decoder.get_result();
}

TEST_CASE("test_parse_integer_space")
{
    jsoncons::json_decoder<json> decoder;
    yaml::yaml_parser parser;

    parser.reset();

    static std::string s("10 ");

    parser.update(s.data(),s.length());
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());

    json j = decoder.get_result();
}

TEST_CASE("test_parse_double_space")
{
    jsoncons::json_decoder<json> decoder;
    yaml::yaml_parser parser;

    parser.reset();

    static std::string s("10.0 ");

    parser.update(s.data(),s.length());
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());

    json j = decoder.get_result();
}

TEST_CASE("test_parse_false")
{
    jsoncons::json_decoder<json> decoder;
    yaml::yaml_parser parser;

    parser.reset();

    static std::string s("false");

    parser.update(s.data(),s.length());
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());

    json j = decoder.get_result();
}

TEST_CASE("test_parse_true")
{
    jsoncons::json_decoder<json> decoder;
    yaml::yaml_parser parser;

    parser.reset();

    static std::string s("true");

    parser.update(s.data(),s.length());
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());

    json j = decoder.get_result();
}

TEST_CASE("test_parse_null")
{
    jsoncons::json_decoder<json> decoder;
    yaml::yaml_parser parser;

    parser.reset();

    static std::string s("null");

    parser.update(s.data(),s.length());
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());

    json j = decoder.get_result();
}

TEST_CASE("test_parse_array_string")
{
    jsoncons::json_decoder<json> decoder;
    yaml::yaml_parser parser;

    parser.reset();

    static std::string s1("[\"\"");

    parser.update(s1.data(),s1.length());
    parser.parse_some(decoder);
    CHECK_FALSE(parser.done());
    static std::string s2("]");
    parser.update(s2.data(), s2.length());
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());

    json j = decoder.get_result();
}

TEST_CASE("test_incremental_parsing")
{
    jsoncons::json_decoder<json> decoder;
    yaml::yaml_parser parser;

    parser.reset();

    parser.update("[fal",4);
    parser.parse_some(decoder);
    CHECK_FALSE(parser.done());
    CHECK(parser.source_exhausted());
    parser.update("se]",3);
    parser.parse_some(decoder);

    parser.finish_parse(decoder);
    CHECK(parser.done());

    json j = decoder.get_result();
    REQUIRE(j.is_array());
    CHECK_FALSE(j[0].as<bool>());
}





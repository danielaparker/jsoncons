// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_parser.hpp>
#include <jsoncons/json_decoder.hpp>
#include <jsoncons/diagnostics_visitor.hpp>
#include <sstream>
#include <iostream>
#include <vector>
#include <utility>
#include <ctime>
#include <fstream>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("test_object2")
{
json source = json::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
})");

    std::cout << source << '\n';
}

TEST_CASE("test_object_with_three_members")
{
    SECTION("json object with three members")
    {
        std::string input = R"({"A":"Jane","B":"Roe","C":10})";
        auto j = json::parse(input);

        std::string buffer;
        j.dump(buffer);
        CHECK(input == buffer);
    }
    SECTION("ojson object with three members")
    {
        std::string input = R"({"B":"Roe","A":"Jane","C":10})";
        auto j = ojson::parse(input);

        std::string buffer;
        j.dump(buffer);
        CHECK(input == buffer);
    }
}

TEST_CASE("json::parse with string")
{
    SECTION("test double")
    {
        double expected = 42.229999999999997;
        auto val = json::parse(std::to_string(expected));
        CHECK(expected == val);
    }

    SECTION("test array of integer")
    {
        std::string s = "[1,2,3]";
        auto j = json::parse(s);
        CHECK(true == j.is_array());
        CHECK(3 == j.size());
    }

    SECTION("test skip bom")
    {
        std::string s = "\xEF\xBB\xBF[1,2,3]";
        auto j = json::parse(s);
        CHECK(true == j.is_array());
        CHECK(3 == j.size());
    }
}

TEST_CASE("test_parse_empty_object")
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser;

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
    json_parser parser;

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
    json_parser parser;

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
    json_parser parser;

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
    json_parser parser;

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
    json_parser parser;

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
    json_parser parser;

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
    json_parser parser;

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
    json_parser parser;

    parser.reset();

    static std::string s("null");

    parser.update(s.data(),s.length());
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());

    json j = decoder.get_result();
}

TEST_CASE("test incremental parsing")
{
    SECTION("array of bool")
    {
        jsoncons::json_decoder<json> decoder;
        json_parser parser;

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
        REQUIRE(j[0].is<bool>());
        CHECK_FALSE(j[0].as<bool>());
    }
    SECTION("array of bool 2")
    {
        json_diagnostics_visitor decoder(std::cout, "  ");
        //jsoncons::json_decoder<json> decoder;
        json_parser parser;

        parser.reset();

        parser.update("[fal",4);
        parser.parse_some(decoder);
        CHECK_FALSE(parser.done());
        CHECK(parser.source_exhausted());
        parser.update("se]",3);
        parser.parse_some(decoder);

        parser.finish_parse(decoder);
        CHECK(parser.done());

        //json j = decoder.get_result();
        //REQUIRE(j.is_array());
        //REQUIRE(j[0].is<bool>());
        //CHECK_FALSE(j[0].as<bool>());
    }
    SECTION("test 2")
    {
        std::string data1 = R"("010)";
        std::string data2 = R"(1010)";
        std::string data3 = R"(1010)";
        std::string data4 = R"(101")";
        jsoncons::json_decoder<json> decoder;
        json_parser parser;

        parser.reset();

        parser.update(data1);
        parser.parse_some(decoder);
        CHECK_FALSE(parser.done());
        CHECK(parser.source_exhausted());

        parser.update(data2);
        parser.parse_some(decoder);
        CHECK_FALSE(parser.done());
        CHECK(parser.source_exhausted());

        parser.update(data3);
        parser.parse_some(decoder);
        CHECK_FALSE(parser.done());
        CHECK(parser.source_exhausted());

        parser.update(data4);
        parser.parse_some(decoder);
        CHECK_FALSE(parser.done());
        CHECK(parser.source_exhausted());

        parser.finish_parse(decoder);
        CHECK(parser.done());

        json j = decoder.get_result();
        REQUIRE(j.is_string());
    }

    SECTION("test 3")
    {
        std::string data = R"("010101")";
        std::istringstream is(data);

        std::ostringstream os;
        //json_diagnostics_visitor decoder(os, "  ");
        json_decoder<json> decoder;
        json_parser parser;

        //std::error_code ec{};
        stream_source<char> source(is, 4);
        while (!source.eof())
        {
            auto s = source.read_buffer();
            std::cout << jsoncons::string_view(s.data(), s.size()) << "\n";
            parser.update(s.data(), s.size());
            parser.parse_some(decoder);
            //if (parser.done())
            //{
            //    break;
            //}
        }
        parser.finish_parse(decoder);
        std::cout << "done: " << parser.done() << "\n";
        std::cout << decoder.is_valid() << "\n";
        //std::cout << os.str() << "\n";
    }
}

TEST_CASE("test_parser_reinitialization")
{
    jsoncons::json_decoder<json> decoder;
    json_parser parser;

    parser.reset();
    parser.update("false true", 10);
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());
    CHECK_FALSE(parser.source_exhausted());
    json j1 = decoder.get_result();
    REQUIRE(j1.is_bool());
    CHECK_FALSE(j1.as<bool>());

    parser.reinitialize();
    parser.update("-42", 3);
    parser.parse_some(decoder);
    parser.finish_parse(decoder);
    CHECK(parser.done());
    CHECK(parser.source_exhausted());
    json j2 = decoder.get_result();
    REQUIRE(j2.is_int64());
    CHECK(j2.as<int64_t>() == -42);
}

TEST_CASE("test_diagnostics_visitor", "")
{
    SECTION("narrow char")
    {
        std::ostringstream os;
        json_diagnostics_visitor visitor(os, "  ");
        json_parser parser;
        std::string input(R"({"foo":[42,null]})");
        parser.update(input.data(), input.size());
        parser.parse_some(visitor);
        parser.finish_parse(visitor);
        std::ostringstream expected;
        expected << "visit_begin_object"  << '\n'
                 << "  visit_key:foo"     << '\n'
                 << "  visit_begin_array" << '\n'
                 << "    visit_uint64:42" << '\n'
                 << "    visit_null"      << '\n'
                 << "  visit_end_array"   << '\n'
                 << "visit_end_object"    << '\n';
        CHECK(expected.str() == os.str())             ;
    }
    SECTION("wide char")
    {
        std::wostringstream os;
        wjson_diagnostics_visitor visitor(os, L"  ");
        wjson_parser parser;
        std::wstring input(LR"({"foo":[42,null]})");
        parser.update(input.data(), input.size());
        parser.parse_some(visitor);
        parser.finish_parse(visitor);
        std::wostringstream expected;
        expected << L"visit_begin_object"  << '\n'
                 << L"  visit_key:foo"     << '\n'
                 << L"  visit_begin_array" << '\n'
                 << L"    visit_uint64:42" << '\n'
                 << L"    visit_null"      << '\n'
                 << L"  visit_end_array"   << '\n'
                 << L"visit_end_object"    << '\n';
        CHECK(expected.str() == os.str())             ;
    }
}

TEST_CASE("json_parser skip space tests")
{
    SECTION("test 1")
    {
        jsoncons::json_decoder<json> decoder;
        json_parser parser;
        
        std::string line1 = "[false\r";
        std::string line2 = ",true]";

        parser.update(line1.data(), line1.size());
        parser.parse_some(decoder);
        CHECK_FALSE(parser.done());
        CHECK(parser.source_exhausted());
        parser.update(line2.data(), line2.size());
        parser.parse_some(decoder);
        
        //std::cout << "position:" << parser.position() << "line:" << parser.line() << "column:" << parser.column() << "\n";
        
        CHECK(12 == parser.position());
        CHECK(2 == parser.line());
        CHECK(7 == parser.column());
    }

    SECTION("test 2")
    {
        jsoncons::json_decoder<json> decoder;
        json_parser parser;

        std::string line1 = "[false\r";
        std::string line2 = "\n,true]";

        parser.update(line1.data(), line1.size());
        parser.parse_some(decoder);
        CHECK_FALSE(parser.done());
        CHECK(parser.source_exhausted());
        parser.update(line2.data(), line2.size());
        parser.parse_some(decoder);

        //std::cout << "position:" << parser.position() << "line:" << parser.line() << "column:" << parser.column() << "\n";

        CHECK(13 == parser.position());
        CHECK(2 == parser.line());
        CHECK(7 == parser.column());
    }
    SECTION("test 3")
    {
        jsoncons::json_decoder<json> decoder;
        json_parser parser;

        std::string line1 = "[false\n";
        std::string line2 = ",true]";

        parser.update(line1.data(), line1.size());
        parser.parse_some(decoder);
        CHECK_FALSE(parser.done());
        CHECK(parser.source_exhausted());
        parser.update(line2.data(), line2.size());
        parser.parse_some(decoder);
        parser.finish_parse(decoder);

        //std::cout << "position:" << parser.position() << "line:" << parser.line() << "column:" << parser.column() << "\n";

        CHECK(12 == parser.position());
        CHECK(2 == parser.line());
        CHECK(7 == parser.column());
    }
}


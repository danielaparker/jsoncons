// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/toon/toon_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <iostream>

using namespace jsoncons;

TEST_CASE("toon is_number")
{
    SECTION("test1")
    {
        CHECK_FALSE(toon::toon_stream_encoder::is_number("-"));
        CHECK(toon::toon_stream_encoder::is_number("-0"));
        CHECK(toon::toon_stream_encoder::is_number("-1"));
        CHECK_FALSE(toon::toon_stream_encoder::is_number("-0a"));
        CHECK_FALSE(toon::toon_stream_encoder::is_number("00"));
        CHECK_FALSE(toon::toon_stream_encoder::is_number("-00"));
        CHECK_FALSE(toon::toon_stream_encoder::is_number("0-0"));
        CHECK(toon::toon_stream_encoder::is_number("-0.0"));
        CHECK(toon::toon_stream_encoder::is_number("-1.1"));
        CHECK_FALSE(toon::toon_stream_encoder::is_number("-0.0a"));
        CHECK(toon::toon_stream_encoder::is_number("0"));
        CHECK(toon::toon_stream_encoder::is_number("1"));
        CHECK(toon::toon_stream_encoder::is_number("123456789"));
        CHECK_FALSE(toon::toon_stream_encoder::is_number("123456789."));
        CHECK(toon::toon_stream_encoder::is_number("123456789.0"));
        CHECK_FALSE(toon::toon_stream_encoder::is_number("foo"));
        CHECK_FALSE(toon::toon_stream_encoder::is_number("-foo"));
    }
}

TEST_CASE("toon test string encoding")
{
    SECTION("array of one string")
    {
        std::string expected = R"([1]: Hello World)";

        std::ostringstream os;
        toon::toon_stream_encoder encoder{os};

        encoder.begin_array(1);
        encoder.string_value("Hello World");
        encoder.end_array();
        encoder.flush();

        CHECK(expected == os.str());
    }
    SECTION("string with embedded quote")
    {
        std::string expected = R"([1]: "Hello \"World\"")";

        std::ostringstream os;
        toon::toon_stream_encoder encoder{os};

        encoder.begin_array(1);
        encoder.string_value("Hello \"World\"");
        encoder.end_array();
        encoder.flush();

        CHECK(expected == os.str());
    }
    SECTION("array of two strings")
    {
        std::string expected = R"([2]: Hello World,Foo)";

        std::ostringstream os;
        toon::toon_stream_encoder encoder{os};

        encoder.begin_array(2);
        encoder.string_value("Hello World");
        encoder.string_value("Foo");
        encoder.end_array();
        encoder.flush();

        CHECK(expected == os.str());
    }
    SECTION("array of array")
    {
        std::string expected = R"([1]:
  - [2]: Hello World,Foo)";

        std::ostringstream os;
        toon::toon_stream_encoder encoder{os};

        encoder.begin_array(1);
        encoder.begin_array(2);
        encoder.string_value("Hello World");
        encoder.string_value("Foo");
        encoder.end_array();
        encoder.end_array();
        encoder.flush();

        CHECK(expected == os.str());
    }
    SECTION("array of array of array")
    {
        std::string expected = R"([1]:
  - [1]:
    - [2]: Hello World,Foo)";

        std::ostringstream os;
        toon::toon_stream_encoder encoder{os};

        encoder.begin_array(1);
        encoder.begin_array(1);
        encoder.begin_array(2);
        encoder.string_value("Hello World");
        encoder.string_value("Foo");
        encoder.end_array();
        encoder.end_array();
        encoder.end_array();
        encoder.flush();

        CHECK(expected == os.str());
    }
}


// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/toon/encode_toon.hpp>
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
        CHECK_FALSE(toon::detail::is_number<char>("-"));
        CHECK(toon::detail::is_number<char>("-0"));
        CHECK(toon::detail::is_number<char>("-1"));
        CHECK_FALSE(toon::detail::is_number<char>("-0a"));
        CHECK_FALSE(toon::detail::is_number<char>("00"));
        CHECK_FALSE(toon::detail::is_number<char>("-00"));
        CHECK_FALSE(toon::detail::is_number<char>("0-0"));
        CHECK(toon::detail::is_number<char>("-0.0"));
        CHECK(toon::detail::is_number<char>("-1.1"));
        CHECK_FALSE(toon::detail::is_number<char>("-0.0a"));
        CHECK(toon::detail::is_number<char>("0"));
        CHECK(toon::detail::is_number<char>("1"));
        CHECK(toon::detail::is_number<char>("123456789"));
        CHECK_FALSE(toon::detail::is_number<char>("123456789."));
        CHECK(toon::detail::is_number<char>("123456789.0"));
        CHECK_FALSE(toon::detail::is_number<char>("foo"));
        CHECK_FALSE(toon::detail::is_number<char>("-foo"));
    }
}

TEST_CASE("toon array encode")
{
    SECTION("array of one string")
    {
        std::string expected = R"([1]: Hello World)";

        std::ostringstream os;
        json j{json_array_arg};
        j.emplace_back("Hello World");

        toon::encode_toon(j, os);

        CHECK(expected == os.str());
    }
#if 0
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
    SECTION("object")
    {
        std::string expected = R"(baz: qux
foo: bar)";

        std::ostringstream os;
        toon::toon_stream_encoder encoder{os};

        encoder.begin_object(1);
        encoder.key("baz");
        encoder.string_value("qux");
        encoder.key("foo");
        encoder.string_value("bar");
        encoder.end_object();
        encoder.flush();

        CHECK(expected == os.str());
    }
    SECTION("array of object")
    {
        std::string expected = R"([1]{baz,foo}:
  qux,bar)";

        std::ostringstream os;
        toon::toon_stream_encoder encoder{os};

        encoder.begin_array(1);
        encoder.begin_object();
        encoder.key("baz");
        encoder.string_value("qux");
        encoder.key("foo");
        encoder.string_value("bar");
        encoder.end_object();
        encoder.end_array();
        encoder.flush();

        CHECK(expected == os.str());
    }
#endif
}

// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/utility/bigdec.hpp>
#include <iostream>
#include <catch/catch.hpp>

TEST_CASE("basic_bigdec success tests")
{
    SECTION("0")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "0";
        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(sv == buf);
    }
    SECTION("123456")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "123456";
        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(sv == buf);
    }
    SECTION("123456.123456")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "123456.123456";
        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(sv == buf);
    }
    SECTION("-123456")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "-123456";
        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(sv == buf);
    }
    SECTION("-123456.123456")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "-123456.123456";
        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(sv == buf);
    }
    SECTION("123456e5")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "123456e5";
        jsoncons::string_view expected = "1.23456e10";

        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(expected == buf);
    }
    SECTION("123456e-5")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "123456e-5";
        jsoncons::string_view expected = "1.23456";

        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(expected == buf);
    }
    SECTION("123456e-6")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "123456e-6";
        jsoncons::string_view expected = "0.123456";

        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(expected == buf);
    }
    SECTION("123456e-7")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "123456e-7";
        jsoncons::string_view expected = "0.0123456";

        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(expected == buf);
    }
    SECTION("123456.123456e7")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "123456.123456e7";
        jsoncons::string_view expected = "1.23456123456e12";

        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(expected == buf);
    }
    SECTION("123456.123456e-7")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "123456.123456e-7";
        jsoncons::string_view expected = "0.0123456123456";

        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(expected == buf);
    }
}

TEST_CASE("basic_bigdec wchar_t success tests")
{
    SECTION("0")
    {
        jsoncons::bigdec value;
        jsoncons::wstring_view sv = L"0";
        auto result = jsoncons::to_bigdec(sv.data(), sv.size(), value);
        CHECK(result);

        std::wstring buf;
        to_buffer(value, buf);
        CHECK(sv == buf);
    }
}

TEST_CASE("basic_bigdec end tests")
{
    SECTION("10[1]")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "10";
        jsoncons::string_view expected = sv.substr(0,sv.size()-1);

        auto result = jsoncons::to_bigdec(sv.data(), sv.size()-1, value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(expected == buf);
    }
    SECTION("1[.]")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "1.";
        jsoncons::string_view expected = sv.substr(0,sv.size()-1);

        auto result = jsoncons::to_bigdec(sv.data(), sv.size()-1, value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(expected == buf);
    }
    SECTION("12.12[e]")
    {
        jsoncons::bigdec value;
        jsoncons::string_view sv = "12.12e";
        jsoncons::string_view expected = sv.substr(0,sv.size()-1);

        auto result = jsoncons::to_bigdec(sv.data(), sv.size()-1, value);
        CHECK(result);

        std::string buf;
        to_buffer(value, buf);
        CHECK(expected == buf);
    }
}

TEST_CASE("basic_bigdec multiply tests")
{
    SECTION("123456.123456 * 123456789.123456789")
    {
        jsoncons::bigdec a("123456.123456");
        jsoncons::bigdec b("123456789.123456789");
        jsoncons::bigdec expected("15241496599506.839368265342784");
        jsoncons::bigdec c;
        auto r = jsoncons::multiply(a, b, c);
        REQUIRE(r);
        CHECK(expected == c);
    }
}

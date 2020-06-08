// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>

TEST_CASE("json integer as string")
{
    SECTION("0xabcdef")
    {
        jsoncons::json j("0xabcdef");
        CHECK(j.as<int32_t>() == 11259375);
    }
    SECTION("0x123456789")
    {
        jsoncons::json j("0x123456789");
        CHECK(j.as<int64_t>() == 4886718345);
    }
    SECTION("0XABCDEF")
    {
        jsoncons::json j("0XABCDEF");
        CHECK(j.as<uint32_t>() == 11259375u);
    }
    SECTION("0X123456789")
    {
        jsoncons::json j("0X123456789");
        CHECK(j.as<uint64_t>() == 4886718345);
    }
    SECTION("0x0")
    {
        jsoncons::json j("0x0");
        CHECK(j.as<int>() == 0);
    }
    SECTION("0777")
    {
        jsoncons::json j("0777");
        CHECK(j.as<int>() == 511);
    }
    SECTION("0b1001")
    {
        jsoncons::json j("0b1001");
        CHECK(j.as<int>() == 9);
    }
    SECTION("0B1001")
    {
        jsoncons::json j("0B1001");
        CHECK(j.as<int>() == 9);
    }
}

TEST_CASE("json::is_object on proxy")
{
    jsoncons::json root = jsoncons::json::parse(R"({"key":"value"})");

    CHECK_FALSE(root["key1"].is_object());
}

TEST_CASE("json::as<jsoncons::string_view>()")
{
    std::string s1("Short");
    jsoncons::json j1(s1);
    CHECK(j1.as<jsoncons::string_view>() == jsoncons::string_view(s1));

    std::string s2("String to long for short string");
    jsoncons::json j2(s2);
    CHECK(j2.as<jsoncons::string_view>() == jsoncons::string_view(s2));
}

TEST_CASE("json::as<jsoncons::bigint>()")
{
    SECTION("from integer")
    {
        jsoncons::json j(-1000);
        CHECK(bool(j.as<jsoncons::bigint>() == jsoncons::bigint(-1000)));
    }
    SECTION("from unsigned integer")
    {
        jsoncons::json j(1000u);
        CHECK(bool(j.as<jsoncons::bigint>() == jsoncons::bigint(1000u)));
    }
    SECTION("from double")
    {
        jsoncons::json j(1000.0);
        CHECK(bool(j.as<jsoncons::bigint>() == jsoncons::bigint(1000)));
    }
    SECTION("from bigint")
    {
        std::string s = "-18446744073709551617";
        jsoncons::json j(s,  jsoncons::semantic_tag::bigint);
        CHECK(bool(j.as<jsoncons::bigint>() == jsoncons::bigint::from_string(s)));
    }
}

#if (defined(__GNUC__) || defined(__clang__)) && (!defined(__STRICT_ANSI__) && defined(_GLIBCXX_USE_INT128))
TEST_CASE("json::as<__int128>()")
{
    std::string s = "-18446744073709551617";

    jsoncons::detail::to_integer_result<__int128> result = jsoncons::detail::to_integer_unchecked<__int128>(s.data(),s.size());
    REQUIRE(result);

    jsoncons::json j(s);

    __int128 val = j.as<__int128>();
    CHECK(result.value() == val);
}

TEST_CASE("json::as<unsigned __int128>()")
{
    std::string s = "18446744073709551616";

    jsoncons::detail::to_integer_result<unsigned __int128> result = jsoncons::detail::to_integer_unchecked<unsigned __int128>(s.data(),s.size());
    REQUIRE(result);

    jsoncons::json j(s);

    unsigned __int128 val = j.as<unsigned __int128>();
    CHECK(result.value() == val);
}
#endif

TEST_CASE("as byte string tests")
{
    SECTION("byte_string_arg hint")
    {
        std::vector<uint8_t> v = {'H','e','l','l','o'};
        jsoncons::json j(jsoncons::byte_string_arg, v, jsoncons::semantic_tag::base64);
        jsoncons::json sj(j.as<std::string>());

        auto u = sj.as<std::vector<uint8_t>>(jsoncons::byte_string_arg,
                                             jsoncons::semantic_tag::base64);

        CHECK(u == v);
    }
    SECTION("as std::vector<char>")
    {
        std::vector<char> v = {'H','e','l','l','o'};
        jsoncons::json j(jsoncons::byte_string_arg, v, jsoncons::semantic_tag::base64);

        auto u = j.as<std::vector<char>>();

        CHECK(u == v);
    }
    SECTION("as<std::vector<char>>(jsoncons::byte_string_arg, ...")
    {
        std::vector<char> v = {'H','e','l','l','o'};
        jsoncons::json j(jsoncons::byte_string_arg, v, jsoncons::semantic_tag::base64);
        jsoncons::json sj(j.as<std::string>());

        auto u = sj.as<std::vector<char>>(jsoncons::byte_string_arg,
                                          jsoncons::semantic_tag::base64);

        CHECK(u == v);
    }
}


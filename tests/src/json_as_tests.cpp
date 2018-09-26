// Copyright 2018 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <map>

TEST_CASE("json::as<jsoncons::string_view>()")
{
    std::string s1("Short");
    jsoncons::json j1(s1);
    CHECK(j1.as<jsoncons::string_view>() == jsoncons::string_view(s1));

    std::string s2("String to long for short string");
    jsoncons::json j2(s2);
    CHECK(j2.as<jsoncons::string_view>() == jsoncons::string_view(s2));
}

#if defined (__clang__)
#if __has_include(<string_view>)
#include <string_view>
TEST_CASE("json::as<std::string_view>()")
{
    std::cout << "<string_view>\n";
    std::string s1("Short");
    jsoncons::json j1(s1);
    CHECK(j1.as<std::string_view>() == std::string_view(s1));

    std::string s2("String to long for short string");
    jsoncons::json j2(s2);
    CHECK(j2.as<std::string_view>() == std::string_view(s2));
}
#endif
#endif

#if defined (__clang__)
#if __has_include(<experimental/string_view>)
#include <experimental/string_view>
TEST_CASE("json::as<std::experimental::string_view>()")
{
    std::cout << "<experimental/string_view>\n";
    std::string s1("Short");
    jsoncons::json j1(s1);
    CHECK(j1.as<std::experimental::string_view>() == std::experimental::string_view(s1));

    std::string s2("String to long for short string");
    jsoncons::json j2(s2);
    CHECK(j2.as<std::experimental::string_view>() == std::experimental::string_view(s2));
}
#endif
#endif


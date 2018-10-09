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

TEST_CASE("json::as<jsoncons::bignum>()")
{
    SECTION("from integer")
    {
        jsoncons::json j(-1000);
        CHECK(j.as<jsoncons::bignum>() == jsoncons::bignum(-1000));
    }
    SECTION("from unsigned integer")
    {
        jsoncons::json j(1000u);
        CHECK(j.as<jsoncons::bignum>() == jsoncons::bignum(1000u));
    }
    SECTION("from double")
    {
        jsoncons::json j(1000.0);
        CHECK(j.as<jsoncons::bignum>() == jsoncons::bignum(1000.0));
    }
    SECTION("from bignum")
    {
        std::string s = "-18446744073709551617";
        jsoncons::json j(s,  jsoncons::semantic_tag_type::bignum);
        CHECK(j.as<jsoncons::bignum>() == jsoncons::bignum(s));
    }
}

#if defined(__GNUC__) || defined(__clang__)
TEST_CASE("json::as<__int128>()")
{

    std::string s = "-18446744073709551617";

    jsoncons::detail::to_integer_result<__int128 val> val = = jsoncons::detail::to_integer<__int128,char>(s.data(),s.size());

}
#endif

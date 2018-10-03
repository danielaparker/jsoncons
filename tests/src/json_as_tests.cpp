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

TEST_CASE("json::as<long long>()")
{
    jsoncons::json j(jsoncons::bignum("18446744073709551616"));
    CHECK(j.is_bignum());

#if defined(__GNUC__)  && (__GNUC__ >= 6)
    std::string s1 = "18446744073709551616"; 
    CHECK(jsoncons::detail::is_integer(s1.data(), s1.length()));
    to_integer_result result1 = jsoncons::detail::as_integer<__int128>(s1.data(), s1.length());
    CHECK_NOT(result1.overflow);

    const __int128 m = 18446744073709551616;
    ChECK(result1.value == m);

    std::string s2 = "-18446744073709551617";
    CHECK(jsoncons::detail::is_integer(s2.data(), s2.length()));
    to_integer_result result2 = jsoncons::detail::as_integer<__int128>(s2.data(), s2.length());
    CHECK_NOT(result2.overflow);

    const __int128 m2 = -18446744073709551617;
    ChECK(result2.value == m2);
    //std::cout << "sizeof(__int128): " << sizeof(__int128) << "\n\n";
#endif
}


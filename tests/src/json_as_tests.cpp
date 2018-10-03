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

#if defined(__GNUC__) 
    std::cout << "GCC sizeof(long long): " << sizeof(long long) << "\n\n";
#elif defined (__clang__)
    std::cout << "clang sizeof(long long): " << sizeof(long long) << "\n\n";
#else
    std::cout << "vs sizeof(long long): " << sizeof(long long) << "\n\n";
#endif
}


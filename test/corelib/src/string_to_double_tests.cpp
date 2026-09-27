// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/utility/number_readers.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cwchar>
#include <iostream>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("test_string_to_double")
{
    std::cout << "sizeof(jsoncons::json): " << sizeof(jsoncons::json) << '\n'; 

    const char* s1 = "0.0";
    jsoncons::json j1 = jsoncons::json::parse(s1);
    double expected1 = 0.0;
    CHECK( j1.as<double>() == expected1);

    const char* s2 = "0.123456789";
    jsoncons::json j2 = jsoncons::json::parse(s2);
    double expected2 = 0.123456789;
    CHECK( j2.as<double>() == expected2);

    const char* s3 = "123456789.123456789";
    jsoncons::json j3 = jsoncons::json::parse(s3);
    char* end3 = nullptr;
    double expected3 = strtod(s3,&end3);
    CHECK( j3.as<double>() == expected3);
}

TEST_CASE("test_exponent")
{
    const char* begin = "1.15507e-173";
    char* endptr = nullptr;
    const double value1 = 1.15507e-173;
    const double value2 = strtod(begin, &endptr );
    double value3{ 0 };
    jsoncons::decstr_to_double(begin, endptr-begin, value3);

    CHECK(value1 == value2);
    CHECK(value2 == value3);

    const char* s1 = "1.15507e+173";
    jsoncons::json j1 = jsoncons::json::parse(s1);
    double expected1 = 1.15507e+173;
    CHECK( j1.as<double>() == expected1);

}


// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cwchar>
#include <jsoncons/detail/parse_number.hpp>

using namespace jsoncons;

TEST_CASE("test_string_to_double")
{
    std::cout << "sizeof(json): " << sizeof(json) << std::endl; 

    const char* s1 = "0.0";
    json j1 = json::parse(s1);
    double expected1 = 0.0;
    CHECK(expected1 == j1.as<double>());

    const char* s2 = "0.123456789";
    json j2 = json::parse(s2);
    double expected2 = 0.123456789;
    CHECK(expected2 == j2.as<double>());

    const char* s3 = "123456789.123456789";
    json j3 = json::parse(s3);
    char* end3 = nullptr;
    double expected3 = strtod(s3,&end3);
    CHECK(expected3 == j3.as<double>());
}

TEST_CASE("test_exponent")
{
    detail::string_to_double reader;
    const char* begin = "1.15507e-173";
    const char* endptr = begin + strlen(begin);
    const double value1 = 1.15507e-173;
    const double value2 = strtod((char*) begin, (char**)&endptr );
    const double value3 = reader(begin,endptr-begin);

    CHECK(value1 == value2);
    CHECK(value2 == value3);

    const char* s1 = "1.15507e+173";
    json j1 = json::parse(s1);
    double expected1 = 1.15507e+173;
    CHECK(expected1 == j1.as<double>());

    //std::cout << std::numeric_limits<double>::max() << std::endl;
}


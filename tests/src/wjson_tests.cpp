// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_filter.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

TEST_CASE("test_wjson")
{
    wjson root;
    root[L"field1"] = L"test";
    root[L"field2"] = 3.9;
    root[L"field3"] = true;

    CHECK(root[L"field1"].as<std::wstring>() == L"test");
    CHECK(root[L"field2"].as<double>() == 3.9);
    CHECK(root[L"field3"].as<bool>() == true);

    std::wstring s1 = root[L"field1"].as<std::wstring>();
    CHECK(s1 == L"test");
}

TEST_CASE("test_wjson_escape_u")
{
    std::wstring input = L"[\"\\uABCD\"]";
    std::wistringstream is(input);

    wjson root = wjson::parse(is);

    std::wstring s = root[0].as<std::wstring>();
    CHECK( s.length() == 1 );
    CHECK( s[0] == 0xABCD );
}



// Copyright 2013 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <vector>

using jsoncons::json_type_traits;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::decode_json;
using jsoncons::encode_json;

TEST_CASE("deser_traits string tests")
{
    SECTION("test 1")
    {
        std::string s = "foo";

        std::wstring buf;
        encode_json(s,buf);

        auto s2 = decode_json<std::string>(buf);

        CHECK(s2 == s);
    }
    SECTION("test 2")
    {
        std::wstring s = L"foo";

        std::string buf;
        encode_json(s,buf);

        auto s2 = decode_json<std::wstring>(buf);

        CHECK(s2 == s);
    }
}

TEST_CASE("deser_traits vector of string tests")
{
    SECTION("test 1")
    {
        std::vector<std::string> v = {"foo","bar","baz"};

        std::wstring buf;
        encode_json(v,buf);

        auto v2 = decode_json<std::vector<std::string>>(buf);

        CHECK(v2 == v);
    }
    SECTION("test 2")
    {
        std::vector<std::wstring> v = {L"foo",L"bar",L"baz"};

        std::string buf;
        encode_json(v,buf);

        auto v2 = decode_json<std::vector<std::wstring>>(buf);

        CHECK(v2 == v);
    }
}

TEST_CASE("deser_traits std::pair tests")
{
    SECTION("test 1")
    {
        auto p = std::make_pair<int,std::string>(1,"foo");

        std::wstring buf;
        encode_json(p,buf);

        auto p2 = decode_json<std::pair<int,std::string>>(buf);

        CHECK(p2 == p);
    }
    SECTION("test 2")
    {
        auto p = std::make_pair<int,std::wstring>(1,L"foo");

        std::wstring buf;
        encode_json(p,buf);

        auto p2 = decode_json<std::pair<int,std::wstring>>(buf);

        CHECK(p2 == p);
    }
}


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

TEST_CASE("json_type_traits string tests")
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


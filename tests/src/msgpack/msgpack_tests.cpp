// Copyright 2016 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <catch/catch.hpp>

using namespace jsoncons;
using namespace jsoncons::msgpack;
#if 0
TEST_CASE("msgpack_test")
{
    json j1;
    j1["zero"] = 0;
    j1["one"] = 1;
    j1["two"] = 2;
    j1["null"] = null_type();
    j1["true"] = true;
    j1["false"] = false;
    j1["max int64_t"] = (std::numeric_limits<int64_t>::max)();
    j1["max uint64_t"] = (std::numeric_limits<uint64_t>::max)();
    j1["min int64_t"] = (std::numeric_limits<int64_t>::lowest)();
    j1["max int32_t"] = (std::numeric_limits<int32_t>::max)();
    j1["max uint32_t"] = (std::numeric_limits<uint32_t>::max)();
    j1["min int32_t"] = (std::numeric_limits<int32_t>::lowest)();
    j1["max int16_t"] = (std::numeric_limits<int16_t>::max)();
    j1["max uint16_t"] = (std::numeric_limits<uint16_t>::max)();
    j1["min int16_t"] = (std::numeric_limits<int16_t>::lowest)();
    j1["max int8_t"] = (std::numeric_limits<int8_t>::max)();
    j1["max uint8_t"] = (std::numeric_limits<uint8_t>::max)();
    j1["min int8_t"] = (std::numeric_limits<int8_t>::lowest)();
    j1["max double"] = (std::numeric_limits<double>::max)();
    j1["min double"] = (std::numeric_limits<double>::lowest)();
    j1["max float"] = (std::numeric_limits<float>::max)();
    j1["zero float"] = 0.0;
    j1["min float"] = (std::numeric_limits<float>::lowest)();
    j1["String too long for small string optimization"] = "String too long for small string optimization"; 

    json ja(json_array_arg);
    ja.push_back(0);
    ja.push_back(1);
    ja.push_back(2);
    ja.push_back(null_type());
    ja.push_back(true);
    ja.push_back(false);
    ja.push_back((std::numeric_limits<int64_t>::max)());
    ja.push_back((std::numeric_limits<uint64_t>::max)());
    ja.push_back((std::numeric_limits<int64_t>::lowest)());
    ja.push_back((std::numeric_limits<int32_t>::max)());
    ja.push_back((std::numeric_limits<uint32_t>::max)());
    ja.push_back((std::numeric_limits<int32_t>::lowest)());
    ja.push_back((std::numeric_limits<int16_t>::max)());
    ja.push_back((std::numeric_limits<uint16_t>::max)());
    ja.push_back((std::numeric_limits<int16_t>::lowest)());
    ja.push_back((std::numeric_limits<int8_t>::max)());
    ja.push_back((std::numeric_limits<uint8_t>::max)());
    ja.push_back((std::numeric_limits<int8_t>::lowest)());
    ja.push_back((std::numeric_limits<double>::max)());
    ja.push_back((std::numeric_limits<double>::lowest)());
    ja.push_back((std::numeric_limits<float>::max)());
    ja.push_back(0.0);
    ja.push_back((std::numeric_limits<float>::lowest)());
    ja.push_back("String too long for small string optimization");

    j1["An array"] = ja; 

    std::vector<uint8_t> v;
    encode_msgpack(j1, v);

    json j2 = decode_msgpack<json>(v);

    CHECK(j1 == j2);
} 

TEST_CASE("msgpack_test2")
{
    wjson j1;
    j1[L"zero"] = 0;
    j1[L"one"] = 1;
    j1[L"two"] = 2;
    j1[L"null"] = null_type();
    j1[L"true"] = true;
    j1[L"false"] = false;
    j1[L"max int64_t"] = (std::numeric_limits<int64_t>::max)();
    j1[L"max uint64_t"] = (std::numeric_limits<uint64_t>::max)();
    j1[L"min int64_t"] = (std::numeric_limits<int64_t>::lowest)();
    j1[L"max int32_t"] = (std::numeric_limits<int32_t>::max)();
    j1[L"max uint32_t"] = (std::numeric_limits<uint32_t>::max)();
    j1[L"min int32_t"] = (std::numeric_limits<int32_t>::lowest)();
    j1[L"max int16_t"] = (std::numeric_limits<int16_t>::max)();
    j1[L"max uint16_t"] = (std::numeric_limits<uint16_t>::max)();
    j1[L"min int16_t"] = (std::numeric_limits<int16_t>::lowest)();
    j1[L"max int8_t"] = (std::numeric_limits<int8_t>::max)();
    j1[L"max uint8_t"] = (std::numeric_limits<uint8_t>::max)();
    j1[L"min int8_t"] = (std::numeric_limits<int8_t>::lowest)();
    j1[L"max double"] = (std::numeric_limits<double>::max)();
    j1[L"min double"] = (std::numeric_limits<double>::lowest)();
    j1[L"max float"] = (std::numeric_limits<float>::max)();
    j1[L"zero float"] = 0.0;
    j1[L"min float"] = (std::numeric_limits<float>::lowest)();
    j1[L"S"] = L"S";
    j1[L"String too long for small string optimization"] = L"String too long for small string optimization";

    wjson ja(json_array_arg);
    ja.push_back(0);
    ja.push_back(1);
    ja.push_back(2);
    ja.push_back(null_type());
    ja.push_back(true);
    ja.push_back(false);
    ja.push_back((std::numeric_limits<int64_t>::max)());
    ja.push_back((std::numeric_limits<uint64_t>::max)());
    ja.push_back((std::numeric_limits<int64_t>::lowest)());
    ja.push_back((std::numeric_limits<int32_t>::max)());
    ja.push_back((std::numeric_limits<uint32_t>::max)());
    ja.push_back((std::numeric_limits<int32_t>::lowest)());
    ja.push_back((std::numeric_limits<int16_t>::max)());
    ja.push_back((std::numeric_limits<uint16_t>::max)());
    ja.push_back((std::numeric_limits<int16_t>::lowest)());
    ja.push_back((std::numeric_limits<int8_t>::max)());
    ja.push_back((std::numeric_limits<uint8_t>::max)());
    ja.push_back((std::numeric_limits<int8_t>::lowest)());
    ja.push_back((std::numeric_limits<double>::max)());
    ja.push_back((std::numeric_limits<double>::lowest)());
    ja.push_back((std::numeric_limits<float>::max)());
    ja.push_back(0.0);
    ja.push_back((std::numeric_limits<float>::lowest)());
    ja.push_back(L"S");
    ja.push_back(L"String too long for small string optimization");

    j1[L"An array"] = ja;

    std::vector<uint8_t> v;
    encode_msgpack(j1, v);

    //wjson j2 = decode_msgpack<wjson>(v);

    //CHECK(j1 == j2);
}
#endif

TEST_CASE("msgpack bin tests")
{
    SECTION("[]")
    {
        std::vector<uint8_t> expected;

        std::vector<uint8_t> input1 = {0xc4,0x00};
        auto v1 = decode_msgpack<std::vector<uint8_t>>(input1);
        CHECK(v1 == expected);

        std::vector<uint8_t> input2 = {0xc5,0x00,0x00};
        auto v2 = decode_msgpack<std::vector<uint8_t>>(input2);
        CHECK(v2 == expected);

        std::vector<uint8_t> input3 = {0xc6,0x00,0x00,0x00,0x00};
        auto v3 = decode_msgpack<std::vector<uint8_t>>(input3);
        CHECK(v3 == expected);

        std::vector<uint8_t> output1;
        encode_msgpack(byte_string_view(v1.data(),v1.size()),output1);
        CHECK(output1 == input1);
    }
    SECTION("[1]")
    {
        std::vector<uint8_t> expected = {1};

        std::vector<uint8_t> input1 = {0xc4,0x01,0x01};
        auto v1 = decode_msgpack<std::vector<uint8_t>>(input1);
        CHECK(v1 == expected);

        std::vector<uint8_t> input2 = {0xc5,0x00,0x01,0x01};
        auto v2 = decode_msgpack<std::vector<uint8_t>>(input2);
        CHECK(v2 == expected);

        std::vector<uint8_t> input3 = {0xc6,0x00,0x00,0x00,0x01,0x01};
        auto v3 = decode_msgpack<std::vector<uint8_t>>(input3);
        CHECK(v3 == expected);

        std::vector<uint8_t> output1;
        encode_msgpack(byte_string_view(v1.data(),v1.size()),output1);
        CHECK(output1 == input1);
    }
    SECTION("[0,255]")
    {
        std::vector<uint8_t> expected = {0,255};

        std::vector<uint8_t> input1 = {0xc4,0x02,0x00,0xff};
        auto v1 = decode_msgpack<std::vector<uint8_t>>(input1);
        CHECK(v1 == expected);

        std::vector<uint8_t> input2 = {0xc5,0x00,0x02,0x00,0xff};
        auto v2 = decode_msgpack<std::vector<uint8_t>>(input2);
        CHECK(v2 == expected);

        std::vector<uint8_t> input3 = {0xc6,0x00,0x00,0x00,0x02,0x00,0xff};
        auto v3 = decode_msgpack<std::vector<uint8_t>>(input3);
        CHECK(v3 == expected);

        std::vector<uint8_t> output1;
        encode_msgpack(byte_string_view(v1.data(),v1.size()),output1);
        CHECK(output1 == input1);
    }
}

TEST_CASE("msgpack ext tests")
{
    SECTION("fixext1, 1, [0x10]")
    {
        std::vector<uint8_t> expected = {0x10};

        std::vector<uint8_t> input1 = {0xd4,0x01,0x10};
        auto v1 = decode_msgpack<std::vector<uint8_t>>(input1);
        CHECK(v1 == expected);
    }
    SECTION("fixext2, 2, [20,21]")
    {
        std::vector<uint8_t> expected = {0x20,0x21};

        std::vector<uint8_t> input1 = {0xd5,0x02,0x20,0x21};
        auto v1 = decode_msgpack<std::vector<uint8_t>>(input1);

        CHECK((v1 == expected));
    }
    SECTION("fixext4, 3, [0x30,0x31,0x32,0x33]")
    {
        std::vector<uint8_t> expected = {0x30,0x31,0x32,0x33};

        std::vector<uint8_t> input1 = {0xd6,0x03,0x30,0x31,0x32,0x33};
        auto v1 = decode_msgpack<std::vector<uint8_t>>(input1);
        CHECK(v1 == expected);
    }
}

TEST_CASE("msgpack timestamp tests")
{
    SECTION("test 1")
    {
        std::vector<uint8_t> u = {0xce,0x5a,0x4a,0xf6,0xa5};
        uint64_t expected = decode_msgpack<uint64_t>(u);
        CHECK(expected == 1514862245);

        std::vector<uint8_t> input1 = {0xd6,0xff,0x5a,0x4a,0xf6,0xa5};
        auto r = decode_msgpack<uint64_t>(input1);

        CHECK(r == expected);
    }
    SECTION("test 2")
    {
        std::vector<uint64_t> expected = {1514862245,678901234};

        std::vector<uint8_t> input1 = {0xd7,0xff,0xa1,0xdc,0xd7,0xc8,0x5a,0x4a,0xf6,0xa5};
        auto r = decode_msgpack<std::vector<uint64_t>>(input1);

        CHECK(r == expected);
    }
}

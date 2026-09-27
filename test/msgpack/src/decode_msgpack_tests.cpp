// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif

#include <jsoncons_ext/msgpack/msgpack.hpp>

#include <jsoncons/json.hpp>

#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <catch/catch.hpp>

namespace msgpack = jsoncons::msgpack;

void check_decode_msgpack(const std::vector<uint8_t>& v, const jsoncons::json& expected)
{
    auto result = msgpack::decode_msgpack<jsoncons::json>(v);
    REQUIRE(result == expected);
}

TEST_CASE("decode_number_msgpack_test")
{
    // positive fixint 0x00 - 0x7f
    check_decode_msgpack({0x00},jsoncons::json(0U));
    check_decode_msgpack({0x01},jsoncons::json(1U));
    check_decode_msgpack({0x0a},jsoncons::json(10U));
    check_decode_msgpack({0x17},jsoncons::json(23U));
    check_decode_msgpack({0x18},jsoncons::json(24U));
    check_decode_msgpack({0x7f},jsoncons::json(127U)); 

    check_decode_msgpack({0xcc,0xff},jsoncons::json(255U));
    check_decode_msgpack({0xcd,0x01,0x00},jsoncons::json(256U));
    check_decode_msgpack({0xcd,0xff,0xff},jsoncons::json(65535U));
    check_decode_msgpack({0xce,0,1,0x00,0x00},jsoncons::json(65536U));
    check_decode_msgpack({0xce,0xff,0xff,0xff,0xff},jsoncons::json(4294967295U));
    check_decode_msgpack({0xcf,0,0,0,1,0,0,0,0},jsoncons::json(4294967296U));
    check_decode_msgpack({0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},jsoncons::json((std::numeric_limits<uint64_t>::max)()));

    check_decode_msgpack({0x01},jsoncons::json(1));
    check_decode_msgpack({0x0a},jsoncons::json(10));
    check_decode_msgpack({0x17},jsoncons::json(23)); 
    check_decode_msgpack({0x18},jsoncons::json(24)); 
    check_decode_msgpack({0x7f},jsoncons::json(127)); 

    check_decode_msgpack({0xcc,0xff},jsoncons::json(255));
    check_decode_msgpack({0xcd,0x01,0x00},jsoncons::json(256));
    check_decode_msgpack({0xcd,0xff,0xff},jsoncons::json(65535));
    check_decode_msgpack({0xce,0,1,0x00,0x00},jsoncons::json(65536));
    check_decode_msgpack({0xce,0xff,0xff,0xff,0xff},jsoncons::json(4294967295));
    check_decode_msgpack({0xd3,0,0,0,1,0,0,0,0},jsoncons::json(4294967296));
    check_decode_msgpack({0xd3,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff},jsoncons::json((std::numeric_limits<int64_t>::max)()));

    // negative fixint 0xe0 - 0xff
    check_decode_msgpack({0xe0},jsoncons::json(-32));
    check_decode_msgpack({0xff},jsoncons::json(-1)); //

    // negative integers
    check_decode_msgpack({0xd1,0xff,0},jsoncons::json(-256));
    check_decode_msgpack({0xd1,0xfe,0xff},jsoncons::json(-257));
    check_decode_msgpack({0xd2,0xff,0xff,0,0},jsoncons::json(-65536));
    check_decode_msgpack({0xd2,0xff,0xfe,0xff,0xff},jsoncons::json(-65537));
    check_decode_msgpack({0xd3,0xff,0xff,0xff,0xff,0,0,0,0},jsoncons::json(-4294967296));
    check_decode_msgpack({0xd3,0xff,0xff,0xff,0xfe,0xff,0xff,0xff,0xff},jsoncons::json(-4294967297));

    // null, true, false
    check_decode_msgpack({0xc0},jsoncons::json::null()); // 
    check_decode_msgpack({0xc3},jsoncons::json(true)); //
    check_decode_msgpack({0xc2},jsoncons::json(false)); //

    // floating point
    check_decode_msgpack({0xcb,0,0,0,0,0,0,0,0},jsoncons::json(0.0));
    check_decode_msgpack({0xcb,0xbf,0xf0,0,0,0,0,0,0},jsoncons::json(-1.0));
    check_decode_msgpack({0xcb,0xc1,0x6f,0xff,0xff,0xe0,0,0,0},jsoncons::json(-16777215.0));

    // string
    check_decode_msgpack({0xa0},jsoncons::json(""));
    check_decode_msgpack({0xa1,' '},jsoncons::json(" "));
    check_decode_msgpack({0xbf,'1','2','3','4','5','6','7','8','9','0',
                       '1','2','3','4','5','6','7','8','9','0',
                       '1','2','3','4','5','6','7','8','9','0',
                       '1'},
                 jsoncons::json("1234567890123456789012345678901"));
    check_decode_msgpack({0xd9,0x20,'1','2','3','4','5','6','7','8','9','0',
                            '1','2','3','4','5','6','7','8','9','0',
                            '1','2','3','4','5','6','7','8','9','0',
                            '1','2'},
                 jsoncons::json("12345678901234567890123456789012"));

}

TEST_CASE("decode_msgpack_arrays_and_maps")
{
    // fixarray
    check_decode_msgpack({0x90}, jsoncons::json(jsoncons::json_array_arg));
    check_decode_msgpack({0x80}, jsoncons::json(jsoncons::json_object_arg));

    check_decode_msgpack({0x91,'\0'},jsoncons::json::parse("[0]"));
    check_decode_msgpack({0x92,'\0','\0'}, jsoncons::json(jsoncons::json_array_arg, {0,0}));
    check_decode_msgpack({0x92,0x91,'\0','\0'}, jsoncons::json::parse("[[0],0]"));
    check_decode_msgpack({0x91,0xa5,'H','e','l','l','o'},jsoncons::json::parse("[\"Hello\"]"));

    check_decode_msgpack({0x81,0xa2,'o','c',0x91,'\0'}, jsoncons::json::parse("{\"oc\": [0]}"));
    check_decode_msgpack({0x81,0xa2,'o','c',0x94,'\0','\1','\2','\3'}, jsoncons::json::parse("{\"oc\": [0, 1, 2, 3]}"));
}

TEST_CASE("Compare msgpack packed item and jsoncons item")
{
    std::vector<uint8_t> bytes;
    msgpack::msgpack_bytes_encoder encoder(bytes);
    encoder.begin_array(2); // Must be definite length array
    encoder.string_value("foo");
    encoder.byte_string_value(jsoncons::byte_string{'b','a','r'});
    encoder.end_array();
    encoder.flush();

    jsoncons::json expected(jsoncons::json_array_arg);

    expected.emplace_back("foo");
    expected.emplace_back(jsoncons::byte_string{ 'b','a','r' });

    auto j = msgpack::decode_msgpack<jsoncons::json>(bytes);

    REQUIRE(expected == j);
}

TEST_CASE("decode msgpack from source")
{
    SECTION("from string")
    {
        std::vector<uint8_t> v = {0x91,0xa5,'H','e','l','l','o'};
        std::string s(reinterpret_cast<const char*>(v.data()),v.size());

        auto j = msgpack::decode_msgpack<jsoncons::json>(s);

        REQUIRE(1 == j.size());
        CHECK(j[0].as<std::string>() == std::string("Hello"));
    }
    SECTION("from vector iterator pair")
    {
        std::vector<uint8_t> v = {0x91,0xa5,'H','e','l','l','o'};

        auto j = msgpack::decode_msgpack<jsoncons::json>(v.begin(), v.end());

        REQUIRE(1 == j.size());
        CHECK(j[0].as<std::string>() == std::string("Hello"));
    }
}

TEST_CASE("decode msgpack str size tests")
{
    SECTION("str8")
    {
        std::string input = R"(
{"title": "Новое расписание на автобусных маршрутах №№8, 15, 64 будет действовать с 4.07.2016"}
        )";

        jsoncons::json j = jsoncons::json::parse(input);
        std::vector<uint8_t> buf;
        msgpack::encode_msgpack(j, buf);
        jsoncons::json other = msgpack::decode_msgpack<jsoncons::json>(buf);
        CHECK(j == other);
    }
    SECTION("str8 0")
    {
        std::string input = "{\"\":\"\"}";

        jsoncons::json j = jsoncons::json::parse(input);
        std::vector<uint8_t> buf;
        msgpack::encode_msgpack(j, buf);
        jsoncons::json other = msgpack::decode_msgpack<jsoncons::json>(buf);
        CHECK(j == other);
    }
    SECTION("str8 max")
    {
        std::string input = "{\"";
        input.append((std::numeric_limits<uint8_t>::max)(), '0');
        input.append("\":\"");
        input.append((std::numeric_limits<uint8_t>::max)(), '0');
        input.append("\"}");

        jsoncons::json j = jsoncons::json::parse(input);
        std::vector<uint8_t> buf;
        msgpack::encode_msgpack(j, buf);
        jsoncons::json other = msgpack::decode_msgpack<jsoncons::json>(buf);
        CHECK(j == other);
    }
    SECTION("str16 max")
    {
        std::string input = "{\"";
        input.append((std::numeric_limits<uint16_t>::max)(), '0');
        input.append("\":\"");
        input.append((std::numeric_limits<uint16_t>::max)(), '0');
        input.append("\"}");

        jsoncons::json j = jsoncons::json::parse(input);
        std::vector<uint8_t> buf;
        msgpack::encode_msgpack(j, buf);
        jsoncons::json other = msgpack::decode_msgpack<jsoncons::json>(buf);
        CHECK(j == other);
    }
    SECTION("str8 max (bytes)")
    {
        std::vector<uint8_t> in = {0xd9,0xff};
        in.insert(in.end(), (std::numeric_limits<uint8_t>::max)(), ' ');

        std::vector<uint8_t> out;
        msgpack::msgpack_bytes_encoder visitor(out);

        msgpack::msgpack_bytes_reader reader(in, visitor);
        reader.read();

        CHECK(in == out);
    }
    SECTION("str16 max (bytes)")
    {
        std::vector<uint8_t> in = {0xda,0xff,0xff};
        in.insert(in.end(), (std::numeric_limits<uint16_t>::max)(), ' ');

        std::vector<uint8_t> out;
        msgpack::msgpack_bytes_encoder visitor(out);

        msgpack::msgpack_bytes_reader reader(in, visitor);
        reader.read();

        CHECK(in == out);
    }
    SECTION("bin8 max (bytes)")
    {
        std::vector<uint8_t> in = {0xc4,0xff};
        in.insert(in.end(), (std::numeric_limits<uint8_t>::max)(), ' ');

        std::vector<uint8_t> out;
        msgpack::msgpack_bytes_encoder visitor(out);

        msgpack::msgpack_bytes_reader reader(in, visitor);
        reader.read();

        CHECK(in == out);
    }
    SECTION("bin16 max (bytes)")
    {
        std::vector<uint8_t> in = {0xc5,0xff,0xff};
        in.insert(in.end(), (std::numeric_limits<uint16_t>::max)(), ' ');

        std::vector<uint8_t> out;
        msgpack::msgpack_bytes_encoder visitor(out);

        msgpack::msgpack_bytes_reader reader(in, visitor);
        reader.read();

        CHECK(in == out);
    }
}



// Copyright 2016 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("cbor tagged array tests")
{
    SECTION("Tag 64 (uint8 Typed Array)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x40, // Tag 64, uint8, Typed Array
            0x43, // Byte string value of length 3
                0x00,0x01,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<uint8_t>() == std::numeric_limits<uint8_t>::lowest());
        CHECK(j[1].as<uint8_t>() == uint8_t(1));
        CHECK(j[2].as<uint8_t>() == (std::numeric_limits<uint8_t>::max)());
    }
    SECTION("Tags 65 (uint16, big endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x41, // Tag 65, uint16, Typed Array
            0x46, // Byte string value of length 6
                0x00,0x00,0x00,0x01,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<uint16_t>() == std::numeric_limits<uint16_t>::lowest());
        CHECK(j[1].as<uint16_t>() == uint16_t(1));
        CHECK(j[2].as<uint16_t>() == (std::numeric_limits<uint16_t>::max)());
    }
    SECTION("Tag 66 (uint32, big endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x42, // Tag 66, uint32, Typed Array
            0x4c, // Byte string value of length 12
                0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x01,
                0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<uint32_t>() == std::numeric_limits<uint32_t>::lowest());
        CHECK(j[1].as<uint32_t>() == uint32_t(1));
        CHECK(j[2].as<uint32_t>() == (std::numeric_limits<uint32_t>::max)());
    }
    SECTION("Tags 67 (uint64,big endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x43, // Tag 67, uint64, Typed Array
            0x58,0x18, // Byte string value of length 24
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<uint64_t>() == std::numeric_limits<uint64_t>::lowest());
        CHECK(j[1].as<uint64_t>() == uint64_t(1));
        CHECK(j[2].as<uint64_t>() == (std::numeric_limits<uint64_t>::max)());
    }
    SECTION("Tag 68 (uint8, Typed Array, clamped arithmetic)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x44, // Tag 68, uint8, Typed Array, clamped arithmetic
            0x43, // Byte string value of length 3
                0x00,0x01,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<uint8_t>() == std::numeric_limits<uint8_t>::lowest());
        CHECK(j[1].as<uint8_t>() == uint8_t(1));
        CHECK(j[2].as<uint8_t>() == (std::numeric_limits<uint8_t>::max)());
    }
    SECTION("Tags 69 (uint16, little endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x45, // Tag 69, uint16, Typed Array
            0x46, // Byte string value of length 6
                0x00,0x00,0x01,0x00,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<uint16_t>() == std::numeric_limits<uint16_t>::lowest());
        CHECK(j[1].as<uint16_t>() == uint16_t(1));
        CHECK(j[2].as<uint16_t>() == (std::numeric_limits<uint16_t>::max)());
    }
    SECTION("Tags 70 (uint32, little endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x46, // Tag 70, uint32, Typed Array
            0x4c, // Byte string value of length 12
                0x00,0x00,0x00,0x00,
                0x01,0x00,0x00,0x00,
                0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<uint32_t>() == std::numeric_limits<uint32_t>::lowest());
        CHECK(j[1].as<uint32_t>() == uint32_t(1));
        CHECK(j[2].as<uint32_t>() == (std::numeric_limits<uint32_t>::max)());
    }
    SECTION("Tag 71 (uint64,little endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x47, // Tag 71, uint64, Typed Array
            0x58,0x18, // Byte string value of length 24
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<uint64_t>() == std::numeric_limits<uint64_t>::lowest());
        CHECK(j[1].as<uint64_t>() == uint64_t(1));
        CHECK(j[2].as<uint64_t>() == (std::numeric_limits<uint64_t>::max)());
    }
    SECTION("Tag 72 (int8)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x48, // Tag 72, int8, Typed Array
            0x43, // Byte string value of length 3
                0x80,0x01,0x7f
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<int8_t>() == std::numeric_limits<int8_t>::lowest());
        CHECK(j[1].as<int8_t>() == int8_t(1));
        CHECK(j[2].as<int8_t>() == (std::numeric_limits<int8_t>::max)());
    }
    SECTION("Tag 73 (int16, big endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x49, // Tag 73, int16, Typed Array
            0x46, // Byte string value of length 6
                0x80,0x00,
                0x00,0x01,
                0x7f,0xff        
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<int16_t>() == std::numeric_limits<int16_t>::lowest());
        CHECK(j[1].as<int16_t>() == int16_t(1));
        CHECK(j[2].as<int16_t>() == (std::numeric_limits<int16_t>::max)());
    }
    SECTION("Tag 74 (int32, big endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x4a, // Tag 66, int32, Typed Array
            0x4c, // Byte string value of length 12
            0x80,0x00,0x00,0x00,
            0x00,0x00,0x00,0x01,
            0x7f,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<int32_t>() == std::numeric_limits<int32_t>::lowest());
        CHECK(j[1].as<int32_t>() == int32_t(1));
        CHECK(j[2].as<int32_t>() == (std::numeric_limits<int32_t>::max)());
    }
    SECTION("Tag 75 (int64,big endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x4b, // Tag 75, uint64, Typed Array
            0x58,0x18, // Byte string value of length 24
                0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,
                0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<int64_t>() == std::numeric_limits<int64_t>::lowest());
        CHECK(j[1].as<int64_t>() == int64_t(1));
        CHECK(j[2].as<int64_t>() == (std::numeric_limits<int64_t>::max)());
    }
    SECTION("Tag 77 (int16, big endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x4d, // Tag 77, int16, Typed Array
            0x46, // Byte string value of length 6
                0x00,0x80,
                0x01,0x00,
                0xff,0x7f
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<int16_t>() == std::numeric_limits<int16_t>::lowest());
        CHECK(j[1].as<int16_t>() == int16_t(1));
        CHECK(j[2].as<int16_t>() == (std::numeric_limits<int16_t>::max)());
    }
    SECTION("Tags 78 (int32, little endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x4e, // Tag 78, int32, Typed Array
            0x4c, // Byte string value of length 12
                0x00,0x00,0x00,0x80,
                0x01,0x00,0x00,0x00,
                0xff,0xff,0xff,0x7f
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<int32_t>() == std::numeric_limits<int32_t>::lowest());
        CHECK(j[1].as<int32_t>() == int32_t(1));
        CHECK(j[2].as<int32_t>() == (std::numeric_limits<int32_t>::max)());
    }
    SECTION("Tag 79 (int64,little endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x4f, // Tag 79, uint64, Typed Array
            0x58,0x18, // Byte string value of length 24
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80,
                0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<int64_t>() == std::numeric_limits<int64_t>::lowest());
        CHECK(j[1].as<int64_t>() == int64_t(1));
        CHECK(j[2].as<int64_t>() == (std::numeric_limits<int64_t>::max)());
    }
    SECTION("Tag 81, float32, big endian")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x51, // Tag 81, float32, big endian, Typed Array
            0x48, // Byte string value of length 8
                0xff,0x7f,0xff,0xff,
                0x7f,0x7f,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 2);
        CHECK(j[0].as<float>() == std::numeric_limits<float>::lowest());
        CHECK(j[1].as<float>() == (std::numeric_limits<float>::max)());
    }
    SECTION("Tag 82, float64, big endian")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x52, // Tag 82, float64, big endian, Typed Array
            0x50, // Byte string value of length 16
                0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0x7f, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 2);
        CHECK(j[0].as<double>() == std::numeric_limits<double>::lowest());
        CHECK(j[1].as<double>() == (std::numeric_limits<double>::max)());
    }
#if 0
    SECTION("Tag 83, float128, big endian")
    {
        std::cout << "sizeof(long double): " << sizeof(long double) << "\n";

        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x53, // Tag 83, float128, little endian, Typed Array
            0x58,0x20, // Byte string value of length 32
                0xff,0xef,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                0x7f,0xef,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 2);

        std::cout << pretty_print(j) << "\n";
        //CHECK(j[0].as<double>() == std::numeric_limits<double>::lowest());
        //CHECK(j[1].as<double>() == (std::numeric_limits<double>::max)());
    }
#endif
    SECTION("Tag 85, float32, little endian")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x55, // Tag 85, float64, little endian, Typed Array
            0x48, // Byte string value of length 8
                0xff,0xff,0x7f,0xff,
                0xff,0xff,0x7f,0x7f 
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 2);
        CHECK(j[0].as<float>() == std::numeric_limits<float>::lowest());
        CHECK(j[1].as<float>() == (std::numeric_limits<float>::max)());
    }
    SECTION("Tag 86, float64, little endian")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x56, // Tag 86, float64, little endian, Typed Array
            0x50, // Byte string value of length 16
                0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,
                0xff,0xff,0xff,0xff,0xff,0xff,0xef,0x7f
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 2);
        CHECK(j[0].as<double>() == std::numeric_limits<double>::lowest());
        CHECK(j[1].as<double>() == (std::numeric_limits<double>::max)());
    }
#if 0
    SECTION("Tag 87, float128, little endian")
    {
        std::cout << "sizeof(long double): " << sizeof(long double) << "\n";

        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x57, // Tag 87, float128, little endian, Typed Array
            0x58,0x20, // Byte string value of length 32
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xef,0x7f
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 2);

        std::cout << pretty_print(j) << "\n";
        //CHECK(j[0].as<double>() == std::numeric_limits<double>::lowest());
        //CHECK(j[1].as<double>() == (std::numeric_limits<double>::max)());
    }
#endif
} 


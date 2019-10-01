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
    SECTION("Tag 81, float32, big endian")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x51, // Tag 81, float32, big endian, typed array
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
                0x52, // Tag 82, float64, big endian, typed array
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
                0x53, // Tag 83, float128, little endian, typed array
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
                0x55, // Tag 85, float64, little endian, typed array
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
                0x56, // Tag 86, float64, little endian, typed array
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
                0x57, // Tag 87, float128, little endian, typed array
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


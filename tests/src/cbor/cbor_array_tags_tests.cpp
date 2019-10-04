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
        std::cout << (int)detail::endian::native << "\n";

        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x40, // Tag 64, uint8, Typed Array
            0x43, // Byte string value of length 3
                0x00,0x01,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 64\n" << pretty_print(j) << "\n";
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
                0x41, // Tag 65, uint16, big endian, Typed Array
            0x46, // Byte string value of length 6
                0x00,0x00,0x00,0x01,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 65\n" << pretty_print(j) << "\n";
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
                0x42, // Tag 66, uint32, big endian, Typed Array
            0x4c, // Byte string value of length 12
                0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x01,
                0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 66\n" << pretty_print(j) << "\n";
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
                0x43, // Tag 67, uint64, big endian, Typed Array
            0x58,0x18, // Byte string value of length 24
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 67\n" << pretty_print(j) << "\n";
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
        std::cout << "Tag 68\n" << pretty_print(j) << "\n";
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
                0x45, // Tag 69, uint16, little endian, Typed Array
            0x46, // Byte string value of length 6
                0x00,0x00,0x01,0x00,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 69\n" << pretty_print(j) << "\n";
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
                0x46, // Tag 70, uint32, little endian, Typed Array
            0x4c, // Byte string value of length 12
                0x00,0x00,0x00,0x00,
                0x01,0x00,0x00,0x00,
                0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 70\n" << pretty_print(j) << "\n";
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
                0x47, // Tag 71, uint64, little endian, Typed Array
            0x58,0x18, // Byte string value of length 24
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 71\n" << pretty_print(j) << "\n";
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
                0x48, // Tag 72, sint8, Typed Array
            0x43, // Byte string value of length 3
                0x80,0x01,0x7f
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 72\n" << pretty_print(j) << "\n";
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
                0x49, // Tag 73, sint16, big endian, Typed Array
            0x46, // Byte string value of length 6
                0x80,0x00,
                0x00,0x01,
                0x7f,0xff        
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 73\n" << pretty_print(j) << "\n";
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
                0x4a, // Tag 74, sint32, big endian, Typed Array
            0x4c, // Byte string value of length 12
            0x80,0x00,0x00,0x00,
            0x00,0x00,0x00,0x01,
            0x7f,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 74\n" << pretty_print(j) << "\n";
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
                0x4b, // Tag 75, sint64, big endian, Typed Array
            0x58,0x18, // Byte string value of length 24
                0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,
                0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 75\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<int64_t>() == std::numeric_limits<int64_t>::lowest());
        CHECK(j[1].as<int64_t>() == int64_t(1));
        CHECK(j[2].as<int64_t>() == (std::numeric_limits<int64_t>::max)());
    }
    SECTION("Tag 77 (int16, little endian)")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x4d, // Tag 77, sint16, little endian, Typed Array
            0x46, // Byte string value of length 6
                0x00,0x80,
                0x01,0x00,
                0xff,0x7f
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 77\n" << pretty_print(j) << "\n";
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
                0x4e, // Tag 78, sint32, little endian, Typed Array
            0x4c, // Byte string value of length 12
                0x00,0x00,0x00,0x80,
                0x01,0x00,0x00,0x00,
                0xff,0xff,0xff,0x7f
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 78\n" << pretty_print(j) << "\n";
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
                0x4f, // Tag 79, sint64, little endian, Typed Array
            0x58,0x18, // Byte string value of length 24
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80,
                0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 79\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 3);
        CHECK(j[0].as<int64_t>() == std::numeric_limits<int64_t>::lowest());
        CHECK(j[1].as<int64_t>() == int64_t(1));
        CHECK(j[2].as<int64_t>() == (std::numeric_limits<int64_t>::max)());
    }
    SECTION("Tag 80, float16, big endian")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x50, // Tag 80, float16, big endian, Typed Array
            0x48, // Byte string value of length 8
                0x00,0x01,
                0x03,0xff,
                0x04,0x00,
                0x7b,0xff
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 80\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 4);
        CHECK(j[0].as<float>() == Approx(0.000000059605).epsilon(0.00001));
        CHECK(j[1].as<float>() == Approx(0.000060976).epsilon(0.00001));
        CHECK(j[2].as<float>() == Approx(0.000061035).epsilon(0.00001));
        CHECK(j[3].as<float>() == 65504);
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
        std::cout << "Tag 81\n" << pretty_print(j) << "\n";
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
        std::cout << "Tag 82\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 2);
        CHECK(j[0].as<double>() == std::numeric_limits<double>::lowest());
        CHECK(j[1].as<double>() == (std::numeric_limits<double>::max)());
    }

    SECTION("Tag 83, float128, big endian")
    {
        std::cout << "sizeof(long double): " << sizeof(long double) << "\n";
        const uint8_t* endp;

        uint8_t buf[sizeof(long double)];
        auto x = std::numeric_limits<long double>::lowest();
        auto y = (std::numeric_limits<long double>::max)();
        memcpy(buf,&x,sizeof(long double));
        for (size_t i = sizeof(buf) -1; i+1 > 0; --i)
        {
            std::cout << std::hex << (int)buf[i] << " ";
        }
        std::cout << "\n";
        auto val1 = jsoncons::detail::little_to_native<long double>(buf, buf + sizeof(long double), &endp);
        std::cout << "float 128 val1: " << val1 << "\n";

        memcpy(buf,&y,sizeof(long double));
        for (size_t i = sizeof(buf) -1; i+1 > 0; --i)
        {
            std::cout << std::hex << (int)buf[i] << " ";
        }
        auto val2 = jsoncons::detail::little_to_native<long double>(buf, buf + sizeof(long double), &endp);
        std::cout << "float 128 val2: " << val1 << "\n";

        long double w = static_cast<long double>(std::numeric_limits<double>::lowest());
        long double z = static_cast<long double>((std::numeric_limits<double>::max)());
        std::cout << "\n\n";
        memcpy(buf,&w,sizeof(long double));
        for (size_t i = sizeof(buf)-1; i+1 > 0; --i)
        {
            std::cout << std::hex << (int)buf[i] << " ";
        }
        std::cout << "\n\n";
        auto val3 = jsoncons::detail::little_to_native<long double>(buf, buf + sizeof(long double), &endp);
        std::cout << "float 128 val3: " << val3 << "\n";
        memcpy(buf,&z,sizeof(long double));
        for (size_t i = sizeof(buf) -1; i+1 > 0; --i)
        {
            std::cout << std::hex << (int)buf[i] << " ";
        }
        std::cout << "\n\n";

        auto val4 = jsoncons::detail::little_to_native<long double>(buf, buf + sizeof(long double), &endp);
        std::cout << "float 128 val4: " << val4 << "\n";

        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x53, // Tag 83, float128, little endian, Typed Array
            0x58,0x40, // Byte string value of length 64
                0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xfe,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xfe,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xef,0xff,0xff,0xff,0xff,0xff,0xff,
                0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0xfe,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff 
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        //REQUIRE(j.size() == 2);

        std::cout << "Tag 83\n" << pretty_print(j) << "\n";
        //CHECK(j[0].as<double>() == std::numeric_limits<double>::lowest());
        //CHECK(j[1].as<double>() == (std::numeric_limits<double>::max)());
    }

    SECTION("Tag 84, float16, little endian")
    {
        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x54, // Tag 84, float16, little endian, Typed Array
            0x48, // Byte string value of length 8
                0x01,0x00,
                0xff,0x03,
                0x00,0x04,
                0xff,0x7b
        };

        json j = cbor::decode_cbor<json>(input);
        std::cout << "Tag 84\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 4);
        CHECK(j[0].as<float>() == Approx(0.000000059605).epsilon(0.00001));
        CHECK(j[1].as<float>() == Approx(0.000060976).epsilon(0.00001));
        CHECK(j[2].as<float>() == Approx(0.000061035).epsilon(0.00001));
        CHECK(j[3].as<float>() == 65504);
    }
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
        std::cout << "Tag 85\n" << pretty_print(j) << "\n";
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
        std::cout << "Tag 86\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(j.size() == 2);
        CHECK(j[0].as<double>() == std::numeric_limits<double>::lowest());
        CHECK(j[1].as<double>() == (std::numeric_limits<double>::max)());
    }

    SECTION("Tag 87, float128, little endian")
    {
        std::cout << "sizeof(long double): " << sizeof(long double) << "\n";

        const uint8_t* endp = nullptr;

        uint8_t buf[sizeof(long double)];
        auto x = std::numeric_limits<long double>::lowest();
        auto y = (std::numeric_limits<long double>::max)();
        memcpy(buf,&x,sizeof(long double));
        for (auto b : buf)
        {
            std::cout << std::hex << (int)b << " ";
        }
        std::cout << "\n";
        auto val1 = jsoncons::detail::little_to_native<long double>(buf, buf + sizeof(long double), &endp);
        std::cout << "float 128 val1: " << val1 << "\n";

        memcpy(buf,&y,sizeof(long double));
        for (auto b : buf)
        {
            std::cout << std::hex << (int)b << " ";
        }
        auto val2 = jsoncons::detail::little_to_native<long double>(buf, buf + sizeof(long double), &endp);
        std::cout << "float 128 val2: " << val2 << "\n";

        long double w = static_cast<long double>(std::numeric_limits<double>::lowest());
        long double z = static_cast<long double>((std::numeric_limits<double>::max)());
        std::cout << "\n\n";
        memcpy(buf,&w,sizeof(long double));
        for (auto b : buf)
        {
            std::cout << std::hex << (int)b << " ";
        }
        auto val3 = jsoncons::detail::little_to_native<long double>(buf, buf + sizeof(long double), &endp);
        std::cout << "float 128 val3: " << val3 << "\n";
        std::cout << "\n\n";
        memcpy(buf,&z,sizeof(long double));
        for (auto b : buf)
        {
            std::cout << std::hex << (int)b << " ";
        }
        std::cout << "\n\n";

        auto val4 = jsoncons::detail::little_to_native<long double>(buf, buf + sizeof(long double), &endp);
        std::cout << "float 128 val4: " << val4 << "\n";

        const std::vector<uint8_t> input = {
            0xD8, // Tag
                0x57, // Tag 87, float128, little endian, Typed Array
            0x58,0x40, // Byte string value of length 64
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0xff,0x00,0x00,0x00,0x00,0x00,0x00,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0x7f,0x00,0x00,0x00,0x00,0x00,0x00,
                0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0x7f,0x00,0x00,0x00,0x00,0x00,0x00
        };

        json j = cbor::decode_cbor<json>(input);
        REQUIRE(j.is_array());
        //REQUIRE(j.size() == 2);

        std::cout << "Tag 87\n" << pretty_print(j) << "\n";
        //CHECK(j[0].as<double>() == std::numeric_limits<double>::lowest());
        //CHECK(j[1].as<double>() == (std::numeric_limits<double>::max)());
    }

} 


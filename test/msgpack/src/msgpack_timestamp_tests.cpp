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
#include <iostream>
#include <catch/catch.hpp>

using namespace jsoncons;
using namespace jsoncons::msgpack;

TEST_CASE("msgpack timestamp tests")
{
    SECTION("test 1")
    {
        std::vector<uint8_t> u = {0xce,0x5a,0x4a,0xf6,0xa5};
        uint64_t expected = decode_msgpack<uint64_t>(u);
        CHECK(expected == 1514862245);

        std::vector<uint8_t> input = {0xd6,0xff,0x5a,0x4a,0xf6,0xa5};
        auto r = decode_msgpack<uint64_t>(input);

        CHECK(expected == r);

        std::vector<uint8_t> output;
        jsoncons::json j = decode_msgpack<jsoncons::json>(input);
        encode_msgpack(j,output);
        CHECK(output == input);
    }
    SECTION("test 2")
    {
        std::vector<uint64_t> expected = {1514862245,678901234};

        std::vector<uint8_t> input = {0xd7,0xff,0xa1,0xdc,0xd7,0xc8,0x5a,0x4a,0xf6,0xa5};
        auto r = decode_msgpack<jsoncons::json>(input);
        std::cout << pretty_print(r) << "\n\n";

        //CHECK(expected == r);
    }
    SECTION("test 3")
    {
        std::vector<int64_t> expected = {-int64_t(2208988801),999999999};

        std::vector<uint8_t> input = {0xc7,0x0c,0xff,0x3b,0x9a,0xc9,0xff,0xff,0xff,0xff,0xff,0x7c,0x55,0x81,0x7f};
        auto r = decode_msgpack<jsoncons::json>(input);
        std::cout << pretty_print(r) << "\n\n";

        CHECK(r.as_string() == "-2208988800000000001");

        std::vector<uint8_t> output;
        json j = decode_msgpack<jsoncons::json>(input);
        encode_msgpack(j,output);
        CHECK(output == input);
    }
    SECTION("test 4")
    {
        std::vector<uint64_t> expected = {2147483648,1};

        std::vector<uint8_t> input = {0xd7,0xff,0x00,0x00,0x00,0x04,0x80,0x00,0x00,0x00};
        auto r = decode_msgpack<jsoncons::json>(input);
        std::cout << r << "\n\n";

        //CHECK(expected == r);

        //std::vector<uint8_t> output;
        //json j = decode_msgpack<jsoncons::json>(input);
        //encode_msgpack(j,output);
        //CHECK(output == input);
    }

    SECTION("test 5")
    {
        std::vector<int64_t> expected = {-int64_t(2208988801),999999999};

        std::vector<uint8_t> input = {
            0xc7,0x0c,0xff, // timestamp 96
            0x3b,0x9a,0xc9,0xff, // 999999999 nanoseconds in 32-bit unsigned int
            0xff,0xff,0xff,0xff,0x7c,0x55,0x81,0x7f // -2208988801 seconds in 64-bit signed int
        };

        auto j = decode_msgpack<jsoncons::json>(input);
        std::cout << "j: " << j << "\n\n";
        auto milliseconds = j.as<std::chrono::milliseconds>();
        std::cout << "milliseconds elapsed since 1970-01-01 00:00:00 UTC: " << milliseconds.count() << "\n";

        std::vector<uint8_t> data;
        msgpack::encode_msgpack(milliseconds, data);
        std::cout << "MessagePack bytes:\n" << jsoncons::byte_string_view(data) << "\n\n";
        auto j2 = decode_msgpack<jsoncons::json>(data);
        std::cout << "j2: " << j2 << "\n\n";
    }
}

TEST_CASE("msgpack timestamp before 1970")
{
    SECTION("epoch_nano -1")
    {
        std::vector<uint8_t> expected = {
            0xc7,0x0c,0xff, // timestamp 96
            0x3b,0x9a,0xc9,0xff, // 999999999 nanoseconds in 32-bit unsigned int
            0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff // -1 seconds in 64-bit signed int
        };

        std::vector<uint8_t> output;
        encode_msgpack(jsoncons::json(-1, semantic_tag::epoch_nano), output);
        CHECK(output == expected);
        CHECK(decode_msgpack<jsoncons::json>(output).as_string() == "-1");
    }
    SECTION("epoch_milli -500")
    {
        std::vector<uint8_t> expected = {
            0xc7,0x0c,0xff, // timestamp 96
            0x1d,0xcd,0x65,0x00, // 500000000 nanoseconds in 32-bit unsigned int
            0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff // -1 seconds in 64-bit signed int
        };

        std::vector<uint8_t> output;
        encode_msgpack(jsoncons::json(-500, semantic_tag::epoch_milli), output);
        CHECK(output == expected);
        output.clear();
        encode_msgpack(jsoncons::json("-500", semantic_tag::epoch_milli), output);
        CHECK(output == expected);
        CHECK(decode_msgpack<jsoncons::json>(output).as_string() == "-500000000");
    }
}

TEST_CASE("msgpack timestamp seconds outside int64_t")
{
    std::vector<uint8_t> output;
    msgpack_bytes_encoder encoder(output);
    std::error_code ec;

    SECTION("epoch_nano above")
    {
        encoder.string_value("9223372036854775808000000000", semantic_tag::epoch_nano, ser_context(), ec);
        CHECK(ec == msgpack_errc::invalid_timestamp);
    }
    SECTION("epoch_nano below")
    {
        encoder.string_value("-9223372036854775808000000001", semantic_tag::epoch_nano, ser_context(), ec);
        CHECK(ec == msgpack_errc::invalid_timestamp);
    }
    SECTION("epoch_milli below")
    {
        encoder.string_value("-9223372036854775809000", semantic_tag::epoch_milli, ser_context(), ec);
        CHECK(ec == msgpack_errc::invalid_timestamp);
    }
    SECTION("epoch_second uint64")
    {
        encoder.uint64_value(9223372036854775808u, semantic_tag::epoch_second, ser_context(), ec);
        CHECK(ec == msgpack_errc::invalid_timestamp);
    }
    SECTION("epoch_nano largest")
    {
        std::vector<uint8_t> expected = {
            0xc7,0x0c,0xff, // timestamp 96
            0x3b,0x9a,0xc9,0xff, // 999999999 nanoseconds in 32-bit unsigned int
            0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff // 9223372036854775807 seconds in 64-bit signed int
        };
        encoder.string_value("9223372036854775807999999999", semantic_tag::epoch_nano, ser_context(), ec);
        encoder.flush();
        CHECK_FALSE(ec);
        CHECK(output == expected);
    }
    SECTION("epoch_nano uint64 above INT64_MAX")
    {
        std::vector<uint8_t> expected = {
            0xd7,0xff, // timestamp 64
            0xee,0x6b,0x27,0xff,0xff,0xff,0xff,0xff // 999999999 nanoseconds, 17179869183 seconds
        };
        encoder.uint64_value(17179869183999999999u, semantic_tag::epoch_nano, ser_context(), ec);
        encoder.flush();
        CHECK_FALSE(ec);
        CHECK(output == expected);
    }
    SECTION("epoch_milli uint64 above INT64_MAX")
    {
        encoder.uint64_value(18446744073709551615u, semantic_tag::epoch_milli, ser_context(), ec);
        encoder.flush();
        CHECK_FALSE(ec);
        CHECK(decode_msgpack<jsoncons::json>(output).as_string() == "18446744073709551615000000");
    }
}

TEST_CASE("msgpack timestamp in array and map")
{
    std::vector<uint8_t> input = {
        0x92, // array, length 2
          0xd7,0xff, // timestamp 64
            0xa1,0xdc,0xd7,0xc8,0x5a,0x4a,0xf6,0xa5, // 678901234 nanoseconds, 1514862245 seconds
          0x81, // map, length 1
            0xa1,'t', // string, length 1
            0xc7,0x0c,0xff, // timestamp 96
              0x00,0x00,0x00,0x00, // 0 nanoseconds in 32-bit unsigned int
              0xff,0xff,0xff,0xff,0x7c,0x55,0x81,0x80 // -2208988800 seconds in 64-bit signed int
    };

    auto j = decode_msgpack<jsoncons::json>(input);
    std::vector<uint8_t> output;
    REQUIRE_NOTHROW(encode_msgpack(j, output));
    CHECK(output == input);
}


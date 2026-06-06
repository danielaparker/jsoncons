// Copyright 2013-2026 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif

#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/trace_json_visitor.hpp>

#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <catch/catch.hpp>
#include <iostream>

using namespace jsoncons;
namespace cbor = jsoncons::cbor;

static void check_native(std::true_type, 
                         const std::vector<uint8_t>& u,
                         const std::vector<uint8_t>& v)
{
    if (u != v)
    {
        for (auto c : u)
        {
            std::cout << std::hex  << std::setw(2) << std::setfill('0') << (int)c << " ";
        }
        std::cout << "\n";
        for (auto c : v)
        {
            std::cout << std::hex  << std::setw(2) << std::setfill('0') << (int)c << " ";
        }
        std::cout << "\n\n";
    }
    CHECK((u == v));
}

static void check_native(std::false_type, 
                         const std::vector<uint8_t>&,
                         const std::vector<uint8_t>&)
{
}

TEST_CASE("cbor typed_array mdarray tests")
{
    SECTION("row major 2d")
    {
        // [[1 2 3 4 5 6 ][7 8 9 10 11 12 ]]
        std::error_code ec;

        std::vector<int> v = {1,2,3,4,5,6,7,8,9,10,11,12};
        std::vector<std::size_t> extents = { 2,6 };

        cbor::mdarray_iterator<int> iter(v, extents);
        jsoncons::json_decoder<jsoncons::json> decoder;
        while (!iter.done())
        {
            iter.next(decoder, jsoncons::ser_context{}, ec);
        }
        std::cout << decoder.get_result() << "\n\n";
    }
    SECTION("row major 3d")
    {
        // [[[1 2 ][3 4 ][5 6 ]][[7 8 ][9 10 ][11 12 ]]]
        std::error_code ec;

        std::vector<int> v = {1,2,3,4,5,6,7,8,9,10,11,12};
        std::vector<std::size_t> extents = { 2,3,2 };

        cbor::mdarray_iterator<int> iter(v, extents);
        jsoncons::json_decoder<jsoncons::json> decoder;
        while (!iter.done())
        {
            iter.next(decoder, jsoncons::ser_context{}, ec);
        }
        std::cout << decoder.get_result() << "\n\n";
    }
    SECTION("column major 2d")
    {
        std::string expected = "[[1 3 5 7 9 11 ][2 4 6 8 10 12 ]]";
        std::error_code ec;

        std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        std::vector<std::size_t> extents = {2, 6};

        std::cout << "\nexpected: " << expected << "\n";
        cbor::mdarray_iterator<int> iter(v, extents, jsoncons::mdarray_order::column_major);
        jsoncons::json_decoder<jsoncons::json> decoder;
        while (!iter.done())
        {
            iter.next(decoder, jsoncons::ser_context{}, ec);
        }
        std::cout << decoder.get_result() << "\n\n";
    }
    SECTION("column major 3d")
    {
        std::string expected = "[[[1 7 ][3 9 ][5 11 ]][[2 8 ][4 10 ][6 12 ]]]";
        std::error_code ec;

        std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        std::vector<std::size_t> extents = {2, 3, 2};

        std::cout << "\nexpected: " << expected << "\n";
        cbor::mdarray_iterator<int> iter(v, extents, jsoncons::mdarray_order::column_major);
        jsoncons::json_decoder<jsoncons::json> decoder;
        while (!iter.done())
        {
            iter.next(decoder, jsoncons::ser_context{}, ec);
        }
        std::cout << decoder.get_result() << "\n\n";
    }

}

TEST_CASE("cbor typed array cursor tests")
{
    SECTION("Tag 86, float64, little endian")
    {
        //std::cout << "CBOR cursor typed array Tag 86, float64, little endian" << '\n';

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x56, // Tag 86, float64, little endian, typed array
            0x50, // Byte string value of length 16
                0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,
                0xff,0xff,0xff,0xff,0xff,0xff,0xef,0x7f
        };

        cbor::cbor_bytes_cursor cursor(data);
        CHECK(staj_events::begin_array == cursor.current().event_type());
        CHECK(cursor.is_typed_array());

        std::vector<double> v;
        cursor.read_typed_array(v);
        REQUIRE(2 == v.size());
        CHECK( -1.79769e+308 == Approx(v[0]));
        CHECK(1.79769e+308 == Approx(v[1]));
    }
    SECTION("Tag 86, float64, little endian, read_typed_array")
    {
        //std::cout << "CBOR cursor typed array Tag 86, float64, little endian" << '\n';

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x56, // Tag 86, float64, little endian, typed array
            0x50, // Byte string value of length 16
                0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,
                0xff,0xff,0xff,0xff,0xff,0xff,0xef,0x7f
        };

        cbor::cbor_bytes_cursor cursor(data);
        CHECK(staj_events::begin_array == cursor.current().event_type());
        CHECK(cursor.is_typed_array());

        std::vector<double> v1;
        cursor.read_typed_array(v1);
        REQUIRE(2 == v1.size());
        CHECK( -1.79769e+308 == Approx(v1[0]));
        CHECK(1.79769e+308 == Approx(v1[1]));
    }
}

TEST_CASE("cbor typed array tests")
{
    SECTION("Tag 64 (uint8 typed array)")
    {
        //std::cout << "CBOR typed array Tag 64 (uint8 typed array)" << '\n';

        //std::cout << (int)detail::endian::native << "\n";

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x40, // Tag 64, uint8, typed array
            0x43, // Byte string value of length 3
                0x00,0x01,0xff
        };

        auto u = cbor::decode_cbor<std::vector<uint8_t>>(data);
        std::vector<uint8_t> v;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, v, options);
        CHECK((v == data));
    }
    SECTION("Tags 65 (uint16, big endian)")
    {
        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x41, // Tag 65, uint16, big endian, typed array
            0x46, // Byte string value of length 6
                0x00,0x00,0x00,0x01,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 65\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<uint16_t>() == std::numeric_limits<uint16_t>::lowest());
        CHECK(j[1].as<uint16_t>() == uint16_t(1));
        CHECK(j[2].as<uint16_t>() == (std::numeric_limits<uint16_t>::max)());

        auto u = cbor::decode_cbor<std::vector<uint16_t>>(data);
        std::vector<uint8_t> v;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, v, options);
        check_native(std::integral_constant<bool,jsoncons::endian::native == jsoncons::endian::big>(),
                     data, v);
    }
    SECTION("Tag 66 (uint32, big endian)")
    {
        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x42, // Tag 66, uint32, big endian, typed array
            0x4c, // Byte string value of length 12
                0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x01,
                0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 66\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<uint32_t>() == std::numeric_limits<uint32_t>::lowest());
        CHECK(j[1].as<uint32_t>() == uint32_t(1));
        CHECK(j[2].as<uint32_t>() == (std::numeric_limits<uint32_t>::max)());

        auto u = cbor::decode_cbor<std::vector<uint32_t>>(data);
        std::vector<uint8_t> v;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, v, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::big>(),
            data, v);
    }

    SECTION("Tags 67 (uint64,big endian)")
    {
        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x43, // Tag 67, uint64, big endian, typed array
            0x58,0x18, // Byte string value of length 24
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 67\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<uint64_t>() == std::numeric_limits<uint64_t>::lowest());
        CHECK(j[1].as<uint64_t>() == uint64_t(1));
        CHECK(j[2].as<uint64_t>() == (std::numeric_limits<uint64_t>::max)());

        auto u = cbor::decode_cbor<std::vector<uint64_t>>(data);
        std::vector<uint8_t> v;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, v, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::big>(),
            data, v);
    }

    SECTION("Tag 68 (uint8, typed array, clamped arithmetic)")
    {
        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x44, // Tag 68, uint8, typed array, clamped arithmetic
            0x43, // Byte string value of length 3
                0x00,0x01,0xff
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 68\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        CHECK(j.tag() == semantic_tag::clamped);
        REQUIRE(3 == j.size());
        CHECK(j[0].as<uint8_t>() == std::numeric_limits<uint8_t>::lowest());
        CHECK(j[1].as<uint8_t>() == uint8_t(1));
        CHECK(j[2].as<uint8_t>() == (std::numeric_limits<uint8_t>::max)());

        auto v = cbor::decode_cbor<std::vector<uint8_t>>(data);
        REQUIRE(3 == v.size());
        CHECK(v[0] == std::numeric_limits<uint8_t>::lowest());
        CHECK(v[1] == uint8_t(1));
        CHECK(v[2] == (std::numeric_limits<uint8_t>::max)());
    }

    SECTION("Tags 69 (uint16, little endian)")
    {
        //std::cout << "CBOR typed array Tags 69 (uint16, little endian)" << '\n';

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x45, // Tag 69, uint16, little endian, typed array
            0x46, // Byte string value of length 6
                0x00,0x00,0x01,0x00,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 69\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<uint16_t>() == std::numeric_limits<uint16_t>::lowest());
        CHECK(j[1].as<uint16_t>() == uint16_t(1));
        CHECK(j[2].as<uint16_t>() == (std::numeric_limits<uint16_t>::max)());

        auto u = cbor::decode_cbor<std::vector<uint16_t>>(data);
        REQUIRE(3 == u.size());
        CHECK(u[0] == std::numeric_limits<uint16_t>::lowest());
        CHECK(u[1] == uint16_t(1));
        CHECK(u[2] == (std::numeric_limits<uint16_t>::max)());

        auto v = cbor::decode_cbor<std::vector<uint32_t>>(data);
        REQUIRE(3 == v.size());
        CHECK(v[0] == std::numeric_limits<uint16_t>::lowest());
        CHECK(v[1] == uint16_t(1));
        CHECK(v[2] == (std::numeric_limits<uint16_t>::max)());

        auto w = cbor::decode_cbor<std::vector<uint64_t>>(data);
        REQUIRE(3 == w.size());
        CHECK(w[0] == std::numeric_limits<uint16_t>::lowest());
        CHECK(w[1] == uint16_t(1));
        CHECK(w[2] == (std::numeric_limits<uint16_t>::max)());

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::little>(),
            data, buf);
    }

    SECTION("Tags 70 (uint32, little endian)")
    {
        //std::cout << "CBOR typed array Tags 70 (uint32, little endian)" << '\n';

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x46, // Tag 70, uint32, little endian, typed array
            0x4c, // Byte string value of length 12
                0x00,0x00,0x00,0x00,
                0x01,0x00,0x00,0x00,
                0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 70\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<uint32_t>() == std::numeric_limits<uint32_t>::lowest());
        CHECK(j[1].as<uint32_t>() == uint32_t(1));
        CHECK(j[2].as<uint32_t>() == (std::numeric_limits<uint32_t>::max)());

        auto u = cbor::decode_cbor<std::vector<uint32_t>>(data);
        REQUIRE(3 == u.size());
        CHECK(u[0] == std::numeric_limits<uint32_t>::lowest());
        CHECK(u[1] == uint32_t(1));
        CHECK(u[2] == (std::numeric_limits<uint32_t>::max)());

        auto v = cbor::decode_cbor<std::vector<uint64_t>>(data);
        REQUIRE(3 == v.size());
        CHECK(v[0] == std::numeric_limits<uint32_t>::lowest());
        CHECK(v[1] == uint32_t(1));
        CHECK(v[2] == (std::numeric_limits<uint32_t>::max)());

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::little>(),
            data, buf);
    }

    SECTION("Tag 71 (uint64,little endian)")
    {
        //std::cout << "CBOR typed array Tag 71 (uint64,little endian)" << '\n';

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x47, // Tag 71, uint64, little endian, typed array
            0x58,0x18, // Byte string value of length 24
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 71\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<uint64_t>() == std::numeric_limits<uint64_t>::lowest());
        CHECK(j[1].as<uint64_t>() == uint64_t(1));
        CHECK(j[2].as<uint64_t>() == (std::numeric_limits<uint64_t>::max)());

        auto u = cbor::decode_cbor<std::vector<uint64_t>>(data);
        REQUIRE(3 == u.size());
        CHECK(u[0] == std::numeric_limits<uint64_t>::lowest());
        CHECK(u[1] == uint64_t(1));
        CHECK(u[2] == (std::numeric_limits<uint64_t>::max)());

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::little>(),
            data, buf);
    }
    SECTION("Tag 72 (int8)")
    {
        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x48, // Tag 72, sint8, typed array
            0x43, // Byte string value of length 3
                0x80,0x01,0x7f
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 72\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<int8_t>() == std::numeric_limits<int8_t>::lowest());
        CHECK(j[1].as<int8_t>() == int8_t(1));
        CHECK(j[2].as<int8_t>() == (std::numeric_limits<int8_t>::max)());

        auto u = cbor::decode_cbor<std::vector<int8_t>>(data);

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::little>(),
            data, buf);
    }
    SECTION("Tag 73 (int16, big endian)")
    {
        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x49, // Tag 73, sint16, big endian, typed array
            0x46, // Byte string value of length 6
                0x80,0x00,
                0x00,0x01,
                0x7f,0xff        
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 73\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<int16_t>() == std::numeric_limits<int16_t>::lowest());
        CHECK(j[1].as<int16_t>() == int16_t(1));
        CHECK(j[2].as<int16_t>() == (std::numeric_limits<int16_t>::max)());

        auto u = cbor::decode_cbor<std::vector<int16_t>>(data);
        REQUIRE(3 == u.size());
        CHECK(u[0] == std::numeric_limits<int16_t>::lowest());
        CHECK(u[1] == int16_t(1));
        CHECK(u[2] == (std::numeric_limits<int16_t>::max)());

        auto v = cbor::decode_cbor<std::vector<int32_t>>(data);
        REQUIRE(3 == v.size());
        CHECK(v[0] == std::numeric_limits<int16_t>::lowest());
        CHECK(v[1] == int16_t(1));
        CHECK(v[2] == (std::numeric_limits<int16_t>::max)());

        auto w = cbor::decode_cbor<std::vector<int64_t>>(data);
        REQUIRE(3 == w.size());
        CHECK(w[0] == std::numeric_limits<int16_t>::lowest());
        CHECK(w[1] == int16_t(1));
        CHECK(w[2] == (std::numeric_limits<int16_t>::max)());

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::big>(),
            data, buf);
    }

    SECTION("Tag 74 (int32, big endian)")
    {
        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x4a, // Tag 74, sint32, big endian, typed array
            0x4c, // Byte string value of length 12
            0x80,0x00,0x00,0x00,
            0x00,0x00,0x00,0x01,
            0x7f,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 74\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<int32_t>() == std::numeric_limits<int32_t>::lowest());
        CHECK(j[1].as<int32_t>() == int32_t(1));
        CHECK(j[2].as<int32_t>() == (std::numeric_limits<int32_t>::max)());

        auto u = cbor::decode_cbor<std::vector<int32_t>>(data);
        REQUIRE(3 == u.size());
        CHECK(u[0] == std::numeric_limits<int32_t>::lowest());
        CHECK(u[1] == int32_t(1));
        CHECK(u[2] == (std::numeric_limits<int32_t>::max)());

        auto v = cbor::decode_cbor<std::vector<int64_t>>(data);
        REQUIRE(3 == v.size());
        CHECK(v[0] == std::numeric_limits<int32_t>::lowest());
        CHECK(v[1] == int32_t(1));
        CHECK(v[2] == (std::numeric_limits<int32_t>::max)());

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::big>(),
            data, buf);
    }
    SECTION("Tag 75 (int64,big endian)")
    {
        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x4b, // Tag 75, sint64, big endian, typed array
            0x58,0x18, // Byte string value of length 24
                0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,
                0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 75\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<int64_t>() == std::numeric_limits<int64_t>::lowest());
        CHECK(j[1].as<int64_t>() == int64_t(1));
        CHECK(j[2].as<int64_t>() == (std::numeric_limits<int64_t>::max)());

        auto u = cbor::decode_cbor<std::vector<int64_t>>(data);
        REQUIRE(3 == u.size());
        CHECK(u[0] == std::numeric_limits<int64_t>::lowest());
        CHECK(u[1] == int64_t(1));
        CHECK(u[2] == (std::numeric_limits<int64_t>::max)());

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::big>(),
            data, buf);
    }
    SECTION("Tag 77 (int16, little endian)")
    {
        //std::cout << "CBOR typed array Tag 77 (int16, little endian)" << '\n';

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x4d, // Tag 77, sint16, little endian, typed array
            0x46, // Byte string value of length 6
                0x00,0x80,
                0x01,0x00,
                0xff,0x7f
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 77\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<int16_t>() == std::numeric_limits<int16_t>::lowest());
        CHECK(j[1].as<int16_t>() == int16_t(1));
        CHECK(j[2].as<int16_t>() == (std::numeric_limits<int16_t>::max)());

        auto u = cbor::decode_cbor<std::vector<int16_t>>(data);
        REQUIRE(3 == u.size());
        CHECK(u[0] == std::numeric_limits<int16_t>::lowest());
        CHECK(u[1] == int16_t(1));
        CHECK(u[2] == (std::numeric_limits<int16_t>::max)());

        auto v = cbor::decode_cbor<std::vector<int32_t>>(data);
        REQUIRE(3 == v.size());
        CHECK(v[0] == std::numeric_limits<int16_t>::lowest());
        CHECK(v[1] == int16_t(1));
        CHECK(v[2] == (std::numeric_limits<int16_t>::max)());

        auto w = cbor::decode_cbor<std::vector<int64_t>>(data);
        REQUIRE(3 == w.size());
        CHECK(w[0] == std::numeric_limits<int16_t>::lowest());
        CHECK(w[1] == int16_t(1));
        CHECK(w[2] == (std::numeric_limits<int16_t>::max)());

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::little>(),
            data, buf);
    }
    SECTION("Tags 78 (int32, little endian)")
    {
        //std::cout << "CBOR typed array Tags 78 (int32, little endian)" << '\n';

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x4e, // Tag 78, sint32, little endian, typed array
            0x4c, // Byte string value of length 12
                0x00,0x00,0x00,0x80,
                0x01,0x00,0x00,0x00,
                0xff,0xff,0xff,0x7f
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 78\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<int32_t>() == std::numeric_limits<int32_t>::lowest());
        CHECK(j[1].as<int32_t>() == int32_t(1));
        CHECK(j[2].as<int32_t>() == (std::numeric_limits<int32_t>::max)());

        auto u = cbor::decode_cbor<std::vector<int32_t>>(data);
        REQUIRE(3 == u.size());
        CHECK(u[0] == std::numeric_limits<int32_t>::lowest());
        CHECK(u[1] == int32_t(1));
        CHECK(u[2] == (std::numeric_limits<int32_t>::max)());

        auto v = cbor::decode_cbor<std::vector<int64_t>>(data);
        REQUIRE(3 == v.size());
        CHECK(v[0] == std::numeric_limits<int32_t>::lowest());
        CHECK(v[1] == int32_t(1));
        CHECK(v[2] == (std::numeric_limits<int32_t>::max)());

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::little>(),
            data, buf);
    }

    SECTION("Tag 79 (int64,little endian)")
    {
        //std::cout << "CBOR typed array Tag 79 (int64,little endian)" << '\n';

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x4f, // Tag 79, sint64, little endian, typed array
            0x58,0x18, // Byte string value of length 24
                0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x80,
                0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 79\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(3 == j.size());
        CHECK(j[0].as<int64_t>() == std::numeric_limits<int64_t>::lowest());
        CHECK(j[1].as<int64_t>() == int64_t(1));
        CHECK(j[2].as<int64_t>() == (std::numeric_limits<int64_t>::max)());

        auto u = cbor::decode_cbor<std::vector<int64_t>>(data);
        REQUIRE(3 == u.size());
        CHECK(u[0] == std::numeric_limits<int64_t>::lowest());
        CHECK(u[1] == int64_t(1));
        CHECK(u[2] == (std::numeric_limits<int64_t>::max)());

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::little>(),
            data, buf);
    }

    SECTION("Tag 80, float16, big endian")
    {
        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x50, // Tag 80, float16, big endian, typed array
            0x48, // Byte string value of length 8
                0x00,0x01,
                0x03,0xff,
                0x04,0x00,
                0x7b,0xff
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 80\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(4 == j.size());
        CHECK(j[0].as<float>() == Approx(0.000000059605).epsilon(0.00001));
        CHECK(j[1].as<float>() == Approx(0.000060976).epsilon(0.00001));
        CHECK(j[2].as<float>() == Approx(0.000061035).epsilon(0.00001));
        CHECK(j[3].as<float>() == 65504);
    }

    SECTION("Tag 81, float32, big endian")
    {
        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x51, // Tag 81, float32, big endian, typed array
            0x48, // Byte string value of length 8
                0xff,0x7f,0xff,0xff,
                0x7f,0x7f,0xff,0xff
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 81\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(2 == j.size());
        CHECK(j[0].as<float>() == std::numeric_limits<float>::lowest());
        CHECK(j[1].as<float>() == (std::numeric_limits<float>::max)());

        auto u = cbor::decode_cbor<std::vector<float>>(data);

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::big>(),
            data, buf);
    }

    SECTION("Tag 82, float64, big endian")
    {
        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x52, // Tag 82, float64, big endian, typed array
            0x50, // Byte string value of length 16
                0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0x7f, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
        };

        json j = cbor::decode_cbor<json>(data);
        REQUIRE(j.is_array());
        //std::cout << pretty_print(j) << "\n";

        auto u = cbor::decode_cbor<std::vector<double>>(data);

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::big>(),
            data, buf);
    }

    SECTION("Tag 83, float128, big endian")
    {
        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x53, // Tag 83, float128, big endian, typed array
            0x58,0x40, // Byte string value of length 64
                0xff,0xfe,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                0x7f,0xfe,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                0xbf,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                0x3f,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        };

        //json j = cbor::decode_cbor<json>(data);
        //REQUIRE(j.is_array());
        //REQUIRE(2 == j.size());

        //CHECK(j[0].as<double>() == std::numeric_limits<double>::lowest());
        //CHECK(j[1].as<double>() == (std::numeric_limits<double>::max)());
    }

    SECTION("Tag 84, float16, little endian")
    {
        //std::cout << "CBOR typed array Tag 84, float16, little endian" << '\n';

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x54, // Tag 84, float16, little endian, typed array
            0x48, // Byte string value of length 8
                0x01,0x00,
                0xff,0x03,
                0x00,0x04,
                0xff,0x7b
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 84\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(4 == j.size());
        CHECK(j[0].as<float>() == Approx(0.000000059605).epsilon(0.00001));
        CHECK(j[1].as<float>() == Approx(0.000060976).epsilon(0.00001));
        CHECK(j[2].as<float>() == Approx(0.000061035).epsilon(0.00001));
        CHECK(j[3].as<float>() == 65504);
    }
    SECTION("Tag 85, float32, little endian")
    {
        //std::cout << "CBOR typed array Tag 85, float32, little endian" << '\n';

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x55, // Tag 85, float64, little endian, typed array
            0x48, // Byte string value of length 8
                0xff,0xff,0x7f,0xff,
                0xff,0xff,0x7f,0x7f 
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 85\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(2 == j.size());
        CHECK(j[0].as<float>() == std::numeric_limits<float>::lowest());
        CHECK(j[1].as<float>() == (std::numeric_limits<float>::max)());

        auto u = cbor::decode_cbor<std::vector<float>>(data);
        REQUIRE(2 == u.size());
        CHECK(u[0] == std::numeric_limits<float>::lowest());
        CHECK(u[1] == (std::numeric_limits<float>::max)());

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::little>(),
            data, buf);
    }
    SECTION("Tag 86, float64, little endian")
    {
        //std::cout << "CBOR typed array Tag 86, float64, little endian" << '\n';

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x56, // Tag 86, float64, little endian, typed array
            0x50, // Byte string value of length 16
                0xff,0xff,0xff,0xff,0xff,0xff,0xef,0xff,
                0xff,0xff,0xff,0xff,0xff,0xff,0xef,0x7f
        };

        json j = cbor::decode_cbor<json>(data);
        //std::cout << "Tag 86\n" << pretty_print(j) << "\n";
        REQUIRE(j.is_array());
        REQUIRE(2 == j.size());
        CHECK(j[0].as<double>() == std::numeric_limits<double>::lowest());
        CHECK(j[1].as<double>() == (std::numeric_limits<double>::max)());

        auto u = cbor::decode_cbor<std::vector<double>>(data);
        REQUIRE(2 == u.size());
        CHECK(u[0] == std::numeric_limits<double>::lowest());
        CHECK(u[1] == (std::numeric_limits<double>::max)());

        std::vector<uint8_t> buf;
        auto options = cbor::cbor_options{}
            .use_typed_arrays(true);
        cbor::encode_cbor(u, buf, options);
        check_native(std::integral_constant<bool, jsoncons::endian::native == jsoncons::endian::little>(),
            data, buf);
    }

    SECTION("Tag 87, float128, little endian")
    {
        //std::cout << "CBOR typed array Tag 87, float128, little endian" << '\n';

        const std::vector<uint8_t> data = {
            0xd8, // Tag
                0x57, // Tag 87, float128, little endian, typed array
            0x58,0x40, // Byte string value of length 64
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0xff,
                0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0x7f,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xbf,
                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x3f 
        };

        //json j = cbor::decode_cbor<json>(data);
        //REQUIRE(j.is_array());
        //REQUIRE(2 == j.size());
    }
} 

TEST_CASE("cbor multi-dim, row major, uint64, classical array tests")
{
    const std::vector<uint8_t> data = {
        0xd8,0x28, // semantic tag 40, row major storage
        0x82,  // array(2)
        0x82,0x02,0x03, // array(2) -> [2,3]
        0x86,  // array(6)
        0x02,  // 2
        0x04,  // 4
        0x08,  // 8 
        0x04,  // 4
        0x10,  // 16
        0x19,0x01,0x00  // 256
    };

    auto parser_expected = jsoncons::json::parse(R"(
[[2,4,8],[4,16,256]]
    )");

    auto cursor_expected = jsoncons::json::parse(R"(
[2,4,8,4,16,256]
    )");

    SECTION("parser test")
    {
        std::error_code ec;

        jsoncons::json_decoder<json> decoder;
        cbor::cbor_bytes_reader reader(data, decoder);
        reader.read(ec);

        json result = decoder.get_result();
        CHECK(parser_expected == result);
    }
    SECTION("cursor test")
    {
        cbor::cbor_bytes_cursor cursor(data);
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::begin_array == cursor.current().event_type());
        CHECK(cursor.is_multi_dim());
        CHECK_FALSE(cursor.is_typed_array());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::end_array == cursor.current().event_type());
        cursor.next();
        REQUIRE(cursor.done());
    }
    SECTION("cursor read_to test")
    {
        jsoncons::json_decoder<jsoncons::json> decoder;

        cbor::cbor_bytes_cursor cursor(data);
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::begin_array == cursor.current().event_type());
        CHECK(cursor.is_multi_dim());
        CHECK_FALSE(cursor.is_typed_array());
        cursor.read_to(decoder);
        CHECK(staj_events::end_array == cursor.current().event_type());
        cursor.next();
        REQUIRE(cursor.done());
        REQUIRE(decoder.is_valid());

        CHECK(cursor_expected == decoder.get_result());
    }
}

TEST_CASE("cbor multi-dim typed array parse tests")
{
    const std::vector<uint8_t> data = {
        0xd8, 0x28, // Tag 40 Indicates a multi-dimensional array (row-major)
        0x82,       // Array(2) The outer structure containing [dimensions, data]
        0x82,       // The shape array
        0x02,       // 1st dimension size (Rows)
        0x03,       // 2nd dimension size (Columns)
        0xd8, 0x41, // Tag 65 Typed array tag for uint16 (Big-Endian)
        0x4c,       // Byte String(12) Raw data length (6 elements x 2 bytes each)
        0x00, 0x02, // First element: 2
        0x00, 0x04, // Second element: 4
        0x00, 0x08, // Third element: 8
        0x00, 0x04, // Fourth element: 4
        0x00, 0x10, // Fifth element: 16 (0x10)
        0x01, 0x00  // Sixth element: 256 (0x0100)
    };

    auto expected = jsoncons::json::parse(R"(
        [[2, 4, 8], [4, 16, 256]]
    )");

    SECTION("parser test")
    {
        std::error_code ec;

        jsoncons::json_decoder<json> decoder;
        cbor::cbor_bytes_reader reader(data, decoder);
        reader.read(ec);

        json result = decoder.get_result();
        CHECK(expected == result);
    }
}

TEST_CASE("cbor multi-dim typed array parse tests 2")
{
    const std::vector<uint8_t> data = {
        0xDA, 0x00, 0x00, 0x04, 0x10, // Tag 1040 (Column-major multi-dim array)
        0x82, // Array of 2 elements
        0x82, 0x02, 0x03, // Array [2, 3] Dimensions
        0xD8, 0x40, // Tag 64 (unsigned 8-bit integers) typed array Tag
        0x46, // byte string (6)
        0x01, 0x04, 0x02, 0x05, 0x03, 0x06 // 6 bytes of data in column order
    };

    auto expected = jsoncons::json::parse(R"(
        [[1, 2, 3], [4, 5, 6]]
    )");

    SECTION("parser test")
    {
        std::error_code ec;

        jsoncons::json_decoder<json> decoder;
        cbor::cbor_bytes_reader reader(data, decoder);
        reader.read(ec);

        json result = decoder.get_result();
        CHECK(expected == result);
    }
}
 
TEST_CASE("cbor multi-dim typed array row major, uint64, little endian")
{
    const std::vector<uint8_t> data = {
        0xd8, 0x28, // Tag 40 Indicates a multi-dimensional array (row-major)
        0x82,       // Array(2) The outer structure containing [dimensions, data]
        0x82,       // The shape array.
        0x02,       // 1st dimension size (Rows).
        0x03,       // 2nd dimension size (Columns).
        0xd8, 0x41, // Tag 65 Typed array tag for uint16 (Big-Endian).
        0x4c,       // Byte String(12)	Raw data length (6 elements x 2 bytes each).
        0x00, 0x02, // First element: 2.
        0x00, 0x04, // Second element: 4.
        0x00, 0x08, // Third element: 8.
        0x00, 0x04, // Fourth element: 4.
        0x00, 0x10, // Fifth element: 16 (0x10).
        0x01, 0x00  // Sixth element: 256 (0x0100).
    };

    auto expected = jsoncons::json::parse(R"(
        [[2, 4, 8], [4, 16, 256]]
    )");

    SECTION("parser test")
    {
        std::error_code ec;

        jsoncons::json_decoder<json> decoder;
        cbor::cbor_bytes_reader reader(data, decoder);
        reader.read(ec);

        CHECK(decoder.is_valid());
        json result = decoder.get_result();
        CHECK(expected == result);
    }

    SECTION("cursor tests")
    {
        cbor::cbor_bytes_cursor cursor(data);
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::begin_array == cursor.current().event_type());
        CHECK(cursor.is_multi_dim());
        cursor.next();
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::end_array == cursor.current().event_type());
        cursor.next();
        REQUIRE(cursor.done());
        /*for (; !cursor.done(); cursor.next())
        {
            const auto& event = cursor.current();
            std::cout << event.event_type() << " " << event.tag() << "\n";
        }*/
    }
}

TEST_CASE("cbor multi-dim typed array column major, cursor tests 2")
{
    SECTION("Tag 86, float64, little endian")
    {
        //std::cout << "CBOR multi-dim typed array Tag 86, uint8" << '\n';

        auto expected = jsoncons::json::parse(R"(
            [[1, 2, 3], [4, 5, 6]]
        )");

        const std::vector<uint8_t> v = {
            0xDA, 0x00, 0x00, 0x04, 0x10, // Tag 1040 (Column-major multi-dim array)
            0x82,                         // Array of 2 elements
            0x82, 0x02, 0x03,             // Array [2, 3] Dimensions
            0xD8, 0x40,                   // Tag 64 (unsigned 8-bit integers) typed array Tag
            0x46,                         // byte string (6)
            0x01, 0x04, 0x02, 0x05, 0x03, 0x06 
        };

        cbor::cbor_bytes_cursor cursor(v);
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::begin_array == cursor.current().event_type());
        CHECK(cursor.is_multi_dim());
        cursor.next();
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::end_array == cursor.current().event_type());
        cursor.next();
        REQUIRE(cursor.done());
        /*for (; !cursor.done(); cursor.next())
        {
            const auto& event = cursor.current();
            std::cout << event.event_type() << " " << event.tag() << std::endl;
        }*/
    }
}

TEST_CASE("cbor multi-dim, row-major, classical indefinite array tests")
{
    const std::vector<uint8_t> data = {
        0xD8, 0x28,        // CBOR Tag 40 (Hex 28 is decimal 40).
        0x9F,              // Starts the outer indefinite-length wrapper array.
        0x82,              // The shape array. It is a definite-length array of 2 elements
        0x02, 0x02,        // The dimensions
        0x9F,              // Starts the flat data contents indefinite-length array.
        0x01, 0x02, 0x03, 0x04, // The raw flattened numbers 1, 2, 3, 4 poured into a single linear stream.
        0xFF,  // The break byte terminating the flat contents array. 
        0xFF   // The break byte terminating the outer wrapper array. 
    };

    auto parse_expected  = jsoncons::json::parse(R"(
        [[1, 2], [3, 4]]
    )");

    auto cursor_expected  = jsoncons::json::parse(R"(
        [1, 2, 3, 4]
    )");

    SECTION("parse test")
    {
        //std::cout << "CBOR multi-dim typed array Tag 86, uint16, big endian" << '\n';

        jsoncons::json_decoder<json> decoder;

        std::error_code ec;
        cbor::cbor_bytes_reader reader(data, decoder);
        reader.read(ec);
        REQUIRE(!ec);
        REQUIRE(decoder.is_valid());
        CHECK(parse_expected  == decoder.get_result());
    }
    SECTION("cursor read_to test")
    {
        jsoncons::json_decoder<jsoncons::json> decoder;

        cbor::cbor_bytes_cursor cursor(data);
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::begin_array == cursor.current().event_type());
        CHECK(cursor.is_multi_dim());
        CHECK_FALSE(cursor.is_typed_array());
        cursor.read_to(decoder);
        CHECK(staj_events::end_array == cursor.current().event_type());
        cursor.next();
        REQUIRE(cursor.done());
        REQUIRE(decoder.is_valid());

        CHECK(cursor_expected == decoder.get_result());
    }
}

TEST_CASE("cbor multi-dim, column-major, classical indefinite array tests")
{
    auto expected = jsoncons::json::parse(R"(
         [[2,3],[1,4,2,5,3,6]]
    )");

    const std::vector<uint8_t> v = {
        0xD9, 0x04, 0x10, // Tag 1040 Multi-dim Array (Column-Major)
        0x82,             // Outer container for [dimensions, data]
        0x82,             // The shape array
        0x02,             // 2 Rows
        0x03,             // 3 Columns
        0x86,             // Array (6 items)
        0x01, 0x04, 0x02, 0x05, 0x03, 0x06
    };

    SECTION("parse test")
    {
        std::error_code ec;

        jsoncons::json_decoder<json> decoder;
        cbor::cbor_bytes_reader reader(v, decoder);
        reader.read(ec);
        REQUIRE(decoder.is_valid());
        json result = decoder.get_result();
        CHECK(expected == result);
    }
}

TEST_CASE("cbor multi-dim classical array cursor tests")
{
    SECTION("row major")
    {
        //std::cout << "CBOR multi-dim typed array Tag 86, uint16, big endian" << '\n';

        auto expected = jsoncons::json::parse(R"(
            [[1, 2, 3], [4, 5, 6]]
        )");

        const std::vector<uint8_t> data = {
            0xD8, 0x28, // Tag 40 Multi-dimensional array (Row-Major)
            0x82,       // Wrapper for dimensions and data
            0x82,       // The dimensions: [Rows, Columns]
            0x02,       // Number of rows
            0x03,       // Number of columns
            0x86,       // Array (6 elements)
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06
        };

        cbor::cbor_bytes_cursor cursor(data);
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::begin_array == cursor.current().event_type());
        CHECK(cursor.is_multi_dim());
        CHECK_FALSE(cursor.is_typed_array());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::end_array == cursor.current().event_type());
        cursor.next();
        REQUIRE(cursor.done());
    }
    SECTION("column major")
    {
        auto expected = jsoncons::json::parse(R"(
            [[1, 2, 3], [4, 5, 6]]
        )");

        const std::vector<uint8_t> data = {
            0xD9, 0x04, 0x10, // Tag 1040 Multi-dim Array (Column-Major)
            0x82,             // Outer container for [dimensions, data]
            0x82,             // The shape array
            0x02,             // 2 Rows
            0x03,             // 3 Columns
            0x86,             // Array (6 items)
            0x01, 0x04, 0x02, 0x05, 0x03, 0x06
        };

        cbor::cbor_bytes_cursor cursor(data);
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::begin_array == cursor.current().event_type());
        CHECK(cursor.is_multi_dim());
        CHECK_FALSE(cursor.is_typed_array());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::uint64_value == cursor.current().event_type());
        cursor.next();
        REQUIRE_FALSE(cursor.done());
        CHECK(staj_events::end_array == cursor.current().event_type());
        cursor.next();
        REQUIRE(cursor.done());
    }
}

TEST_CASE("multi-dim classical array and typed array")
{
    SECTION("test 1")
    {
        auto expected = jsoncons::json::parse(R"(
[[[40000.0,40000.0],[40000.0,40000.0]],[50000.0]]
        )");

        std::vector<uint8_t> data = {
            0x9f, // array open

            // 2x2 matrix (values as floats):
            0xd8, 0x28, 0x82, 0x82, 0x02, 0x02, 0x84, // header
            0xfa, 0x47, 0x1c, 0x40, 0x00,
            0xfa, 0x47, 0x1c, 0x40, 0x00,
            0xfa, 0x47, 0x1c, 0x40, 0x00,
            0xfa, 0x47, 0x1c, 0x40, 0x00,

            // double tagged array [50000.0]:
            0xd8, 0x56, 0x58, 0x08, // Header
            0x00, 0x00, 0x00, 0x00, 0x00, 0x6a, 0xe8, 0x40,

            0xff, // array close
        };
        std::error_code ec;

        jsoncons::json_decoder<json> decoder;

        cbor::cbor_bytes_reader reader(data, decoder);
        reader.read(ec);

        REQUIRE(decoder.is_valid());
        json result = decoder.get_result();

        REQUIRE(expected == result);
    }
}


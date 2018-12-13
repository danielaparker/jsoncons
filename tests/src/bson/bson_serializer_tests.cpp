// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/bson/bson.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <catch/catch.hpp>

using namespace jsoncons;
using namespace jsoncons::bson;

TEST_CASE("serialize object to bson")
{
    std::vector<uint8_t> v;
    bson_buffer_serializer serializer(v);

    serializer.begin_object();
    serializer.name("null");
    serializer.null_value();
    serializer.end_object();
    serializer.flush();

    try
    {
        json result = decode_bson<json>(v);
        std::cout << result << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

namespace jsoncons { namespace bson {

    void test_equal(const std::vector<uint8_t>& v, const std::vector<uint8_t>& expected)
    {
        REQUIRE(v.size() == expected.size());

        for (size_t i = 0; i < v.size(); ++i)
        {
            CHECK(v[i] == expected[i]);
        }
    }

    void check_equal(const std::vector<uint8_t>& v, const std::vector<uint8_t>& expected)
    {
        test_equal(v, expected);
        try
        {
            json j = decode_bson<json>(v);
            std::vector<uint8_t> u;
            encode_bson(j, u);
            test_equal(v,u);
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
}}

TEST_CASE("serialize to bson")
{
    SECTION("array")
    {
        std::vector<uint8_t> v;
        bson_buffer_serializer serializer(v);

        serializer.begin_array();
        serializer.int64_value((std::numeric_limits<int64_t>::max)());
        serializer.uint64_value((uint64_t)(std::numeric_limits<int64_t>::max)());
        serializer.double_value((std::numeric_limits<double>::max)());
        serializer.bool_value(true);
        serializer.bool_value(false);
        serializer.null_value();
        serializer.string_value("Pussy cat");
        serializer.byte_string_value(byte_string({'h','i','s','s'}));
        serializer.end_array();
        serializer.flush();

        std::vector<uint8_t> bson = {0x4d,0x00,0x00,0x00,
                                     0x12, // int64
                                     0x30, // '0'
                                     0x00, // terminator
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f,
                                     0x12, // int64
                                     0x31, // '1'
                                     0x00, // terminator
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f,
                                     0x01, // double
                                     0x32, // '2'
                                     0x00, // terminator
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xef,0x7f,
                                     0x08, // bool
                                     0x33, // '3'
                                     0x00, // terminator
                                     0x01,
                                     0x08, // bool
                                     0x34, // '4'
                                     0x00, // terminator
                                     0x00,
                                     0x0a, // null
                                     0x35, // '5'
                                     0x00, // terminator
                                     0x02, // string
                                     0x36, // '6'
                                     0x00, // terminator
                                     0x0a,0x00,0x00,0x00, // string length
                                     'P','u','s','s','y',' ','c','a','t',
                                     0x00, // terminator
                                     0x05, // binary
                                     0x37, // '7'
                                     0x00, // terminator
                                     0x04,0x00,0x00,0x00, // byte string length
                                     'h','i','s','s',
                                     0x00 // terminator
                                     };
        jsoncons::bson::check_equal(v,bson);

    }
    SECTION("object")
    {
        std::vector<uint8_t> v;
        bson_buffer_serializer serializer(v);

        serializer.begin_object();
        serializer.name("0");
        serializer.int64_value((std::numeric_limits<int64_t>::max)());
        serializer.name("1");
        serializer.uint64_value((uint64_t)(std::numeric_limits<int64_t>::max)());
        serializer.name("2");
        serializer.double_value((std::numeric_limits<double>::max)());
        serializer.name("3");
        serializer.bool_value(true);
        serializer.name("4");
        serializer.bool_value(false);
        serializer.name("5");
        serializer.null_value();
        serializer.name("6");
        serializer.string_value("Pussy cat");
        serializer.name("7");
        serializer.byte_string_value(byte_string({'h','i','s','s'}));
        serializer.end_object();
        serializer.flush();

        std::vector<uint8_t> bson = {0x4d,0x00,0x00,0x00,
                                     0x12, // int64
                                     0x30, // '0'
                                     0x00, // terminator
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f,
                                     0x12, // int64
                                     0x31, // '1'
                                     0x00, // terminator
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f,
                                     0x01, // double
                                     0x32, // '2'
                                     0x00, // terminator
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xef,0x7f,
                                     0x08, // bool
                                     0x33, // '3'
                                     0x00, // terminator
                                     0x01,
                                     0x08, // bool
                                     0x34, // '4'
                                     0x00, // terminator
                                     0x00,
                                     0x0a, // null
                                     0x35, // '5'
                                     0x00, // terminator
                                     0x02, // string
                                     0x36, // '6'
                                     0x00, // terminator
                                     0x0a,0x00,0x00,0x00, // string length
                                     'P','u','s','s','y',' ','c','a','t',
                                     0x00, // terminator
                                     0x05, // binary
                                     0x37, // '7'
                                     0x00, // terminator
                                     0x04,0x00,0x00,0x00, // byte string length
                                     'h','i','s','s',
                                     0x00 // terminator
                                     };
        jsoncons::bson::check_equal(v,bson);
    }

    SECTION("outer object")
    {
        std::vector<uint8_t> v;
        bson_buffer_serializer serializer(v);

        serializer.begin_object();
        serializer.name("a");
        serializer.begin_object();
        serializer.name("0");
        serializer.int64_value((std::numeric_limits<int64_t>::max)());
        serializer.end_object();
        serializer.end_object();
        serializer.flush();

        std::vector<uint8_t> bson = {0x18,0x00,0x00,0x00,
                                     0x03, // object
                                     'a',
                                     0x00,
                                     0x10,0x00,0x00,0x00,
                                     0x12, // int64
                                     0x30, // '0'
                                     0x00, // terminator
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f,
                                     0x00, // terminator
                                     0x00 // terminator
                                     };
        jsoncons::bson::check_equal(v,bson);
    }

    SECTION("outer array")
    {
        std::vector<uint8_t> v;
        bson_buffer_serializer serializer(v);

        serializer.begin_array();
        serializer.begin_object();
        serializer.name("0");
        serializer.int64_value((std::numeric_limits<int64_t>::max)());
        serializer.end_object();
        serializer.end_array();
        serializer.flush();

        std::vector<uint8_t> bson = {0x18,0x00,0x00,0x00,
                                     0x03, // object
                                     '0',
                                     0x00,
                                     0x10,0x00,0x00,0x00,
                                     0x12, // int64
                                     0x30, // '0'
                                     0x00, // terminator
                                     0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x7f,
                                     0x00, // terminator
                                     0x00 // terminator
                                     };
        jsoncons::bson::check_equal(v,bson);
    }
}


// Copyright 2016 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <fstream>
#include <catch/catch.hpp>

using namespace jsoncons;
using namespace jsoncons::cbor;

TEST_CASE("test_serialize_to_stream")
{
json j = json::parse(R"(
{
   "application": "hiking",
   "reputons": [
   {
       "rater": "HikingAsylum.example.com",
       "assertion": "is-good",
       "rated": "sk",
       "rating": 0.90
     }
   ]
}
)");

    std::ofstream os;
    os.open("./output/store.cbor", std::ios::binary | std::ios::out);
    encode_cbor(j,os);

    std::vector<uint8_t> v;
    std::ifstream is;
    is.open("./output/store.cbor", std::ios::binary | std::ios::in);

    json j2 = decode_cbor<json>(is);

    //std::cout << pretty_print(j2) << std::endl; 

    CHECK(j == j2);
}

TEST_CASE("serialize array to cbor")
{
    std::vector<uint8_t> v;
    cbor_buffer_serializer serializer(v);
    //serializer.begin_object(1);
    serializer.begin_array(3);
    serializer.bool_value(true);
    serializer.bool_value(false);
    serializer.null_value();
    serializer.end_array();
    //serializer.end_object();
    serializer.flush();

    try
    {
        json result = decode_cbor<json>(v);
        std::cout << result << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

TEST_CASE("test_serialize_indefinite_length_array")
{
    std::vector<uint8_t> v;
    cbor_buffer_serializer serializer(v);
    serializer.begin_array();
    serializer.begin_array(4);
    serializer.bool_value(true);
    serializer.bool_value(false);
    serializer.null_value();
    serializer.string_value("Hello");
    serializer.end_array();
    serializer.end_array();
    serializer.flush();

    try
    {
        json result = decode_cbor<json>(v);
        std::cout << result << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 
TEST_CASE("test_serialize_bignum")
{
    std::vector<uint8_t> v;
    cbor_buffer_serializer serializer(v);
    serializer.begin_array();

    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    bignum n(1, bytes.data(), bytes.size());
    std::string s;
    n.dump(s);
    serializer.big_integer_value(s);
    serializer.end_array();
    serializer.flush();

    try
    {
        json result = decode_cbor<json>(v);
        CHECK(result[0].as<std::string>() == std::string("18446744073709551616"));
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

TEST_CASE("test_serialize_negative_bignum1")
{
    std::vector<uint8_t> v;
    cbor_buffer_serializer serializer(v);
    serializer.begin_array();

    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    bignum n(-1, bytes.data(), bytes.size());
    std::string s;
    n.dump(s);
    serializer.big_integer_value(s);
    serializer.end_array();
    serializer.flush();

    try
    {
        json result = decode_cbor<json>(v);
        CHECK(result[0].as<std::string>() == std::string("-18446744073709551617"));
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

TEST_CASE("test_serialize_negative_bignum2")
{
    std::vector<uint8_t> v;
    cbor_buffer_serializer serializer(v);
    serializer.begin_array();

    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    bignum n(-1, bytes.data(), bytes.size());
    std::string s;
    n.dump(s);
    serializer.big_integer_value(s);
    serializer.end_array();
    serializer.flush();

    try
    {
        json result = decode_cbor<json>(v);
        json_options options;
        options.big_integer_format(big_integer_chars_format::number);
        std::string text;
        result.dump(text,options);
        CHECK(text == std::string("[-18446744073709551617]"));
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

TEST_CASE("test_serialize_negative_bignum3")
{
    std::vector<uint8_t> v;
    cbor_buffer_serializer serializer(v);
    serializer.begin_array();

    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    bignum n(-1, bytes.data(), bytes.size());
    std::string s;
    n.dump(s);
    serializer.big_integer_value(s);
    serializer.end_array();
    serializer.flush();

    try
    {
        json result = decode_cbor<json>(v);
        json_options options;
        options.big_integer_format(big_integer_chars_format::base64url);
        std::string text;
        result.dump(text,options);
        CHECK(text == std::string("[\"~AQAAAAAAAAAA\"]"));
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

TEST_CASE("serialize big_decimal to cbor")
{
    SECTION("-1 184467440737095516160")
    {
        std::vector<uint8_t> v;
        cbor_buffer_serializer serializer(v);
        serializer.string_value("18446744073709551616.0", semantic_tag_type::big_decimal);
        serializer.flush();
        try
        {
            json result = decode_cbor<json>(v);
            CHECK(result.as<std::string>() == std::string("18446744073709551616.0"));
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
    SECTION("18446744073709551616e-5")
    {
        std::vector<uint8_t> v;
        cbor_buffer_serializer serializer(v);
        serializer.string_value("18446744073709551616e-5", semantic_tag_type::big_decimal);
        serializer.flush();
        try
        {
            json result = decode_cbor<json>(v);
            CHECK(result.as<std::string>() == std::string("184467440737095.51616"));
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
    SECTION("-18446744073709551616e-5")
    {
        std::vector<uint8_t> v;
        cbor_buffer_serializer serializer(v);
        serializer.string_value("-18446744073709551616e-5", semantic_tag_type::big_decimal);
        serializer.flush();
        try
        {
            json result = decode_cbor<json>(v);
            CHECK(result.as<std::string>() == std::string("-184467440737095.51616"));
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
    SECTION("-18446744073709551616e5")
    {
        std::vector<uint8_t> v;
        cbor_buffer_serializer serializer(v);
        serializer.string_value("-18446744073709551616e5", semantic_tag_type::big_decimal);
        serializer.flush();
        try
        {
            json result = decode_cbor<json>(v);
            CHECK(result.as<std::string>() == std::string("-18446744073709551616e5"));
        }
        catch (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
} 

TEST_CASE("Too many and too few items in CBOR map or array")
{
    std::error_code ec{};
    std::vector<uint8_t> v;
    cbor_buffer_serializer serializer(v);

    SECTION("Too many items in array")
    {
        CHECK(serializer.begin_array(3));
        CHECK(serializer.bool_value(true));
        CHECK(serializer.bool_value(false));
        CHECK(serializer.null_value());
        CHECK(serializer.begin_array(2));
        CHECK(serializer.string_value("cat"));
        CHECK(serializer.string_value("feline"));
        CHECK(serializer.end_array());
        REQUIRE_THROWS_WITH(serializer.end_array(), cbor_error_category_impl().message((int)cbor_errc::too_many_items).c_str());
        serializer.flush();
    }
    SECTION("Too few items in array")
    {
        CHECK(serializer.begin_array(5));
        CHECK(serializer.bool_value(true));
        CHECK(serializer.bool_value(false));
        CHECK(serializer.null_value());
        CHECK(serializer.begin_array(2));
        CHECK(serializer.string_value("cat"));
        CHECK(serializer.string_value("feline"));
        CHECK(serializer.end_array());
        REQUIRE_THROWS_WITH(serializer.end_array(), cbor_error_category_impl().message((int)cbor_errc::too_few_items).c_str());
        serializer.flush();
    }
    SECTION("Too many items in map")
    {
        CHECK(serializer.begin_object(3));
        CHECK(serializer.name("a"));
        CHECK(serializer.bool_value(true));
        CHECK(serializer.name("b"));
        CHECK(serializer.bool_value(false));
        CHECK(serializer.name("c"));
        CHECK(serializer.null_value());
        CHECK(serializer.name("d"));
        CHECK(serializer.begin_array(2));
        CHECK(serializer.string_value("cat"));
        CHECK(serializer.string_value("feline"));
        CHECK(serializer.end_array());
        REQUIRE_THROWS_WITH(serializer.end_object(), cbor_error_category_impl().message((int)cbor_errc::too_many_items).c_str());
        serializer.flush();
    }
    SECTION("Too few items in map")
    {
        CHECK(serializer.begin_object(5));
        CHECK(serializer.name("a"));
        CHECK(serializer.bool_value(true));
        CHECK(serializer.name("b"));
        CHECK(serializer.bool_value(false));
        CHECK(serializer.name("c"));
        CHECK(serializer.null_value());
        CHECK(serializer.name("d"));
        CHECK(serializer.begin_array(2));
        CHECK(serializer.string_value("cat"));
        CHECK(serializer.string_value("feline"));
        CHECK(serializer.end_array());
        REQUIRE_THROWS_WITH(serializer.end_object(), cbor_error_category_impl().message((int)cbor_errc::too_few_items).c_str());
        serializer.flush();
    }
    SECTION("Just enough items")
    {
        CHECK(serializer.begin_array(4)); // a fixed length array
        CHECK(serializer.string_value("foo"));
        CHECK(serializer.byte_string_value(byte_string{'P','u','s','s'})); // no suggested conversion
        CHECK(serializer.big_integer_value("-18446744073709551617"));
        CHECK(serializer.big_decimal_value("273.15"));
        CHECK(serializer.end_array());
        CHECK_FALSE(ec);
        serializer.flush();
    }
}

// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

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

TEST_CASE("test_serialize_array")
{
    std::vector<uint8_t> v;
    cbor_bytes_serializer serializer(v);
    serializer.begin_document();
    //serializer.begin_object(1);
    serializer.begin_array(3);
    serializer.write_bool(true);
    serializer.write_bool(false);
    serializer.write_null();
    serializer.end_array();
    //serializer.end_object();
    serializer.end_document();

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
    cbor_bytes_serializer serializer(v);
    serializer.begin_document();
    serializer.begin_array();
    serializer.begin_array(4);
    serializer.write_bool(true);
    serializer.write_bool(false);
    serializer.write_null();
    serializer.write_string("Hello");
    serializer.end_array();
    serializer.end_array();
    serializer.end_document();

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
    cbor_bytes_serializer serializer(v);
    serializer.begin_document();
    serializer.begin_array();

    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    serializer.write_bignum(1, bytes.data(), bytes.size());
    serializer.end_array();
    serializer.end_document();

    try
    {
        json result = decode_cbor<json>(v);
        CHECK(std::string("18446744073709551616") == result[0].as<std::string>());
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

TEST_CASE("test_serialize_negative_bignum1")
{
    std::vector<uint8_t> v;
    cbor_bytes_serializer serializer(v);
    serializer.begin_document();
    serializer.begin_array();

    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    serializer.write_bignum(-1, bytes.data(), bytes.size());
    serializer.end_array();
    serializer.end_document();

    try
    {
        json result = decode_cbor<json>(v);
        CHECK(std::string("-18446744073709551617") ==result[0].as<std::string>());
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

TEST_CASE("test_serialize_negative_bignum2")
{
    std::vector<uint8_t> v;
    cbor_bytes_serializer serializer(v);
    serializer.begin_document();
    serializer.begin_array();

    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    serializer.write_bignum(-1, bytes.data(), bytes.size());
    serializer.end_array();
    serializer.end_document();

    try
    {
        json result = decode_cbor<json>(v);
        json_serializing_options options;
        options.bignum_format(bignum_chars_format::integer);
        std::string s;
        result.dump(s,options);
        CHECK(std::string("[-18446744073709551617]") ==s);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

TEST_CASE("test_serialize_negative_bignum3")
{
    std::vector<uint8_t> v;
    cbor_bytes_serializer serializer(v);
    serializer.begin_document();
    serializer.begin_array();

    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    serializer.write_bignum(-1, bytes.data(), bytes.size());
    serializer.end_array();
    serializer.end_document();

    try
    {
        json result = decode_cbor<json>(v);
        json_serializing_options options;
        options.bignum_format(bignum_chars_format::base64url);
        std::string s;
        result.dump(s,options);
        CHECK(std::string("[\"~AQAAAAAAAAAA\"]") == s);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 


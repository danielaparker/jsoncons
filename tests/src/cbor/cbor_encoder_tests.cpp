// Copyright 2016 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/cbor/cbor_encoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <fstream>
#include <iomanip>
#include <catch/catch.hpp>

using namespace jsoncons;

TEST_CASE("cbor encode multi dim array test")
{
    std::vector<uint8_t> v;

    cbor::cbor_bytes_encoder encoder(v);
    std::vector<std::size_t> shape = { 2,3 };
    encoder.begin_multi_dim(shape);
    encoder.begin_array(6);
    encoder.uint64_value(2);
    encoder.uint64_value(4);
    encoder.uint64_value(8);
    encoder.uint64_value(4);
    encoder.uint64_value(16);
    encoder.uint64_value(256);
    encoder.end_array();
    encoder.end_multi_dim();

    byte_string_view bstr(v);
    std::cout << "bstr: " << bstr << "\n\n";

    for (auto ch : bstr)
    {
        std::cout << (int)ch << " ";
    }
    std::cout << "\n\n";

    auto j = cbor::decode_cbor<json>(v);
    std::cout << pretty_print(j) << "\n\n";

}

TEST_CASE("test_encode_to_stream")
{
json j = json::parse(R"(
{
   "application": "hiking",
   "reputons": [
   {
       "rater": "HikingAsylum",
       "assertion": "advanced",
       "rated": "Marilyn C",
       "rating": 0.90
     }
   ]
}
)");

    std::ofstream os;
    os.open("./output/store.cbor", std::ios::binary | std::ios::out);
    cbor::encode_cbor(j,os);

    std::vector<uint8_t> v;
    std::ifstream is;
    is.open("./output/store.cbor", std::ios::binary | std::ios::in);

    json j2 = cbor::decode_cbor<json>(is);

    //std::cout << pretty_print(j2) << std::endl; 

    CHECK(j == j2);
}

TEST_CASE("serialize array to cbor")
{
    std::vector<uint8_t> v;
    cbor::cbor_bytes_encoder encoder(v);
    //encoder.begin_object(1);
    encoder.begin_array(3);
    encoder.bool_value(true);
    encoder.bool_value(false);
    encoder.null_value();
    encoder.end_array();
    //encoder.end_object();
    encoder.flush();

    JSONCONS_TRY
    {
        json result = cbor::decode_cbor<json>(v);
        //std::cout << result << std::endl;
    }
    JSONCONS_CATCH (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}

TEST_CASE("test_serialize_indefinite_length_array")
{
    std::vector<uint8_t> v;
    cbor::cbor_bytes_encoder encoder(v);
    encoder.begin_array();
    encoder.begin_array(4);
    encoder.bool_value(true);
    encoder.bool_value(false);
    encoder.null_value();
    encoder.string_value("Hello");
    encoder.end_array();
    encoder.end_array();
    encoder.flush();

    JSONCONS_TRY
    {
        json result = cbor::decode_cbor<json>(v);
        //std::cout << result << std::endl;
    }
    JSONCONS_CATCH (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 
TEST_CASE("test_serialize_bignum")
{
    std::vector<uint8_t> v;
    cbor::cbor_bytes_encoder encoder(v);
    encoder.begin_array();

    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    bigint n = bigint::from_bytes_be(1, bytes.data(), bytes.size());
    std::string s = n.to_string();
    encoder.string_value(s, semantic_tag::bigint);
    encoder.end_array();
    encoder.flush();

    JSONCONS_TRY
    {
        json result = cbor::decode_cbor<json>(v);
        CHECK(result[0].as<std::string>() == std::string("18446744073709551616"));
    }
    JSONCONS_CATCH (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

TEST_CASE("test_serialize_negative_bignum1")
{
    std::vector<uint8_t> v;
    cbor::cbor_bytes_encoder encoder(v);
    encoder.begin_array();

    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    bigint n = bigint::from_bytes_be(1, bytes.data(), bytes.size());
    n = -1 - n;
    std::string s = n.to_string();
    encoder.string_value(s, semantic_tag::bigint);
    encoder.end_array();
    encoder.flush();

    JSONCONS_TRY
    {
        json result = cbor::decode_cbor<json>(v);
        CHECK(result[0].as<std::string>() == std::string("-18446744073709551617"));
    }
    JSONCONS_CATCH (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

TEST_CASE("test_serialize_negative_bignum2")
{
    std::vector<uint8_t> v;
    cbor::cbor_bytes_encoder encoder(v);
    encoder.begin_array();

    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    bigint n = bigint::from_bytes_be(1, bytes.data(), bytes.size());
    n = -1 - n;
    std::string s = n.to_string();
    encoder.string_value(s, semantic_tag::bigint);
    encoder.end_array();
    encoder.flush();

    JSONCONS_TRY
    {
        json result = cbor::decode_cbor<json>(v);
        json_options options;
        options.bigint_format(bigint_chars_format::number);
        std::string text;
        result.dump(text,options);
        CHECK(text == std::string("[-18446744073709551617]"));
    }
    JSONCONS_CATCH (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

TEST_CASE("test_serialize_negative_bignum3")
{
    std::vector<uint8_t> v;
    cbor::cbor_bytes_encoder encoder(v);
    encoder.begin_array();

    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    bigint n = bigint::from_bytes_be(1, bytes.data(), bytes.size());
    n = -1 - n;
    std::string s = n.to_string();
    encoder.string_value(s, semantic_tag::bigint);
    encoder.end_array();
    encoder.flush();

    JSONCONS_TRY
    {
        json result = cbor::decode_cbor<json>(v);
        json_options options;
        options.bigint_format(bigint_chars_format::base64url);
        std::string text;
        result.dump(text,options);
        CHECK(text == std::string("[\"~AQAAAAAAAAAA\"]"));
    }
    JSONCONS_CATCH (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
} 

TEST_CASE("serialize bigdec to cbor")
{
    SECTION("-1 184467440737095516160")
    {
        std::vector<uint8_t> v;
        cbor::cbor_bytes_encoder encoder(v);
        encoder.string_value("18446744073709551616.0", semantic_tag::bigdec);
        encoder.flush();
        JSONCONS_TRY
        {
            json result = cbor::decode_cbor<json>(v);
            CHECK(result.as<std::string>() == std::string("1.84467440737095516160e+19"));
        }
        JSONCONS_CATCH (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
    SECTION("18446744073709551616e-5")
    {
        std::vector<uint8_t> v;
        cbor::cbor_bytes_encoder encoder(v);
        encoder.string_value("18446744073709551616e-5", semantic_tag::bigdec);
        encoder.flush();
        JSONCONS_TRY
        {
            json result = cbor::decode_cbor<json>(v);
            CHECK(result.as<std::string>() == std::string("184467440737095.51616"));
        }
        JSONCONS_CATCH (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
    SECTION("-18446744073709551616e-5")
    {
        std::vector<uint8_t> v;
        cbor::cbor_bytes_encoder encoder(v);
        encoder.string_value("-18446744073709551616e-5", semantic_tag::bigdec);
        encoder.flush();
        JSONCONS_TRY
        {
            json result = cbor::decode_cbor<json>(v);
            CHECK(result.as<std::string>() == std::string("-184467440737095.51616"));
        }
        JSONCONS_CATCH (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
    SECTION("-18446744073709551616e5")
    {
        std::vector<uint8_t> v;
        cbor::cbor_bytes_encoder encoder(v);
        encoder.string_value("-18446744073709551616e5", semantic_tag::bigdec);
        encoder.flush();
        JSONCONS_TRY
        {
            json result = cbor::decode_cbor<json>(v);
            CHECK(result.as<std::string>() == std::string("-1.8446744073709551616e+24"));
        }
        JSONCONS_CATCH (const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }
} 

TEST_CASE("Too many and too few items in CBOR map or array")
{
    std::error_code ec{};
    std::vector<uint8_t> v;
    cbor::cbor_bytes_encoder encoder(v);

    SECTION("Too many items in array")
    {
        CHECK(encoder.begin_array(3));
        CHECK(encoder.bool_value(true));
        CHECK(encoder.bool_value(false));
        CHECK(encoder.null_value());
        CHECK(encoder.begin_array(2));
        CHECK(encoder.string_value("cat"));
        CHECK(encoder.string_value("feline"));
        CHECK(encoder.end_array());
        REQUIRE_THROWS_WITH(encoder.end_array(), cbor::cbor_error_category_impl().message((int)cbor::cbor_errc::too_many_items).c_str());
        encoder.flush();
    }
    SECTION("Too few items in array")
    {
        CHECK(encoder.begin_array(5));
        CHECK(encoder.bool_value(true));
        CHECK(encoder.bool_value(false));
        CHECK(encoder.null_value());
        CHECK(encoder.begin_array(2));
        CHECK(encoder.string_value("cat"));
        CHECK(encoder.string_value("feline"));
        CHECK(encoder.end_array());
        REQUIRE_THROWS_WITH(encoder.end_array(), cbor::cbor_error_category_impl().message((int)cbor::cbor_errc::too_few_items).c_str());
        encoder.flush();
    }
    SECTION("Too many items in map")
    {
        CHECK(encoder.begin_object(3));
        CHECK(encoder.key("a"));
        CHECK(encoder.bool_value(true));
        CHECK(encoder.key("b"));
        CHECK(encoder.bool_value(false));
        CHECK(encoder.key("c"));
        CHECK(encoder.null_value());
        CHECK(encoder.key("d"));
        CHECK(encoder.begin_array(2));
        CHECK(encoder.string_value("cat"));
        CHECK(encoder.string_value("feline"));
        CHECK(encoder.end_array());
        REQUIRE_THROWS_WITH(encoder.end_object(), cbor::cbor_error_category_impl().message((int)cbor::cbor_errc::too_many_items).c_str());
        encoder.flush();
    }
    SECTION("Too few items in map")
    {
        CHECK(encoder.begin_object(5));
        CHECK(encoder.key("a"));
        CHECK(encoder.bool_value(true));
        CHECK(encoder.key("b"));
        CHECK(encoder.bool_value(false));
        CHECK(encoder.key("c"));
        CHECK(encoder.null_value());
        CHECK(encoder.key("d"));
        CHECK(encoder.begin_array(2));
        CHECK(encoder.string_value("cat"));
        CHECK(encoder.string_value("feline"));
        CHECK(encoder.end_array());
        REQUIRE_THROWS_WITH(encoder.end_object(), cbor::cbor_error_category_impl().message((int)cbor::cbor_errc::too_few_items).c_str());
        encoder.flush();
    }
    SECTION("Just enough items")
    {
        CHECK(encoder.begin_array(4)); // a fixed length array
        CHECK(encoder.string_value("foo"));
        CHECK(encoder.byte_string_value(std::vector<uint8_t>{'P','u','s','s'})); // no suggested conversion
        CHECK(encoder.string_value("-18446744073709551617", semantic_tag::bigint));
        CHECK(encoder.string_value("273.15", semantic_tag::bigdec));
        CHECK(encoder.end_array());
        CHECK_FALSE(ec);
        encoder.flush();
    }
}

TEST_CASE("encode stringref")
{
    ojson j = ojson::parse(R"(
[
     {
       "name" : "Cocktail",
       "count" : 417,
       "rank" : 4
     },
     {
       "rank" : 4,
       "count" : 312,
       "name" : "Bath"
     },
     {
       "count" : 691,
       "name" : "Food",
       "rank" : 4
     }
  ]
)");

    cbor::cbor_options options;
    options.pack_strings(true);
    std::vector<uint8_t> buf;

    cbor::encode_cbor(j, buf, options);

    ojson j2 = cbor::decode_cbor<ojson>(buf);
    CHECK(j2 == j);
}

TEST_CASE("cbor encode with semantic_tags")
{
    SECTION("string")
    {
        json original;
        original["uri"] = json("https://gmail.com/", semantic_tag::uri);
        original["base64url"] = json("Zm9vYmFy", semantic_tag::base64url);
        original["base64"] = json("Zm9vYmE=", semantic_tag::base64);

        std::vector<uint8_t> buffer;
        cbor::encode_cbor(original, buffer);
        json j = cbor::decode_cbor<json>(buffer);

        CHECK(j == original);
    }
    SECTION("byte_string")
    {
        const std::vector<uint8_t> s1 = {'f','o'};
        const std::vector<uint8_t> s2 = {'f','o','o','b','a'};
        const std::vector<uint8_t> s3 = {'f','o','o','b','a','r'};

        json original;
        original["base64url"] = json(byte_string_arg, s1, semantic_tag::base64url);
        original["base64"] = json(byte_string_arg, s2, semantic_tag::base64);
        original["base16"] = json(byte_string_arg, s3, semantic_tag::base16);

        std::vector<uint8_t> buffer;
        cbor::encode_cbor(original, buffer);
        json j = cbor::decode_cbor<json>(buffer);

        CHECK(j == original);
    }
}


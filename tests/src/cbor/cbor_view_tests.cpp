// Copyright 2016 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

using namespace jsoncons;
using namespace jsoncons::cbor;

BOOST_AUTO_TEST_SUITE(cbor_view_tests)

BOOST_AUTO_TEST_CASE(cbor_view_test)
{
    ojson j1 = ojson::parse(R"(
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
 
    std::vector<uint8_t> c;
    encode_cbor(j1, c);

    cbor_view v(c); 
    BOOST_CHECK(v.is_object());
    BOOST_CHECK(!v.is_array());

    ojson jv = decode_cbor<ojson>(v);
    BOOST_CHECK(jv == j1);

    cbor_view reputons = v.at("reputons");
    BOOST_CHECK(reputons.is_array());

    cbor_view reputons_0 = reputons.at(0);

    cbor_view reputons_0_rated = reputons_0.at("rated");

    cbor_view rating = reputons_0.at("rating");
    BOOST_CHECK(rating.as_double() == 0.90);

    for (const auto& member : v.object_range())
    {
        const auto& key = member.key();
        const auto& val = member.value();
        json jval = decode_cbor<json>(val);

        //std::cout << key << ": " << jval << std::endl;
    }
    //std::cout << std::endl;

    for (auto element : reputons.array_range())
    {
        json j = decode_cbor<json>(element);
        //std::cout << j << std::endl;
    }
    //std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(jsonpointer_test)
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

    std::vector<uint8_t> buffer;
    encode_cbor(j, buffer);

    cbor_view bv = buffer;
    std::string s;
    bv.dump(s);
    json j1 = json::parse(s);
    BOOST_CHECK(j1 == j);

    std::error_code ec;
    cbor_view application = jsonpointer::get(cbor_view(buffer), "/application", ec);
    BOOST_CHECK(!ec);

    json j2 = decode_cbor<json>(application);
    BOOST_CHECK(j2 == j["application"]);

    cbor_view reputons_0_rated = jsonpointer::get(cbor_view(buffer), "/reputons", ec);
    BOOST_CHECK(!ec);

    json j3 = decode_cbor<json>(reputons_0_rated);
    json j4 = j["reputons"];
    BOOST_CHECK(j3 == j4);

    //std::cout << pretty_print(j3) << std::endl;
}

BOOST_AUTO_TEST_CASE(as_string_test)
{
    std::vector<uint8_t> b;
    jsoncons::cbor::cbor_bytes_serializer serializer(b);
    serializer.begin_document();
    serializer.begin_array(10);
    serializer.bool_value(true);
    serializer.bool_value(false);
    serializer.null_value();
    serializer.string_value("Toronto");
    serializer.byte_string_value({'H','e','l','l','o'});
    serializer.integer_value(-100);
    serializer.uinteger_value(100);
    serializer.bignum_value("18446744073709551616");
    serializer.double_value(10.5);
    serializer.bignum_value("-18446744073709551617");
    serializer.end_array();
    serializer.end_document();

    jsoncons::cbor::cbor_view bv = b;

    std::string s0;
    bv[0].dump(s0);
    BOOST_CHECK_EQUAL(std::string("true"), s0);
    BOOST_CHECK_EQUAL(std::string("true"), bv[0].as_string());
    BOOST_CHECK_EQUAL(true, bv[0].as<bool>());

    std::string s1;
    bv[1].dump(s1);
    BOOST_CHECK_EQUAL(std::string("false"), s1);
    BOOST_CHECK_EQUAL(std::string("false"), bv[1].as_string());
    BOOST_CHECK_EQUAL(false, bv[1].as<bool>());

    std::string s2;
    bv[2].dump(s2);
    BOOST_CHECK_EQUAL(std::string("null"), s2);
    BOOST_CHECK_EQUAL(std::string("null"), bv[2].as_string());

    std::string s3;
    bv[3].dump(s3);
    BOOST_CHECK_EQUAL(std::string("\"Toronto\""), s3);
    BOOST_CHECK_EQUAL(std::string("Toronto"), bv[3].as_string());
    BOOST_CHECK_EQUAL(std::string("Toronto"), bv[3].as<std::string>());

    std::string s4;
    bv[4].dump(s4);
    BOOST_CHECK_EQUAL(std::string("\"SGVsbG8\""), s4);
    BOOST_CHECK_EQUAL(std::string("SGVsbG8"), bv[4].as_string());
    BOOST_CHECK_EQUAL(byte_string({'H','e','l','l','o'}), bv[4].as<byte_string>());

    std::string s5;
    bv[5].dump(s5);
    BOOST_CHECK_EQUAL(std::string("-100"), s5);
    BOOST_CHECK_EQUAL(std::string("-100"), bv[5].as_string());
    BOOST_CHECK_EQUAL(-100, bv[5].as<int>());

    std::string s6;
    bv[6].dump(s6);
    BOOST_CHECK_EQUAL(std::string("100"), s6);
    BOOST_CHECK_EQUAL(std::string("100"), bv[6].as_string());

    std::string s7;
    bv[7].dump(s7);
    BOOST_CHECK_EQUAL(std::string("\"18446744073709551616\""), s7);
    BOOST_CHECK_EQUAL(std::string("18446744073709551616"), bv[7].as_string());

    std::string s8;
    bv[8].dump(s8);
    BOOST_CHECK_EQUAL(std::string("10.5"), s8);
    BOOST_CHECK_EQUAL(std::string("10.5"), bv[8].as_string());

    std::string s9;
    bv[9].dump(s9);
    BOOST_CHECK_EQUAL(std::string("\"-18446744073709551617\""), s9);
    BOOST_CHECK_EQUAL(std::string("-18446744073709551617"), bv[9].as_string());
}

BOOST_AUTO_TEST_CASE(test_dump_to_string)
{
    std::vector<uint8_t> b;
    cbor_bytes_serializer serializer(b);
    serializer.begin_document();
    serializer.begin_array();
    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    serializer.bignum_value(-1, bytes.data(), bytes.size());
    serializer.end_array();
    serializer.end_document();

    cbor_view bv = b;

    std::string s0;
    bv.dump(s0);
    BOOST_CHECK_EQUAL("[\"-18446744073709551617\"]",s0.c_str());
    //std::cout << s0 << std::endl;

    std::string s1;
    json_serializing_options options1;
    options1.bignum_format(bignum_chars_format::integer);
    bv.dump(s1,options1);
    BOOST_CHECK_EQUAL("[-18446744073709551617]",s1.c_str());
    //std::cout << s1 << std::endl;

    std::string s2;
    json_serializing_options options2;
    options2.bignum_format(bignum_chars_format::base10);
    bv.dump(s2,options2);
    BOOST_CHECK_EQUAL("[\"-18446744073709551617\"]",s2.c_str());
    //std::cout << s2 << std::endl;

    std::string s3;
    json_serializing_options options3;
    options3.bignum_format(bignum_chars_format::base64url);
    bv.dump(s3,options3);
    BOOST_CHECK_EQUAL("[\"~AQAAAAAAAAAA\"]",s3.c_str());
    //std::cout << s3 << std::endl;
} 

BOOST_AUTO_TEST_CASE(test_dump_to_stream)
{
    std::vector<uint8_t> b;
    cbor_bytes_serializer serializer(b);
    serializer.begin_document();
    serializer.begin_array();
    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    serializer.bignum_value(-1, bytes.data(), bytes.size());
    serializer.end_array();
    serializer.end_document();

    cbor_view bv = b;

    std::ostringstream os0;
    bv.dump(os0);
    BOOST_CHECK_EQUAL("[\"-18446744073709551617\"]",os0.str().c_str());
    //std::cout << os0.str() << std::endl;

    std::ostringstream os1;
    json_serializing_options options1;
    options1.bignum_format(bignum_chars_format::integer);
    bv.dump(os1,options1);
    BOOST_CHECK_EQUAL("[-18446744073709551617]",os1.str().c_str());
    //std::cout << os1.str() << std::endl;

    std::ostringstream os2;
    json_serializing_options options2;
    options2.bignum_format(bignum_chars_format::base10);
    bv.dump(os2,options2);
    BOOST_CHECK_EQUAL("[\"-18446744073709551617\"]",os2.str().c_str());
    //std::cout << os2.str() << std::endl;

    std::ostringstream os3;
    json_serializing_options options3;
    options3.bignum_format(bignum_chars_format::base64url);
    bv.dump(os3,options3);
    BOOST_CHECK_EQUAL("[\"~AQAAAAAAAAAA\"]",os3.str().c_str());
    //std::cout << os3.str() << std::endl;
} 

BOOST_AUTO_TEST_CASE(test_indefinite_length_object_iterator)
{
    std::vector<uint8_t> b1;
    cbor::cbor_bytes_serializer serializer1(b1);
    serializer1.begin_document();
    serializer1.begin_object(); // indefinite length object
    serializer1.end_object(); 
    serializer1.end_document();
    cbor_view bv1 = b1;
    BOOST_CHECK(bv1.object_range().begin() == bv1.object_range().end());

    std::vector<uint8_t> b2;
    cbor::cbor_bytes_serializer serializer2(b2);
    serializer2.begin_document();
    serializer2.begin_object(); // indefinite length object
    serializer2.name("City");
    serializer2.string_value("Toronto");
    serializer2.name("Province");
    serializer2.string_value("Ontario");
    serializer2.end_object(); 
    serializer2.end_document();
    cbor_view bv2 = b2;

    auto it2 = bv2.object_range().begin();
    BOOST_CHECK(it2 != bv2.object_range().end());
    BOOST_CHECK(++it2 != bv2.object_range().end());
    BOOST_CHECK(++it2 == bv2.object_range().end());
}

BOOST_AUTO_TEST_CASE(test_indefinite_length_array_iterator)
{
    std::vector<uint8_t> b1;
    cbor::cbor_bytes_serializer serializer1(b1);
    serializer1.begin_document();
    serializer1.begin_array(); // indefinite length array
    serializer1.end_array(); 
    serializer1.end_document();
    cbor_view bv1 = b1;
    BOOST_CHECK(bv1.array_range().begin() == bv1.array_range().end());

    std::vector<uint8_t> b2;
    cbor::cbor_bytes_serializer serializer2(b2);
    serializer2.begin_document();
    serializer2.begin_array(); // indefinite length array
    serializer2.string_value("Toronto");
    serializer2.string_value("Ontario");
    serializer2.end_array(); 
    serializer2.end_document();
    cbor_view bv2 = b2;

    BOOST_CHECK(bv2.size() == 2);

    auto it2 = bv2.array_range().begin();
    BOOST_CHECK(it2 != bv2.array_range().end());
    BOOST_CHECK(++it2 != bv2.array_range().end());
    BOOST_CHECK(++it2 == bv2.array_range().end());
}

BOOST_AUTO_TEST_SUITE_END()


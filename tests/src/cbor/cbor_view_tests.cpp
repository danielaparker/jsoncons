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

    json jv = decode_cbor<json>(v);
    std::cout << pretty_print(jv) << std::endl;

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

        std::cout << key << ": " << jval << std::endl;
    }
    std::cout << std::endl;

    for (auto element : reputons.array_range())
    {
        json j = decode_cbor<json>(element);
        std::cout << j << std::endl;
    }
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(jsonpointer_test)
{
    ojson j = ojson::parse(R"(
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
    std::cout << "Object dump" << std::endl;
    std::cout << s << std::endl;

    std::error_code ec;
    cbor_view application = jsonpointer::get(cbor_view(buffer), "/application", ec);
    BOOST_CHECK(!ec);

    json j2 = decode_cbor<json>(application);
    std::cout << pretty_print(j2) << std::endl;

    cbor_view reputons_0_rated = jsonpointer::get(cbor_view(buffer), "/reputons", ec);
    BOOST_CHECK(!ec);

    json j3 = decode_cbor<json>(reputons_0_rated);
    std::cout << pretty_print(j3) << std::endl;

}

BOOST_AUTO_TEST_CASE(as_string_test)
{
    std::vector<uint8_t> b;
    jsoncons::cbor::cbor_bytes_serializer serializer(b);
    serializer.begin_document();
    serializer.begin_array(9);
    serializer.bool_value(true);
    serializer.bool_value(false);
    serializer.null_value();
    serializer.string_value("Toronto");
    serializer.byte_string_value({'H','e','l','l','o'});
    serializer.integer_value(-100);
    serializer.uinteger_value(100);
    serializer.bignum_value("18446744073709551616");
    serializer.double_value(10.5);
    serializer.end_array();
    serializer.end_document();

    for (auto u : b)
    {
        std::cout << std::hex << (int)u;
    }
    std::cout << "\n\n";

    jsoncons::cbor::cbor_view bv = b;

    std::string s0;
    bv.at(0).dump(s0);
    BOOST_CHECK_EQUAL(std::string("true"), s0);

    std::string s1;
    bv.at(1).dump(s1);
    BOOST_CHECK_EQUAL(std::string("false"), s1);

    std::string s2;
    bv.at(2).dump(s2);
    BOOST_CHECK_EQUAL(std::string("null"), s2);

    std::string s3;
    bv.at(3).dump(s3);
    BOOST_CHECK_EQUAL(std::string("\"Toronto\""), s3);

    std::string s4;
    bv.at(4).dump(s4);
    BOOST_CHECK_EQUAL(std::string("\"SGVsbG8\""), s4);

    std::string s5;
    bv.at(5).dump(s5);
    BOOST_CHECK_EQUAL(std::string("-100"), s5);

    std::string s6;
    bv.at(6).dump(s6);
    BOOST_CHECK_EQUAL(std::string("100"), s6);

    std::string s7;
    bv.at(7).dump(s7);
    BOOST_CHECK_EQUAL(std::string("\"18446744073709551616\""), s7);

    std::string s8;
    bv.at(8).dump(s8);
    BOOST_CHECK_EQUAL(std::string("10.5"), s8);

    std::string s9;
    bv.dump(s9);
    std::cout << s9 << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()


// Copyright 2016 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h"
#endif
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>
#include <catch/catch.hpp>

using namespace jsoncons;
using namespace jsoncons::cbor;

#if !defined(JSONCONS_NO_DEPRECATED)

TEST_CASE("cbor_view_test")
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
    CHECK(v.is_object());
    CHECK_FALSE(v.is_array());

    ojson jv = decode_cbor<ojson>(v);
    CHECK(jv == j1);

    cbor_view reputons = v.at("reputons");
    CHECK(reputons.is_array());

    cbor_view reputons_0 = reputons.at(0);

    cbor_view reputons_0_rated = reputons_0.at("rated");
    (void)reputons_0_rated;

    cbor_view rating = reputons_0.at("rating");
    CHECK(rating.as_double() == 0.90);

    for (const auto& member : v.object_range())
    {
        const auto& key = member.key();
        const auto& val = member.value();

        (void)key;
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

TEST_CASE("jsonpointer_test")
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
    CHECK(j1 == j);

    std::error_code ec;
    cbor_view application = jsonpointer::get(cbor_view(buffer), "/application", ec);
    CHECK_FALSE(ec);

    json j2 = decode_cbor<json>(application);
    CHECK(j2 == j["application"]);

    cbor_view reputons_0_rated = jsonpointer::get(cbor_view(buffer), "/reputons", ec);
    CHECK_FALSE(ec);

    json j3 = decode_cbor<json>(reputons_0_rated);
    json j4 = j["reputons"];
    CHECK(j3 == j4);

    //std::cout << pretty_print(j3) << std::endl;
}

TEST_CASE("as_string_test")
{
    std::vector<uint8_t> b;
    jsoncons::cbor::cbor_buffer_serializer serializer(b);
    serializer.begin_array(10);
    serializer.bool_value(true);
    serializer.bool_value(false);
    serializer.null_value();
    serializer.string_value("Toronto");
    serializer.byte_string_value(byte_string{'H','e','l','l','o'});
    serializer.int64_value(-100);
    serializer.uint64_value(100);
    serializer.big_integer_value("18446744073709551616");
    serializer.double_value(10.5);
    serializer.big_integer_value("-18446744073709551617");
    serializer.end_array();
    serializer.flush();

    jsoncons::cbor::cbor_view bv = b;

    std::string s0;
    bv[0].dump(s0);
    CHECK(std::string("true") == s0);
    CHECK(std::string("true") == bv[0].as_string());
    CHECK(true == bv[0].as<bool>());
    CHECK(bv[0].is<bool>());

    std::string s1;
    bv[1].dump(s1);
    CHECK(std::string("false") == s1);
    CHECK(std::string("false") == bv[1].as_string());
    CHECK(false == bv[1].as<bool>());
    CHECK(bv[1].is<bool>());

    std::string s2;
    bv[2].dump(s2);
    CHECK(std::string("null") == s2);
    CHECK(std::string("null") == bv[2].as_string());

    std::string s3;
    bv[3].dump(s3);
    CHECK(std::string("\"Toronto\"") == s3);
    CHECK(std::string("Toronto") == bv[3].as_string());
    CHECK(std::string("Toronto") ==bv[3].as<std::string>());

    std::string s4;
    bv[4].dump(s4);
    CHECK(std::string("\"SGVsbG8\"") == s4);
    CHECK(std::string("SGVsbG8") == bv[4].as_string());
    CHECK(byte_string({'H','e','l','l','o'}) == bv[4].as<byte_string>());

    std::string s5;
    bv[5].dump(s5);
    CHECK(std::string("-100") ==s5);
    CHECK(std::string("-100") == bv[5].as_string());
    CHECK(-100 ==bv[5].as<int>());

    std::string s6;
    bv[6].dump(s6);
    CHECK(std::string("100") == s6);
    CHECK(std::string("100") == bv[6].as_string());

    std::string s7;
    bv[7].dump(s7);
    CHECK(std::string("\"18446744073709551616\"") == s7);
    CHECK(std::string("18446744073709551616") == bv[7].as_string());

    std::string s8;
    bv[8].dump(s8);
    CHECK(std::string("10.5") == s8);
    CHECK(std::string("10.5") == bv[8].as_string());

    std::string s9;
    bv[9].dump(s9);
    CHECK(std::string("\"-18446744073709551617\"") == s9);
    CHECK(std::string("-18446744073709551617") == bv[9].as_string());
}
TEST_CASE("test_dump_to_string")
{
    std::vector<uint8_t> b;
    cbor_buffer_serializer serializer(b);
    serializer.begin_array();
    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    bignum n(-1, bytes.data(), bytes.size());
    std::string s;
    n.dump(s);
    serializer.big_integer_value(s);
    serializer.end_array();
    serializer.flush();

    cbor_view bv = b;

    std::string s0;
    bv.dump(s0);
    CHECK("[\"-18446744073709551617\"]" == s0);
    //std::cout << s0 << std::endl;

    std::string s1;
    json_options options1;
    options1.big_integer_format(big_integer_chars_format::number);
    bv.dump(s1,options1);
    CHECK("[-18446744073709551617]" == s1);
    //std::cout << s1 << std::endl;

    std::string s2;
    json_options options2;
    options2.big_integer_format(big_integer_chars_format::base10);
    bv.dump(s2,options2);
    CHECK("[\"-18446744073709551617\"]" == s2);
    //std::cout << s2 << std::endl;

    std::string s3;
    json_options options3;
    options3.big_integer_format(big_integer_chars_format::base64url);
    bv.dump(s3,options3);
    CHECK("[\"~AQAAAAAAAAAA\"]" == s3);
    //std::cout << s3 << std::endl;
} 

TEST_CASE("test_dump_to_stream")
{
    std::vector<uint8_t> b;
    cbor_buffer_serializer serializer(b);
    serializer.begin_array();
    std::vector<uint8_t> bytes = {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    bignum n(-1, bytes.data(), bytes.size());
    std::string s;
    n.dump(s);
    serializer.big_integer_value(s);
    serializer.end_array();
    serializer.flush();

    cbor_view bv = b;

    std::ostringstream os0;
    bv.dump(os0);
    CHECK("[\"-18446744073709551617\"]" == os0.str());
    //std::cout << os0.str() << std::endl;

    std::ostringstream os1;
    json_options options1;
    options1.big_integer_format(big_integer_chars_format::number);
    bv.dump(os1,options1);
    CHECK("[-18446744073709551617]" == os1.str());
    //std::cout << os1.str() << std::endl;

    std::ostringstream os2;
    json_options options2;
    options2.big_integer_format(big_integer_chars_format::base10);
    bv.dump(os2,options2);
    CHECK("[\"-18446744073709551617\"]" == os2.str());
    //std::cout << os2.str() << std::endl;

    std::ostringstream os3;
    json_options options3;
    options3.big_integer_format(big_integer_chars_format::base64url);
    bv.dump(os3,options3);
    CHECK("[\"~AQAAAAAAAAAA\"]" == os3.str());
    //std::cout << os3.str() << std::endl;
} 

TEST_CASE("test_indefinite_length_object_iterator")
{
    std::vector<uint8_t> b;
    cbor::cbor_buffer_serializer serializer(b);
    serializer.begin_object(); // indefinite length object
    serializer.name("City");
    serializer.string_value("Toronto");
    serializer.name("Province");
    serializer.string_value("Ontario");
    serializer.end_object(); 
    serializer.flush();
    cbor_view bv2 = b;

    auto it2 = bv2.object_range().begin();
    CHECK_FALSE((it2 == bv2.object_range().end()));
    CHECK_FALSE((++it2 == bv2.object_range().end()));
    CHECK((++it2 == bv2.object_range().end()));
}

TEST_CASE("test_indefinite_length_array_iterator")
{
    std::vector<uint8_t> b;
    cbor::cbor_buffer_serializer serializer(b);
    serializer.begin_array(); // indefinite length array
    serializer.string_value("Toronto");
    serializer.string_value("Ontario");
    serializer.end_array(); 
    serializer.flush();
    cbor_view bv2 = b;

    CHECK(bv2.size() == 2);

    auto it2 = bv2.array_range().begin();
    CHECK(it2 != bv2.array_range().end());
    CHECK(++it2 != bv2.array_range().end());
    CHECK(++it2 == bv2.array_range().end());

}

TEST_CASE("cbor_view array comparison test")
{
    std::vector<uint8_t> buf1;
    cbor::cbor_buffer_serializer serializer1(buf1);
    serializer1.begin_array(); // indefinite length array
    serializer1.string_value("Toronto");
    serializer1.string_value("Vancouver");
    serializer1.end_array(); 
    serializer1.flush();
    cbor_view v1 = buf1;

    std::vector<uint8_t> buf2;
    cbor::cbor_buffer_serializer serializer2(buf2);
    serializer2.begin_array(); // indefinite length array
    serializer2.string_value("Toronto");
    serializer2.string_value("Vancouver");
    serializer2.end_array(); 
    serializer2.flush();
    cbor_view v2 = buf2;

    std::vector<uint8_t> buf3;
    cbor::cbor_buffer_serializer serializer3(buf3);
    serializer3.begin_array(); // indefinite length array
    serializer3.string_value("Toronto");
    serializer3.string_value("Montreal");
    serializer3.end_array(); 
    serializer3.flush();
    cbor_view v3 = buf3;

    SECTION("operator== test")
    {
        CHECK(v1 == v2);
        REQUIRE(v1.size() == 2);
        REQUIRE(v2.size() == 2);
        CHECK(v1[0] == v2[0]);
        CHECK(v1[1] == v2[1]);
    }

    SECTION("element operator== test")
    {
        CHECK_FALSE(v1 == v3);
        REQUIRE(v1.size() == 2);
        REQUIRE(v1.size() == v3.size());
        CHECK(v1[0] == v3[0]);
        CHECK_FALSE(v1[1] == v3[1]);
    }
}

TEST_CASE("cbor_view object comparison")
{
    std::vector<uint8_t> buf1;
    cbor::cbor_buffer_serializer serializer1(buf1);
    serializer1.begin_object(); // indefinite length array
    serializer1.name("City");
    serializer1.string_value("Montreal");
    serializer1.name("Amount");
    serializer1.big_decimal_value("273.15");
    serializer1.name("Date");
    serializer1.date_time_value("2018-05-07 12:41:07-07:00");
    serializer1.end_object(); 
    serializer1.flush();
    cbor_view view1 = buf1;

    REQUIRE(view1.size() == 3);

    std::vector<uint8_t> buf2;
    cbor::cbor_buffer_serializer serializer2(buf2);
    serializer2.begin_object(); // indefinite length array
    serializer2.name("City");
    serializer2.string_value("Toronto");
    serializer2.name("Amount");
    serializer2.big_decimal_value("273.15");
    serializer2.name("Date");
    serializer2.date_time_value("2018-10-18 12:41:07-07:00");
    serializer2.end_object(); 
    serializer2.flush();
    cbor_view view2 = buf2;
    REQUIRE(view2.size() == view1.size());

    std::vector<uint8_t> buf3;
    cbor::cbor_buffer_serializer serializer3(buf3);
    serializer3.begin_object(); // indefinite length array
    serializer3.name("empty-object");
    serializer3.begin_object(0);
    serializer3.end_object();
    serializer3.name("empty-array");
    serializer3.begin_array(0);
    serializer3.end_array();
    serializer3.name("empty-string");
    serializer3.string_value("");
    serializer3.name("empty-byte_string");
    serializer3.byte_string_value(jsoncons::byte_string{});
    serializer3.end_object(); 
    serializer3.flush();
    cbor_view view3 = buf3;

    SECTION("contains")
    {
        CHECK(view1.contains("City"));
        CHECK(view1.contains("Amount"));
        CHECK(view1.contains("Date"));
        CHECK_FALSE(view1.contains("Country"));
    }

    SECTION("empty")
    {
        CHECK_FALSE(view3.empty());
        CHECK(view3["empty-object"].empty());
        CHECK(view3["empty-array"].empty());
        CHECK(view3["empty-string"].empty());
        CHECK(view3["empty-byte_string"].empty());
    }

    SECTION("size")
    {
        CHECK(view1.size() == 3);
    }

    SECTION("operator==")
    {
        CHECK_FALSE(view1 == view2);
        CHECK_FALSE(view1["City"] == view2["City"]);
        CHECK(view1["Amount"] == view2["Amount"]);
        CHECK_FALSE(view1["Date"] == view2["Date"]);
    }

}

TEST_CASE("cbor_view member tests")
{
    std::vector<uint8_t> buf;
    cbor::cbor_buffer_serializer serializer(buf);
    serializer.begin_object(); // indefinite length object
    serializer.name("empty-object");
    serializer.begin_object(0);
    serializer.end_object();
    serializer.name("empty-array");
    serializer.begin_array(0);
    serializer.end_array();
    serializer.name("empty-string");
    serializer.string_value("");
    serializer.name("empty-byte_string");
    serializer.byte_string_value(jsoncons::byte_string{});

    serializer.name("City");
    serializer.string_value("Montreal");
    serializer.name("Amount");
    serializer.big_decimal_value("273.15");
    serializer.name("Date");
    serializer.date_time_value("2018-05-07 12:41:07-07:00");

    serializer.end_object(); 
    serializer.flush();
    cbor_view view = buf;

    SECTION("contains")
    {
        CHECK(view.contains("City"));
        CHECK(view.contains("Amount"));
        CHECK(view.contains("Date"));
        CHECK_FALSE(view.contains("Country"));
    }

    SECTION("empty")
    {
        CHECK_FALSE(view.empty());
        CHECK(view["empty-object"].empty());
        CHECK(view["empty-array"].empty());
        CHECK(view["empty-string"].empty());
        CHECK(view["empty-byte_string"].empty());
    }

    SECTION("size")
    {
        CHECK(view.size() == 7);
    }
}

TEST_CASE("cbor conversion tests")
{
    std::vector<uint8_t> b;
    cbor::cbor_buffer_serializer writer(b);
    writer.begin_array(); // indefinite length outer array
    writer.begin_array(4); // a fixed length array
    writer.string_value("foo");
    writer.byte_string_value(byte_string{'P','u','s','s'}); // no suggested conversion
    writer.big_integer_value("-18446744073709551617");
    writer.big_decimal_value("273.15");
    writer.end_array();
    writer.end_array();
    writer.flush();

    cbor_view bv = b;
    REQUIRE(bv.size() == 1);

    auto range1 = bv.array_range();
    auto it = range1.begin();
    cbor_view inner_array = *it++;
    REQUIRE(inner_array.size() == 4);
    REQUIRE(it == range1.end());

    auto range2 = inner_array.array_range();
    auto it2 = range2.begin();
    CHECK(it2->as_string() == "foo");
    it2++;
    CHECK(it2->as_byte_string() == byte_string{'P','u','s','s'});
    it2++;
    CHECK(bool(it2->as_bignum() == bignum{"-18446744073709551617"}));
    it2++;
    CHECK(bool(it2->as_string() == bignum{"273.15"}));
    it2++;
    CHECK(it2 == range2.end());
}
#endif

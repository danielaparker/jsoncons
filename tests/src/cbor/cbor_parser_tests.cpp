// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <catch/catch.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/cbor/cbor_parser.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <limits>

using namespace jsoncons;
using namespace jsoncons::cbor;

void check_parsing(const std::vector<uint8_t>& v, const json& expected)
{
    try
    {
        std::error_code ec;

        jsoncons::json_decoder<json> decoder;
        cbor_parser parser(decoder);
        parser.update(v.data(),v.size());
        parser.parse_some(ec);

        json result = decoder.get_result();

        if (!(expected == result))
        {
            std::cout << "v: ";
            for (auto b : v)
            {
                std::cout << "0x" << std::hex << (int)b;
            }
            std::cout << "\n";
            std::cout << "expected: " << expected << "\n";
            std::cout << "result: " << result << "\n";
        }

        REQUIRE(expected == result);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        std::cout << expected.to_string() << std::endl;
    }
}

TEST_CASE("test_cbor_parsing")
{
    // unsigned integer
    check_parsing({0x00},json(0U));
    check_parsing({0x01},json(1U));
    check_parsing({0x0a},json(10U));
    check_parsing({0x17},json(23U));
    check_parsing({0x18,0x18},json(24U));
    check_parsing({0x18,0xff},json(255U));
    check_parsing({0x19,0x01,0x00},json(256U));
    check_parsing({0x19,0xff,0xff},json(65535U));
    check_parsing({0x1a,0,1,0x00,0x00},json(65536U));
    check_parsing({0x1a,0xff,0xff,0xff,0xff},json(4294967295U));
    check_parsing({0x1b,0,0,0,1,0,0,0,0},json(4294967296U));
    check_parsing({0x1b,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},json(std::numeric_limits<uint64_t>::max()));

    // positive signed integer
    check_parsing({0x00},json(0));
    check_parsing({0x01},json(1));
    check_parsing({0x0a},json(10));
    check_parsing({0x17},json(23));
    check_parsing({0x18,0x18},json(24));
    check_parsing({0x18,0xff},json(255));
    check_parsing({0x19,0x01,0x00},json(256));
    check_parsing({0x19,0xff,0xff},json(65535));
    check_parsing({0x1a,0,1,0x00,0x00},json(65536));
    check_parsing({0x1a,0xff,0xff,0xff,0xff},json(4294967295));
    check_parsing({0x1b,0,0,0,1,0,0,0,0},json(4294967296));
    check_parsing({0x1b,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff},json(std::numeric_limits<int64_t>::max()));
    // negative integers
    check_parsing({0x20},json(-1));
    check_parsing({0x21},json(-2));
    check_parsing({0x37},json(-24));
    check_parsing({0x38,0x18},json(-25));
    check_parsing({0x38,0xff},json(-256));
    check_parsing({0x39,0x01,0x00},json(-257));
    check_parsing({0x39,0xff,0xff},json(-65536));
    check_parsing({0x3a,0,1,0x00,0x00},json(-65537));

    check_parsing({0x3a,0xff,0xff,0xff,0xff},json(-4294967296));
    check_parsing({0x3b,0,0,0,1,0,0,0,0},json(-4294967297));

    // null, true, false
    check_parsing({0xf6},json::null());

    check_parsing({0xf5},json(true));
    check_parsing({0xf4},json(false));

    // floating point
    check_parsing({0xfb,0,0,0,0,0,0,0,0},json(0.0));
    check_parsing({0xfb,0xbf,0xf0,0,0,0,0,0,0},json(-1.0));
    check_parsing({0xfb,0xc1,0x6f,0xff,0xff,0xe0,0,0,0},json(-16777215.0));

    // byte string
    std::vector<uint8_t> v;
    check_parsing({0x40},json(byte_string_view(v.data(),v.size())));
    v = {' '};
    check_parsing({0x41,' '},json(byte_string_view(v.data(),v.size())));
    v = {0};
    check_parsing({0x41,0},json(byte_string_view(v.data(),v.size())));
    v = {'H','e','l','l','o'};
    check_parsing({0x45,'H','e','l','l','o'},json(byte_string_view(v.data(),v.size())));
    v = {'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'};
    check_parsing({0x58,0x18,'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'},
                 json(byte_string_view(v.data(),v.size())));

    // string
    check_parsing({0x60},json(""));
    check_parsing({0x61,' '},json(" "));
    check_parsing({0x78,0x18,'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0','1','2','3','4'},
                 json("123456789012345678901234"));

    // byte strings with undefined length
    check_parsing({0x5f,0xff}, json(byte_string()));
    check_parsing({0x5f,0x40,0xff}, json(byte_string()));
    check_parsing({0x5f,0x40,0x40,0xff}, json(byte_string()));

    check_parsing({0x5f,0x43,'H','e','l',0x42,'l','o',0xff}, json(byte_string("Hello")));
    check_parsing({0x5f,0x41,'H',0x41,'e',0x41,'l',0x41,'l',0x41,'o',0xff}, json(byte_string("Hello")));
    check_parsing({0x5f,0x41,'H',0x41,'e',0x40,0x41,'l',0x41,'l',0x41,'o',0xff}, json(byte_string("Hello")));

    // text strings with undefined length

    check_parsing({0x7f,0xff}, json(""));

    check_parsing({0x7f,0x60,0xff}, json(""));
    check_parsing({0x7f,0x60,0x60,0xff}, json(""));
    check_parsing({0x7f,0x63,'H','e','l',0x62,'l','o',0xff}, json("Hello"));
    check_parsing({0x7f,0x61,'H',0x61,'e',0x61,'l',0x61,'l',0x61,'o',0xff}, json("Hello"));
    check_parsing({0x7f,0x61,'H',0x61,'e',0x61,'l',0x60,0x61,'l',0x61,'o',0xff}, json("Hello"));

    // arrays
    check_parsing({0x80},json::array());
    check_parsing({0x81,'\0'},json::parse("[0]"));
    check_parsing({0x82,'\0','\0'},json::array({0,0}));
    check_parsing({0x82,0x81,'\0','\0'}, json::parse("[[0],0]"));
    check_parsing({0x81,0x65,'H','e','l','l','o'},json::parse("[\"Hello\"]"));

    // indefinite length arrays
    check_parsing({0x9f,0xff},json::array());
    check_parsing({0x9f,0x9f,0xff,0xff},json::parse("[[]]"));

    // maps
    check_parsing({0xa0},json::object());
    check_parsing({0xa1,0x62,'o','c',0x81,'\0'}, json::parse("{\"oc\": [0]}"));
    check_parsing({0xa1,0x62,'o','c',0x84,'\0','\1','\2','\3'}, json::parse("{\"oc\": [0, 1, 2, 3]}"));

    // indefinite length maps
    check_parsing({0xbf,0xff},json::object());
    check_parsing({0xbf,0x64,'N','a','m','e',0xbf,0xff,0xff},json::parse("{\"Name\":{}}"));

    // bignum
    check_parsing({0xc2,0x49,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
                  json(bignum(1,{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00})));

    // date_time
    check_parsing({0xc0,0x78,0x19,'2','0','1','5','-','0','5','-','0','7',' ','1','2',':','4','1',':','0','7','-','0','7',':','0','0'},
                  json("2015-05-07 12:41:07-07:00", semantic_tag_type::date_time));

    // epoch_time
    check_parsing({0xc1,0x1a,0x55,0x4b,0xbf,0xd3},
                  json(1431027667, semantic_tag_type::epoch_time));
}

TEST_CASE("cbor decimal fraction")
{
    check_parsing({0xc4, // Tag 4
                   0x82, // Array of length 2
                   0x21, // -2
                   0x19,0x6a,0xb3 // 27315
                   },
                  json("273.15", semantic_tag_type::decimal));
    check_parsing({0xc4, // Tag 4
                   0x82, // Array of length 2
                   0x22, // -3
                   0x19,0x6a,0xb3 // 27315
                   },
                  json("27.315", semantic_tag_type::decimal));
    check_parsing({0xc4, // Tag 4
                   0x82, // Array of length 2
                   0x23, // -4
                   0x19,0x6a,0xb3 // 27315
                   },
                  json("2.7315", semantic_tag_type::decimal));
    check_parsing({0xc4, // Tag 4
                   0x82, // Array of length 2
                   0x24, // -5
                   0x19,0x6a,0xb3 // 27315
                   },
                  json("0.27315", semantic_tag_type::decimal));
    check_parsing({0xc4, // Tag 4
                   0x82, // Array of length 2
                   0x25, // -6
                   0x19,0x6a,0xb3 // 27315
                   },
                  json("0.27315e-1", semantic_tag_type::decimal));

    check_parsing({0xc4, // Tag 4
                   0x82, // Array of length 2
                   0x04, // 4
                   0x19,0x6a,0xb3 // 27315
                   },
                  json("27315e4", semantic_tag_type::decimal));
}

TEST_CASE("test_decimal_as_string")
{
    SECTION("-2 27315")
    {
        std::vector<uint8_t> v = {0xc4, // Tag 4
                                  0x82, // Array of length 2
                                  0x21, // -2
                                  0x19,0x6a,0xb3 // 27315
                                  };

        const uint8_t* endp = nullptr;
        std::string s = cbor::detail::get_decimal_as_string(v.data(),v.data()+v.size(),&endp);
        REQUIRE_FALSE(endp == v.data());
        REQUIRE(endp == (v.data()+v.size()));
        CHECK(std::string("273.15") == s);
    }
    SECTION("-6 27315")
    {
        std::vector<uint8_t> v = {0xc4, // Tag 4
                                  0x82, // Array of length 2
                                  0x25, // -6
                                  0x19,0x6a,0xb3 // 27315
                                  };

        const uint8_t* endp = nullptr;
        std::string s = cbor::detail::get_decimal_as_string(v.data(),v.data()+v.size(),&endp);
        REQUIRE_FALSE(endp == v.data());
        REQUIRE(endp == (v.data()+v.size()));
        CHECK(std::string("0.27315e-1") == s);
    }
    SECTION("-5 27315")
    {
        std::vector<uint8_t> v = {0xc4, // Tag 4
                                  0x82, // Array of length 2
                                  0x24, // -5
                                  0x19,0x6a,0xb3 // 27315
                                  };

        const uint8_t* endp = nullptr;
        std::string s = cbor::detail::get_decimal_as_string(v.data(),v.data()+v.size(),&endp);
        REQUIRE_FALSE(endp == v.data());
        REQUIRE(endp == (v.data()+v.size()));
        CHECK(std::string("0.27315") == s);
    }
    SECTION("0 27315")
    {
        std::vector<uint8_t> v = {0xc4, // Tag 4
                                  0x82, // Array of length 2
                                  0x00, // 0
                                  0x19,0x6a,0xb3 // 27315
                                  };

        const uint8_t* endp = nullptr;
        std::string s = cbor::detail::get_decimal_as_string(v.data(),v.data()+v.size(),&endp);
        REQUIRE_FALSE(endp == v.data());
        REQUIRE(endp == (v.data()+v.size()));
        CHECK(std::string("27315.0") == s);
    }
    SECTION("2 27315")
    {
        std::vector<uint8_t> v = {0xc4, // Tag 4
                                  0x82, // Array of length 2
                                  0x02, // 2
                                  0x19,0x6a,0xb3 // 27315
                                  };

        const uint8_t* endp = nullptr;
        std::string s = cbor::detail::get_decimal_as_string(v.data(),v.data()+v.size(),&endp);
        REQUIRE_FALSE(endp == v.data());
        REQUIRE(endp == (v.data()+v.size()));
        CHECK(s == std::string("27315e2"));
    }
    SECTION("-2 18446744073709551616")
    {
        std::vector<uint8_t> v = {0xc4, // Tag 4
                                  0x82, // Array of length 2
                                  0x21, // -2
                                  0xc2,0x49,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 // 18446744073709551616
                                  };

        const uint8_t* endp = nullptr;
        std::string s = cbor::detail::get_decimal_as_string(v.data(),v.data()+v.size(),&endp);
        REQUIRE_FALSE(endp == v.data());
        REQUIRE(endp == (v.data()+v.size()));
        CHECK(std::string("184467440737095516.16") == s);
    }
    SECTION("-2 -65537")
    {
        std::vector<uint8_t> v = {0xc4, // Tag 4
                                  0x82, // Array of length 2
                                  0x21, // -2
                                  0x3a,0,1,0x00,0x00 // -65537
                                  };

        const uint8_t* endp = nullptr;
        std::string s = cbor::detail::get_decimal_as_string(v.data(),v.data()+v.size(),&endp);
        REQUIRE_FALSE(endp == v.data());
        REQUIRE(endp == (v.data()+v.size()));
        CHECK(s == std::string("-655.37"));
    }
    SECTION("-5 -65537")
    {
        std::vector<uint8_t> v = {0xc4, // Tag 4
                                  0x82, // Array of length 2
                                  0x24, // -5
                                  0x3a,0,1,0x00,0x00 // -65537
                                  };

        const uint8_t* endp = nullptr;
        std::string s = cbor::detail::get_decimal_as_string(v.data(),v.data()+v.size(),&endp);
        REQUIRE_FALSE(endp == v.data());
        REQUIRE(endp == (v.data()+v.size()));
        CHECK(s == std::string("-0.65537"));
    }
    SECTION("-6 -65537")
    {
        std::vector<uint8_t> v = {0xc4, // Tag 4
                                  0x82, // Array of length 2
                                  0x25, // -6
                                  0x3a,0,1,0x00,0x00 // -65537
                                  };

        const uint8_t* endp = nullptr;
        std::string s = cbor::detail::get_decimal_as_string(v.data(),v.data()+v.size(),&endp);
        REQUIRE_FALSE(endp == v.data());
        REQUIRE(endp == (v.data()+v.size()));
        CHECK(s == std::string("-0.65537e-1"));
    }
}

TEST_CASE("Compare CBOR packed item and jsoncons item")
{
    std::vector<uint8_t> bytes;
    cbor::cbor_bytes_serializer writer(bytes);
    writer.begin_array(); // indefinite length outer array
    writer.string_value("foo");
    writer.byte_string_value({'b','a','r'});
    writer.bignum_value("-18446744073709551617");
    writer.decimal_value("273.15");
    writer.date_time_value("2018-10-19 12:41:07-07:00");
    writer.epoch_time_value(1431027667);
    writer.int64_value(-1431027667, semantic_tag_type::epoch_time);
    writer.double_value(1431027667.5, jsoncons::floating_point_options(), semantic_tag_type::epoch_time);
    writer.end_array();
    writer.flush();

    //std::cout << "(1)\n";
    //for (auto c : bytes)
    //{
    //    std::cout << std::hex << std::noshowbase << std::setprecision(2) << std::setw(2)
    //              << std::setfill('0') << static_cast<int>(c);
    //}
    //std::cout << "\n\n";

/*
9f -- Start indefinite length array 
  63 -- String value of length 3 
    666f6f -- "foo"
  43 -- Byte string value of length 3
    626172 -- 'b''a''r'
  c3 -- Tag 3 (negative bignum)
    49 Byte string value of length 9
      010000000000000000 -- Bytes content
  c4  - Tag 4 (decimal fraction)
    82 -- Array of length 2
      21 -- -2
      19 6ab3 -- 27315
  c0 -- Tag 0 (date-time)
    78 19 -- Length (25)
      323031352d30352d30372031323a34313a30372d30373a3030 -- "2015-05-07 12:41:07-07:00"
  c1 -- Tag 1 (epoch time)
    1a -- uint32_t
      554bbfd3 -- 1431027667 
  c1
    3a
      554bbfd2
  c1
    fb
      41d552eff4e00000
  ff -- "break" 
*/

    json expected = json::array();

    expected.emplace_back("foo");
    expected.emplace_back(byte_string{ 'b','a','r' });
    expected.emplace_back("-18446744073709551617", semantic_tag_type::bignum);
    expected.emplace_back("273.15", semantic_tag_type::decimal);
    expected.emplace_back("2018-10-19 12:41:07-07:00", semantic_tag_type::date_time);
    expected.emplace_back(1431027667, semantic_tag_type::epoch_time);
    expected.emplace_back(-1431027667, semantic_tag_type::epoch_time);
    expected.emplace_back(1431027667.5, semantic_tag_type::epoch_time);

    json j = cbor::decode_cbor<json>(bytes);

    CHECK(expected == j);
}



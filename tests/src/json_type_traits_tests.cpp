// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstdint>

using namespace jsoncons;
// own vector will always be of an even length 
struct own_vector : std::vector<int64_t> { using  std::vector<int64_t>::vector; };

namespace jsoncons {
template<class Json>
struct json_type_traits<Json, own_vector> {
    static bool is(const Json& rhs) noexcept { return true; }
    static own_vector as(const Json& rhs) { return own_vector(); }
    static Json to_json(const own_vector& val) {
        Json j;
        for (uint64_t i = 0; i<val.size(); i = i + 2) {
            j[std::to_string(val[i])] = val[i + 1];
        }
        return j;
    }
};
};

BOOST_AUTO_TEST_SUITE(json_type_traits_tests)

BOOST_AUTO_TEST_CASE(test_trait_type_erasure)
{
    json::object o;

    json val;

    val = o;

    val.insert_or_assign("A",o);
}

BOOST_AUTO_TEST_CASE(test_assign_non_const_cstring)
{
    json root;

    const char* p = "A string";
    char* q = const_cast<char*>(p);

    root["Test"] = q;
}

BOOST_AUTO_TEST_CASE(test_uint8_t)
{
    uint8_t x = 10;

    json o;
    o["u"] = x;

    BOOST_CHECK(o["u"].is_number());

    uint8_t y = o["u"].as<uint8_t>();

    BOOST_CHECK(y == 10);
}

BOOST_AUTO_TEST_CASE(test_float_assignment)
{
    float x = 10.5;

    json o;
    o["float"] = x;

    BOOST_CHECK(o["float"].is_number());

    float y = o["float"].as<float>();

    BOOST_CHECK_CLOSE(10.5,y,0.00001);
}

BOOST_AUTO_TEST_CASE(test_float)
{
    float x = 10.5;

    json o(x);

    BOOST_CHECK(o.is<float>());

    float y = o.as<float>();

    BOOST_CHECK_CLOSE(10.5,y,0.00001);
}

BOOST_AUTO_TEST_CASE(test_unsupported_type)
{
    json o;

    //o["u"] = Info; 
    // compile error
}

BOOST_AUTO_TEST_CASE(test_as_json_value)
{
    json a;

    a["first"] = "first"; 
    a["second"] = "second"; 

    BOOST_CHECK_EQUAL(true,a.is<json>());
    
    json b = a.as<json>();
    BOOST_CHECK_EQUAL("first",b["first"].as<std::string>());
    BOOST_CHECK_EQUAL("second",b["second"].as<std::string>());
}

BOOST_AUTO_TEST_CASE(test_byte_string_as_vector)
{
    json a(byte_string{'H','e','l','l','o'});

    BOOST_REQUIRE(a.is_byte_string());

    auto bs = a.as<byte_string>();

    BOOST_REQUIRE(5 == bs.size());
    BOOST_CHECK('H' == bs[0]);
    BOOST_CHECK('e' == bs[1]);
    BOOST_CHECK('l' == bs[2]);
    BOOST_CHECK('l' == bs[3]);
    BOOST_CHECK('o' == bs[4]);
}
/*
BOOST_AUTO_TEST_CASE(test_own_vector)
{
    jsoncons::json j = own_vector({0,9,8,7});
    std::cout << j;
}
*/

BOOST_AUTO_TEST_SUITE_END()

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

BOOST_AUTO_TEST_SUITE(json_type_traits_tests)

BOOST_AUTO_TEST_CASE(test_trait_type_erasure)
{
    json::object o;

    json val;

    val = o;

    val.set("A",o);
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

BOOST_AUTO_TEST_SUITE_END()



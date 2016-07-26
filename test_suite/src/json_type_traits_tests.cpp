// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "my_any_specializations.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstdint>

using namespace jsoncons;
using boost::numeric::ublas::matrix;

BOOST_AUTO_TEST_SUITE(json_type_traits_test_suite)

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
    json o;

    uint8_t x = 10;
    o["u"] = x;

    BOOST_CHECK(o["u"].is_number());

    uint8_t y = o["u"].as<uint8_t>();

    BOOST_CHECK(y == 10);
}

BOOST_AUTO_TEST_CASE(test_unsupported_type)
{
    json o;

    //o["u"] = Info; 
    // compile error
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_bool)
{
    json a = json::parse("[true,false,true]");

    std::vector<bool> v = a.as<std::vector<bool>>();

    BOOST_CHECK_EQUAL(v[0],true);
    BOOST_CHECK_EQUAL(v[1],false);
    BOOST_CHECK_EQUAL(v[2],true);
}

BOOST_AUTO_TEST_CASE(test_assign_vector_of_bool)
{
    std::vector<bool> v = {true,false,true};
    json a = v;

    BOOST_CHECK_EQUAL(a[0],true);
    BOOST_CHECK_EQUAL(a[1],false);
    BOOST_CHECK_EQUAL(a[2],true);

    json b;
    b = v;

    BOOST_CHECK_EQUAL(b[0],true);
    BOOST_CHECK_EQUAL(b[1],false);
    BOOST_CHECK_EQUAL(b[2],true);
}

BOOST_AUTO_TEST_SUITE_END()



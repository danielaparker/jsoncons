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

enum log_level_t
{
        Trace = 0,
        Debug,
        Info,
        Warning,
        Error,
        Critical
};

BOOST_AUTO_TEST_CASE(test_trait_type_erasure)
{
    json::object o;

	json val;

	val = o;

	val.set("A",o);
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

BOOST_AUTO_TEST_SUITE_END()



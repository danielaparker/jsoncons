// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_integer_test_suite)

BOOST_AUTO_TEST_CASE(test_integer_limits)
{
    const long long max_value = (std::numeric_limits<long long>::max)();

    const unsigned long long max_uvalue = (std::numeric_limits<unsigned long long>::max)();
    {
        std::ostringstream os;

        //os << max_value;

        os << "{\"max_longlong\":-" << max_value << "}";
        json val = json::parse(os.str());
        std::cout << val << std::endl;
        BOOST_CHECK(val["max_longlong"].is_integer());
    }
    {
        std::ostringstream os;

        //os << max_value;

        std::cout << "TEST LIMITS" << std::endl;
        os << "{\"max_longlong_overflow\":-" << max_value << "0}";
        std::cout << os.str() << std::endl;


        json val = json::parse(os.str());
        std::cout << val << std::endl;
        BOOST_CHECK(val["max_longlong_overflow"].is_double());
    }
    {
        std::ostringstream os;

        os << "{\"max_ulonglong\":" << max_uvalue << "}";
        json val = json::parse(os.str());
        std::cout << val << std::endl;
        BOOST_CHECK(val["max_ulonglong"].is_uinteger());
    }
    {
        std::ostringstream os;

        os << "{\"max_ulonglong_overflow\":" << max_uvalue << "0}";
        json val = json::parse(os.str());
        std::cout << val << std::endl;
        BOOST_CHECK(val["max_ulonglong_overflow"].is_double());
    }
}

BOOST_AUTO_TEST_SUITE_END()


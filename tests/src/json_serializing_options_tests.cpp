// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include <boost/optional.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_serializing_options_tests)

BOOST_AUTO_TEST_CASE(test_default_nan_replacement)
{
    json obj;
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308 * 1000;
    obj["field3"] = -1.79e308 * 1000;

    std::ostringstream os;
    os << print(obj);
    std::string expected = R"({"field1":null,"field2":null,"field3":null})";

    BOOST_CHECK_EQUAL(expected,os.str());
}

BOOST_AUTO_TEST_CASE(test_write_nan_replacement)
{
    json obj;
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308 * 1000;
    obj["field3"] = -1.79e308 * 1000;

    json_serializing_options options;
    options.nan_replacement("null");
    options.pos_inf_replacement("1e9999");
    options.neg_inf_replacement("-1e9999");

    std::ostringstream os;
    os << print(obj, options);
    std::string expected = R"({"field1":null,"field2":1e9999,"field3":-1e9999})";

    BOOST_CHECK_EQUAL(expected,os.str());
}

BOOST_AUTO_TEST_CASE(test_read_nan_replacement)
{
    std::string s = R"({"field1":"NaN","field2":"PositiveInfinity","field3":"NegativeInfinity"})";

    json_serializing_options options;
    options.nan_replacement("\"NaN\"");
    options.pos_inf_replacement("\"PositiveInfinity\"");
    options.neg_inf_replacement("\"NegativeInfinity\"");

    json j = json::parse(s,options);

    std::cout << "\n(1)\n" << pretty_print(j) << std::endl;

    std::cout << "\n(2)\n" << pretty_print(j,options) << std::endl;

    json expected;
    expected["field1"] = std::nan("");
    expected["field2"] = std::numeric_limits<double>::infinity();
    expected["field3"] = -std::numeric_limits<double>::infinity();

    BOOST_CHECK(expected.to_string(options) == j.to_string(options));
    BOOST_CHECK(expected.to_string() == j.to_string());
}

BOOST_AUTO_TEST_SUITE_END()


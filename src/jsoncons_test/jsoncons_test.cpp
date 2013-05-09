#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "jsoncons/json_parser.hpp"
#include "jsoncons/json_value.hpp"
#include <sstream>

using jsoncons::json_value;
using jsoncons::json_parser;
using std::string;

BOOST_AUTO_TEST_CASE( test1 )
{
    std::istringstream is("{\"unicode_string_1\":\"\\uD800\\uDC00\"}");

    json_value root = json_value::parse(is);

    root["double_1"] = 10.0;

    json_value double_1_value = root["double_1"];

    double double_1 = root["double_1"];

    BOOST_CHECK_CLOSE(double_1, 10.0, 0.000001);

    json_value copy = root;
}

BOOST_AUTO_TEST_CASE(test_assignment)
{
    json_value root;

	root["double_1"] = 10.0;

    json_value double_1_value = root["double_1"];

    double double_1 = root["double_1"];

    BOOST_CHECK_CLOSE(double_1, 10.0, 0.000001);

    root["myobject"] = json_value::object_prototype;
    root["myobject"]["double_2"] = 7.0;
    root["myobject"]["bool_2"] = true;
    root["myobject"]["int_2"] = 0;
    root["myobject"]["string_2"] = "my string";
    root["myarray"] = json_value::array_prototype;

	std::cout << root.to_string() << std::endl;

    //json_value double_2_value = root["double_2"];

    //BOOST_CHECK(double_2_value.is_null());
}



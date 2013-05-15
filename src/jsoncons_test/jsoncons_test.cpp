// Copyright 2013 Daniel Parker
// Distributed under Boost license

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_parser.hpp"
#include <sstream>
#include <vector>
#include <utility>

using jsoncons::json;
using jsoncons::json_parser;
using std::string;

BOOST_AUTO_TEST_CASE( test1 )
{
    std::istringstream is("{\"unicode_string_1\":\"\\uD800\\uDC00\"}");

    json root = json::parse(is);

    root["double_1"] = 10.0;

    json double_1_value = root["double_1"];

    double double_1 = root["double_1"];

    BOOST_CHECK_CLOSE(double_1, 10.0, 0.000001);

    json copy = root;
}

BOOST_AUTO_TEST_CASE(test_assignment)
{
    json root;

	root["double_1"] = 10.0;

    json double_1_value = root["double_1"];

    double double_1 = root["double_1"];

    BOOST_CHECK_CLOSE(double_1, 10.0, 0.000001);

    root["myobject"] = json::object_prototype;
    root["myobject"]["double_2"] = 7.0;
    root["myobject"]["bool_2"] = true;
    root["myobject"]["int_2"] = 0;
    root["myobject"]["string_2"] = "my string";
    root["myarray"] = json::array_prototype;

    double double_2 = root["myobject"]["double_2"];
    double int_2 = root["myobject"]["double_2"];

    BOOST_CHECK_CLOSE(double_2, 7.0, 0.000001);
    BOOST_CHECK_CLOSE(int_2, 7.0, 0);

	std::cout << root.to_string() << std::endl;

    //json double_2_value = root["double_2"];

    //BOOST_CHECK(double_2_value.is_null());
}

BOOST_AUTO_TEST_CASE(test_array)
{
    json root;

    root["addresses"];

    std::vector<json> addresses;
    json address1;
    address1["city"] = "San Francisco";
    address1["state"] = "CA";
    address1["zip"] = "94107";
    address1["country"] = "USA";
    addresses.push_back(address1);

    json address2;
    address2["city"] = "Sunnyvale";
    address2["state"] = "CA";
    address2["zip"] = "94085";
    address2["country"] = "USA";
    addresses.push_back(address2);
    
    root["addresses"] = json(addresses.begin(),addresses.end());

    std::cout << root << std::endl;

    BOOST_CHECK(root["addresses"].size() == 2);

    //for (size_t i = 0; i < root["addresses"].size(); ++i)
    //{
    //    std::cout << root["addresses"][i] << std::endl;
    //}
}



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
using jsoncons::integer_type;
using jsoncons::uinteger_type;
using jsoncons::json_parser;
using std::string;

BOOST_AUTO_TEST_CASE( test1 )
{

    std::istringstream is("{\"unicode_string_1\":\"\\uD800\\uDC00\"}");

    json root = json::parse(is);

    root["double_1"] = json(10.0);

    json double_1 = root["double_1"];

    BOOST_CHECK_CLOSE(double_1.as_double(), 10.0, 0.000001);

    json copy = root;
}

BOOST_AUTO_TEST_CASE(test_assignment)
{
    json root;

	root["double_1"] = json(10.0);

    json double_1 = root["double_1"];

    BOOST_CHECK_CLOSE(double_1.as_double(), 10.0, 0.000001);

    root["myobject"] = json::object_prototype;
    root["myobject"]["double_2"] = json(7.0);
    root["myobject"]["bool_2"] = json(true);
    root["myobject"]["int_2"] = json(integer_type(0));
    root["myobject"]["string_2"] = json("my string");
    root["myarray"] = json::array_prototype;

    json double_2 = root["myobject"]["double_2"];
    json int_2 = root["myobject"]["double_2"];

    BOOST_CHECK_CLOSE(double_2.as_double(), 7.0, 0.000001);
    BOOST_CHECK_CLOSE(int_2.as_int(), 7.0, 0);

	std::cout << root << std::endl;

    //json double_2_value = root["double_2"];

    //BOOST_CHECK(double_2_value.is_null());
}

BOOST_AUTO_TEST_CASE(test_array)
{
    json root;

    root["addresses"];

    std::vector<json> addresses;
    json address1;
    address1["city"] = json("San Francisco");
    address1["state"] = json("CA");
    address1["zip"] = json("94107");
    address1["country"] = json("USA");
    addresses.push_back(address1);

    json address2;
    address2["city"] = json("Sunnyvale");
    address2["state"] = json("CA");
    address2["zip"] = json("94085");
    address2["country"] = json("USA");
    addresses.push_back(address2);
    
    root["addresses"] = json(addresses.begin(),addresses.end());

    std::cout << root << std::endl;

    BOOST_CHECK(root["addresses"].size() == 2);

    std::cout << "size=" << root["addresses"].size() << std::endl;
    for (size_t i = 0; i < root["addresses"].size(); ++i)
    {
        std::cout << root["addresses"][i].to_string() << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(example)
{
    std::string in = "\"getValuesReturn\" : {\"return\" : \"true\",\"TextTag\" : \"Text!\",\"String\" : [\"First item\",\"Second item\",\"Third item\"],\"TagWithAttrsAndText\" : {\"content\" : \"Text!\",\"attr3\" : \"value3\",\"attr2\" : \"value2\",\"attr1\" : \"value1\"},\"EmptyTag\" : true,\"attribute\" : {\"attrValue\" : \"value\"},\"TagWithAttrs\" : {\"attr3\" : \"value3\",\"attr2\" : \"value2\",\"attr1\" : \"value1\"}}}";
    std::istringstream is(in);

    json root = json::parse(is);

    std::cout << root << std::endl;
}

BOOST_AUTO_TEST_CASE(test_null)
{
    json nullval = json::null_prototype;
    BOOST_CHECK(nullval.is_null());
}


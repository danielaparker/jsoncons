// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_stream_writer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::json_stream_writer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::basic_json_parser;
using std::string;

BOOST_AUTO_TEST_CASE(test_construction_from_string)
{
    std::string input = "{\"first_name\":\"Jane\",\"last_name\":\"Roe\",\"events_attended\":10}";

    json val = json::parse_string(input);

    std::cout << val << std::endl;
}

BOOST_AUTO_TEST_CASE(test_construction_from_file)
{
    json val = json::parse_file("../../../examples/members.json");

    output_format format(true);
    val.to_stream(std::cout,format);
	std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(test_construction_in_code)
{
    // A null value
    json null_val;

    // A boolean value
    json flag(true);

    // A numeric value
    json number(10.5);

    // An object value with four members
    json obj(json::an_object);
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";
    obj["events_attended"] = 10;
    obj["accept_waiver_of_liability"] = true;

    // An array value with four elements
    json arr(json::an_array);
    arr.push_back(null_val);
    arr.push_back(flag);
    arr.push_back(number);
    arr.push_back(obj);

    output_format format(true);
    arr.to_stream(std::cout,format);
    std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE(test_from_container)
{
    std::vector<int> vec;
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);

    json val1(vec.begin(), vec.end());
    std::cout << val1 << std::endl;

    std::list<double> list;
    list.push_back(10.5);
    list.push_back(20.5);
    list.push_back(30.5);

    json val2(list.begin(), list.end());
    std::cout << val2 << std::endl;
}

BOOST_AUTO_TEST_CASE(test_accessing)
{
    json val = json::parse_file("../../../examples/members.json");

    json member = val["members"][0];
    std::cout << member.to_string() << std::endl;

    json& ref = val["members"][0].get("first_name");
    std::cout << ref.to_string() << std::endl;

    const json& cref = val["members"][1].get("first_name");
    std::cout << cref.to_string() << std::endl;

}

BOOST_AUTO_TEST_CASE(test_exceptions)
{
    json val = json::parse_file("../../../examples/members.json");

    try
    {
        json members = val.get("members1");
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}


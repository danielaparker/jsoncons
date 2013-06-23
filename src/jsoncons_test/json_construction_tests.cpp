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
    #undef true

    // A null value
    json null_val;

    // A boolean value
    json flag(true);

    // A numberic value
    json number(10.5);

    // An empty object value
    json obj(json::an_object);

    // An empty array value
    json arr(json::an_array);

    arr.push_back(null_val);
    arr.push_back(flag);
    arr.push_back(number);
    arr.push_back(obj);

    output_format format(true);
    arr.to_stream(std::cout,format);
    std::cout << std::endl;
}



// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "jsoncons/json_filter.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include "my_custom_data.hpp"

using jsoncons::parsing_context;
using jsoncons::json_serializer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::json_reader;
using jsoncons::json_input_handler;
using jsoncons::json_output_handler;
using std::string;
using jsoncons::json_filter;
using jsoncons::pretty_print;

BOOST_AUTO_TEST_CASE(test_undefined_to_object)
{
    json value;
    BOOST_CHECK(value.is_undefined());
    value["field1"] = 10.9;
    BOOST_CHECK(value.is_object());
}

BOOST_AUTO_TEST_CASE(test_undefined_to_array)
{
    json value;
    BOOST_CHECK(value.is_undefined());
    value.add(10.9);
    BOOST_CHECK(value.is_array());
}

BOOST_AUTO_TEST_CASE(test_undefined_to_array_and_move)
{
    json container;
    BOOST_CHECK(container.is_undefined());

    json element;
    BOOST_CHECK(element.is_undefined());
    element.add(10.9);
    BOOST_CHECK(element.is_array());

    container.add(std::move(element));
    BOOST_CHECK(container.is_array());

    element.clear();
    element.add(9.9);
    BOOST_CHECK(element.is_array());
    container.add(std::move(element));
}

BOOST_AUTO_TEST_CASE(test_undefined_reserve)
{
    json container;
    BOOST_CHECK(container.is_undefined());
    BOOST_CHECK(container.capacity() == 0);

    container.reserve(10);
    BOOST_CHECK(container.capacity() == 10);

    json element;
    BOOST_CHECK(element.is_undefined());
    element.add(10.9);
    BOOST_CHECK(element.is_array());

    container.add(std::move(element));
    BOOST_CHECK(container.is_array());
    BOOST_CHECK(container.capacity() == 10);

    element.clear();
    element.add(9.9);
    BOOST_CHECK(element.is_array());
    container.add(std::move(element));
}

BOOST_AUTO_TEST_CASE(test_undefined_serialization)
{
    std::ostringstream os;

    json root;

    os << root;

    BOOST_CHECK(os.str().length() == 4); // for now
 }

BOOST_AUTO_TEST_CASE(test_serialize_undefined_member)
{
    std::ostringstream os;

    json root;
    root["field1"] = json();
    BOOST_CHECK(root.is_object());
    BOOST_CHECK(root["field1"].is_undefined());

    std::cout << root << std::endl;
    os << root;

    BOOST_CHECK(os.str() == std::string("{\"field1\":null}")); // for now
}


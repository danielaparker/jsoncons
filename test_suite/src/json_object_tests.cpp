// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "my_custom_data.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::pretty_print;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::basic_json_reader;
using std::string;
using boost::numeric::ublas::matrix;

BOOST_AUTO_TEST_CASE(test_remove_member)
{
    json o;
    o["key"] = "Hello";

    BOOST_CHECK(o.size() == 1);
    o.remove_member("key");
    BOOST_CHECK(o.size() == 0);
}

BOOST_AUTO_TEST_CASE(test_empty_object)
{
    json a;
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.is_object());

    json::object_iterator begin = a.begin_members();
    json::object_iterator end = a.end_members();

    for (json::object_iterator it = begin; it != end; ++it)
    {
        BOOST_CHECK(false);
    }

    a["key"] = "Hello";
    BOOST_CHECK(a.size() == 1);
    BOOST_CHECK(a.is_object());
}

BOOST_AUTO_TEST_CASE(test_const_empty_object)
{
    const json b;
    BOOST_CHECK(b.size() == 0);
    BOOST_CHECK(b.is_object());

    json::const_object_iterator begin = b.begin_members();
    json::const_object_iterator end = b.end_members();

    for (json::const_object_iterator it = begin; it != end; ++it)
    {
        BOOST_CHECK(false);
    }
}

BOOST_AUTO_TEST_CASE(test_empty_object_reserve)
{
    json c;
    BOOST_CHECK(c.size() == 0);
    BOOST_CHECK(c.is_object());
    c.reserve(100);
    BOOST_CHECK(c.capacity() == 100);
    c["key"] = "Hello";
    BOOST_CHECK(c.size() == 1);
    BOOST_CHECK(c.is_object());
    BOOST_CHECK(c.capacity() == 100);
}

BOOST_AUTO_TEST_CASE(test_empty_object_copy)
{
    json a;
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.is_object());

    json b = a;
    BOOST_CHECK(b.size() == 0);
    BOOST_CHECK(b.is_object());
}

BOOST_AUTO_TEST_CASE(test_empty_object_assignment)
{
    json a;
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.is_object());

    json b = json::make_array(10);
    BOOST_CHECK(b.size() == 10);
    BOOST_CHECK(b.is_array());

    b = a;  
    BOOST_CHECK(b.size() == 0);
    BOOST_CHECK(b.is_object());

    json c;
    c["key"] = "value";
    BOOST_CHECK(c.size() == 1);
    BOOST_CHECK(c.is_object());

    c = a;
    BOOST_CHECK(c.size() == 0);
    BOOST_CHECK(c.is_object());
}


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
using jsoncons::json_exception;

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
    BOOST_CHECK(a.is<json::object_type>());

    json::object_iterator begin = a.begin_members();
    json::object_iterator end = a.end_members();

    for (json::object_iterator it = begin; it != end; ++it)
    {
        BOOST_CHECK(false);
    }

    a["key"] = "Hello";
    BOOST_CHECK(a.size() == 1);
    BOOST_CHECK(a.is_object());
    BOOST_CHECK(a.is<json::object_type>());
}

BOOST_AUTO_TEST_CASE(test_const_empty_object)
{
    const json b;
    BOOST_CHECK(b.size() == 0);
    BOOST_CHECK(b.is_object());
    BOOST_CHECK(b.is<json::object_type>());

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
    BOOST_CHECK(c.is<json::object_type>());
    c.reserve(100);
    BOOST_CHECK(c.capacity() == 100);
    c["key"] = "Hello";
    BOOST_CHECK(c.size() == 1);
    BOOST_CHECK(c.is_object());
    BOOST_CHECK(c.is<json::object_type>());
    BOOST_CHECK(c.capacity() == 100);
}

BOOST_AUTO_TEST_CASE(test_empty_object_copy)
{
    json a;
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.is_object());
    BOOST_CHECK(a.is<json::object_type>());

    json b = a;
    BOOST_CHECK(b.size() == 0);
    BOOST_CHECK(b.is_object());
    BOOST_CHECK(b.is<json::object_type>());
}

BOOST_AUTO_TEST_CASE(test_empty_object_assignment)
{
    json a;
    BOOST_CHECK(a.size() == 0);
    BOOST_CHECK(a.is_object());
    BOOST_CHECK(a.is<json::object_type>());

    json b = json::make_array(10);
    BOOST_CHECK(b.size() == 10);
    BOOST_CHECK(b.is_array());
    BOOST_CHECK(b.is<json::array_type>());

    b = a;  
    BOOST_CHECK(b.size() == 0);
    BOOST_CHECK(b.is_object());
    BOOST_CHECK(b.is<json::object_type>());

    json c;
    c["key"] = "value";
    BOOST_CHECK(c.size() == 1);
    BOOST_CHECK(c.is_object());
    BOOST_CHECK(c.is<json::object_type>());

    c = a;
    BOOST_CHECK(c.size() == 0);
    BOOST_CHECK(c.is_object());
    BOOST_CHECK(c.is<json::object_type>());
}

BOOST_AUTO_TEST_CASE(test_get)
{
    json a;

    a["field1"] = "value1";

    std::string s1 = a.get("field1").as_string();
    std::string s1a = a.at("field1").as_string();
    std::string s2 = a.get("field2").as_string();
    BOOST_REQUIRE_THROW(a.at("field2"), json_exception);

    BOOST_CHECK(s1 == std::string("value1"));
    BOOST_CHECK(s1a == std::string("value1"));
    BOOST_CHECK(s2 == std::string("null"));
}

BOOST_AUTO_TEST_CASE(test_proxy_get)
{
    json a;

    a["object1"] = json();
    a["object1"]["field1"] = "value1";

    std::string s1 = a["object1"].get("field1").as_string();
    std::string s1a = a["object1"].at("field1").as_string();
    std::string s2 = a["object1"].get("field2").as_string();
    BOOST_REQUIRE_THROW(a["object1"].at("field2"), json_exception);

    BOOST_CHECK(s1 == std::string("value1"));
    BOOST_CHECK(s1a == std::string("value1"));
    BOOST_CHECK(s2 == std::string("null"));
}

BOOST_AUTO_TEST_CASE(test_const_member_read)
{
    json a;

    a["field1"] = 10;

    a["field2"];

    const json b(a);

    int val1 = b["field1"].as_int(); 
    BOOST_CHECK(val1 == 10);
    BOOST_REQUIRE_THROW(b["field2"], json_exception);
}

BOOST_AUTO_TEST_CASE(test_proxy_const_member_read)
{
    json a;

    a["object1"] = json();
    a["object1"]["field1"] = "value1";
    a["object1"]["field2"]; // No throw yet

    const json b(a);

    std::string s1 = b["object1"]["field1"].as_string();
    BOOST_REQUIRE_THROW(b["object1"]["field2"], json_exception);

    BOOST_CHECK(s1 == std::string("value1"));
}


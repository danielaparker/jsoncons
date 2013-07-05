// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::json_serializer;
using jsoncons::output_format;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::basic_json_reader;
using std::string;
using boost::numeric::ublas::matrix;

BOOST_AUTO_TEST_CASE(test_is_type)
{
    json obj(json::an_object);

    // tests for proxy is_type methods
    obj["string"] = "val1";
    BOOST_CHECK(obj["string"].is_string());

    obj["double"] = 10.7;
    BOOST_CHECK(obj["double"].is_double());

    obj["int"] = -10;
    BOOST_CHECK(obj["int"].is_longlong());

    obj["uint"] = 10u;
    BOOST_CHECK(obj["uint"].is_ulonglong());

    obj["long"] = static_cast<long>(10);
    BOOST_CHECK(obj["long"].is_longlong());

    obj["ulong"] = static_cast<unsigned long>(10);
    BOOST_CHECK(obj["ulong"].is_ulonglong());

    obj["longlong"] = static_cast<long long>(10);
    BOOST_CHECK(obj["longlong"].is_longlong());

    obj["ulonglong"] = static_cast<unsigned long long>(10);
    BOOST_CHECK(obj["ulonglong"].is_ulonglong());

    obj["true"] = true;
    BOOST_CHECK(obj["true"].is_bool());

    obj["false"] = false;
    BOOST_CHECK(obj["false"].is_bool());

    obj["null1"] = json::null;
    BOOST_CHECK(obj["null1"].is_null());

    obj["null2"] = json();
    BOOST_CHECK(obj["null2"].is_null());

    obj["object"] = json(json::an_object);
    BOOST_CHECK(obj["object"].is_object());

    obj["array"] = json(json::an_array);
    BOOST_CHECK(obj["array"].is_array());

    matrix<double> A;
    obj.insert_custom_member("custom",A);
    BOOST_CHECK(obj["custom"].is_custom());

    // tests for json is_type methods

    BOOST_CHECK(obj.is_object());

    json str = obj["string"];
    BOOST_CHECK(str.is_string());

}


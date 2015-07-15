// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using jsoncons::pretty_print;
using jsoncons::json;
using jsoncons::wjson;
using jsoncons::basic_json_reader;
using std::string;

BOOST_AUTO_TEST_CASE(test_object_equals_basic)
{
    json o1;
    o1["a"] = 1;
    o1["b"] = 2;
    o1["c"] = 3;

    json o2;
    o2["c"] = 3;
    o2["a"] = 1;
    o2["b"] = 2;

    BOOST_CHECK(o1 == o2);
    BOOST_CHECK(o2 == o1);
    BOOST_CHECK(!(o1 != o2));
    BOOST_CHECK(!(o2 != o1));
}

BOOST_AUTO_TEST_CASE(test_object_equals_diff_vals)
{
    json o1;
    o1["a"] = 1;
    o1["b"] = 2;
    o1["c"] = 3;

    json o2;
    o2["a"] = 1;
    o2["b"] = 4;
    o2["c"] = 3;

    BOOST_CHECK(!(o1 == o2));
    BOOST_CHECK(!(o2 == o1));
    BOOST_CHECK(o1 != o2);
    BOOST_CHECK(o2 != o1);
}

BOOST_AUTO_TEST_CASE(test_object_equals_diff_el_names)
{
    json o1;
    o1["a"] = 1;
    o1["b"] = 2;
    o1["c"] = 3;

    json o2;
    o2["d"] = 1;
    o2["e"] = 2;
    o2["f"] = 3;

    BOOST_CHECK(!(o1 == o2));
    BOOST_CHECK(!(o2 == o1));
    BOOST_CHECK(o1 != o2);
    BOOST_CHECK(o2 != o1);
}

BOOST_AUTO_TEST_CASE(test_object_equals_diff_sizes)
{
    json o1;
    o1["a"] = 1;
    o1["b"] = 2;
    o1["c"] = 3;

    json o2;
    o2["a"] = 1;
    o2["b"] = 2;

    BOOST_CHECK(!(o1 == o2));
    BOOST_CHECK(!(o2 == o1));
    BOOST_CHECK(o1 != o2);
    BOOST_CHECK(o2 != o1);
}

BOOST_AUTO_TEST_CASE(test_object_equals_subtle_offsets)
{
    json o1;
    o1["a"] = 1;
    o1["b"] = 1;

    json o2;
    o2["b"] = 1;
    o2["c"] = 1;

    BOOST_CHECK(!(o1 == o2));
    BOOST_CHECK(!(o2 == o1));
    BOOST_CHECK(o1 != o2);
    BOOST_CHECK(o2 != o1);
}



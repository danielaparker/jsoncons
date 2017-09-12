
// Copyright 2016 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_reader.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_exception_tests)

BOOST_AUTO_TEST_CASE(test_object_at)
{
    json a;
    BOOST_REQUIRE_THROW(a.at("key1"), std::out_of_range);
    BOOST_REQUIRE_THROW(static_cast<const json&>(a).at("key1"), std::out_of_range);

    a["key1"] = "value1";
    BOOST_REQUIRE_THROW(a.at("key2"), std::out_of_range);
    BOOST_REQUIRE_THROW(static_cast<const json&>(a).at("key2"), std::out_of_range);

    json b = json::array();
    BOOST_REQUIRE_THROW(b.at("key1"), std::runtime_error);
    BOOST_REQUIRE_THROW(static_cast<const json&>(b).at("key1"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(test_object_find)
{
    json b = json::array();
    b.resize(3);
    BOOST_REQUIRE_THROW(b.find("key1"), std::runtime_error);
    BOOST_REQUIRE_THROW(static_cast<const json&>(b).find("key1"), std::runtime_error);
    BOOST_REQUIRE_THROW(b.find(std::string("key1")), std::runtime_error);
    BOOST_REQUIRE_THROW(static_cast<const json&>(b).find(std::string("key1")), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(test_array_at)
{
    json a = json::array();
    BOOST_REQUIRE_THROW(a.at(0), std::out_of_range);
    BOOST_REQUIRE_THROW(static_cast<const json&>(a).at(0), std::out_of_range);

    a.resize(3);
    BOOST_REQUIRE_THROW(a.at(3), std::out_of_range);
    BOOST_REQUIRE_THROW(static_cast<const json&>(a).at(3), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(test_object_set)
{
    json b = json::array();
    b.resize(3);
    BOOST_REQUIRE_THROW(b.insert_or_assign("key1","value1"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(test_array_add)
{
    json b;
    b["key1"] = "value1";
    BOOST_REQUIRE_THROW(b.push_back(0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(test_object_index)
{
    json b;
    BOOST_REQUIRE_THROW(b["key1"].as<std::string>(), std::out_of_range);

    b["key1"] = "value1";
    BOOST_REQUIRE_THROW(b["key2"].as<std::string>(), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()


// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <new>
#include <boost/optional.hpp>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(ojson_tests)

BOOST_AUTO_TEST_CASE(test_index)
{
    ojson o = ojson::parse(R"(
    {
        "street_number" : "100",
        "street_name" : "Queen St W",
        "city" : "Toronto",
        "country" : "Canada"
    }
    )");

    BOOST_CHECK_EQUAL("100", o[0].as<std::string>());
    BOOST_CHECK_EQUAL("Queen St W", o[1].as<std::string>());
    BOOST_CHECK_EQUAL("Toronto", o[2].as<std::string>());
    BOOST_CHECK_EQUAL("Canada", o[3].as<std::string>());

    BOOST_CHECK_EQUAL("100", o.at(0).as<std::string>());
    BOOST_CHECK_EQUAL("Queen St W", o.at(1).as<std::string>());
    BOOST_CHECK_EQUAL("Toronto", o.at(2).as<std::string>());
    BOOST_CHECK_EQUAL("Canada", o.at(3).as<std::string>());
}

BOOST_AUTO_TEST_CASE(test_object)
{
    ojson o = ojson::parse(R"(
    {
        "street_number" : "100",
        "street_name" : "Queen St W",
        "city" : "Toronto",
        "country" : "Canada"
    }
    )");

    o.insert_or_assign("postal_code", "M5H 2N2");

    ojson o2 = o;
    BOOST_CHECK(o == o2);

    ojson o3 = o;
    o3["street_name"] = "Queen St W";
    //BOOST_CHECK_EQUAL(o,o3);

    //BOOST_CHECK_EQUAL("Queen St W",o["street_name"].as<std::string>());
    //BOOST_CHECK_EQUAL(2,o["city"].as<int>());
    //BOOST_CHECK_EQUAL(4,o["street_number"].as<int>());

    auto it = o.find("country");
    BOOST_CHECK(it != o.object_range().end());
    o.insert_or_assign(it,"province","Ontario");

    o.insert_or_assign("unit_type","O");

    o.erase("unit_type");
}

BOOST_AUTO_TEST_CASE(test_object_emplace)
{
    ojson o = ojson::parse(R"(
    {
        "street_number" : "100",
        "street_name" : "Queen St W",
        "city" : "Toronto",
        "country" : "Canada"
    }
    )");

    o.try_emplace("postal_code", "M5H 2N2");

    ojson o2 = o;
    BOOST_CHECK(o == o2);

    ojson o3 = o;
    o3["street_name"] = "Queen St W";
    //BOOST_CHECK_EQUAL(o,o3);

    //BOOST_CHECK_EQUAL("Queen St W",o["street_name"].as<std::string>());
    //BOOST_CHECK_EQUAL(2,o["city"].as<int>());
    //BOOST_CHECK_EQUAL(4,o["street_number"].as<int>());

    auto it = o.find("country");
    BOOST_CHECK(it != o.object_range().end());
    o.try_emplace(it,"province","Ontario");

    o.try_emplace("unit_type","O");

    o.erase("unit_type");
}

BOOST_AUTO_TEST_SUITE_END()


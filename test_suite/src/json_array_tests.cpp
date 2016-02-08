// Copyright 2013 Daniel Parker
// Distributed under Boost license

#ifdef __linux__
#define BOOST_TEST_DYN_LINK
#endif

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_array_test_suite)

BOOST_AUTO_TEST_CASE(test_array_constructor)
{
    json arr = json::array();
    arr.resize(10,10.0);
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.size() == 10);
    BOOST_CHECK_CLOSE(arr[0].as<double>(), 10.0, 0.0000001);

}

BOOST_AUTO_TEST_CASE(test_make_array)
{
    json arr = json::make_array();
    BOOST_CHECK(arr.size() == 0);
    arr.resize(10,10.0);
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.size() == 10);
    BOOST_CHECK_CLOSE(arr[0].as<double>(), 10.0, 0.0000001);

}

BOOST_AUTO_TEST_CASE(test_add_element_to_array)
{
    json arr = json::make_array();
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.is<json::array>());
    arr.add("Toronto");
    arr.add("Vancouver");
    arr.add(arr.elements().begin(),"Montreal");

    BOOST_CHECK(arr.size() == 3);

    BOOST_CHECK(arr[0].as<std::string>() == std::string("Montreal"));
    BOOST_CHECK(arr[1].as<std::string>() == std::string("Toronto"));
    BOOST_CHECK(arr[2].as<std::string>() == std::string("Vancouver"));
}

BOOST_AUTO_TEST_CASE(test_array_add_pos)
{
    json arr = json::make_array();
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.is<json::array>());
    arr.add("Toronto");
    arr.add("Vancouver");
    arr.add(arr.elements().begin(),"Montreal");

    BOOST_CHECK(arr.size() == 3);

    BOOST_CHECK(arr[0].as<std::string>() == std::string("Montreal"));
    BOOST_CHECK(arr[1].as<std::string>() == std::string("Toronto"));
    BOOST_CHECK(arr[2].as<std::string>() == std::string("Vancouver"));
}

BOOST_AUTO_TEST_CASE(test_array_erase_range)
{
    json arr = json::make_array();
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.is<json::array>());
    arr.add("Toronto");
    arr.add("Vancouver");
    arr.add(arr.elements().begin(),"Montreal");

    BOOST_CHECK(arr.size() == 3);

    arr.erase(arr.elements().begin()+1,arr.elements().end());

    BOOST_CHECK(arr.size() == 1);
    BOOST_CHECK(arr[0].as<std::string>() == std::string("Montreal"));
}

BOOST_AUTO_TEST_CASE(test_object_erase_range)
{
    json o;
    o["key1"] = "value1";
    o["key2"] = "value2";
    o["key3"] = "value3";
    o["key4"] = "value4";

    o.erase(o.members().begin()+1,o.members().begin()+3);
    
    BOOST_CHECK_EQUAL(2,o.size());
    BOOST_CHECK_EQUAL(1,o.count("key1"));
    BOOST_CHECK_EQUAL(1,o.count("key4"));
}

BOOST_AUTO_TEST_CASE(test_reserve_array_capacity)
{
    json cities = json::make_array();
    BOOST_CHECK(cities.is_array());
    BOOST_CHECK(cities.is<json::array>());
    cities.reserve(10);  // storage is allocated
    BOOST_CHECK(cities.capacity() == 10);
    BOOST_CHECK(cities.size() == 0);

    cities.add("Toronto");
    BOOST_CHECK(cities.is_array());
    BOOST_CHECK(cities.is<json::array>());
    BOOST_CHECK(cities.capacity() == 10);
    BOOST_CHECK(cities.size() == 1);
    cities.add("Vancouver");
    cities.add(cities.elements().begin(),"Montreal");
	BOOST_CHECK(cities.capacity() == 10);
    BOOST_CHECK(cities.size() == 3);
}

BOOST_AUTO_TEST_CASE(test_one_dim_array)
{
    basic_json<std::string,std::allocator<char>> a = basic_json<std::string,std::allocator<char>>::make_array<1>(10,0);
    /*BOOST_CHECK(a.size() == 10);
    BOOST_CHECK(a[0].as_integer() == 0);
    a[1] = 1;
    a[2] = 2;
    BOOST_CHECK(a[1].as_integer() == 1);
    BOOST_CHECK(a[2].as_integer() == 2);
    BOOST_CHECK(a[9].as_integer() == 0);

    BOOST_CHECK(a[1].as<long long>() == 1);
    BOOST_CHECK(a[2].as<long long>() == 2);
    BOOST_CHECK(a[9].as<long long>() == 0);*/
}

BOOST_AUTO_TEST_CASE(test_two_dim_array)
{
    json a = json::make_array<2>(3,4,0);
    BOOST_CHECK(a.size() == 3);
    a[0][0] = "Tenor";
    a[0][1] = "ATM vol";
    a[0][2] = "25-d-MS";
    a[0][3] = "25-d-RR";
    a[1][0] = "1Y";
    a[1][1] = 0.20;
    a[1][2] = 0.009;
    a[1][3] = -0.006;
    a[2][0] = "2Y";
    a[2][1] = 0.18;
    a[2][2] = 0.009;
    a[2][3] = -0.005;

    BOOST_CHECK_EQUAL(a[0][0].as<std::string>(), std::string("Tenor"));
    BOOST_CHECK_CLOSE(a[2][3].as<double>(), -0.005, 0.00000001);

    BOOST_CHECK_EQUAL(a[0][0].as<std::string>(), std::string("Tenor"));
    BOOST_CHECK_CLOSE(a[2][3].as<double>(), -0.005, 0.00000001);
}

BOOST_AUTO_TEST_CASE(test_three_dim_array)
{
    json a = json::make_array<3>(4,3,2,0);
    BOOST_CHECK(a.size() == 4);
    a[0][2][0] = 2;
    a[0][2][1] = 3;

    BOOST_CHECK(a[0][2][0].as_integer() == 2);
    BOOST_CHECK(a[0][2][1].as_integer() == 3);
    BOOST_CHECK(a[3][2][1].as_integer() == 0);

    BOOST_CHECK(a[0][2][0].as<long long>() == 2);
    BOOST_CHECK(a[0][2][1].as<long long>() == 3);
    BOOST_CHECK(a[3][2][1].as<long long>() == 0);
}

BOOST_AUTO_TEST_CASE(test_assign_vector)
{
    std::vector<std::string> vec;
    vec.push_back("Toronto");
    vec.push_back("Vancouver");
    vec.push_back("Montreal");

    json val;
    val = vec;

    BOOST_CHECK(val.size() == 3);
    BOOST_CHECK_EQUAL(val[0].as<std::string>(), std::string("Toronto"));
    BOOST_CHECK_EQUAL(val[1].as<std::string>(), std::string("Vancouver"));
    BOOST_CHECK_EQUAL(val[2].as<std::string>(), std::string("Montreal"));

}

BOOST_AUTO_TEST_SUITE_END()


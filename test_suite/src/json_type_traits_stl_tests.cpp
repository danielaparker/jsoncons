// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include "jsoncons/json.hpp"
#include "jsoncons/json_serializer.hpp"
#include "my_any_specializations.hpp"
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <cstdint>

using namespace jsoncons;
using boost::numeric::ublas::matrix;

BOOST_AUTO_TEST_SUITE(json_type_traits_stl_suite)

BOOST_AUTO_TEST_CASE(test_characters)
{
    json a = "short";
    json b = "a long string";

    BOOST_CHECK_EQUAL(true, a.is<std::string>());
    BOOST_CHECK_EQUAL(true, b.is<std::string>());

    std::string s = a.as<std::string>();
    std::string t = b.as<std::string>();

    json c = { "short","a long string" };
    auto u = c.as<std::vector<std::string>>();
}

BOOST_AUTO_TEST_CASE(test_is_json_vector)
{
    json a = {0,1,2,3,4}; 

    BOOST_CHECK_EQUAL(true,a.is<std::vector<uint64_t>>());
}

BOOST_AUTO_TEST_CASE(test_as_vector)
{
    json a = {0,1,2,3,4}; 
    std::vector<int> v = a.as<std::vector<int>>();

    BOOST_CHECK_EQUAL(v[0],0);
    BOOST_CHECK_EQUAL(v[1],1);
    BOOST_CHECK_EQUAL(v[2],2);
    BOOST_CHECK_EQUAL(v[3],3);
    BOOST_CHECK_EQUAL(v[4],4);
}

BOOST_AUTO_TEST_CASE(test_assign_vector)
{
    std::vector<int> v = {0,1,2,3,4}; 
    json a = v;

    BOOST_CHECK_EQUAL(a[0],0);
    BOOST_CHECK_EQUAL(a[1],1);
    BOOST_CHECK_EQUAL(a[2],2);
    BOOST_CHECK_EQUAL(a[3],3);
    BOOST_CHECK_EQUAL(a[4],4);
}

BOOST_AUTO_TEST_CASE(test_as_vector_of_bool)
{
    json a = json::parse("[true,false,true]");

    std::vector<bool> v = a.as<std::vector<bool>>();

    BOOST_CHECK_EQUAL(v[0],true);
    BOOST_CHECK_EQUAL(v[1],false);
    BOOST_CHECK_EQUAL(v[2],true);
}

BOOST_AUTO_TEST_CASE(test_assign_vector_of_bool)
{
    std::vector<bool> v = {true,false,true};
    json a = v;

    BOOST_CHECK_EQUAL(a[0],true);
    BOOST_CHECK_EQUAL(a[1],false);
    BOOST_CHECK_EQUAL(a[2],true);

    json b;
    b = v;

    BOOST_CHECK_EQUAL(b[0],true);
    BOOST_CHECK_EQUAL(b[1],false);
    BOOST_CHECK_EQUAL(b[2],true);
}

BOOST_AUTO_TEST_CASE(test_construct_vector_of_bool)
{
    std::vector<bool> v = {true,false,true};
    json a(v.begin(),v.end());

    BOOST_CHECK_EQUAL(a[0],true);
    BOOST_CHECK_EQUAL(a[1],false);
    BOOST_CHECK_EQUAL(a[2],true);
}

BOOST_AUTO_TEST_CASE(test_construct_const_vector_of_bool)
{
    const std::vector<bool> v = {true,false,true};
    json a(v.begin(),v.end());

    BOOST_CHECK_EQUAL(a[0],true);
    BOOST_CHECK_EQUAL(a[1],false);
    BOOST_CHECK_EQUAL(a[2],true);
}

BOOST_AUTO_TEST_CASE(test_is_json_map)
{
    json a;
    a["a"] = 0; 
    a["b"] = 1; 
    a["c"] = 2; 

    BOOST_CHECK_EQUAL(true,(a.is<std::map<std::string,int> >()));

    std::cout << std::is_same<std::string::value_type,json::char_type>::value << std::endl;
    std::cout << std::is_same<std::string,json::string_type>::value << std::endl;
    std::cout << std::is_constructible<json::string_type,std::string>::value << std::endl;
}

BOOST_AUTO_TEST_CASE(test_is_json_map2)
{
    json a;
    a["a"] = "0"; 
    a["b"] = "1"; 
    a["c"] = "2"; 

    BOOST_CHECK_EQUAL(true,(a["a"].is_string()));

    json b("0");
    BOOST_CHECK_EQUAL(true,(b.is<std::string>()));

    BOOST_CHECK_EQUAL(true,(a["a"].is<std::string>()));

    BOOST_CHECK_EQUAL(true,(a.is<std::map<std::string,std::string> >()));
}

BOOST_AUTO_TEST_CASE(test_as_map)
{
    json o;
    o["first"] = "first";
    o["second"] = "second";

    auto m = o.as<std::map<std::string,std::string>>();
    BOOST_CHECK_EQUAL(std::string("first"),m.at("first"));
    BOOST_CHECK_EQUAL(std::string("second"),m.at("second"));

    json o2(m);
    BOOST_CHECK_EQUAL(o,o2);

    json o3;
    o3 = m;
    BOOST_CHECK_EQUAL(o,o3);
}

BOOST_AUTO_TEST_CASE(test_as_map2)
{
    json o;
    o["first"] = 1;
    o["second"] = true;
    o["third"] = jsoncons::null_type();

    auto m = o.as<std::map<std::string,std::string>>();
    BOOST_CHECK_EQUAL(std::string("1"),m.at("first"));
    BOOST_CHECK_EQUAL(std::string("true"),m.at("second"));
    BOOST_CHECK_EQUAL(std::string("null"),m.at("third"));

    json o2(m);
    BOOST_CHECK_EQUAL("1",o2["first"]);
}

BOOST_AUTO_TEST_SUITE_END()



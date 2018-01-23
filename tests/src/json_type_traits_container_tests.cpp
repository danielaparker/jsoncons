// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <boost/test/unit_test.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons/json_serializer.hpp>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>
#include <cstdint>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <deque>
#include <forward_list>
#include <list>
#include <array>
#include <valarray>

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_type_traits_stl_suite)

BOOST_AUTO_TEST_CASE(test_json_as_pair)
{
    json j = json::array{false,1};
    auto t = j.as<std::pair<bool,int>>();
    BOOST_CHECK(std::get<0>(t) == false);
    BOOST_CHECK(std::get<1>(t) == 1);
}

BOOST_AUTO_TEST_CASE(test_tuple_to_json)
{
    auto t = std::make_tuple(false,1,"foo");
    json j(t);

    BOOST_REQUIRE(j.is_array());
    BOOST_REQUIRE(j.size() == 3);
    BOOST_CHECK_EQUAL(false,j[0].as<bool>());
    BOOST_CHECK_EQUAL(1,j[1].as<int>());
    BOOST_CHECK_EQUAL(std::string("foo"),j[2].as<std::string>());
}

BOOST_AUTO_TEST_CASE(test_json_as_tuple)
{
    json j = json::array{false,1,"foo"};

    auto t = j.as<std::tuple<bool,int,std::string>>();

    BOOST_CHECK(std::get<0>(t) == false);
    BOOST_CHECK(std::get<1>(t) == 1);
    BOOST_CHECK(std::get<2>(t) == std::string("foo"));
}

BOOST_AUTO_TEST_CASE(test_characters)
{
    const json a = "short";
    const json b = "a long string";
 
    BOOST_CHECK_EQUAL(true, a.is<std::string>());
    BOOST_CHECK_EQUAL(true, b.is<std::string>());

    std::string s2 = a.as<std::string>();
    std::string t2 = b.as<std::string>();

    json c = json::array{ "short","a long string" };
    auto u = c.as<std::vector<std::string>>();
}

// vector

BOOST_AUTO_TEST_CASE(test_is_json_vector)
{
    json a = json::array{0,1,2,3,4}; 

    BOOST_CHECK_EQUAL(true,a.is<std::vector<uint64_t>>());
}

BOOST_AUTO_TEST_CASE(test_as_vector)
{
    json a = json::array{0,1,2,3,4}; 
    std::vector<int> v = a.as<std::vector<int>>();

    BOOST_CHECK_EQUAL(v[0],0);
    BOOST_CHECK_EQUAL(v[1],1);
    BOOST_CHECK_EQUAL(v[2],2);
    BOOST_CHECK_EQUAL(v[3],3);
    BOOST_CHECK_EQUAL(v[4],4);
}

BOOST_AUTO_TEST_CASE(test_assign_vector)
{
    std::vector<int> v {0,1,2,3,4}; 
    json a = v;

    BOOST_CHECK_EQUAL(a[0],json(0));
    BOOST_CHECK_EQUAL(a[1],json(1));
    BOOST_CHECK_EQUAL(a[2],json(2));
    BOOST_CHECK_EQUAL(a[3],json(3));
    BOOST_CHECK_EQUAL(a[4],json(4));
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

    BOOST_CHECK_EQUAL(a[0],json(true));
    BOOST_CHECK_EQUAL(a[1],json(false));
    BOOST_CHECK_EQUAL(a[2],json(true));

    json b;
    b = v;

    BOOST_CHECK_EQUAL(b[0],json(true));
    BOOST_CHECK_EQUAL(b[1],json(false));
    BOOST_CHECK_EQUAL(b[2],json(true));
}

BOOST_AUTO_TEST_CASE(test_construct_vector_of_bool)
{
    std::vector<bool> v = {true,false,true};
    json a = v;

    BOOST_CHECK_EQUAL(a[0],json(true));
    BOOST_CHECK_EQUAL(a[1],json(false));
    BOOST_CHECK_EQUAL(a[2],json(true));
}

BOOST_AUTO_TEST_CASE(test_construct_const_vector_of_bool)
{
    const std::vector<bool> v = {true,false,true};
    json a = v;

    BOOST_CHECK_EQUAL(a[0],json(true));
    BOOST_CHECK_EQUAL(a[1],json(false));
    BOOST_CHECK_EQUAL(a[2],json(true));
}

// valarray

BOOST_AUTO_TEST_CASE(test_is_json_valarray)
{
    json a = json::array{0,1,2,3,4}; 

    BOOST_CHECK_EQUAL(true,a.is<std::valarray<uint64_t>>());
}

BOOST_AUTO_TEST_CASE(test_as_valarray)
{
    json a = json::array{0,1,2,3,4}; 
    std::valarray<int> v = a.as<std::valarray<int>>();

    BOOST_CHECK_EQUAL(v[0],0);
    BOOST_CHECK_EQUAL(v[1],1);
    BOOST_CHECK_EQUAL(v[2],2);
    BOOST_CHECK_EQUAL(v[3],3);
    BOOST_CHECK_EQUAL(v[4],4);
}

BOOST_AUTO_TEST_CASE(test_assign_valarray)
{
    std::valarray<int> v {0,1,2,3,4}; 
    json a = v;

    BOOST_CHECK_EQUAL(a[0],json(0));
    BOOST_CHECK_EQUAL(a[1],json(1));
    BOOST_CHECK_EQUAL(a[2],json(2));
    BOOST_CHECK_EQUAL(a[3],json(3));
    BOOST_CHECK_EQUAL(a[4],json(4));
}

BOOST_AUTO_TEST_CASE(test_is_json_map)
{
    json a;
    a["a"] = 0; 
    a["b"] = 1; 
    a["c"] = 2; 

    BOOST_CHECK_EQUAL(true,(a.is<std::map<std::string,int> >()));
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

BOOST_AUTO_TEST_CASE(test_from_stl_container)
{
    std::vector<int> a_vector{1, 2, 3, 4};
    json j_vector(a_vector);
    BOOST_CHECK_EQUAL(1, j_vector[0].as<int>());
    BOOST_CHECK_EQUAL(2, j_vector[1].as<int>());
    BOOST_CHECK_EQUAL(3, j_vector[2].as<int>());
    BOOST_CHECK_EQUAL(4, j_vector[3].as<int>());

    std::vector<unsigned long> a_vector2{1ul, 2ul, 3ul, 4ul};
    json j_vec2(a_vector2);
    BOOST_CHECK_EQUAL(1, j_vec2[0].as<int>());
    BOOST_CHECK_EQUAL(2, j_vec2[1].as<int>());
    BOOST_CHECK_EQUAL(3, j_vec2[2].as<int>());
    BOOST_CHECK_EQUAL(4, j_vec2[3].as<int>());

    std::deque<double> a_deque{1.123, 2.234, 3.456, 4.567};
    json j_deque(a_deque);
    BOOST_CHECK_EQUAL(1.123, j_deque[0].as<double>());
    BOOST_CHECK_EQUAL(2.234, j_deque[1].as<double>());
    BOOST_CHECK_EQUAL(3.456, j_deque[2].as<double>());
    BOOST_CHECK_EQUAL(4.567, j_deque[3].as<double>());

    std::list<bool> a_list{true, true, false, true};
    json j_list(a_list);
    BOOST_CHECK_EQUAL(true, j_list[0].as<bool>());
    BOOST_CHECK_EQUAL(true, j_list[1].as<bool>());
    BOOST_CHECK_EQUAL(false, j_list[2].as<bool>());
    BOOST_CHECK_EQUAL(true, j_list[3].as<bool>());

    std::forward_list<int64_t>a_flist {12345678909876, 23456789098765, 34567890987654, 45678909876543};
    json j_flist(a_flist);
    BOOST_CHECK_EQUAL(static_cast<int64_t>(12345678909876), j_flist[0].as<int64_t>());
    BOOST_CHECK_EQUAL(static_cast<int64_t>(23456789098765), j_flist[1].as<int64_t>());
    BOOST_CHECK_EQUAL(static_cast<int64_t>(34567890987654), j_flist[2].as<int64_t>());
    BOOST_CHECK_EQUAL(static_cast<int64_t>(45678909876543), j_flist[3].as<int64_t>());

    std::array<unsigned long, 4> a_array {{1, 2, 3, 4}};
    json j_array(a_array);
    BOOST_CHECK_EQUAL(1, j_array[0].as<int>());
    BOOST_CHECK_EQUAL(2, j_array[1].as<int>());
    BOOST_CHECK_EQUAL(3, j_array[2].as<int>());
    BOOST_CHECK_EQUAL(4, j_array[3].as<int>());

    std::set<std::string> a_set{"one", "two", "three", "four", "one"};
    json j_set(a_set); // only one entry for "one" is used
    // ["four", "one", "three", "two"]

    std::unordered_set<std::string> a_uset{"one", "two", "three", "four", "one"};
    json j_uset(a_uset); // only one entry for "one" is used
    // maybe ["two", "three", "four", "one"]

    std::multiset<std::string> a_mset{"one", "two", "one", "four"};
    json j_mset(a_mset); // only one entry for "one" is used
    // maybe ["one", "two", "four"]

    std::unordered_multiset<std::string> a_umset {"one", "two", "one", "four"};
    json j_umset(a_umset); // both entries for "one" are used
    // maybe ["one", "two", "one", "four"]

    std::map<std::string, int> a_map{ {"one", 1}, {"two", 2}, {"three", 3} };
    json j_map(a_map);
    BOOST_CHECK_EQUAL(1, j_map["one"].as<int>());
    BOOST_CHECK_EQUAL(2, j_map["two"].as<int>());
    BOOST_CHECK_EQUAL(3, j_map["three"].as<int>());

    std::unordered_map<std::string, double> a_umap{ {"one", 1.2}, {"two", 2.3}, {"three", 3.4} };
    json j_umap(a_umap);
    BOOST_CHECK_EQUAL(1.2, j_umap["one"].as<double>());
    BOOST_CHECK_EQUAL(2.3, j_umap["two"].as<double>());
    BOOST_CHECK_EQUAL(3.4, j_umap["three"].as<double>());

    std::multimap<std::string, bool> a_mmap{ {"one", true}, {"two", true}, {"three", false}, {"three", true} };
    json j_mmap(a_mmap); // one entry for key "three"
    BOOST_CHECK_EQUAL(true, j_mmap.find("one")->value().as<bool>());
    BOOST_CHECK_EQUAL(true, j_mmap.find("two")->value().as<bool>());
    BOOST_CHECK_EQUAL(true, j_mmap.find("three")->value().as<bool>());

    std::unordered_multimap<std::string, bool> a_ummap { {"one", true}, {"two", true}, /*{"three", false},*/ {"three", true} };
    json j_ummap(a_ummap); // two entries for key "three"
    BOOST_CHECK_EQUAL(true, j_ummap.find("one")->value().as<bool>());
    BOOST_CHECK_EQUAL(true, j_ummap.find("two")->value().as<bool>());
    BOOST_CHECK_EQUAL(true, j_ummap.find("three")->value().as<bool>());
}

BOOST_AUTO_TEST_SUITE_END()



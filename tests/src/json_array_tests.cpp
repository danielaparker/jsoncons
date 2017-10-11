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

using namespace jsoncons;

BOOST_AUTO_TEST_SUITE(json_array_tests)

BOOST_AUTO_TEST_CASE(test_initializer_list_of_integers)
{
    json arr = json::array{0,1,2,3};
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.size() == 4);
    for (size_t i = 0; i < arr.size(); ++i)
    {
        BOOST_CHECK_EQUAL(i,arr[i].as<size_t>());
    }
}

BOOST_AUTO_TEST_CASE(test_assignment_to_initializer_list)
{
    json arr = json::array({0,1,2,3});

    arr = json::array{0,1,2,3};
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.size() == 4);
    for (size_t i = 0; i < arr.size(); ++i)
    {
        BOOST_CHECK_EQUAL(i,arr[i].as<size_t>());
    }
}

BOOST_AUTO_TEST_CASE(test_assignment_to_initializer_list2)
{
    json val;
    val["data"]["id"] = json::array{0,1,2,3,4,5,6,7};
    val["data"]["item"] = json::array{json::array{2},
                                      json::array{4,5,2,3},
                                      json::array{4},
                                      json::array{4,5,2,3},
                                      json::array{2},
                                      json::array{4,5,3},
                                      json::array{2},
                                      json::array{4,3}};

    BOOST_CHECK(val["data"]["item"][0][0] == json(2));
    BOOST_CHECK(val["data"]["item"][1][0] == json(4));
    BOOST_CHECK(val["data"]["item"][2][0] == json(4));
    BOOST_CHECK(val["data"]["item"][3][0] == json(4));
    BOOST_CHECK(val["data"]["item"][4][0] == json(2));
    BOOST_CHECK(val["data"]["item"][5][0] == json(4));
    BOOST_CHECK(val["data"]["item"][6][0] == json(2));
    BOOST_CHECK(val["data"]["item"][7][0] == json(4));
    BOOST_CHECK(val["data"]["item"][7][1] == json(3));
}

BOOST_AUTO_TEST_CASE(test_assignment_to_initializer_list3)
{
    json val;
    val["data"]["id"] = json::array{0,1,2,3,4,5,6,7};
    val["data"]["item"] = json::array{json::object{{"first",1},{"second",2}}};

    json expected_id = json::parse(R"(
[0,1,2,3,4,5,6,7]
    )");

    json expected_item = json::parse(R"(
    [{"first":1,"second":2}]
    )");

    BOOST_CHECK(expected_id == val["data"]["id"]);
    BOOST_CHECK(expected_item == val["data"]["item"]);
}

BOOST_AUTO_TEST_CASE(test_assign_initializer_list_of_object)
{
    json arr = json::array();

    json transaction;
    transaction["Debit"] = 10000;

    arr = json::array{transaction};
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.size() == 1);
    BOOST_CHECK_EQUAL(arr[0], transaction);
}

BOOST_AUTO_TEST_CASE(test_initializer_list_of_objects)
{
    json book1;
    book1["author"] = "Smith";
    book1["title"] = "Old Bones";

    json book2;
    book2["author"] = "Jones";
    book2["title"] = "New Things";

    json arr = json::array{book1, book2};
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.size() == 2);

    BOOST_CHECK_EQUAL(book1,arr[0]);
    BOOST_CHECK_EQUAL(book2,arr[1]);
}

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
    json arr = json::array();
    BOOST_CHECK(arr.size() == 0);
    arr.resize(10,10.0);
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.size() == 10);
    BOOST_CHECK_CLOSE(arr[0].as<double>(), 10.0, 0.0000001);

}

BOOST_AUTO_TEST_CASE(test_add_element_to_array)
{
    json arr = json::array();
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.is<json::array>());
    arr.push_back("Toronto");
    arr.push_back("Vancouver");
    arr.insert(arr.array_range().begin(),"Montreal");

    BOOST_CHECK(arr.size() == 3);

    BOOST_CHECK(arr[0].as<std::string>() == std::string("Montreal"));
    BOOST_CHECK(arr[1].as<std::string>() == std::string("Toronto"));
    BOOST_CHECK(arr[2].as<std::string>() == std::string("Vancouver"));
}

BOOST_AUTO_TEST_CASE(test_emplace_element_to_array)
{
    json a = json::array();
    BOOST_CHECK(a.is_array());
    BOOST_CHECK(a.is<json::array>());
    a.emplace_back("Toronto");
    a.emplace_back("Vancouver");
    a.emplace(a.array_range().begin(),"Montreal");

    BOOST_CHECK(a.size() == 3);

    BOOST_CHECK(a[0].as<std::string>() == std::string("Montreal"));
    BOOST_CHECK(a[1].as<std::string>() == std::string("Toronto"));
    BOOST_CHECK(a[2].as<std::string>() == std::string("Vancouver"));
}

BOOST_AUTO_TEST_CASE(test_array_add_pos)
{
    json arr = json::array();
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.is<json::array>());
    arr.push_back("Toronto");
    arr.push_back("Vancouver");
    arr.insert(arr.array_range().begin(),"Montreal");

    BOOST_CHECK(arr.size() == 3);

    BOOST_CHECK(arr[0].as<std::string>() == std::string("Montreal"));
    BOOST_CHECK(arr[1].as<std::string>() == std::string("Toronto"));
    BOOST_CHECK(arr[2].as<std::string>() == std::string("Vancouver"));
}

BOOST_AUTO_TEST_CASE(test_array_erase_range)
{
    json arr = json::array();
    BOOST_CHECK(arr.is_array());
    BOOST_CHECK(arr.is<json::array>());
    arr.push_back("Toronto");
    arr.push_back("Vancouver");
    arr.insert(arr.array_range().begin(),"Montreal");

    BOOST_CHECK(arr.size() == 3);

    arr.erase(arr.array_range().begin()+1,arr.array_range().end());

    BOOST_CHECK(arr.size() == 1);
    BOOST_CHECK(arr[0].as<std::string>() == std::string("Montreal"));
}

BOOST_AUTO_TEST_CASE(test_reserve_array_capacity)
{
    json cities = json::array();
    BOOST_CHECK(cities.is_array());
    BOOST_CHECK(cities.is<json::array>());
    cities.reserve(10);  // storage is allocated
    BOOST_CHECK(cities.capacity() == 10);
    BOOST_CHECK(cities.size() == 0);

    cities.push_back("Toronto");
    BOOST_CHECK(cities.is_array());
    BOOST_CHECK(cities.is<json::array>());
    BOOST_CHECK(cities.capacity() == 10);
    BOOST_CHECK(cities.size() == 1);
    cities.push_back("Vancouver");
    cities.insert(cities.array_range().begin(),"Montreal");
    BOOST_CHECK(cities.capacity() == 10);
    BOOST_CHECK(cities.size() == 3);
}


BOOST_AUTO_TEST_CASE(test_one_dim_array)
{
    basic_json<char,sorted_policy,std::allocator<char>> a = basic_json<char,sorted_policy,std::allocator<char>>::make_array<1>(10,0);
    BOOST_CHECK(a.size() == 10);
    BOOST_CHECK(a[0].as_integer() == 0);
    a[1] = 1;
    a[2] = 2;
    BOOST_CHECK(a[1].as_integer() == 1);
    BOOST_CHECK(a[2].as_integer() == 2);
    BOOST_CHECK(a[9].as_integer() == 0);

    BOOST_CHECK(a[1].as<long long>() == 1);
    BOOST_CHECK(a[2].as<long long>() == 2);
    BOOST_CHECK(a[9].as<long long>() == 0);
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

BOOST_AUTO_TEST_CASE(test_assign_vector_of_bool)
{
    std::vector<bool> vec;
    vec.push_back(true);
    vec.push_back(false);
    vec.push_back(true);

    json val;
    val = vec;

    BOOST_CHECK(val.size() == 3);
    BOOST_CHECK_EQUAL(val[0].as<bool>(), true);
    BOOST_CHECK_EQUAL(val[1].as<bool>(), false);
    BOOST_CHECK_EQUAL(val[2].as<bool>(), true);

}

BOOST_AUTO_TEST_CASE(test_add_null)
{
    json a = json::array();
    a.push_back(jsoncons::null_type());
    a.push_back(json::null());
    BOOST_CHECK(a[0].is_null());
    BOOST_CHECK(a[1].is_null());
}

BOOST_AUTO_TEST_CASE(test_from_container)
{
    std::vector<int> vec;
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);

    json val1 = vec;
    BOOST_REQUIRE(vec.size() == 3);
    BOOST_CHECK(vec[0] == 10);
    BOOST_CHECK(vec[1] == 20);
    BOOST_CHECK(vec[2] == 30);

    std::list<double> list;
    list.push_back(10.5);
    list.push_back(20.5);
    list.push_back(30.5);

    json val2 = list;
    BOOST_REQUIRE(val2.size() == 3);
    BOOST_CHECK_CLOSE(val2[0].as<double>(),10.5,0.000001);
    BOOST_CHECK_CLOSE(val2[1].as<double>(),20.5,0.000001);
    BOOST_CHECK_CLOSE(val2[2].as<double>(),30.5,0.000001);
}

BOOST_AUTO_TEST_SUITE_END()


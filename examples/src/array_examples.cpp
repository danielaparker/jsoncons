// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include "jsoncons/json.hpp"

using jsoncons::json;
using jsoncons::pretty_print;
using jsoncons::output_format;
using std::string;

void accessing_a_json_value_as_a_vector()
{
    std::string s = "{\"my-array\" : [1,2,3,4]}";
    json val = json::parse_string(s);
    std::vector<int> v = val["my-array"].as<std::vector<int>>();
    for (size_t i = 0; i < v.size(); ++i)
    {
        if (i > 0)
        {
            std::cout << ",";
        }
        std::cout << v[i];
    }
    std::cout << std::endl;
}

void construct_json_from_vector()
{
    json root;

    root["addresses"];

    std::vector<json> addresses;
    json address1;
    address1["city"] = "San Francisco";
    address1["state"] = "CA";
    address1["zip"] = "94107";
    address1["country"] = "USA";
    addresses.push_back(address1);

    json address2;
    address2["city"] = "Sunnyvale";
    address2["state"] = "CA";
    address2["zip"] = "94085";
    address2["country"] = "USA";
    addresses.push_back(address2);

    root["addresses"] = json(addresses.begin(),addresses.end());

    std::cout << pretty_print(root) << std::endl;

    std::cout << "size=" << root["addresses"].size() << std::endl;
    for (size_t i = 0; i < root["addresses"].size(); ++i)
    {
        std::cout << root["addresses"][i] << std::endl;
    }
}

void add_element_to_array()
{
    json cities(json::an_array);  // an empty array
    std::cout << cities << std::endl;  // output is "[]"
    cities.add("Toronto");
    cities.add("Vancouver");
    cities.add(0,"Montreal");  // inserts "Montreal" at beginning of array

    std::cout << cities << std::endl;
}

void reserve_array_capacity()
{
    json cities(json::an_array);
    cities.reserve(10);  // storage is allocated
    std::cout << "capacity=" << cities.capacity() << ", size=" << cities.size() << std::endl;

    cities.add("Toronto");
    cities.add("Vancouver");
    cities.add(0,"Montreal");
    std::cout << "capacity=" << cities.capacity() << ", size=" << cities.size() << std::endl;

    std::cout << cities << std::endl;
}

void make_empty_array()
{
    std::cout << "empty array" <<std::endl;
    json a(json::an_array);
    a.add(1);
    a.add(2);
    std::cout << pretty_print(a) << std::endl;
}

void make_1_dimensional_array_1()
{
    std::cout << "1 dimensional array 1" <<std::endl;
    json a = json::make_array<1>();
    a.resize_array(10,0);
    a[1] = 1;
    a[2] = 2;
    std::cout << pretty_print(a) << std::endl;
}

void make_1_dimensional_array_2()
{
    std::cout << "1 dimensional array 2" <<std::endl;
    json a = json::make_array<1>(10,0);
    a[1] = 1;
    a[2] = 2;
    std::cout << pretty_print(a) << std::endl;
}

void make_2_dimensional_array()
{
    std::cout << "2 dimensional array" <<std::endl;
    json a = json::make_array<2>(3,4,0);
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

    std::cout << pretty_print(a) << std::endl;
}

void make_3_dimensional_array()
{
    std::cout << "3 dimensional array" <<std::endl;
    json a = json::make_array<3>(4,3,2,0);
    a[0][2][0] = 2;
    a[0][2][1] = 3;
    std::cout << pretty_print(a) << std::endl;
}

void array_examples()
{
    std::cout << "Array examples\n" << std::endl;
    construct_json_from_vector();
    add_element_to_array();
    reserve_array_capacity();
    accessing_a_json_value_as_a_vector();
    make_empty_array();
    make_1_dimensional_array_1();
    make_1_dimensional_array_2();
    make_2_dimensional_array();
    make_3_dimensional_array();

    std::cout << std::endl;
}


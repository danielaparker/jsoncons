// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include "jsoncons/json.hpp"

using jsoncons::json;
using jsoncons::pretty_print;
using jsoncons::output_format;
using std::string;

void add_element_to_undefined()
{
    json cities;  // undefined
    std::cout << cities << std::endl;  // no output
    cities.add("Toronto");  // becomes a json array
    cities.add("Vancouver");
    cities.add(0,"Montreal");  // inserts "Montreal" at beginning of array

    std::cout << cities << std::endl;
}

void add_element_to_array()
{
    json cities(json::an_array);  // an array
    std::cout << cities << std::endl;  // output is "[]"
    cities.add("Toronto");  
    cities.add("Vancouver");
    cities.add(0,"Montreal");  // inserts "Montreal" at beginning of array

    std::cout << cities << std::endl;
}

void reserve_array_capacity()
{
    json cities;  // undefined
    cities.reserve(10);  // still undefined, storage is still unallocated
    std::cout << "capacity=" << cities.capacity() << ", size=" << cities.size() << std::endl;

    cities.add("Toronto");  // becomes a json array, storage for 10 elements is allocated, size is 1
    cities.add("Vancouver");
    cities.add(0,"Montreal");
    std::cout << "capacity=" << cities.capacity() << ", size=" << cities.size() << std::endl;

    std::cout << cities << std::endl;
}

void make_1_dimensional_array()
{
    std::cout << "1 dimensional array" <<std::endl;
    json a = json::make_array(10,0);
    a[1] = 1;
    a[2] = 2;
    std::cout << pretty_print(a) << std::endl;
}

void make_2_dimensional_array()
{
    std::cout << "2 dimensional array" <<std::endl;
    json a = json::make_2d_array(3,4,0);
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

    output_format format;
    format.set_floating_point_format(output_format::fixed,6);
    std::cout << pretty_print(a,format) << std::endl;
}

void make_3_dimensional_array()
{
    std::cout << "3 dimensional array" <<std::endl;
    json a = json::make_3d_array(4,3,2,0);
    a[0][2][0] = 2;
    a[0][2][1] = 3;
    std::cout << pretty_print(a) << std::endl;
}



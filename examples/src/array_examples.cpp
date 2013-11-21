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

    std::cout << pretty_print(cities) << std::endl;
}



// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>
#include <iostream>

using namespace jsoncons;

void json_constructor_examples()
{   
    json j1; // An empty object
    std::cout << "(1) " << j1 << std::endl;

    json j2(json_object_arg, {{"baz", "qux"}, {"foo", "bar"}}); // An object 
    std::cout << "(2) " << j2 << std::endl;

    json j3(json_array_arg, {"bar", "baz"}); // An array 
    std::cout << "(3) " << j3 << std::endl;
  
    json j4(json::null()); // A null value
    std::cout << "(4) " << j4 << std::endl;
    
    json j5(true); // A boolean value
    std::cout << "(5) " << j5 << std::endl;

    double x = 1.0/7.0;

    json j6(x); // A double value
    std::cout << "(6) " << j6 << std::endl;

    json j8("Hello"); // A text string
    std::cout << "(8) " << j8 << std::endl;

    std::vector<int> v = {10,20,30};
    json j9 = v; // From a sequence container
    std::cout << "(9) " << j9 << std::endl;

    std::map<std::string, int> m{ {"one", 1}, {"two", 2}, {"three", 3} };
    json j10 = m; // From an associative container
    std::cout << "(10) " << j10 << std::endl;

    std::vector<uint8_t> bytes = {'H','e','l','l','o'};
    json j11(byte_string_arg, bytes); // A byte string
    std::cout << "(11) " << j11 << std::endl;

    json j12(half_arg, 0x3bff);
    std::cout << "(12) " << j12.as_double() << std::endl;

    // An object value with four members
    json obj;
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";
    obj["events_attended"] = 10;
    obj["accept_waiver_of_liability"] = true;

    std::string first_name = obj["first_name"].as<std::string>();
    std::string last_name = obj.at("last_name").as<std::string>();
    int events_attended = obj["events_attended"].as<int>();
    bool accept_waiver_of_liability = obj["accept_waiver_of_liability"].as<bool>();

    // An array value with four elements
    json arr(json_array_arg);
    arr.push_back(j1);
    arr.push_back(j2);
    arr.push_back(j3);
    arr.push_back(j4);

    json_options options;
    std::cout << pretty_print(arr) << std::endl;
}


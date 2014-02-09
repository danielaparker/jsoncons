// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include "jsoncons/json.hpp"
#include "my_custom_data.hpp"

using jsoncons::json;
using jsoncons::basic_json;
using jsoncons::pretty_print;
using std::string;

using jsoncons::basic_custom_data;
using jsoncons::custom_data_wrapper;

void put_custom_data_in_object()
{
    json obj;
    std::vector<double> v(4);
    v[0] = 0;
    v[1] = 1;
    v[2] = 2;
    v[3] = 3;

    obj.set_custom_data("myvector",v);
    std::cout << pretty_print(obj) << std::endl;

    std::vector<double> v1 = obj["myvector"].custom_data<std::vector<double>>();
    for (size_t i = 0; i < v1.size(); ++i)
    {
        std::cout << v1[i] << " ";
    }
    std::cout << std::endl;
}

void put_custom_data_in_array()
{
    json arr(json::an_array);
    std::vector<double> v(4);
    v[0] = 0;
    v[1] = 1;
    v[2] = 2;
    v[3] = 3;

    arr.add_custom_data(v);

    std::vector<double> v1 = arr[0].custom_data<std::vector<double>>();

    std::cout << v1.size() << std::endl;

    for (size_t i = 0; i < v1.size(); ++i)
    {
        std::cout << v1[i] << " ";
    }
    std::cout << std::endl;
}

void custom_data_examples()
{
    std::cout << "Custom data examples\n" << std::endl;

    put_custom_data_in_object();
    put_custom_data_in_array();

    std::cout << std::endl;
}


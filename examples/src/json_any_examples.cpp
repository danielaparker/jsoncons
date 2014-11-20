// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include "jsoncons/json.hpp"
#include "my_any_specializations.hpp"

using jsoncons::json;
using jsoncons::basic_json;
using jsoncons::pretty_print;
using std::string;

using jsoncons::json_any_impl;
using jsoncons::typed_json_any;

void put_any_value_in_object()
{
    json obj;
    std::vector<double> v(4);
    v[0] = 0;
    v[1] = 1;
    v[2] = 2;
    v[3] = 3;

    obj.set("myvector",json::any(v));
    std::cout << pretty_print(obj) << std::endl;

    std::vector<double> v1 = obj["myvector"].as<json::any>().cast<std::vector<double>>();
    for (size_t i = 0; i < v1.size(); ++i)
    {
        std::cout << v1[i] << " ";
    }
    std::cout << std::endl;
}

void put_any_value_in_arry()
{
    json arr(json::an_array);
    std::vector<double> v(4);
    v[0] = 0;
    v[1] = 1;
    v[2] = 2;
    v[3] = 3;

    arr.add(json::any(v));

    std::vector<double> v1 = arr[0].as<json::any>().cast<std::vector<double>>();

    std::cout << v1.size() << std::endl;

    for (size_t i = 0; i < v1.size(); ++i)
    {
        std::cout << v1[i] << " ";
    }
    std::cout << std::endl;
}

void json_any_examples()
{
    std::cout << "Custom data examples\n" << std::endl;

    put_any_value_in_object();
    put_any_value_in_arry();

    std::cout << std::endl;
}


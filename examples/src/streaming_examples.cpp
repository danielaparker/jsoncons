// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <iostream>
#include <map>
#include <tuple>
#include <jsoncons/json_stream_traits.hpp>

using namespace jsoncons;

void streaming_example1()
{
    std::map<std::string,std::tuple<std::string,std::string,double>> employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    std::cout << "(1)\n" << std::endl; 
    dump(employees,std::cout);
    std::cout << "\n\n";

    std::cout << "(2) Again, with pretty print\n" << std::endl; 
    dump(employees,std::cout,true);
}


void streaming_examples()
{
    std::cout << "\nStreaming examples\n\n";
    streaming_example1();

    std::cout << std::endl;
}


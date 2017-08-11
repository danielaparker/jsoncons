// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <iostream>
#include <map>
#include <tuple>
#include <jsoncons/json_stream_traits.hpp>

using namespace jsoncons;

namespace examples { namespace streaming {

class Employee
{
    std::string name_;
public:
    Employee(const std::string& name)
        : name_(name)
    {
    }
    virtual ~Employee() = default;
    const std::string& name() const
    {
        return name_;
    }
    virtual double calculatePay() const = 0;
};

class HourlyEmployee : public Employee
{
public:
    HourlyEmployee(const std::string& name)
        : Employee(name)
    {
    }
    double calculatePay() const override
    {
        return 10000;
    }
};

class CommissionedEmployee : public Employee
{
public:
    CommissionedEmployee(const std::string& name)
        : Employee(name)
    {
    }
    double calculatePay() const override
    {
        return 20000;
    }
};

}}
using namespace examples::streaming;

namespace jsoncons
{
    template <>
    struct json_stream_traits<char,Employee>
    {
        static void encode(const Employee& val, json_output_handler& handler)
        {
            handler.begin_object();
            handler.name("Name");
            handler.string_value(val.name());
            handler.name("Pay");
            handler.double_value(val.calculatePay());
            handler.end_object();
        }
    };
};


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


void streaming_example2()
{
    std::shared_ptr<Employee> j1 = std::make_shared<HourlyEmployee>("John Smith");
    std::shared_ptr<Employee> j2 = std::make_shared<CommissionedEmployee>("Jane Doe");

    std::cout << "\n\n";
    dump(j1,std::cout,true);
    std::cout << "\n\n";
    dump(j2,std::cout, true);
    std::cout << "\n\n";
}

void streaming_examples()
{
    std::cout << "\nStreaming examples\n\n";

    streaming_example1();
    streaming_example2();

    std::cout << std::endl;
}


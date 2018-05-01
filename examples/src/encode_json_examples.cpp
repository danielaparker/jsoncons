// Copyright 2017 Daniel Parker
// Distributed under Boost license

#include <iostream>
#include <map>
#include <tuple>
#include <jsoncons/json.hpp>

using namespace jsoncons;

namespace examples { namespace encode_json {

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
using namespace examples::encode_json;

namespace jsoncons
{
    template <>
    struct json_convert_traits<std::shared_ptr<Employee>>
    {
        static void encode(const std::shared_ptr<Employee>& val, json_output_handler& handler)
        {
            handler.begin_object();
            handler.name("Name");
            handler.string_value(val->name());
            handler.name("Pay");
            handler.double_value(val->calculatePay());
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
    encode_json(employees,std::cout);
    std::cout << "\n\n";

    std::cout << "(2) Again, with pretty print\n" << std::endl; 
    encode_json(employees, std::cout, jsoncons::pretty_printer());

    std::cout << "\n\n";
}

void streaming_example2()
{
    std::map<std::string,std::tuple<std::string,std::string,double>> employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    // `true` means pretty print
    json_serializer serializer(std::cout, jsoncons::pretty_printer()); 

    serializer.begin_json();       
    serializer.begin_object();       
    serializer.name("Employees");       
    encode_fragment(employees, serializer);
    serializer.end_object();       
    serializer.end_json();       

    std::cout << "\n\n";
}


void streaming_example3()
{
    std::vector<std::shared_ptr<Employee>> employees; 

    employees.push_back(std::make_shared<HourlyEmployee>("John Smith"));
    employees.push_back(std::make_shared<CommissionedEmployee>("Jane Doe"));

    encode_json(employees, std::cout, jsoncons::pretty_printer());

    std::cout << "\n\n";
}

void encode_json_examples()
{
    std::cout << "\nStreaming examples\n\n";

    streaming_example1();
    streaming_example2();
    streaming_example3();

    std::cout << std::endl;
}


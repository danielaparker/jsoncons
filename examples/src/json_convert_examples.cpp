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
    template <class T>
    static T decode(std::istringstream& is,
                    const json_serializing_options& options)
    {
        json j = json::parse(is, options);
        return j. template as<T>();
    }

    template <>
    struct json_convert_traits<std::shared_ptr<Employee>>
    {
        static void encode(const std::shared_ptr<Employee>& val, json_content_handler& handler)
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
    typedef std::map<std::string,std::tuple<std::string,std::string,double>> employee_collection;

    employee_collection employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    std::cout << "(1)\n" << std::endl; 
    encode_json(employees,std::cout);
    std::cout << "\n\n";

    std::cout << "(2) Again, with pretty print\n" << std::endl; 
    encode_json(employees, std::cout, jsoncons::indenting::indent);

    std::cout << "\n\n";
}

void streaming_example2()
{
    std::map<std::string,std::tuple<std::string,std::string,double>> employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    json_serializer serializer(std::cout, jsoncons::indenting::indent); 

    serializer.begin_document();       
    serializer.begin_object();       
    serializer.name("Employees");       
    encode_fragment(employees, serializer);
    serializer.end_object();       
    serializer.end_document();       

    std::cout << "\n\n";
}


void streaming_example3()
{
    std::vector<std::shared_ptr<Employee>> employees; 

    employees.push_back(std::make_shared<HourlyEmployee>("John Smith"));
    employees.push_back(std::make_shared<CommissionedEmployee>("Jane Doe"));

    encode_json(employees, std::cout, jsoncons::indenting::indent);

    std::cout << "\n\n";
}

void json_decode_example()
{
    std::string s = R"(
    {
        "Jane Doe": ["Commission","Sales",20000.0],
        "John Smith": ["Hourly","Software Engineer",10000.0]
    }
    )";

    typedef std::map<std::string,std::tuple<std::string,std::string,double>> employee_collection;

    employee_collection employees = jsoncons::decode_json<employee_collection>(s);

    for (const auto& pair : employees)
    {
        std::cout << pair.first << ": " << std::get<1>(pair.second) << std::endl;
    }
}

void json_convert_examples()
{
    std::cout << "\njson convert examples\n\n";

    streaming_example1();
    streaming_example2();
    streaming_example3();
    json_decode_example();

    std::cout << std::endl;
}



// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include "sample_types.hpp"
#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <iomanip>
#include <jsoncons/json.hpp>

using namespace jsoncons;

namespace ns {

    class Employee
    {
        std::string firstName_;
        std::string lastName_;
    public:
        Employee(const std::string& firstName, const std::string& lastName)
            : firstName_(firstName), lastName_(lastName)
        {
        }
        virtual ~Employee() = default;

        virtual double calculatePay() const = 0;

        const std::string& firstName() const {return firstName_;}
        const std::string& lastName() const {return lastName_;}
    };

    class HourlyEmployee : public Employee
    {
        double wage_;
        unsigned hours_;
    public:
        HourlyEmployee(const std::string& firstName, const std::string& lastName, 
                       double wage, unsigned hours)
            : Employee(firstName, lastName), 
              wage_(wage), hours_(hours)
        {
        }
        HourlyEmployee(const HourlyEmployee&) = default;
        HourlyEmployee(HourlyEmployee&&) = default;
        HourlyEmployee& operator=(const HourlyEmployee&) = default;
        HourlyEmployee& operator=(HourlyEmployee&&) = default;

        double wage() const {return wage_;}

        unsigned hours() const {return hours_;}

        double calculatePay() const override
        {
            return wage_*hours_;
        }
    };

    class CommissionedEmployee : public Employee
    {
        double baseSalary_;
        double commission_;
        unsigned sales_;
    public:
        CommissionedEmployee(const std::string& firstName, const std::string& lastName, 
                             double baseSalary, double commission, unsigned sales)
            : Employee(firstName, lastName), 
              baseSalary_(baseSalary), commission_(commission), sales_(sales)
        {
        }
        CommissionedEmployee(const CommissionedEmployee&) = default;
        CommissionedEmployee(CommissionedEmployee&&) = default;
        CommissionedEmployee& operator=(const CommissionedEmployee&) = default;
        CommissionedEmployee& operator=(CommissionedEmployee&&) = default;

        double baseSalary() const
        {
            return baseSalary_;
        }

        double commission() const
        {
            return commission_;
        }

        unsigned sales() const
        {
            return sales_;
        }

        double calculatePay() const override
        {
            return baseSalary_ + commission_*sales_;
        }
    };

} // ns

JSONCONS_GETTER_CTOR_TRAITS_DECL(ns::HourlyEmployee, firstName, lastName, wage, hours)
JSONCONS_GETTER_CTOR_TRAITS_DECL(ns::CommissionedEmployee, firstName, lastName, baseSalary, commission, sales)
JSONCONS_POLYMORPHIC_TRAITS_DECL(ns::Employee, ns::HourlyEmployee, ns::CommissionedEmployee)

void employee_polymorphic_decode_example()
{
    std::string input = R"(
[
    {
        "firstName": "John",
        "hours": 1000,
        "lastName": "Smith",
        "type": "Hourly",
        "wage": 40.0
    },
    {
        "baseSalary": 30000.0,
        "commission": 0.25,
        "firstName": "Jane",
        "lastName": "Doe",
        "sales": 1000,
        "type": "Commissioned"
    }
]
    )"; 

    auto v = jsoncons::decode_json<std::vector<std::shared_ptr<ns::Employee>>>(input);

    for (auto p : v)
    {
        std::cout << p->firstName() << " " << p->lastName() << " " << p->calculatePay() << "\n";
    }
}

void employee_polymorphic_encode_example()
{
    std::vector<std::shared_ptr<ns::Employee>> v;

    v.push_back(std::make_shared<ns::HourlyEmployee>("John", "Smith", 40.0, 1000));
    v.push_back(std::make_shared<ns::CommissionedEmployee>("Jane", "Doe", 30000, 0.25, 1000));

    jsoncons::json j(v);
    std::cout << pretty_print(j) << "\n\n";
}

void polymorphic_examples()
{
    std::cout << "\nPolymorhic examples\n\n";

    employee_polymorphic_decode_example();

    employee_polymorphic_encode_example();

    std::cout << std::endl;
}


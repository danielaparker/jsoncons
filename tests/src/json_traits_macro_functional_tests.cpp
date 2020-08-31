// Copyright 2020 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstdint>
#include <jsoncons/json.hpp>

namespace {
namespace ns {

    class Employee_NGSN
    {
        std::string name_;
        std::string surname_;
    public:
        std::string getName() const
        {
            return name_;
        }
        void setName(const std::string& name)
        {
            name_ = name;
        }
        std::string getSurname()const
        {
            return surname_;
        }
        void setSurname(const std::string& surname)
        {
            surname_ = surname;
        }

        friend bool operator<(const Employee_NGSN& lhs, const Employee_NGSN& rhs)
        {
            if (lhs.surname_ < rhs.surname_)
                return true;
            return lhs.name_ < rhs.name_;
        }
    };

    class Company_NGSN 
    {
        std::string name_;
        std::vector<uint64_t> employeeIds_;
    public:
        std::string getName() const
        {
            return name_;
        }
        void setName(const std::string& name)
        {
            name_ = name;
        }
        const std::vector<uint64_t> getIds() const
        {
            return employeeIds_;
        }
        void setIds(const std::vector<uint64_t>& employeeIds)
        {
            employeeIds_ = employeeIds;
        }
    };

    class Employee_AGSN
    {
        std::string name_;
        std::string surname_;
    public:
        std::string getName() const
        {
            return name_;
        }
        void setName(const std::string& name)
        {
            name_ = name;
        }
        std::string getSurname()const
        {
            return surname_;
        }
        void setSurname(const std::string& surname)
        {
            surname_ = surname;
        }

        friend bool operator<(const Employee_AGSN& lhs, const Employee_AGSN& rhs)
        {
            if (lhs.surname_ < rhs.surname_)
                return true;
            return lhs.name_ < rhs.name_;
        }
    };

    class Company_AGSN 
    {
        std::string name_;
        std::vector<uint64_t> employeeIds_;
    public:
        std::string getName() const
        {
            return name_;
        }
        void setName(const std::string& name)
        {
            name_ = name;
        }
        const std::vector<uint64_t> getIds() const
        {
            return employeeIds_;
        }
        void setIds(const std::vector<uint64_t>& employeeIds)
        {
            employeeIds_ = employeeIds;
        }
    };

    template <class Employee>
    std::vector<Employee> fromIdsToEmployees(const std::vector<uint64_t>& ids)
    {
        Employee employee1;
        employee1.setName("John");
        employee1.setSurname("Smith");
        Employee employee2;
        employee2.setName("Jane");
        employee2.setSurname("Doe");
        std::map<uint64_t, Employee> id_employee_map = { {1,employee1},{2,employee2} };

        std::vector<Employee> employees;
        for (auto id : ids)
        {
            employees.push_back(id_employee_map.at(id));
        }
        return employees;
    };

    template <class Employee>
    std::vector<uint64_t> fromEmployeesToIds(const std::vector<Employee>& employees)
    {
        Employee employee1;
        employee1.setName("John");
        employee1.setSurname("Smith");
        Employee employee2;
        employee2.setName("Jane");
        employee2.setSurname("Doe");
        std::map<Employee, uint64_t> employee_id_map = { {employee1,1},{employee2,2} };

        std::vector<uint64_t> ids;
        for (auto employee : employees)
        {
            ids.push_back(employee_id_map.at(employee));
        }
        return ids;
    };

} // namespace
} // ns

JSONCONS_N_GETTER_SETTER_NAME_TRAITS(ns::Employee_NGSN, 2,
                                      (getName, setName, "employee_name"),
                                      (getSurname, setSurname, "employee_surname")
                                    )

JSONCONS_N_GETTER_SETTER_NAME_TRAITS(ns::Company_NGSN, 2,
  (getName, setName, "company"),
  (getIds, ns::fromIdsToEmployees<ns::Employee_NGSN>, ns::fromEmployeesToIds<ns::Employee_NGSN>, setIds, "resources")
)

JSONCONS_ALL_GETTER_SETTER_NAME_TRAITS(ns::Employee_AGSN,
    (getName, setName, "employee_name"),
    (getSurname, setSurname, "employee_surname")
)

JSONCONS_ALL_GETTER_SETTER_NAME_TRAITS(ns::Company_AGSN,
    (getName, setName, "company"),
    (getIds, ns::fromIdsToEmployees<ns::Employee_AGSN>, ns::fromEmployeesToIds<ns::Employee_AGSN>, setIds, "resources")
)

using namespace jsoncons;

TEST_CASE("JSONCONS_N_GETTER_SETTER_NAME_TRAITS functional tests")
{
    SECTION("test 1")
    {
        std::vector<uint64_t> ids = {1,2};

        ns::Company_NGSN company;
        company.setName("Example");
        company.setIds(ids);

        std::string output2;
        encode_json(company, output2);
        //std::cout << output2 << "\n\n";
        auto company2 = decode_json<ns::Company_NGSN>(output2);
        std::string output3;
        encode_json(company, output3);
        CHECK(output3 == output2);

        auto j = decode_json<json>(output3);
        CHECK(j.is<ns::Company_NGSN>());
        CHECK(j.is<ns::Company_AGSN>());
    }
} 

TEST_CASE("JSONCONS_ALL_GETTER_SETTER_NAME_TRAITS functional tests")
{
    SECTION("test 1")
    {
        std::vector<uint64_t> ids = {1,2};
        
        ns::Company_AGSN company;
        company.setName("Example");
        company.setIds(ids);

        std::string output2;
        encode_json(company, output2);
        //std::cout << output2 << "\n\n";
        auto company2 = decode_json<ns::Company_AGSN>(output2);
        std::string output3;
        encode_json(company, output3);
        CHECK(output3 == output2);

        auto j = decode_json<json>(output3);
        CHECK(j.is<ns::Company_NGSN>());
        CHECK(j.is<ns::Company_AGSN>());
    }
} 


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

#define NEW_CONCAT_RAW(a, b) a ## b
#define NEW_CONCAT(a, b) NEW_CONCAT_RAW(a, b)

#define UNUSABLE_VALUE { static_assert (false, "bad value"); }

#define NEW_GETTER_SETTER_IS(P1, P2, P3, Seq, Count) NEW_GETTER_SETTER_IS_LAST(P1, P2, P3, Seq, Count)
#define NEW_GETTER_SETTER_IS_LAST(P1, P2, P3, Seq, Count) if ((num_params-Count) < num_mandatory_params1 && JSONCONS_EXPAND(NEW_CONCAT(NEW_GETTER_SETTER_IS_,JSONCONS_NARGS Seq) Seq)
#define NEW_GETTER_SETTER_IS_3(Getter, Setter, Name) !ajson.contains(Name)) return false;
#define NEW_GETTER_SETTER_IS_5(Getter, F1, F2, Setter, Name) !ajson.contains(Name)) return false;

#define NEW_N_GETTER_SETTER_NAME_AS(P1, P2, P3, Seq, Count) NEW_N_GETTER_SETTER_NAME_AS_LAST(P1, P2, P3, Seq, Count)
#define NEW_N_GETTER_SETTER_NAME_AS_LAST(P1, P2, P3, Seq, Count) if ((num_params-Count) < num_mandatory_params2 || JSONCONS_EXPAND(NEW_CONCAT(NEW_N_GETTER_SETTER_NAME_AS_,JSONCONS_NARGS Seq) Seq)
#define NEW_N_GETTER_SETTER_NAME_AS_3(Getter, Setter, Name) ajson.contains(Name)) aval.Setter(ajson.at(Name).template as<typename std::decay<decltype(aval.Getter())>::type>());
#define NEW_N_GETTER_SETTER_NAME_AS_5(Getter, F1, F2, Setter, Name) ajson.contains(Name)) aval.Setter(F2(ajson.at(Name).template as<typename std::decay<decltype(F1(aval.Getter()))>::type>()));

#define NEW_N_GETTER_SETTER_NAME_TO_JSON(P1, P2, P3, Seq, Count) NEW_N_GETTER_SETTER_NAME_TO_JSON_LAST(P1, P2, P3, Seq, Count)
#define NEW_N_GETTER_SETTER_NAME_TO_JSON_LAST(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(NEW_CONCAT(NEW_N_GETTER_SETTER_NAME_TO_JSON_,JSONCONS_NARGS Seq) Seq)
#define NEW_N_GETTER_SETTER_NAME_TO_JSON_3(Getter, Setter, Name) ajson.try_emplace(Name, aval.Getter() );
#define NEW_N_GETTER_SETTER_NAME_TO_JSON_5(Getter, F1, F2, Setter, Name) ajson.try_emplace(Name, F1(aval.Getter()) );

#define NEW_ALL_GETTER_SETTER_NAME_AS(P1, P2, P3, Seq, Count) NEW_ALL_GETTER_SETTER_NAME_AS_LAST(P1, P2, P3, Seq, Count)
#define NEW_ALL_GETTER_SETTER_NAME_AS_LAST(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(NEW_CONCAT(NEW_ALL_GETTER_SETTER_NAME_AS_,JSONCONS_NARGS Seq) Seq)
#define NEW_ALL_GETTER_SETTER_NAME_AS_3(Getter, Setter, Name) aval.Setter(ajson.at(Name).template as<typename std::decay<decltype(aval.Getter())>::type>());
#define NEW_ALL_GETTER_SETTER_NAME_AS_5(Getter, F1, F2, Setter, Name) aval.Setter(F2(ajson.at(Name).template as<typename std::decay<decltype(F1(aval.Getter()))>::type>()));

#define NEW_ALL_GETTER_SETTER_NAME_TO_JSON(P1, P2, P3, Seq, Count) NEW_ALL_GETTER_SETTER_NAME_TO_JSON_LAST(P1, P2, P3, Seq, Count)
#define NEW_ALL_GETTER_SETTER_NAME_TO_JSON_LAST(P1, P2, P3, Seq, Count) if ((num_params-Count) < num_mandatory_params2) JSONCONS_EXPAND(NEW_CONCAT(NEW_ALL_GETTER_SETTER_NAME_TO_JSON_,JSONCONS_NARGS Seq) Seq)
#define NEW_ALL_GETTER_SETTER_NAME_TO_JSON_3(Getter, Setter, Name) \
  ajson.try_emplace(Name, aval.Getter()); \
else \
  {json_traits_helper<Json>::set_optional_json_member(Name, aval.Getter(), ajson);}
#define NEW_ALL_GETTER_SETTER_NAME_TO_JSON_5(Getter, F1, F2, Setter, Name) \
  ajson.try_emplace(Name, F1(aval.Getter())); \
else \
  {json_traits_helper<Json>::set_optional_json_member(Name, F1(aval.Getter()), ajson);}

#define NEW_GETTER_SETTER_NAME_TRAITS_BASE(AsT,ToJ, NumTemplateParams, ValueType,NumMandatoryParams1,NumMandatoryParams2, ...)  \
namespace jsoncons \
{ \
    template<typename Json JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_MORE_TPL_PARAM, NumTemplateParams)> \
    struct json_type_traits<Json, ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> \
    { \
        using value_type = ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams); \
        using allocator_type = typename Json::allocator_type; \
        using char_type = typename Json::char_type; \
        using string_view_type = typename Json::string_view_type; \
        constexpr static size_t num_params = JSONCONS_NARGS(__VA_ARGS__); \
        constexpr static size_t num_mandatory_params1 = NumMandatoryParams1; \
        constexpr static size_t num_mandatory_params2 = NumMandatoryParams2; \
        static bool is(const Json& ajson) noexcept \
        { \
            if (!ajson.is_object()) return false; \
            JSONCONS_VARIADIC_REP_N(NEW_GETTER_SETTER_IS,,,, __VA_ARGS__)\
            return true; \
        } \
        static value_type as(const Json& ajson) \
        { \
            value_type aval{}; \
            JSONCONS_VARIADIC_REP_N(AsT,,,, __VA_ARGS__) \
            return aval; \
        } \
        static Json to_json(const value_type& aval, allocator_type alloc=allocator_type()) \
        { \
            Json ajson(json_object_arg, semantic_tag::none, alloc); \
            JSONCONS_VARIADIC_REP_N(ToJ,,,, __VA_ARGS__) \
            return ajson; \
        } \
    }; \
} \
  /**/

#define NEW_N_GETTER_SETTER_NAME_TRAITS(ValueType,NumMandatoryParams, ...)  \
    NEW_GETTER_SETTER_NAME_TRAITS_BASE(NEW_N_GETTER_SETTER_NAME_AS,NEW_N_GETTER_SETTER_NAME_TO_JSON, 0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/
 
#define NEW_ALL_GETTER_SETTER_NAME_TRAITS(ValueType, ...)  \
    NEW_GETTER_SETTER_NAME_TRAITS_BASE(NEW_ALL_GETTER_SETTER_NAME_AS,NEW_ALL_GETTER_SETTER_NAME_TO_JSON, 0, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/

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

NEW_N_GETTER_SETTER_NAME_TRAITS(ns::Employee_NGSN, 2,
                                      (getName, setName, "employee_name"),
                                      (getSurname, setSurname, "employee_surname")
                                    )

NEW_N_GETTER_SETTER_NAME_TRAITS(ns::Company_NGSN, 2,
  (getName, setName, "company"),
  (getIds, ns::fromIdsToEmployees<ns::Employee_NGSN>, ns::fromEmployeesToIds<ns::Employee_NGSN>, setIds, "resources")
)

NEW_ALL_GETTER_SETTER_NAME_TRAITS(ns::Employee_AGSN,
                                      (getName, setName, "employee_name"),
                                      (getSurname, setSurname, "employee_surname")
                                    )

NEW_ALL_GETTER_SETTER_NAME_TRAITS(ns::Company_AGSN,
  (getName, setName, "company"),
  (getIds, ns::fromIdsToEmployees<ns::Employee_AGSN>, ns::fromEmployeesToIds<ns::Employee_AGSN>, setIds, "resources")
)

using namespace jsoncons;

TEST_CASE("NEW_N_GETTER_SETTER_NAME_TRAITS functional tests")
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

TEST_CASE("NEW_ALL_GETTER_SETTER_NAME_TRAITS functional tests")
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


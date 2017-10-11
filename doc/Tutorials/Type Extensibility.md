In the `json` class, constructors, accessors and modifiers are templated, for example,

```c++
template <class T>
json(const T& val)

template<class T, class... Args>
bool is(Args&&... args) const

template<class T, class... Args>
T as(Args&&... args) const

template <class T>
basic_json& operator=(const T& val)

template <class T>
void add(T&& val)

template <class T>
void insert_or_assign(const string_view_type& name, T&& val)
```
The implementations of these functions and operators make use of the class template `json_type_traits`

```c++
template <class Json, class T, class Enable=void>
struct json_type_traits;
```

If you want to use the json constructor, `is<T>`, `as<T>`, `operator=`, `add`, and `insert_or_assign` to access or modify with a new type, you need to show `json` how to interact with that type, by extending `json_type_traits` in the `jsoncons` namespace.

Note that the json::is<T>() and json::as<T>() functions accept template packs, which they forward to the `json_type_traits` `is` and `as` functions.
This allows user defined `json_type_traits` implementations to resolve, for instance, a name into a C++ object
looked up from a registry, as illustrated in Example 2 below.

### Example 1. Convert to/from user defined type

You can provide a specialization of `json_type_traits` for a `book` class, and then transfer book objects or
standard library collections of book objects to and from `json` values.

```c++
struct book
{
    std::string author;
    std::string title;
    double price;
};

namespace jsoncons
{
    template<class Json>
    struct json_type_traits<Json, book>
    {
        static bool is(const Json& rhs) noexcept
        {
            return rhs.is_object() &&
                   rhs.has_key("author") && 
                   rhs.has_key("title") && 
                   rhs.has_key("price");
        }
        static book as(const Json& rhs)
        {
            book val;
            val.author = rhs["author"]. template as<std::string>();
            val.title = rhs["title"]. template as<std::string>();
            val.price = rhs["price"]. template as<double>();
            return val;
        }
        static Json to_json(const book& val)
        {
            Json j;
            j["author"] = val.author;
            j["title"] = val.title;
            j["price"] = val.price;
            return j;
        }
    };
};

int main()
{
    book book1{"Haruki Murakami", "Kafka on the Shore", 25.17};

    json j = book1;

    std::cout << "(1) " << std::boolalpha << j.is<book>() << "\n\n";

    std::cout << "(2) " << pretty_print(j) << "\n\n";

    book temp = j.as<book>();
    std::cout << "(3) " << temp.author << "," 
                        << temp.title << "," 
                        << temp.price << "\n\n";

    book book2{"Charles Bukowski", "Women: A Novel", 12.0};

    std::vector<book> book_array{book1, book2};

    json ja = book_array;

    std::cout << "(4) " << std::boolalpha 
                        << ja.is<std::vector<book>>() << "\n\n";

    std::cout << "(5)\n" << pretty_print(ja) << "\n\n";

    auto book_list = ja.as<std::list<book>>();

    std::cout << "(6)" << std::endl;
    for (auto b : book_list)
    {
        std::cout << b.author << ", " 
                  << b.title << ", " 
                  << b.price << std::endl;
    }
}
``` 
Output:
```
(1) true

(2) {
    "author": "Haruki Murakami",
    "price": 25.17,
    "title": "Kafka on the Shore"
}

(3) Haruki Murakami,Kafka on the Shore,25.17

(4) true

(5)
[
    {
        "author": "Haruki Murakami",
        "price": 25.17,
        "title": "Kafka on the Shore"
    },
    {
        "author": "Charles Bukowski",
        "price": 12.0,
        "title": "Women: A Novel"
    }
]

(6)
Haruki Murakami, Kafka on the Shore, 25.17
Charles Bukowski, Women: A Novel, 12
```

### Example 2. User defined types that require lookup to resolve

This example takes advantage of the fact that we can pass
parameters to `json::is()` and `json::as()`, which forward them to
the `json_type_traits` functions.

```c++
#include <string>
#include <unordered_map>
#include <memory>
#include <jsoncons/json.hpp>

using namespace jsoncons;
using namespace jsoncons::literals;

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

class EmployeeRegistry
{
    typedef std::unordered_map<std::string,std::shared_ptr<Employee>> employee_map;
    employee_map employees_;
public:
    EmployeeRegistry()
    {
        employees_.try_emplace("John Smith",std::make_shared<HourlyEmployee>("John Smith"));
        employees_.try_emplace("Jane Doe",std::make_shared<CommissionedEmployee>("Jane Doe"));
    }

    bool contains(const std::string& name) const
    {
        return employees_.count(name) > 0; 
    }

    std::shared_ptr<Employee> get(const std::string& name) const
    {
        auto it = employees_.find(name);
        if (it == employees_.end())
        {
            throw std::runtime_error("Employee not found");
        }
        return it->second; 
    }
};

namespace jsoncons
{
    template<class Json>
    struct json_type_traits<Json, std::shared_ptr<Employee>>
    {
        static bool is(const Json& rhs, const EmployeeRegistry& registry) noexcept
        {
            return rhs.is_string() && registry.contains(rhs.as<std::string>());
        }
        static std::shared_ptr<Employee> as(const Json& rhs, 
                                            const EmployeeRegistry& registry)
        {
            return registry.get(rhs.as<std::string>());
        }
        static Json to_json(std::shared_ptr<Employee> val)
        {
            Json j(val->name());
            return j;
        }
    };
};

int main()
{
    json j = R"(
    {
        "EmployeeName" : "John Smith"
    }
    )"_json;

    EmployeeRegistry registry;

    std::shared_ptr<Employee> employee = j["EmployeeName"].as<std::shared_ptr<Employee>>(registry);

    std::cout << "(1) " << employee->name() << " => " 
              << employee->calculatePay() << std::endl;

    // j does not have a key "SalesRep", so get_with_default returns "Jane Doe"
    // The template parameter is explicitly specified as json, to return a json string
    // json::as is then applied to the returned json string  
    std::shared_ptr<Employee> salesRep = j.get_with_default<json>("SalesRep","Jane Doe")
                                          .as<std::shared_ptr<Employee>>(registry);

    std::cout << "(2) " << salesRep->name() << " => " 
              << salesRep->calculatePay() << std::endl;

    json j2;
    j2["EmployeeName"] = employee;
    j2["SalesRep"] = salesRep;

    std::cout << "(3)\n" << pretty_print(j2) << std::endl;
}
```

Output:

```json
(1) John Smith => 10000
(2) Jane Doe => 20000
(3)
{
    "EmployeeName": "John Smith",
    "SalesRep": "Jane Doe"
}
```

### Example 3. Convert to/from `boost::gregorian::date`

```c++
#include <jsoncons/json.hpp>
#include "boost/date_time/gregorian/gregorian.hpp"

namespace jsoncons 
{
    template <class Json>
    struct json_type_traits<Json,boost::gregorian::date>
    {
        static const bool is_assignable = true;

        static bool is(const Json& val) noexcept
        {
            if (!val.is_string())
            {
                return false;
            }
            std::string s = val.template as<std::string>();
            try
            {
                boost::gregorian::from_simple_string(s);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        static boost::gregorian::date as(const Json& val)
        {
            std::string s = val.template as<std::string>();
            return boost::gregorian::from_simple_string(s);
        }

        static Json to_json(boost::gregorian::date val)
        {
            return Json(to_iso_extended_string(val));
        }
    };
}
```

```c++
    using jsoncons::json;
    using boost::gregorian::date;

int main()
{
    json deal = json::parse(R"(
    {
        "Maturity":"2014-10-14",
        "ObservationDates": ["2014-02-14","2014-02-21"]
    }
    )");

    deal["ObservationDates"].add(date(2014,2,28));    

    date maturity = deal["Maturity"].as<date>();
    std::cout << "Maturity: " << maturity << std::endl << std::endl;

    std::cout << "Observation dates: " << std::endl << std::endl;

    for (auto observation_date: deal["ObservationDates"].array_range())
    {
        std::cout << observation_date << std::endl;
    }
    std::cout << std::endl;
}
```
Output:
```
Maturity: 2014-Oct-14

Observation dates:

2014-Feb-14
2014-Feb-21
2014-Feb-28
``` 

### Example 4. Convert to/from `boost::numeric::ublas::matrix<T>`

```c++
#include <jsoncons/json.hpp>
#include <boost/numeric/ublas/matrix.hpp>

namespace jsoncons 
{
    template <class Json,class T>
    struct json_type_traits<Json,boost::numeric::ublas::matrix<T>>
    {
        static bool is(const Json& val) noexcept
        {
            if (!val.is_array())
            {
                return false;
            }
            if (val.size() > 0)
            {
                size_t n = val[0].size();
                for (const auto& a: val.array_range())
                {
                    if (!(a.is_array() && a.size() == n))
                    {
                        return false;
                    }
                    for (auto x: a.array_range())
                    {
                        if (!x.template is<T>())
                        {
                            return false;
                        }
                    }
                }
            }
            return true;
        }

        static boost::numeric::ublas::matrix<T> as(const Json& val)
        {
            if (val.is_array() && val.size() > 0)
            {
                size_t m = val.size();
                size_t n = 0;
                for (const auto& a : val.array_range())
                {
                    if (a.size() > n)
                    {
                        n = a.size();
                    }
                }

                boost::numeric::ublas::matrix<T> A(m,n,T());
                for (size_t i = 0; i < m; ++i)
                {
                    const auto& a = val[i];
                    for (size_t j = 0; j < a.size(); ++j)
                    {
                        A(i,j) = a[j].template as<T>();
                    }
                }
                return A;
            }
            else
            {
                boost::numeric::ublas::matrix<T> A;
                return A;
            }
        }

        static Json to_json(const boost::numeric::ublas::matrix<T>& val)
        {
            Json a = Json::template make_array<2>(val.size1(), val.size2(), T());
            for (size_t i = 0; i < val.size1(); ++i)
            {
                for (size_t j = 0; j < val.size1(); ++j)
                {
                    a[i][j] = val(i,j);
                }
            }
            return a;
        }
    };
}
```

```c++
    using jsoncons::json;
    using boost::gregorian::date;

int main()
{
    matrix<double> A(2, 2);
    A(0, 0) = 1.1;
    A(0, 1) = 2.1;
    A(1, 0) = 3.1;
    A(1, 1) = 4.1;

    json a = A;

    std::cout << "(1) " << std::boolalpha << a.is<matrix<double>>() << "\n\n";

    std::cout << "(2) " << std::boolalpha << a.is<matrix<int>>() << "\n\n";

    std::cout << "(3) \n\n" << pretty_print(a) << "\n\n";

    matrix<double> B = a.as<matrix<double>>();

    std::cout << "(4) \n\n";
    for (size_t i = 0; i < B.size1(); ++i)
    {
        for (size_t j = 0; j < B.size2(); ++j)
        {
            if (j > 0)
            {
                std::cout << ",";
            }
            std::cout << B(i, j);
        }
        std::cout << "\n";
    }
    std::cout << "\n\n";
}
```
Output:
```
(1) true

(2) false

(3)

[
    [1.1,2.1],
    [3.1,4.1]
]

(4)

1.1,2.1
3.1,4.1
``` 


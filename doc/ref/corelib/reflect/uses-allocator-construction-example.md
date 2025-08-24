### Uses-allocator construction example (requires 1.4.0)

This example illustrates user-allocator construction for types with `json_conv_traits` traits defined. In the
example, a user supplied stateful allocator is provided when invoking `jsoncons::try_decode_json`. This
allocator is used for all result allocations, including the allocated memory for `std::shared_ptr<Employee<Alloc>>`
and the string type member of `Employee<Alloc>`. A second allocator is provided for allocating memory for 
temporary storage. 

```cpp
#include <jsoncons/json.hpp>
#include <common/mock_stateful_allocator.hpp>
#include <iostream>
#include <scoped_allocator>
#include <cassert>

// This example requires jsoncons 1.4.0

namespace ns {

template <typename Alloc>
class Employee
{
    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

    string_type firstName_;
    string_type lastName_;
public:
    using allocator_type = Alloc;

    Employee(const Alloc& alloc)
        : firstName_(alloc), lastName_(alloc)
    {
    }
    Employee(const string_type& firstName, const string_type& lastName, const Alloc& alloc)
        : firstName_(firstName, alloc), lastName_(lastName, alloc)
    {
    }
    Employee(const Employee&) = default;
    Employee(Employee&&) = default;
    Employee& operator=(const Employee&) = default;
    Employee& operator=(Employee&&) = default;

    Employee(const Employee& other, const Alloc& alloc)
        : firstName_(other.firstName_, alloc), lastName_(other.lastName_, alloc)
    {
    }

    Employee(const Employee&& other, const Alloc& alloc)
        : firstName_(std::move(other.firstName_), alloc), lastName_(std::move(other.lastName_), alloc)
    {
    }

    virtual ~Employee() noexcept = default;

    virtual double calculatePay() const = 0;

    const string_type& firstName() const { return firstName_; }
    const string_type& lastName() const { return lastName_; }
};

template <typename Alloc>
class HourlyEmployee : public Employee<Alloc>
{
    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

    double wage_;
    unsigned hours_;
public:
    using allocator_type = typename Employee<Alloc>::allocator_type;

    HourlyEmployee(const string_type& firstName, const string_type& lastName,
        double wage, unsigned hours, const Alloc& alloc)
        : Employee<Alloc>(firstName, lastName, alloc),
        wage_(wage), hours_(hours)
    {
    }
    HourlyEmployee(const HourlyEmployee&) = default;
    HourlyEmployee(HourlyEmployee&&) = default;
    HourlyEmployee& operator=(const HourlyEmployee&) = default;
    HourlyEmployee& operator=(HourlyEmployee&&) = default;

    HourlyEmployee(const HourlyEmployee& other, const Alloc& alloc)
        : Employee<Alloc>(other, alloc),
        wage_(other, wage_), hours_(other.hours_)
    {
    }

    HourlyEmployee(HourlyEmployee&& other, const Alloc& alloc)
        : Employee<Alloc>(std::move(other), alloc),
        wage_(other.wage_), hours_(other.hours_)
    {
    }

    double wage() const { return wage_; }

    unsigned hours() const { return hours_; }

    double calculatePay() const override
    {
        return wage_ * hours_;
    }
};

template <typename Alloc>
class CommissionedEmployee : public Employee<Alloc>
{
    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

    double baseSalary_;
    double commission_;
    unsigned sales_;
public:
    using allocator_type = typename Employee<Alloc>::allocator_type;

    CommissionedEmployee(const string_type& firstName, const string_type& lastName,
        double baseSalary, double commission, unsigned sales, const Alloc& alloc)
        : Employee<Alloc>(firstName, lastName, alloc),
        baseSalary_(baseSalary), commission_(commission), sales_(sales)
    {
    }
    CommissionedEmployee(const CommissionedEmployee&) = default;
    CommissionedEmployee(CommissionedEmployee&&) = default;

    CommissionedEmployee(const CommissionedEmployee& other, const Alloc& alloc)
        : Employee<Alloc>(other, alloc),
        baseSalary_(other, baseSalary_), commission_(other.commission_), sales_(other.sales_)
    {
    }

    CommissionedEmployee(CommissionedEmployee&& other, const Alloc& alloc)
        : Employee<Alloc>(std::move(other), alloc),
        baseSalary_(other.baseSalary_), commission_(other.commission_), sales_(other.sales_)
    {
    }

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
        return baseSalary_ + commission_ * sales_;
    }
};

} // namespace ns

template <typename T>
using cust_allocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

JSONCONS_TPL_ALL_CTOR_GETTER_TRAITS(1, ns::HourlyEmployee, firstName, lastName, wage, hours)
JSONCONS_TPL_ALL_CTOR_GETTER_TRAITS(1, ns::CommissionedEmployee, firstName, lastName, baseSalary, commission, sales)
JSONCONS_POLYMORPHIC_TRAITS(ns::Employee<cust_allocator<char>>, ns::HourlyEmployee<cust_allocator<char>>, ns::CommissionedEmployee<cust_allocator<char>>)

int main()
{
    std::string input = R"(
[
    {
        "firstName": "John",
        "hours": 1000,
        "lastName": "Smith",
        "wage": 40.0
    },
    {
        "baseSalary": 30000.0,
        "commission": 0.25,
        "firstName": "Jane",
        "lastName": "Doe",
        "sales": 1000
    }
]
    )";

    using element_type = std::shared_ptr<ns::Employee<cust_allocator<char>>>;
    using value_type = std::vector<element_type, cust_allocator<element_type>>;

    cust_allocator<char> alloc(1);
    cust_allocator<char> alloc_temp(2);
    auto aset = jsoncons::make_alloc_set(alloc, alloc_temp);
    auto r = jsoncons::try_decode_json<value_type>(aset, input);
    if (!r)
    {
        std::cout << r.error().message() << "\n";
    }

    value_type& v{*r};

    assert(2 == v.size());
    assert(v[0]->firstName().c_str() == "John");
    assert(v[0]->lastName().c_str() == "Smith");
    assert(v[0]->calculatePay() == 40000);
    assert(v[1]->firstName().c_str() == "Jane");
    assert(v[1]->lastName().c_str() == "Doe");
    assert(v[1]->calculatePay() == 30250);
}
```

### See also

[basic_json](../basic_json.md)
[allocator_set](../allocator_set)
[decode_json, try_decode_json](../decode_json.md)  
[encode_json, encode_json_pretty, try_encode_json, try_encode_json_pretty](../encode_json.md)  

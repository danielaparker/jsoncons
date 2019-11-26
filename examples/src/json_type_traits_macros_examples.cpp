// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <iomanip>
#include <jsoncons/json.hpp>

namespace json_type_traits_macros_examples_ns {

class Foo
{
public:
    virtual ~Foo() = default;
};

class Bar : public Foo
{
    static const bool bar = true;
    JSONCONS_TYPE_TRAITS_FRIEND
};

class Baz : public Foo 
{
    static const bool baz = true;
    JSONCONS_TYPE_TRAITS_FRIEND
};

enum class BookCategory {fiction,biography};

std::ostream& operator<<(std::ostream& os, BookCategory c)
{
    switch(c)
    {
        case BookCategory::fiction: os << "fiction"; break;
        case BookCategory::biography: os << "biography"; break;
    }
    return os;
}

// #1 Class with public member data and default constructor   
struct Book1
{
    BookCategory category;
    std::string author;
    std::string title;
    double price;
};

// #2 Class with private member data and default constructor   
class Book2
{
    BookCategory category;
    std::string author;
    std::string title;
    double price;
    Book2() = default;

    JSONCONS_TYPE_TRAITS_FRIEND
public:
    BookCategory get_category() const {return category;}

    const std::string& get_author() const {return author;}

    const std::string& get_title() const{return title;}

    double get_price() const{return price;}
};

// #3 Class with getters and initializing constructor
class Book3
{
    BookCategory category_;
    std::string author_;
    std::string title_;
    double price_;
public:
    Book3(BookCategory category,
          const std::string& author,
          const std::string& title,
          double price)
        : category_(category), author_(author), title_(title), price_(price)
    {
    }

    Book3(const Book3&) = default;
    Book3(Book3&&) = default;
    Book3& operator=(const Book3&) = default;
    Book3& operator=(Book3&&) = default;

    BookCategory category() const {return category_;}

    const std::string& author() const{return author_;}

    const std::string& title() const{return title_;}

    double price() const{return price_;}
};

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
}

namespace ns = json_type_traits_macros_examples_ns;

// Declare the traits at global scope
JSONCONS_ENUM_TRAITS_DECL(ns::BookCategory,fiction,biography)

JSONCONS_ALL_MEMBER_TRAITS_DECL(ns::Book1,category,author,title,price)
JSONCONS_ALL_MEMBER_TRAITS_DECL(ns::Book2,category,author,title,price)
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ns::Book3,category,author,title,price)

JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ns::HourlyEmployee, firstName, lastName, wage, hours)
JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ns::CommissionedEmployee, firstName, lastName, baseSalary, commission, sales)
JSONCONS_POLYMORPHIC_TRAITS_DECL(ns::Employee, ns::HourlyEmployee, ns::CommissionedEmployee)

JSONCONS_N_MEMBER_TRAITS_DECL(ns::Bar,1,bar)
JSONCONS_N_MEMBER_TRAITS_DECL(ns::Baz,1,baz)
JSONCONS_POLYMORPHIC_TRAITS_DECL(ns::Foo, ns::Bar, ns::Baz)

using namespace jsoncons;

static void json_type_traits_book_examples()
{
    const std::string input = R"(
    [
        {
            "category" : "fiction",
            "author" : "Haruki Murakami",
            "title" : "Kafka on the Shore",
            "price" : 25.17
        },
        {
            "category" : "biography",
            "author" : "Robert A. Caro",
            "title" : "The Path to Power: The Years of Lyndon Johnson I",
            "price" : 16.99
        }
    ]
    )";

    std::cout << "(1)\n\n";
    auto books1 = decode_json<std::vector<ns::Book1>>(input);
    for (const auto& item : books1)
    {
        std::cout << item.category << ", " 
                  << item.author << ", " 
                  << item.title << ", " 
                  << item.price << "\n";
    }
    std::cout << "\n";
    encode_json(books1, std::cout, indenting::indent);
    std::cout << "\n\n";

    std::cout << "(2)\n\n";
    auto books2 = decode_json<std::vector<ns::Book2>>(input);
    for (const auto& item : books2)
    {
        std::cout << item.get_category() << ", " 
                  << item.get_author() << ", " 
                  << item.get_title() << ", " 
                  << item.get_price() << "\n";
    }
    std::cout << "\n";
    encode_json(books2, std::cout, indenting::indent);
    std::cout << "\n\n";

    std::cout << "(3)\n\n";
    auto books3 = decode_json<std::vector<ns::Book3>>(input);
    for (const auto& item : books3)
    {
        std::cout << item.category() << ", " 
                  << item.author() << ", " 
                  << item.title() << ", " 
                  << item.price() << "\n";
    }
    std::cout << "\n";
    encode_json(books3, std::cout, indenting::indent);
    std::cout << "\n\n";
}

void employee_polymorphic_example()
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

    auto v = decode_json<std::vector<std::shared_ptr<ns::Employee>>>(input);

    std::cout << "(1)\n";
    for (auto p : v)
    {
        std::cout << p->firstName() << " " << p->lastName() << ", " << p->calculatePay() << "\n";
    }

    std::cout << "\n(2)\n";
    encode_json(v, std::cout, indenting::indent);

    std::cout << "\n\n(3)\n";
    json j(v);
    std::cout << pretty_print(j) << "\n\n";
}

void foo_bar_baz_example()
{
    std::vector<std::unique_ptr<ns::Foo>> u;
    u.emplace_back(new ns::Bar());
    u.emplace_back(new ns::Baz());

    std::string buffer;
    encode_json(u, buffer);
    std::cout << "(1)\n" << buffer << "\n\n";

    auto v = decode_json<std::vector<std::unique_ptr<ns::Foo>>>(buffer);

    std::cout << "(2)\n";
    for (const auto& ptr : v)
    {
        if (dynamic_cast<ns::Bar*>(ptr.get()))
        {
            std::cout << "A bar\n";
        }
        else if (dynamic_cast<ns::Baz*>(ptr.get()))
        {
            std::cout << "A baz\n";
        } 
    }
}

void json_type_traits_macros_examples()
{
    std::cout << "\njson_type_traits macro examples\n\n";

    std::cout << std::setprecision(6);

    json_type_traits_book_examples();
    employee_polymorphic_example();
    foo_bar_baz_example();

    std::cout << std::endl;
}


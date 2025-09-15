// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license 

#include <iostream>
#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/decode_json.hpp>
#include <jsoncons/encode_json.hpp>
#include <catch/catch.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstdint>

using namespace jsoncons;

#if defined(JSONCONS_HAS_STATEFUL_ALLOCATOR) && JSONCONS_HAS_STATEFUL_ALLOCATOR == 1

#include <scoped_allocator>
#include <common/mock_stateful_allocator.hpp>

namespace {
namespace ns {

template <typename Alloc>
struct book_all_m
{
    using allocator_type = Alloc;

    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

    string_type author;
    string_type title;
    double price;

    book_all_m(const Alloc& alloc)
        : author(alloc), title(alloc)
    {
    }

    book_all_m(const book_all_m& other) = default; 

    book_all_m(const book_all_m& other, const Alloc& alloc)
        : author(other.author, alloc), title(other.title, alloc), price(other.price)
    {
    }

    book_all_m(book_all_m&& other) = default;

    book_all_m(book_all_m&& other, const Alloc& alloc)
        : author(std::move(other.author), alloc), title(std::move(other.title), alloc), price(other.price)
    {
    }
};

template <typename Alloc>
struct book_all_m_name
{
    using allocator_type = Alloc;

    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

    string_type author;
    string_type title;
    double price;

    book_all_m_name(const Alloc& alloc)
        : author(alloc), title(alloc)
    {
    }

    book_all_m_name(const book_all_m_name& other) = default; 

    book_all_m_name(const book_all_m_name& other, const Alloc& alloc)
        : author(other.author, alloc), title(other.title, alloc), price(other.price)
    {
    }

    book_all_m_name(book_all_m_name&& other) = default;

    book_all_m_name(book_all_m_name&& other, const Alloc& alloc)
        : author(std::move(other.author), alloc), title(std::move(other.title), alloc), price(other.price)
    {
    }
};

template <typename Alloc>
struct book_3_m
{
    using allocator_type = Alloc;

    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

    string_type author;
    string_type title;
    double price;
    jsoncons::optional<string_type> isbn;

    book_3_m(const Alloc& alloc)
        : author(alloc), title(alloc), isbn()
    {
    }

    book_3_m(const book_3_m& other, const Alloc& alloc)
        : author(other.author, alloc), title(other.title, alloc), price(other.price), 
        isbn(other.isbn ? jsoncons::optional<string_type>(jsoncons::in_place, *other.isbn, alloc) : other.isbn)
    {
    }

    book_3_m(book_3_m&& other, const Alloc& alloc)
        : author(std::move(other.author), alloc), title(std::move(other.title), alloc), price(other.price), 
        isbn(other.isbn ? jsoncons::optional<string_type>(jsoncons::in_place, std::move(*other.isbn), alloc) : std::move(other.isbn))
    {
    }
};

template <typename Alloc>
class book_all_gs
{
public:
    using allocator_type = Alloc;

    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;
private:
    string_type author_;
    string_type title_;
    double price_{0};
public:
    book_all_gs(const Alloc& alloc)
        : author_(alloc), title_(alloc)
    {
    }

    book_all_gs(const book_all_gs&) = default;

    book_all_gs(const book_all_gs& other, const Alloc& alloc)
        : author_(other.author_, alloc), 
          title_(other.title_, alloc), 
          price_(other.price_)
    {
    }

    book_all_gs(book_all_gs&&) = default;

    book_all_gs(book_all_gs&& other, const Alloc& alloc)
        : author_(std::move(other.author_), alloc), 
          title_(std::move(other.title_), alloc), 
          price_(other.price_)
    {
    }

    book_all_gs& operator=(const book_all_gs&) = default;
    book_all_gs& operator=(book_all_gs&&) = default;

    const string_type& getAuthor() const
    {
        return author_;
    }

    void setAuthor(const string_type& value)
    {
        author_ = value;
    }

    const string_type& getTitle() const
    {
        return title_;
    }

    void setTitle(const string_type& value)
    {
        title_ = value;
    }

    double getPrice() const
    {
        return price_;
    }

    void setPrice(double value)
    {
        price_ = value;
    }
};

template <typename Alloc>
class book_all_cg
{
public:
    using allocator_type = Alloc;

    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;
private:
    string_type author_;
    string_type title_;
    double price_{0};
public:
    book_all_cg(const string_type& author,
          const string_type& title,
          double price,
          const allocator_type& alloc)
        : author_(author, alloc), title_(title, alloc), price_(price)
    {
    }

    book_all_cg(const book_all_cg&) = default;

    book_all_cg(const book_all_cg& other, const Alloc& alloc)
        : author_(other.author_, alloc), 
          title_(other.title_, alloc), 
          price_(other.price_)
    {
    }

    book_all_cg(book_all_cg&&) = default;

    book_all_cg(book_all_cg&& other, const Alloc& alloc)
        : author_(std::move(other.author_), alloc), 
          title_(std::move(other.title_), alloc), 
          price_(other.price_)
    {
    }

    book_all_cg& operator=(const book_all_cg&) = default;
    book_all_cg& operator=(book_all_cg&&) = default;

    const string_type& author() const
    {
        return author_;
    }

    const string_type& title() const
    {
        return title_;
    }

    double price() const
    {
        return price_;
    }
};

template <typename Alloc>
class book_all_cg_name
{
public:
    using allocator_type = Alloc;

    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;
private:
    string_type author_;
    string_type title_;
    double price_{0};
public:
    book_all_cg_name(const string_type& author,
          const string_type& title,
          double price,
          const allocator_type& alloc)
        : author_(author, alloc), title_(title, alloc), price_(price)
    {
    }

    book_all_cg_name(const book_all_cg_name&) = default;

    book_all_cg_name(const book_all_cg_name& other, const Alloc& alloc)
        : author_(other.author_, alloc), 
          title_(other.title_, alloc), 
          price_(other.price_)
    {
    }

    book_all_cg_name(book_all_cg_name&&) = default;

    book_all_cg_name(book_all_cg_name&& other, const Alloc& alloc)
        : author_(std::move(other.author_), alloc), 
          title_(std::move(other.title_), alloc), 
          price_(other.price_)
    {
    }

    book_all_cg_name& operator=(const book_all_cg_name&) = default;
    book_all_cg_name& operator=(book_all_cg_name&&) = default;

    const string_type& author() const
    {
        return author_;
    }

    const string_type& title() const
    {
        return title_;
    }

    double price() const
    {
        return price_;
    }
};

template <typename Alloc>
class Employee
{
public:
    using allocator_type = Alloc;
    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

private:
    string_type firstName_;
    string_type lastName_;
public:  
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
        : firstName_(other.firstName_, alloc), lastName_(other.lastName_,alloc)
    {
    }

    Employee(const Employee&& other, const Alloc& alloc)
        : firstName_(std::move(other.firstName_), alloc), lastName_(std::move(other.lastName_), alloc)
    {
    }

    virtual ~Employee() noexcept = default;

    virtual double calculatePay() const = 0;

    const string_type& firstName() const {return firstName_;}
    const string_type& lastName() const {return lastName_;}
};

template <typename Alloc>
class HourlyEmployee : public Employee<Alloc>
{
public:
    using allocator_type = typename Employee<Alloc>::allocator_type;
    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;
private:
    double wage_;
    unsigned hours_;
public:
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
          wage_(other,wage_), hours_(other.hours_)
    {
    }

    HourlyEmployee(HourlyEmployee&& other, const Alloc& alloc)
        : Employee<Alloc>(std::move(other), alloc), 
          wage_(other.wage_), hours_(other.hours_)
    {
    }

    double wage() const {return wage_;}

    unsigned hours() const {return hours_;}

    double calculatePay() const override
    {
        return wage_*hours_;
    }
};

template <typename Alloc>
class CommissionedEmployee : public Employee<Alloc>
{
public:
    using allocator_type = typename Employee<Alloc>::allocator_type;
    using char_allocator_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<char>;
    using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;
private:
    double baseSalary_;
    double commission_;
    unsigned sales_;
public:
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
          baseSalary_(other,baseSalary_), commission_(other.commission_), sales_(other.sales_)
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
        return baseSalary_ + commission_*sales_;
    }
};

} // namespace ns
} // namespace 

template <typename T>
using cust_allocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

using cust_json = jsoncons::basic_json<char, jsoncons::sorted_policy, cust_allocator<char>>;

JSONCONS_TPL_ALL_MEMBER_TRAITS(1,ns::book_all_m,author,title,price)
JSONCONS_TPL_N_MEMBER_TRAITS(1,ns::book_3_m,3,author,title,price,isbn)

JSONCONS_TPL_ALL_MEMBER_NAME_TRAITS(1, ns::book_all_m_name,(author,"Author"),(title,"Title"),(price,"Price"))

JSONCONS_TPL_ALL_GETTER_SETTER_TRAITS(1, ns::book_all_gs, get, set, Author, Title, Price)

JSONCONS_TPL_ALL_CTOR_GETTER_TRAITS(1, ns::book_all_cg, author, title, price)
JSONCONS_TPL_ALL_CTOR_GETTER_NAME_TRAITS(1, ns::book_all_cg_name, (author,"Author"),(title,"Title"),(price,"Price"))

JSONCONS_TPL_ALL_CTOR_GETTER_TRAITS(1,ns::HourlyEmployee, firstName, lastName, wage, hours)
JSONCONS_TPL_ALL_CTOR_GETTER_TRAITS(1,ns::CommissionedEmployee, firstName, lastName, baseSalary, commission, sales)
JSONCONS_POLYMORPHIC_TRAITS(ns::Employee<cust_allocator<char>>, ns::HourlyEmployee<cust_allocator<char>>, ns::CommissionedEmployee<cust_allocator<char>>)

TEST_CASE("JSONCONS_ALL_MEMBER_TRAITS using allocator tests")
{

    SECTION("book")
    {
        using book_type = ns::book_all_m<cust_allocator<char>>;

        std::string input = R"(
{
    "author" : "Haruki Murakami",  
    "title" : "Kafka on the Shore",
    "price" : 25.17
}
        )";

        cust_allocator<char> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = try_decode_json<book_type>(aset, input);

        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }

    SECTION("vector of book")
    {
        using book_type = ns::book_all_m<cust_allocator<char>>;
        using books_type = std::vector<book_type,cust_allocator<book_type>>;

        std::string input = R"(
[
    {
        "author" : "Haruki Murakami",
        "title" : "Kafka on the Shore",
        "price" : 25.17
    },
    {
        "author" : "Charles Bukowski",
        "title" : "Pulp",
        "price" : 12  
    },
    {
        "author" : "Ivan Passer",
        "title" : "Cutter's Way",
        "price" : 15.0
    }
]
        )";

        cust_allocator<book_type> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = try_decode_json<books_type>(aset, input);

        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }
}

TEST_CASE("JSONCONS_ALL_MEMBER_NAME_TRAITS using allocator tests")
{

    SECTION("book")
    {
        using book_type = ns::book_all_m_name<cust_allocator<char>>;

        std::string input = R"(
{
    "Author" : "Haruki Murakami",  
    "Title" : "Kafka on the Shore",
    "Price" : 25.17
}
        )";

        cust_allocator<char> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = try_decode_json<book_type>(aset, input);
        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }

    SECTION("vector of book")
    {
        using book_type = ns::book_all_m_name<cust_allocator<char>>;
        using books_type = std::vector<book_type,cust_allocator<book_type>>;

        std::string input = R"(
[
    {
        "Author" : "Haruki Murakami",
        "Title" : "Kafka on the Shore",
        "Price" : 25.17
    },
    {
        "Author" : "Charles Bukowski",
        "Title" : "Pulp",
        "Price" : 12  
    },
    {
        "Author" : "Ivan Passer",
        "Title" : "Cutter's Way",
        "Price" : 15.0
    }
]
        )";

        cust_allocator<book_type> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = try_decode_json<books_type>(aset, input);

        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }
}

TEST_CASE("JSONCONS_N_MEMBER_TRAITS using allocator tests")
{

    SECTION("book")
    {
        using book_type = ns::book_3_m<cust_allocator<char>>;

        std::string input = R"(
{
    "author" : "Haruki Murakami",  
    "title" : "Kafka on the Shore",
    "price" : 25.17
}
        )";

        cust_allocator<char> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = try_decode_json<book_type>(aset, input);
        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }

    SECTION("vector of book")
    {
        using book_type = ns::book_3_m<cust_allocator<char>>;
        using books_type = std::vector<book_type,cust_allocator<book_type>>;

        std::string input = R"(
[
    {
        "author" : "Haruki Murakami",
        "title" : "Kafka on the Shore",
        "price" : 25.17
    },
    {
        "author" : "Charles Bukowski",  
        "title" : "Pulp",
        "price" : 12.0,
        "isbn" : "1852272007"
    },
    {
        "author" : "Ivan Passer",
        "title" : "Cutter's Way",
        "price" : 15.0
    }
]
        )";

        cust_allocator<book_type> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = try_decode_json<books_type>(aset, input);

        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }
}

TEST_CASE("JSONCONS_ALL_GETTER_SETTER_TRAITS using allocator tests")
{

    SECTION("book")
    {
        using book_type = ns::book_all_gs<cust_allocator<char>>;

        std::string input = R"(
{
    "Author" : "Haruki Murakami",  
    "Title" : "Kafka on the Shore",
    "Price" : 25.17
}
        )";

        cust_allocator<char> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = try_decode_json<book_type>(aset, input);
        if (!r)
        {
            std::cout << "Err: " << r.error().message() << "\n";
        }

        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }

    SECTION("vector of book")
    {
        using book_type = ns::book_all_gs<cust_allocator<char>>;
        using books_type = std::vector<book_type,cust_allocator<book_type>>;

        std::string input = R"(
[
    {
        "Author" : "Haruki Murakami",
        "Title" : "Kafka on the Shore",
        "Price" : 25.17
    },
    {
        "Author" : "Charles Bukowski",
        "Title" : "Pulp",
        "Price" : 12  
    },
    {
        "Author" : "Ivan Passer",
        "Title" : "Cutter's Way",
        "Price" : 15.0
    }
]
        )";

        cust_allocator<book_type> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = try_decode_json<books_type>(aset, input);

        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }
}

TEST_CASE("JSONCONS_ALL_CTOR_GETTER_TRAITS using allocator tests")
{

    SECTION("book")
    {
        using book_type = ns::book_all_cg<cust_allocator<char>>;

        std::string input = R"(
{
    "author" : "Haruki Murakami",  
    "title" : "Kafka on the Shore",
    "price" : 25.17
}
        )";

        cust_allocator<char> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = try_decode_json<book_type>(aset, input);
        if (!r)
        {
            std::cout << "Err: " << r.error().message() << "\n";
        }

        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }

    SECTION("vector of book")
    {
        using book_type = ns::book_all_cg<cust_allocator<char>>;
        using books_type = std::vector<book_type,cust_allocator<book_type>>;

        std::string input = R"(
[
    {
        "author" : "Haruki Murakami",
        "title" : "Kafka on the Shore",
        "price" : 25.17
    },
    {
        "author" : "Charles Bukowski",
        "title" : "Pulp",
        "price" : 12  
    },
    {
        "author" : "Ivan Passer",
        "title" : "Cutter's Way",
        "price" : 15.0
    }
]
        )";

        cust_allocator<book_type> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = try_decode_json<books_type>(aset, input);

        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }
}

TEST_CASE("JSONCONS_ALL_CTOR_GETTER_NAME_TRAITS using allocator tests")
{

    SECTION("book")
    {
        using book_type = ns::book_all_cg_name<cust_allocator<char>>;

        std::string input = R"(
{
    "Author" : "Haruki Murakami",  
    "Title" : "Kafka on the Shore",
    "Price" : 25.17
}
        )";

        cust_allocator<char> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = try_decode_json<book_type>(aset, input);
        if (!r)
        {
            std::cout << "Err: " << r.error().message() << "\n";
        }

        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }

    SECTION("vector of book")
    {
        using book_type = ns::book_all_cg_name<cust_allocator<char>>;
        using books_type = std::vector<book_type,cust_allocator<book_type>>;

        std::string input = R"(
[
    {
        "Author" : "Haruki Murakami",
        "Title" : "Kafka on the Shore",
        "Price" : 25.17
    },
    {
        "Author" : "Charles Bukowski",
        "Title" : "Pulp",
        "Price" : 12  
    },
    {
        "Author" : "Ivan Passer",
        "Title" : "Cutter's Way",
        "Price" : 15.0
    }
]
        )";

        cust_allocator<book_type> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = try_decode_json<books_type>(aset, input);

        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }
}

template <typename Alloc>
struct allocator_delete  : public Alloc
{
    using allocator_type = Alloc;
    using pointer = typename std::allocator_traits<Alloc>::pointer;

    allocator_delete(const Alloc& alloc) noexcept
        : Alloc(alloc)
    {
    }

    allocator_delete(const allocator_delete&) noexcept = default;

    void operator()(pointer ptr) noexcept
    {
        using T = std::remove_reference_t<decltype(*ptr)>;
        using alloc_type = typename T::allocator_type;

        std::cout << "type name: " << typeid(ptr).name() << "\n";
        std::cout << "hourly type name: " << typeid(ns::HourlyEmployee<alloc_type>).name() << "\n";

        std::allocator_traits<Alloc>::destroy(*this, ptr);

        if (auto hourly_ptr = dynamic_cast<ns::HourlyEmployee<alloc_type>*>(ptr))
        {
            std::cout << "hourly\n";
            using hourly_alloc_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<ns::HourlyEmployee<alloc_type>>;
            hourly_alloc_type alloc(*this);
            alloc.deallocate(hourly_ptr, 1);
        }
        else if (auto commissioned_ptr = dynamic_cast<ns::CommissionedEmployee<alloc_type>*>(ptr))
        {
            std::cout << "commissioned\n";
            using commissioned_alloc_type = typename std::allocator_traits<Alloc>:: template rebind_alloc<ns::CommissionedEmployee<alloc_type>>;
            commissioned_alloc_type alloc(*this);
            alloc.deallocate(commissioned_ptr, 1);
        }
        else
        {
            std::cout << "not deallocated\n";
        }
    }
};

TEST_CASE("JSONCONS_POLYMORPHIC_TRAITS using allocator tests")
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

    cust_allocator<char> alloc(1);
    using string_type = ns::Employee<cust_allocator<char>>::string_type;
    string_type firstName0{"John", alloc};
    string_type lastName0{"Smith", alloc};
    const double pay0 = 40000;
    string_type firstName1{"Jane", alloc};
    string_type lastName1{"Doe", alloc};
    const double pay1 = 30250;

    SECTION("decode vector of shared_ptr")
    {
        using element_type = std::shared_ptr<ns::Employee<cust_allocator<char>>>;
        using value_type = std::vector<element_type,cust_allocator<element_type>>;

        auto aset = make_alloc_set(alloc);
        auto r = jsoncons::try_decode_json<value_type>(aset, input);
        REQUIRE(r);
        value_type& v{*r};

        REQUIRE(2 == v.size());
        CHECK(v[0]->firstName() == firstName0);
        CHECK(v[0]->lastName() == lastName0);
        CHECK(v[0]->calculatePay() == pay0);
        CHECK(v[1]->firstName() == firstName1);
        CHECK(v[1]->lastName() == lastName1);
        CHECK(v[1]->calculatePay() == pay1); 
    }
/*
    SECTION("decode vector of unique_ptr")
    {
        using employee_type = ns::Employee <cust_allocator<char>>;
        using element_type = std::unique_ptr<employee_type,allocator_delete<cust_allocator<employee_type>>>;
        using value_type = std::vector<element_type,cust_allocator<element_type>>;

        auto aset = make_alloc_set(alloc);
        auto r = jsoncons::try_decode_json<value_type>(aset, input);
        REQUIRE(r);
        value_type& v{*r};

        REQUIRE(2 == v.size());
        CHECK(v[0]->firstName() == firstName0);
        CHECK(v[0]->lastName() == lastName0);
        CHECK(v[0]->calculatePay() == pay0);
        CHECK(v[1]->firstName() == firstName1);
        CHECK(v[1]->lastName() == lastName1);
        CHECK(v[1]->calculatePay() == pay1);
    }

    SECTION("decode vector of unique_ptr test")
    {

        auto v = jsoncons::decode_json<std::vector<std::unique_ptr<ns::Employee<std::allocator<char>>>>>(input);
        REQUIRE(2 == v.size());
        CHECK((v[0]->firstName() == firstName0));
        CHECK((v[0]->lastName() == lastName0));
        CHECK((v[0]->calculatePay() == pay0));
        CHECK((v[1]->firstName() == firstName1));
        CHECK((v[1]->lastName() == lastName1));
        CHECK((v[1]->calculatePay() == pay1));
    }

    SECTION("encode vector of shared_ptr test")
    {
        std::vector<std::shared_ptr<ns::Employee<std::allocator<char>>>> v;

        v.push_back(std::make_shared<ns::HourlyEmployee<std::allocator<char>>>("John", "Smith", 40.0, 1000));
        v.push_back(std::make_shared<ns::CommissionedEmployee<std::allocator<char>>>("Jane", "Doe", 30000, 0.25, 1000));

        jsoncons::json j(v);

        json expected = json::parse(input);
        CHECK(expected == j);
    }
    SECTION("encode vector of unique_ptr test")
    {
        std::vector<std::unique_ptr<ns::Employee<std::allocator<char>>>> v;

        v.emplace_back(new ns::HourlyEmployee<std::allocator<char>>("John", "Smith", 40.0, 1000));
        v.emplace_back(new ns::CommissionedEmployee<std::allocator<char>>("Jane", "Doe", 30000, 0.25, 1000));

        jsoncons::json j(v);

        json expected = json::parse(input);
        CHECK(expected == j);
    }
    */
}

#endif

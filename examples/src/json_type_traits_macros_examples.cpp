// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <iomanip>
#include <jsoncons/json.hpp>

namespace json_type_traits_macros_examples_ns {

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

    JSONCONS_TYPE_TRAITS_FRIEND;
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
}

namespace ns = json_type_traits_macros_examples_ns;

// Declare the traits at global scope
JSONCONS_ENUM_TRAITS_DECL(ns::BookCategory,fiction,biography)

JSONCONS_STRICT_MEMBER_TRAITS_DECL(ns::Book1,category,author,title,price)
JSONCONS_STRICT_MEMBER_TRAITS_DECL(ns::Book2,category,author,title,price)
JSONCONS_GETTER_CTOR_TRAITS_DECL(ns::Book3,category,author,title,price)

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

void json_type_traits_macros_examples()
{
    std::cout << "\njson_type_traits macro examples\n\n";

    std::cout << std::setprecision(6);

    json_type_traits_book_examples();

    std::cout << std::endl;
}

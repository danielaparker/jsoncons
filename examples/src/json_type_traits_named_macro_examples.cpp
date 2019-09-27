// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <iomanip>
#include <jsoncons/json.hpp>

namespace json_type_traits_macro_examples_ns {

// #1 Class with public member data and default constructor   
struct Book1
{
    std::string author;
    std::string title;
    double price;
    std::string pub_date;
};

// #2 Class with private member data and default constructor   
class Book2
{
    std::string author_;
    std::string title_;
    double price_;
    std::string pub_date_;
    Book2() = default;

    JSONCONS_TYPE_TRAITS_FRIEND;
public:
    const std::string& author() const {return author_;}

    const std::string& title() const{return title_;}

    const std::string& pub_date() const{return pub_date_;}

    double price() const{return price_;}
};

// #3 Class with getters and initializing constructor
class Book3
{
    std::string author_;
    std::string title_;
    double price_;
    std::string pub_date_;
public:
    Book3(const std::string& author,
          const std::string& title,
          double price,
          const std::string& pub_date)
        : author_(author), title_(title), price_(price), pub_date_(pub_date)
    {
    }

    Book3(const Book3&) = default;
    Book3(Book3&&) = default;
    Book3& operator=(const Book3&) = default;
    Book3& operator=(Book3&&) = default;

    const std::string& author() const{return author_;}

    const std::string& title() const{return title_;}

    const std::string& pub_date() const{return pub_date_;}

    double price() const{return price_;}
};

// #4 Class with getters, setters and default constructor
class Book4
{
    std::string author_;
    std::string title_;
    double price_;
    std::string pub_date_;
public:
    Book4() = default;
    Book4(const Book4&) = default;
    Book4(Book4&&) = default;
    Book4& operator=(const Book4&) = default;
    Book4& operator=(Book4&&) = default;

    const std::string& getAuthor() const {return author_;}
    void setAuthor(const std::string& value) {author_ = value;}

    const std::string& getTitle() const {return title_;}
    void setTitle(const std::string& value) {title_ = value;}

    double getPrice() const {return price_;}
    void setPrice(double value) {price_ = value;}

    const std::string& getPubDate() const {return pub_date_;}
    void setPubDate(const std::string& value) {pub_date_ = value;}
};
}

namespace ns = json_type_traits_macro_examples_ns;

// Declare the traits at global scope
JSONCONS_STRICT_MEMBER_TRAITS_NAMED_DECL(ns::Book1,(author,"Author"),(title,"Title"),
                                                   (price,"Price"),(pub_date,"Publication Date"))
JSONCONS_STRICT_MEMBER_TRAITS_NAMED_DECL(ns::Book2,(author_,"Author"),(title_,"Title"),
                                                   (price_,"Price"),(pub_date_,"Publication Date"))
JSONCONS_GETTER_CTOR_TRAITS_NAMED_DECL(ns::Book3,(author,"Author"),(title,"Title"),
                                                 (price,"Price"),(pub_date,"Publication Date"))
JSONCONS_STRICT_GETTER_SETTER_TRAITS_NAMED_DECL(ns::Book4,(getAuthor,setAuthor,"Author"),(getTitle,setTitle,"Title"),
                                                          (getPrice,setPrice,"Price"),(getPubDate,setPubDate,"Publication Date"))

using namespace jsoncons;

void json_type_traits_book_examples()
{
    const std::string input = R"(
    [
        {
            "Author" : "Haruki Murakami",
            "Title" : "Kafka on the Shore",
            "Price" : 25.17,
            "Publication Date" : "2006-01-03"
        },
        {
            "Author" : "Charles Bukowski",
            "Title" : "Pulp",
            "Price" : 22.48,
            "Publication Date" : "2002-05-31"
        }
    ]
    )";

    std::cout << "(1)\n\n";
    auto books1 = decode_json<std::vector<ns::Book1>>(input);
    for (const auto& item : books1)
    {
        std::cout << item.author << ", " 
                  << item.title << ", " 
                  << item.price << ", " 
                  << item.pub_date << "\n";
    }
    std::cout << "\n";
    encode_json(books1, std::cout, indenting::indent);
    std::cout << "\n\n";

    std::cout << "(2)\n\n";
    auto books2 = decode_json<std::vector<ns::Book2>>(input);
    for (const auto& item : books2)
    {
        std::cout << item.author() << ", " 
                  << item.title() << ", " 
                  << item.price() << ", " 
                  << item.pub_date() << "\n";
    }
    std::cout << "\n";
    encode_json(books2, std::cout, indenting::indent);
    std::cout << "\n\n";

    std::cout << "(3)\n\n";
    auto books3 = decode_json<std::vector<ns::Book3>>(input);
    for (const auto& item : books3)
    {
        std::cout << item.author() << ", " 
                  << item.title() << ", " 
                  << item.price() << ", " 
                  << item.pub_date() << "\n";
    }
    std::cout << "\n";
    encode_json(books3, std::cout, indenting::indent);
    std::cout << "\n\n";

    std::cout << "(4)\n\n";
    auto books4 = decode_json<std::vector<ns::Book4>>(input);
    for (const auto& item : books4)
    {
        std::cout << item.getAuthor() << ", " 
                  << item.getTitle() << ", " 
                  << item.getPrice() << ", " 
                  << item.getPubDate() << "\n";
    }
    std::cout << "\n";
    encode_json(books4, std::cout, indenting::indent);
    std::cout << "\n\n";
}

void json_type_traits_macro_examples()
{
    std::cout << "\njson_type_traits macro examples\n\n";

    std::cout << std::setprecision(6);

    json_type_traits_book_examples();

    std::cout << std::endl;
}

// Copyright 2013 Daniel Parker
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

using namespace jsoncons;

namespace json_type_traits_macros_tests {

    template <typename T1>
    struct myStruct
    {
          T1 typeContent;
          std::string someString;
    };

    template <typename T1>
    struct myStruct2
    {
          T1 typeContent;
          std::string someString;
    };

    template <typename T1>
    struct myStruct3
    {
        T1 typeContent_;
        std::string someString_;
    public:
        myStruct3(T1 typeContent, const std::string& someString)
            : typeContent_(typeContent), someString_(someString)
        {
        }

        const T1& typeContent() const {return typeContent_;}
        const std::string& someString() const {return someString_;}
    };

    struct book
    {
        std::string author;
        std::string title;
        double price;
    };
    struct book2
    {
        std::string author;
        std::string title;
        double price;
        std::string isbn;
    };
    class book3
    {
        std::string author_;
        std::string title_;
        double price_;
    public:
        book3(const std::string& author,
              const std::string& title,
              double price)
            : author_(author), title_(title), price_(price)
        {
        }

        book3(const book3&) = default;
        book3(book3&&) = default;
        book3& operator=(const book3&) = default;
        book3& operator=(book3&&) = default;

        const std::string& author() const
        {
            return author_;
        }

        const std::string& title() const
        {
            return title_;
        }

        double price() const
        {
            return price_;
        }
    };

} // namespace jsoncons_member_traits_decl_tests
 
namespace ns = json_type_traits_macros_tests;

JSONCONS_GETTER_CTOR_TRAITS_DECL(ns::book3, author, title, price)
JSONCONS_MEMBER_TRAITS_DECL(ns::book,author,title,price)
JSONCONS_MEMBER_TRAITS_DECL(ns::book2,author,title,price,isbn)
JSONCONS_TEMPLATE_MEMBER_TRAITS_DECL(1,ns::myStruct,typeContent,someString)
JSONCONS_TEMPLATE_STRICT_MEMBER_TRAITS_DECL(1,ns::myStruct2,typeContent,someString)
JSONCONS_TEMPLATE_GETTER_CTOR_TRAITS_DECL(1,ns::myStruct3,typeContent,someString)

TEST_CASE("JSONCONS_MEMBER_TRAITS_DECL tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    ns::book book{an_author, a_title, a_price};

    SECTION("book")
    {
        std::string s;

        encode_json(book, s);

        json j = decode_json<json>(s);

        REQUIRE(j.is<ns::book>() == true);
        REQUIRE(j.is<ns::book2>() == false);

        CHECK(j["author"].as<std::string>() == an_author);
        CHECK(j["title"].as<std::string>() == a_title);
        CHECK(j["price"].as<double>() == Approx(a_price).epsilon(0.001));

        json j2(book);

        CHECK(j == j2);

        ns::book val = j.as<ns::book>();

        CHECK(val.author == book.author);
        CHECK(val.title == book.title);
        CHECK(val.price == Approx(book.price).epsilon(0.001));
    }
}

TEST_CASE("JSONCONS_GETTER_CTOR_TRAITS_DECL tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    SECTION("is")
    {
        json j;
        j["author"] = an_author;
        j["title"] = a_title;
        j["price"] = a_price;

        bool val = j.is<ns::book3>();
        CHECK(val == true);
    }
    SECTION("to_json")
    {
        ns::book3 book(an_author,a_title,a_price);

        json j(book);

        CHECK(j["author"].as<std::string>() == an_author);
        CHECK(j["title"].as<std::string>() == a_title);
        CHECK(j["price"].as<double>() == Approx(a_price).epsilon(0.001));
    }

    SECTION("as")
    {
        json j;
        j["author"] = an_author;
        j["title"] = a_title;
        j["price"] = a_price;

        ns::book3 book = j.as<ns::book3>();

        CHECK(book.author() == an_author);
        CHECK(book.title() == a_title);
        CHECK(book.price() == Approx(a_price).epsilon(0.001));
    }
}

TEST_CASE("JSONCONS_TEMPLATE_MEMBER_TRAITS_DECL tests")
{
    SECTION("myStruct<std::pair<int,int>>")
    {
        typedef ns::myStruct<std::pair<int, int>> value_type;

        value_type val;
        val.typeContent = std::make_pair(1,2);
        val.someString = "A string";

        std::string s;
        encode_json(val, s, indenting::indent);

        auto val2 = decode_json<value_type>(s);

        CHECK(val2.typeContent.first == val.typeContent.first);
        CHECK(val2.typeContent.second == val.typeContent.second);
        CHECK(val2.someString == val.someString);

        //std::cout << val.typeContent.first << ", " << val.typeContent.second << ", " << val.someString << "\n";
    }
}

TEST_CASE("JSONCONS_TEMPLATE_GETTER_CTOR_TRAITS_DECL tests")
{
    SECTION("myStruct<std::pair<int,int>>")
    {
        typedef ns::myStruct3<std::pair<int, int>> value_type;

        value_type val(std::make_pair(1,2), "A string");

        std::string s;
        encode_json(val, s, indenting::indent);

        auto val2 = decode_json<value_type>(s);

        CHECK(val2.typeContent().first == val.typeContent().first);
        CHECK(val2.typeContent().second == val.typeContent().second);
        CHECK(val2.someString() == val.someString());

        //std::cout << val.typeContent.first << ", " << val.typeContent.second << ", " << val.someString << "\n";
    }
}


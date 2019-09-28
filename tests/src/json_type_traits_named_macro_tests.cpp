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

namespace json_type_traits_named_macro_tests
{
    struct book
    {
        std::string author;
        std::string title;
        double price;

        book()
            : author(""), title(""), price(0)
        {
        }

        book(const std::string& author, const std::string& title, double price)
            : author(author), title(title), price(price)
        {
        }

        friend std::ostream& operator<<(std::ostream& os, const book& b)
        {
            std::cout << "author: " << b.author << ", title: " << b.title << ", price: " << b.price << "\n";
            return os;
        }
    };

    struct book_undefaulted
    {
        std::string author;
        std::string title;
        double price;

        friend std::ostream& operator<<(std::ostream& os, const book_undefaulted& b)
        {
            std::cout << "author: " << b.author << ", title: " << b.title << ", price: " << b.price << "\n";
            return os;
        }
    };

    class book_with_getters_and_ctor
    {
        std::string author_;
        std::string title_;
        double price_;
    public:
        book_with_getters_and_ctor(const std::string& author,
              const std::string& title,
              double price)
            : author_(author), title_(title), price_(price)
        {
        }

        book_with_getters_and_ctor(const book_with_getters_and_ctor&) = default;
        book_with_getters_and_ctor(book_with_getters_and_ctor&&) = default;
        book_with_getters_and_ctor& operator=(const book_with_getters_and_ctor&) = default;
        book_with_getters_and_ctor& operator=(book_with_getters_and_ctor&&) = default;

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

    class book_with_getters_and_setters
    {
        std::string author_;
        std::string title_;
        double price_;
    public:
        book_with_getters_and_setters()
            : price_(0)
        {
        }

        book_with_getters_and_setters(const std::string& author,
              const std::string& title,
              double price)
            : author_(author), title_(title), price_(price)
        {
        }

        book_with_getters_and_setters(const book_with_getters_and_setters&) = default;
        book_with_getters_and_setters(book_with_getters_and_setters&&) = default;
        book_with_getters_and_setters& operator=(const book_with_getters_and_setters&) = default;
        book_with_getters_and_setters& operator=(book_with_getters_and_setters&&) = default;

        const std::string& get_author() const
        {
            return author_;
        }

        const std::string& get_title() const
        {
            return title_;
        }

        double get_price() const
        {
            return price_;
        }

        void set_author(const std::string& author)
        {
            author_ = author;
        }

        void set_title(const std::string& title)
        {
            title_ = title;
        }

        void set_price(double price)
        {
            price_ = price;
        }
    };

    class book_with_getters_and_setters_undefaulted
    {
        std::string author_;
        std::string title_;
        double price_;
    public:
        book_with_getters_and_setters_undefaulted()
            : price_(0)
        {
        }

        book_with_getters_and_setters_undefaulted(const std::string& author,
              const std::string& title,
              double price)
            : author_(author), title_(title), price_(price)
        {
        }

        book_with_getters_and_setters_undefaulted(const book_with_getters_and_setters_undefaulted&) = default;
        book_with_getters_and_setters_undefaulted(book_with_getters_and_setters_undefaulted&&) = default;
        book_with_getters_and_setters_undefaulted& operator=(const book_with_getters_and_setters_undefaulted&) = default;
        book_with_getters_and_setters_undefaulted& operator=(book_with_getters_and_setters_undefaulted&&) = default;

        const std::string& get_author() const
        {
            return author_;
        }

        const std::string& get_title() const
        {
            return title_;
        }

        double get_price() const
        {
            return price_;
        }

        void set_author(const std::string& author)
        {
            author_ = author;
        }

        void set_title(const std::string& title)
        {
            title_ = title;
        }

        void set_price(double price)
        {
            price_ = price;
        }
    };

    template <typename T1>
    struct TemplatedStruct1
    {
          T1 typeContent;
          std::string someString;
    };

    template <typename T1, typename T2>
    struct TemplatedStruct2
    {
          T1 aT1;
          T2 aT2;
    };
 
    enum class float_format {scientific = 1,fixed = 2,hex = 4,general = fixed | scientific};
} // namespace json_type_traits_named_macro_tests

namespace ns = json_type_traits_named_macro_tests;

JSONCONS_MEMBER_NAMED_TRAITS_DECL(ns::book,(author,"Author"),(title,"Title"),(price,"Price"))
JSONCONS_STRICT_MEMBER_NAMED_TRAITS_DECL(ns::book_undefaulted,(author,"Author"),(title,"Title"),(price,"Price"))
JSONCONS_GETTER_CTOR_NAMED_TRAITS_DECL(ns::book_with_getters_and_ctor, (author,"Author"),(title,"Title"),(price,"Price"))
JSONCONS_TPL_MEMBER_NAMED_TRAITS_DECL(1,ns::TemplatedStruct1,(typeContent,"type-content"),(someString,"some-string"))
JSONCONS_TPL_MEMBER_NAMED_TRAITS_DECL(2,ns::TemplatedStruct2,(aT1,"a-t1"),(aT2,"a-t2"))
JSONCONS_ENUM_NAMED_TRAITS_DECL(ns::float_format, (scientific,"Exponential"), (fixed,"Fixed"), (hex,"Hex"), (general,"General"))
JSONCONS_GETTER_SETTER_NAMED_TRAITS_DECL(ns::book_with_getters_and_setters, (get_author,set_author,"Author"),(get_title,set_title,"Title"),(get_price,set_price,"Price"))
JSONCONS_STRICT_GETTER_SETTER_NAMED_TRAITS_DECL(ns::book_with_getters_and_setters_undefaulted, (get_author,set_author,"Author"),(get_title,set_title,"Title"),(get_price,set_price,"Price"))

TEST_CASE("JSONCONS_MEMBER_NAMED_TRAITS_DECL tests")
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

        CHECK(j["Author"].as<std::string>() == an_author);
        CHECK(j["Title"].as<std::string>() == a_title);
        CHECK(j["Price"].as<double>() == Approx(a_price).epsilon(0.001));

        json j2(book);

        CHECK(j == j2);

        ns::book val = j.as<ns::book>();

        CHECK(val.author == book.author);
        CHECK(val.title == book.title);
        CHECK(val.price == Approx(book.price).epsilon(0.001));
    }
} 

TEST_CASE("JSONCONS_STRICT_MEMBER_NAMED_TRAITS_DECL tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    ns::book_undefaulted book{an_author, a_title, a_price};

    SECTION("book")
    {
        std::string s;

        encode_json(book, s);

        json j = decode_json<json>(s);

        REQUIRE(j.is<ns::book_undefaulted>() == true);

        CHECK(j["Author"].as<std::string>() == an_author);
        CHECK(j["Title"].as<std::string>() == a_title);
        CHECK(j["Price"].as<double>() == Approx(a_price).epsilon(0.001));

        json j2(book);

        CHECK(j == j2);

        ns::book_undefaulted val = j.as<ns::book_undefaulted>();

        CHECK(val.author == book.author);
        CHECK(val.title == book.title);
        CHECK(val.price == Approx(book.price).epsilon(0.001));
    }
}

TEST_CASE("JSONCONS_TPL_MEMBER_NAMED_TRAITS_DECL tests")
{
    SECTION("TemplatedStruct1<std::pair<int,int>>")
    {
        typedef ns::TemplatedStruct1<std::pair<int, int>> value_type;

        value_type val;
        val.typeContent = std::make_pair(1,2);
        val.someString = "A string";

        std::string s;
        encode_json(val, s, indenting::indent);

        json j = decode_json<json>(s);
        CHECK(j["some-string"].as<std::string>() == val.someString);
        CHECK(j["type-content"][0].as<int>() == 1);
        CHECK(j["type-content"][1].as<int>() == 2);        
        //std::cout << pretty_print(j) << "\n";
 
        auto val2 = decode_json<value_type>(s);

        CHECK(val2.typeContent.first == val.typeContent.first);
        CHECK(val2.typeContent.second == val.typeContent.second);
        CHECK(val2.someString == val.someString);

        //std::cout << val.typeContent.first << ", " << val.typeContent.second << ", " << val.someString << "\n";
    }
    SECTION("TemplatedStruct2<int,double>")
    {
        typedef ns::TemplatedStruct2<int,double> value_type;

        value_type val;
        val.aT1 = 1;
        val.aT2 = 2;

        std::string s;
        encode_json(val, s, indenting::indent);

        json j = decode_json<json>(s);
        CHECK(j["a-t1"].as<int>() == 1);
        CHECK(j["a-t2"].as<double>() == 2.0);
        //std::cout << pretty_print(j) << "\n";

        auto val2 = decode_json<value_type>(s);

        CHECK(val2.aT1 == val.aT1);
        CHECK(val2.aT2 == val.aT2);

        //std::cout << val.typeContent.first << ", " << val.typeContent.second << ", " << val.someString << "\n";
    }
}

TEST_CASE("JSONCONS_ENUM_NAMED_TRAITS_DECL tests")
{
    SECTION("float_format default")
    {
        ns::float_format val{ns::float_format::hex};

        std::string s;
        encode_json(val,s);

        json j = decode_json<json>(s);
        CHECK(j.as<std::string>() == std::string("Hex"));

        auto val2 = decode_json<ns::float_format>(s);
        CHECK(val2 == val);
    }
    SECTION("float_format hex")
    {
        ns::float_format val{ns::float_format()};

        std::string s;
        encode_json(val,s);

        json j = decode_json<json>(s);
        CHECK(j.as<std::string>().empty());

        auto val2 = decode_json<ns::float_format>(s);
        CHECK(val2 == val);
    }
}

TEST_CASE("JSONCONS_GETTER_CTOR_NAMED_TRAITS_DECL tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    SECTION("is")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        bool val = j.is<ns::book_with_getters_and_ctor>();
        CHECK(val);
    }
    SECTION("to_json")
    {
        ns::book_with_getters_and_ctor book(an_author,a_title,a_price);

        json j(book);

        CHECK(j["Author"].as<std::string>() == an_author);
        CHECK(j["Title"].as<std::string>() == a_title);
        CHECK(j["Price"].as<double>() == Approx(a_price).epsilon(0.001));
    }

    SECTION("as")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        ns::book_with_getters_and_ctor book = j.as<ns::book_with_getters_and_ctor>();

        CHECK(book.author() == an_author);
        CHECK(book.title() == a_title);
        CHECK(book.price() == Approx(a_price).epsilon(0.001));
    }
}

TEST_CASE("JSONCONS_GETTER_SETTER_NAMED_TRAITS_DECL tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    SECTION("is")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        bool val = j.is<ns::book_with_getters_and_setters>();
        CHECK(val);
    }

    SECTION("as")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        ns::book_with_getters_and_setters book = j.as<ns::book_with_getters_and_setters>();

        CHECK(book.get_author() == an_author);
        CHECK(book.get_title() == a_title);
        CHECK(book.get_price() == Approx(a_price).epsilon(0.001));
    }

    SECTION("to_json")
    {
        ns::book_with_getters_and_setters book(an_author,a_title,a_price);

        json j(book);

        CHECK(j["Author"].as<std::string>() == an_author);
        CHECK(j["Title"].as<std::string>() == a_title);
        CHECK(j["Price"].as<double>() == Approx(a_price).epsilon(0.001));
    }
}

TEST_CASE("JSONCONS_STRICT_GETTER_SETTER_NAMED_TRAITS_DECL tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    SECTION("is")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        bool val = j.is<ns::book_with_getters_and_setters_undefaulted>();
        CHECK(val);
    }

    SECTION("as")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        ns::book_with_getters_and_setters_undefaulted book = j.as<ns::book_with_getters_and_setters_undefaulted>();

        CHECK(book.get_author() == an_author);
        CHECK(book.get_title() == a_title);
        CHECK(book.get_price() == Approx(a_price).epsilon(0.001));
    }

    SECTION("to_json")
    {
        ns::book_with_getters_and_setters_undefaulted book(an_author,a_title,a_price);

        json j(book);

        CHECK(j["Author"].as<std::string>() == an_author);
        CHECK(j["Title"].as<std::string>() == a_title);
        CHECK(j["Price"].as<double>() == Approx(a_price).epsilon(0.001));
    }
}


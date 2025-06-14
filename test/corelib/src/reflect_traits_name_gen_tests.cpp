// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/json_encoder.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstdint>
#include <catch/catch.hpp>

using namespace jsoncons;

namespace {
namespace ns {

    struct book_m_all
    {
        std::string author;
        std::string title;
        double price;

        book_m_all()
            : author(""), title(""), price(0)
        {
        }

        book_m_all(const std::string& author, const std::string& title, double price)
            : author(author), title(title), price(price)
        {
        }
    };

    struct book_m_3
    {
        std::string author;
        std::string title;
        double price;
    };

    class book_cg_all
    {
        std::string author_;
        std::string title_;
        double price_;
    public:
        book_cg_all(const std::string& author,
              const std::string& title,
              double price)
            : author_(author), title_(title), price_(price)
        {
        }

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

    class book_cg_2
    {
        std::string author_;
        std::string title_;
        double price_;
        std::string isbn_;
        jsoncons::optional<std::string> publisher_;
    public:
        book_cg_2(const std::string& author,
              const std::string& title,
              double price,
              const std::string& isbn,
              const jsoncons::optional<std::string>& publisher)
            : author_(author), title_(title), price_(price), isbn_(isbn),
              publisher_(publisher)
        {
        }

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

        const std::string& isbn() const
        {
            return isbn_;
        }

        const jsoncons::optional<std::string>& publisher() const
        {
            return publisher_;
        }
    };

    class book_gs_all
    {
        std::string author_;
        std::string title_;
        double price_;
    public:
        book_gs_all()
            : price_(0)
        {
        }

        book_gs_all(const std::string& author,
              const std::string& title,
              double price)
            : author_(author), title_(title), price_(price)
        {
        }

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

    class book_gs_2
    {
        std::string author_;
        std::string title_;
        double price_;
        std::string isbn_;
    public:
        book_gs_2()
            : author_(), title_(), price_(), isbn_()
        {
        }

        book_gs_2(const std::string& author,
              const std::string& title,
              double price,
              const std::string& isbn)
            : author_(author), title_(title), price_(price), isbn_(isbn)
        {
        }

        const std::string& get_author() const
        {
            return author_;
        }

        void set_author(const std::string& value)
        {
            author_ = value;
        }

        const std::string& get_title() const
        {
            return title_;
        }

        void set_title(const std::string& value)
        {
            title_ = value;
        }

        double get_price() const
        {
            return price_;
        }

        void set_price(double value)
        {
            price_ = value;
        }

        const std::string& get_isbn() const
        {
            return isbn_;
        }

        void set_isbn(const std::string& value)
        {
            isbn_ = value;
        }
    };

    class book5
    {
        std::string author_;
        std::string title_;
        double price_;
    public:
        book5()
            : price_(0)
        {
        }

        book5(const std::string& author,
              const std::string& title,
              double price)
            : author_(author), title_(title), price_(price)
        {
        }

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

    template <typename T1,typename T2>
    struct TemplatedStruct2
    {
          T1 aT1;
          T2 aT2;
    };
 
    enum class float_format {scientific = 1,fixed = 2,hex = 4,general = fixed | scientific};

    struct Person1 
    {
        std::string name;
        std::string surname;
    };

} // ns
} // namespace 

JSONCONS_ALL_MEMBER_NAME_TRAITS(ns::book_m_all,(author,"Author"),(title,"Title"),(price,"Price"))
JSONCONS_ALL_MEMBER_NAME_TRAITS(ns::book_m_3,(author,"Author"),(title,"Title"),(price,"Price"))
JSONCONS_N_MEMBER_NAME_TRAITS(ns::Person1, 1, (name, "n"), (surname, "sn"))
JSONCONS_ALL_CTOR_GETTER_NAME_TRAITS(ns::book_cg_all, (author,"Author"),(title,"Title"),(price,"Price"))
JSONCONS_N_CTOR_GETTER_NAME_TRAITS(ns::book_cg_2, 2, (author,"Author"),(title,"Title"),(price,"Price"), (isbn, "Isbn"), (publisher, "Publisher"))
JSONCONS_ALL_GETTER_SETTER_NAME_TRAITS(ns::book_gs_all, (get_author,set_author,"Author"),(get_title,set_title,"Title"),(get_price,set_price,"Price"))
JSONCONS_N_GETTER_SETTER_NAME_TRAITS(ns::book_gs_2, 2, (get_author,set_author,"Author"),(get_title,set_title,"Title"),(get_price,set_price,"Price"),(get_isbn,set_isbn,"Isbn"))
JSONCONS_TPL_ALL_MEMBER_NAME_TRAITS(1,ns::TemplatedStruct1,(typeContent,"type-content"),(someString,"some-string"))
JSONCONS_TPL_ALL_MEMBER_NAME_TRAITS(2,ns::TemplatedStruct2,(aT1,"a-t1"),(aT2,"a-t2"))
JSONCONS_ENUM_NAME_TRAITS(ns::float_format, (scientific,"Exponential"), (fixed,"Fixed"), (hex,"Hex"), (general,"General"))

TEST_CASE("JSONCONS_ALL_MEMBER_NAME_TRAITS tests 1")
{
   std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    ns::book_m_all book{an_author, a_title, a_price};

    SECTION("success")
    {
        std::string s;

        encode_json(book, s);

        json j = decode_json<json>(s);

        REQUIRE(j.is<ns::book_m_all>() == true);

        CHECK(j["Author"].as<std::string>() == an_author);
        CHECK(j["Title"].as<std::string>() == a_title);
        CHECK(j["Price"].as<double>() == Approx(a_price).epsilon(0.001));

        json j2(book);

        CHECK(j == j2);

        ns::book_m_all val = j.as<ns::book_m_all>();

        CHECK(val.author == book.author);
        CHECK(val.title == book.title);
        CHECK(val.price == Approx(book.price).epsilon(0.001));
    }
    SECTION("unexpected JSON")
    {
        std::string input = R"(["Haruki Murakami", "Kafka on the Shore", 25.17])";

        auto result = jsoncons::try_decode_json<ns::book_m_all>(input);
        REQUIRE_FALSE(result);
        CHECK(result.error().code() == jsoncons::conv_errc::expected_object);
        //std::cout << result.error() << "\n";
    }
    SECTION("missing member")
    {
        std::string input = R"(
{
    "Author" : "Haruki Murakami", 
    "Title" : "Kafka on the Shore"    
}
        )";

        auto result = jsoncons::try_decode_json<ns::book_m_all>(input);
        REQUIRE_FALSE(result);
        CHECK(result.error().code() == jsoncons::conv_errc::missing_required_member);
        CHECK("ns::book_m_all" == result.error().message_arg());
    }
    SECTION("parsing error")
    {
        std::string input = R"(
{
    "Author" : "Haruki Murakami", 
    "Title" : "Kafka on the Shore",
    "Price" 25.17        
}
        )";

        auto result = jsoncons::try_decode_json<ns::book_m_all>(input);
        REQUIRE_FALSE(result);
        CHECK(result.error().code() == json_errc::expected_colon);
        //std::cout << result.error() << "\n";
    }
    SECTION("invalid JSON value")
    {
        std::string input = R"(
{
    "Author" : "Haruki Murakami", 
    "Title" : "Kafka on the Shore",
    "Price" : "foo"
}
        )";

        try
        {
            auto result = jsoncons::try_decode_json<ns::book_m_all>(input);
            REQUIRE_FALSE(result);
            //CHECK(result.error().code() == json_errc::expected_colon);
            std::cout << result.error() << "\n";
        }
        catch (const std::exception& e)
        {
            std::cout << "EXCEPTION " << e.what() << "\n";
        }
    }
} 

TEST_CASE("JSONCONS_ALL_MEMBER_NAME_TRAITS tests 2")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;

    ns::book_m_3 book{an_author, a_title, a_price};

    SECTION("book_m_3")
    {
        std::string s;

        encode_json(book, s);

        json j = decode_json<json>(s);

        REQUIRE(j.is<ns::book_m_3>() == true);

        CHECK(j["Author"].as<std::string>() == an_author);
        CHECK(j["Title"].as<std::string>() == a_title);
        CHECK(j["Price"].as<double>() == Approx(a_price).epsilon(0.001));

        json j2(book);

        CHECK(j == j2);

        ns::book_m_3 val = j.as<ns::book_m_3>();

        CHECK(val.author == book.author);
        CHECK(val.title == book.title);
        CHECK(val.price == Approx(book.price).epsilon(0.001));
    }
}

TEST_CASE("JSONCONS_N_MEMBER_NAME_TRAITS tests")
{
    SECTION("decode")
    {
        std::string data = R"({"n":"Rod"})";
        auto person = jsoncons::decode_json<ns::Person1>(data);
        CHECK(person.name == std::string("Rod"));
        CHECK(person.surname.empty());

        std::string s;
        jsoncons::encode_json(person, s, indenting::indent);

        auto other = jsoncons::decode_json<ns::Person1>(s);
        CHECK(other.name == person.name);
        CHECK(other.surname == person.surname);
    }
}

TEST_CASE("JSONCONS_ALL_TPL_MEMBER_NAME_TRAITS tests 1")
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
        CHECK(1 == j["type-content"][0].as<int>());
        CHECK(2 == j["type-content"][1].as<int>());        
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
        CHECK(1 == j["a-t1"].as<int>());
        CHECK(j["a-t2"].as<double>() == 2.0);
        //std::cout << pretty_print(j) << "\n";

        auto val2 = decode_json<value_type>(s);

        CHECK(val2.aT1 == val.aT1);
        CHECK(val2.aT2 == val.aT2);

        //std::cout << val.typeContent.first << ", " << val.typeContent.second << ", " << val.someString << "\n";
    }
}

TEST_CASE("JSONCONS_ENUM_NAME_TRAITS tests")
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

TEST_CASE("JSONCONS_ALL_CTOR_GETTER_NAME_TRAITS tests")
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

        bool val = j.is<ns::book_cg_all>();
        CHECK(val);
    }
    SECTION("to_json")
    {
        ns::book_cg_all book(an_author,a_title,a_price);

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

        ns::book_cg_all book = j.as<ns::book_cg_all>();

        CHECK(book.author() == an_author);
        CHECK(book.title() == a_title);
        CHECK(book.price() == Approx(a_price).epsilon(0.001));
    }
}

TEST_CASE("JSONCONS_N_CTOR_GETTER_NAME_TRAITS tests")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;
    std::string an_isbn = "1400079276";

    SECTION("decode")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;

        CHECK(j.is<ns::book_cg_2>());
        CHECK_FALSE(j.is<ns::book_cg_all>());

        std::string buffer;
        j.dump(buffer);
        auto book = decode_json<ns::book_cg_2>(buffer);
        CHECK(book.author() == an_author);
        CHECK(book.title() == a_title);
        CHECK(book.price() == double());
        CHECK(book.isbn() == std::string());
    }

    SECTION("encode_json")
    {
        ns::book_cg_2 book(an_author, a_title, a_price, an_isbn, jsoncons::optional<std::string>());

        std::string buffer;
        encode_json_pretty(book, buffer);
        //std::cout << buffer << "\n";

        json j = json::parse(buffer);

        CHECK(j["Author"].as<std::string>() == an_author);
        CHECK(j["Title"].as<std::string>() == a_title);
        CHECK(j["Price"].as<double>() == Approx(a_price).epsilon(0.001));
        CHECK(j["Isbn"].as<std::string>() == an_isbn);
        CHECK_FALSE(j.contains("Publisher"));
    }
}

TEST_CASE("JSONCONS_ALL_GETTER_SETTER_NAME_TRAITS tests")
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

        bool val = j.is<ns::book_gs_all>();
        CHECK(val);
    }

    SECTION("as")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        ns::book_gs_all book = j.as<ns::book_gs_all>();

        CHECK(book.get_author() == an_author);
        CHECK(book.get_title() == a_title);
        CHECK(book.get_price() == Approx(a_price).epsilon(0.001));
    }

    SECTION("to_json")
    {
        ns::book_gs_all book(an_author,a_title,a_price);

        json j(book);

        CHECK(j["Author"].as<std::string>() == an_author);
        CHECK(j["Title"].as<std::string>() == a_title);
        CHECK(j["Price"].as<double>() == Approx(a_price).epsilon(0.001));
    }
}

TEST_CASE("JSONCONS_ALL_GETTER_SETTER_NAME_TRAITS tests 2")
{
    std::string an_author = "Haruki Murakami"; 
    std::string a_title = "Kafka on the Shore";
    double a_price = 25.17;
    std::string an_isbn = "1400079276";

    SECTION("is")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        bool val = j.is<ns::book_gs_2>();
        CHECK(val);
    }

    SECTION("as")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;
        j["Price"] = a_price;

        ns::book_gs_2 book = j.as<ns::book_gs_2>();

        CHECK(book.get_author() == an_author);
        CHECK(book.get_title() == a_title);
        CHECK(book.get_price() == Approx(a_price).epsilon(0.001));
    }

    SECTION("to_json")
    {
        ns::book_gs_2 book(an_author,a_title,a_price, an_isbn);

        json j(book);

        CHECK(j["Author"].as<std::string>() == an_author);
        CHECK(j["Title"].as<std::string>() == a_title);
        CHECK(j["Price"].as<double>() == Approx(a_price).epsilon(0.001));
        CHECK(j["Isbn"].as<std::string>() == an_isbn);
    }

    SECTION("decode")
    {
        json j;
        j["Author"] = an_author;
        j["Title"] = a_title;

        CHECK(j.is<ns::book_gs_2>());
        CHECK_FALSE(j.is<ns::book_gs_all>());

        std::string buffer;
        j.dump(buffer);
        auto book = decode_json<ns::book_gs_2>(buffer);
        CHECK(book.get_author() == an_author);
        CHECK(book.get_title() == a_title);
        CHECK(book.get_price() == double());
        CHECK(book.get_isbn() == std::string());
    }
}


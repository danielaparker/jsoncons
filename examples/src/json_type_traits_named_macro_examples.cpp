// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <iomanip>
#include <jsoncons/json.hpp>

namespace json_type_traits_macro_examples_ns
{
    struct book
    {
        std::string author;
        std::string title;
        double price;
        std::string pub_date;
    };

    class book_with_getters_and_ctor
    {
        std::string author_;
        std::string title_;
        double price_;
        std::string pub_date_;
    public:
        book_with_getters_and_ctor(const std::string& author,
              const std::string& title,
              double price,
              const std::string& pub_date)
            : author_(author), title_(title), price_(price), pub_date_(pub_date)
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

        const std::string& pub_date() const
        {
            return pub_date_;
        }

        double price() const
        {
            return price_;
        }
    };

    class BookWithGettersAndSetters
    {
        std::string author_;
        std::string title_;
        double price_;
        std::string pub_date_;
    public:
        BookWithGettersAndSetters() = default;
        BookWithGettersAndSetters(const BookWithGettersAndSetters&) = default;
        BookWithGettersAndSetters(BookWithGettersAndSetters&&) = default;
        BookWithGettersAndSetters& operator=(const BookWithGettersAndSetters&) = default;
        BookWithGettersAndSetters& operator=(BookWithGettersAndSetters&&) = default;

        const std::string& getAuthor() const
        {
            return author_;
        }

        const std::string& getTitle() const
        {
            return title_;
        }

        double getPrice() const
        {
            return price_;
        }

        const std::string& getPubDate() const
        {
            return pub_date_;
        }

        void setAuthor(const std::string& value)
        {
            author_ = value;
        }

        void setTitle(const std::string& value)
        {
            title_ = value;
        }

        void setPrice(double value)
        {
            price_ = value;
        }

        void setPubDate(const std::string& value)
        {
            pub_date_ = value;
        }
    };
}

namespace ns = json_type_traits_macro_examples_ns;

JSONCONS_STRICT_MEMBER_TRAITS_NAMED_DECL(ns::book,(author,"Author"),(title,"Title"),(price,"Price"),(pub_date,"Publication Date"))
JSONCONS_GETTER_CTOR_TRAITS_NAMED_DECL(ns::book_with_getters_and_ctor,(author,"Author"),(title,"Title"),(price,"Price"),(pub_date,"Publication Date"))
JSONCONS_STRICT_GETTER_SETTER_TRAITS_NAMED_DECL(ns::BookWithGettersAndSetters, (getAuthor,setAuthor,"Author"),(getTitle,setTitle,"Title"),(getPrice,setPrice,"Price"),(getPubDate,setPubDate,"Publication Date"))

using namespace jsoncons;

void json_type_traits_book_examples()
{
    const std::string s = R"(
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

    auto books1 = decode_json<std::vector<ns::book>>(s);
    std::cout << "(1)\n";
    for (const auto& item : books1)
    {
        std::cout << item.author << ", " 
                  << item.title << ", " 
                  << item.price << ", " 
                  << item.pub_date << "\n";
    }
    std::cout << "\n(2)\n";
    encode_json(books1, std::cout, indenting::indent);
    std::cout << "\n\n";

    auto books2 = decode_json<std::vector<ns::book_with_getters_and_ctor>>(s);
    std::cout << "(3)\n";
    for (const auto& item : books2)
    {
        std::cout << item.author() << ", " 
                  << item.title() << ", " 
                  << item.price() << ", " 
                  << item.pub_date() << "\n";
    }
    std::cout << "\n(4)\n";
    encode_json(books2, std::cout, indenting::indent);
    std::cout << "\n\n";

    auto books3 = decode_json<std::vector<ns::BookWithGettersAndSetters>>(s);
    std::cout << "(5)\n";
    for (const auto& item : books3)
    {
        std::cout << item.getAuthor() << ", " 
                  << item.getTitle() << ", " 
                  << item.getPrice() << ", " 
                  << item.getPubDate() << "\n";
    }
    std::cout << "\n(6)\n";
    encode_json(books3, std::cout, indenting::indent);
    std::cout << "\n\n";
}

void json_type_traits_macro_examples()
{
    std::cout << "\njson_type_traits macro examples\n\n";

    std::cout << std::setprecision(6);

    json_type_traits_book_examples();

    std::cout << std::endl;
}

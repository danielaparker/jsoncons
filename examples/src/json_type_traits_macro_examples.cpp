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
}

namespace ns = json_type_traits_macro_examples_ns;

JSONCONS_STRICT_MEMBER_TRAITS_NAMED_DECL(ns::book,(author,"Author"),(title,"Title"),(price,"Price"))
JSONCONS_GETTER_CTOR_NAMED_DECL(ns::book_with_getters_and_ctor,(author,"Author"),(title,"Title"),(price,"Price"))
JSONCONS_STRICT_GETTER_SETTER_TRAITS_NAMED_DECL(ns::book_with_getters_and_setters, (get_author,set_author,"Author"),(get_title,set_title,"Title"),(get_price,set_price,"Price"))

using namespace jsoncons;

void json_type_traits_book_examples()
{
    const std::string s = R"(
    [
        {
            "author" : "Haruki Murakami",
            "title" : "Kafka on the Shore",
            "price" : 25.17
        },
        {
            "author" : "Charles Bukowski",
            "title" : "Pulp",
            "price" : 22.48
        }
    ]
    )";

    auto books1 = decode_json<std::vector<ns::book>>(s);
    std::cout << "(1)\n";
    for (const auto& item : books1)
    {
        std::cout << item.author << ", " 
                  << item.title << ", " 
                  << item.price << "\n";
    }
    std::cout << "\n(1)\n";
    encode_json(books1, std::cout, indenting::indent);
    std::cout << "\n\n";

    auto books2 = decode_json<std::vector<ns::book_with_getters_and_ctor>>(s);
    std::cout << "(2)\n";
    for (const auto& item : books2)
    {
        std::cout << item.author() << ", " 
                  << item.title() << ", " 
                  << item.price() << "\n";
    }
    std::cout << "\n(2)\n";
    encode_json(books2, std::cout, indenting::indent);
    std::cout << "\n\n";

    auto books3 = decode_json<std::vector<ns::book_with_getters_and_setters>>(s);
    std::cout << "(3)\n";
    for (const auto& item : books3)
    {
        std::cout << item.get_author() << ", " 
                  << item.get_title() << ", " 
                  << item.get_price() << "\n";
    }
    std::cout << "\n(3)\n";
    encode_json(books3, std::cout, indenting::indent);
    std::cout << "\n\n";
}

void json_type_traits_macro_examples()
{
    std::cout << std::setprecision(6);

    json_type_traits_book_examples();

    std::cout << "\njson_type_traits macro examples\n\n";

    std::cout << std::endl;
}

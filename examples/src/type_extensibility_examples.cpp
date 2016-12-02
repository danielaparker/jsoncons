// Copyright 2016 Daniel Parker
// Distributed under Boost license

#include <string>
#include <vector>
#include <list>
#include <jsoncons/json.hpp>

using namespace jsoncons;

struct book
{
    std::string author;
    std::string title;
    double price;
};

namespace jsoncons
{
    template<class Json>
    struct json_type_traits<Json, book>
    {
        typedef typename Json::allocator_type allocator_type;

        static bool is(const Json& rhs) JSONCONS_NOEXCEPT
        {
            return rhs.is_object() &&
                   rhs.has_key("author") && 
                   rhs.has_key("title") && 
                   rhs.has_key("price");
        }
        static book as(const Json& rhs)
        {
            book val;
            val.author = rhs["author"]. template as<std::string>();
            val.title = rhs["title"]. template as<std::string>();
            val.price = rhs["price"]. template as<double>();
            return val;
        }
        static Json to_json(const book& val, 
                            const allocator_type& allocator)
        {
            Json j(allocator);
            j["author"] = val.author;
            j["title"] = val.title;
            j["price"] = val.price;
            return j;
        }
    };
};

void type_extensibility_examples()
{
    std::cout << "\nType extensibility examples\n\n";

    book book1{"Haruki Murakami", "Kafka on the Shore", 25.17};

    json j = book1;

    std::cout << "(1) " << std::boolalpha << j.is<book>() << "\n\n";

    std::cout << "(2) " << pretty_print(j) << "\n\n";

    book temp = j.as<book>();
    std::cout << "(3) " << temp.author << "," 
                        << temp.title << "," 
                        << temp.price << "\n\n";

    book book2{"Charles Bukowski", "Women: A Novel", 12.0};

    std::vector<book> book_array{book1, book2};

    json ja = book_array;

    std::cout << "(4) " << std::boolalpha 
                        << ja.is<std::vector<book>>() << "\n\n";

    std::cout << "(5)\n" << pretty_print(ja) << "\n\n";

    auto book_list = ja.as<std::list<book>>();

    std::cout << "(6)" << std::endl;
    for (auto b : book_list)
    {
        std::cout << b.author << ", " 
                  << b.title << ", " 
                  << b.price << std::endl;
    }

    std::cout << std::endl;
}


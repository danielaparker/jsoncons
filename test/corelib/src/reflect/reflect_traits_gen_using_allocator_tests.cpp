// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license 

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

} // namespace ns
} // namespace 
 
JSONCONS_TPL_ALL_MEMBER_TRAITS(1,ns::book_all_m,author,title,price)
JSONCONS_TPL_N_MEMBER_TRAITS(1,ns::book_3_m,3,author,title,price,isbn)

JSONCONS_TPL_ALL_MEMBER_NAME_TRAITS(1, ns::book_all_m_name,(author,"Author"),(title,"Title"),(price,"Price"))

JSONCONS_TPL_ALL_GETTER_SETTER_TRAITS(1, ns::book_all_gs, get, set, Author, Title, Price)

JSONCONS_TPL_ALL_CTOR_GETTER_TRAITS(1, ns::book_all_cg, author, title, price)
JSONCONS_TPL_ALL_CTOR_GETTER_NAME_TRAITS(1, ns::book_all_cg_name, (author,"Author"),(title,"Title"),(price,"Price"))

template <typename T>
using cust_allocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

using cust_json = jsoncons::basic_json<char,jsoncons::sorted_policy,cust_allocator<char>>;

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
        using book_collection_type = std::vector<book_type,cust_allocator<book_type>>;

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
        auto aset = make_alloc_set<cust_allocator<book_type>>(alloc);
        auto r = try_decode_json<book_collection_type>(aset, input);

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
        using book_collection_type = std::vector<book_type,cust_allocator<book_type>>;

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
        auto aset = make_alloc_set<cust_allocator<book_type>>(alloc);
        auto r = try_decode_json<book_collection_type>(aset, input);

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
        using book_collection_type = std::vector<book_type,cust_allocator<book_type>>;

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
        auto aset = make_alloc_set<cust_allocator<book_type>>(alloc);
        auto r = try_decode_json<book_collection_type>(aset, input);

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
            std::cout << "Err: " << r.error() << "\n";
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
        using book_collection_type = std::vector<book_type,cust_allocator<book_type>>;

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
        auto aset = make_alloc_set<cust_allocator<book_type>>(alloc);
        auto r = try_decode_json<book_collection_type>(aset, input);

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
            std::cout << "Err: " << r.error() << "\n";
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
        using book_collection_type = std::vector<book_type,cust_allocator<book_type>>;

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
        auto aset = make_alloc_set<cust_allocator<book_type>>(alloc);
        auto r = try_decode_json<book_collection_type>(aset, input);

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
            std::cout << "Err: " << r.error() << "\n";
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
        using book_collection_type = std::vector<book_type,cust_allocator<book_type>>;

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
        auto aset = make_alloc_set<cust_allocator<book_type>>(alloc);
        auto r = try_decode_json<book_collection_type>(aset, input);

        REQUIRE(r);

        std::string output;
        encode_json(aset, *r, output);

        auto j1 = ojson::parse(input);
        auto j2 = ojson::parse(output);
        CHECK(j1 == j2);
    }
}

#endif

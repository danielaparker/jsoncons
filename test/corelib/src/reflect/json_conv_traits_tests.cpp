// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <jsoncons/json.hpp>
#include <jsoncons/decode_json.hpp>
#include <jsoncons/encode_json.hpp>
#include <sstream>
#include <vector>
#include <utility>
#include <ctime>
#include <cstdint>
#include <catch/catch.hpp>

namespace ns {
    struct book
    {
        std::string author;
        std::string title;
        double price;
    };
} // namespace ns

namespace jsoncons {
namespace reflect {

template <typename Json>
struct json_conv_traits<Json, ns::book>
{
    using result_type = conversion_result<ns::book>;
    using allocator_type = Json::allocator_type;

    static bool is(const Json& j) noexcept
    {
        return j.is_object() && j.contains("author") && 
               j.contains("title") && j.contains("price");
    }
    static result_type try_as(const Json& j)
    {
        ns::book val;
        val.author = j.at("author").template as<std::string>();
        val.title = j.at("title").template as<std::string>();
        val.price = j.at("price").template as<double>();
        return result_type(std::move(val));
    }
    static Json to_json(const ns::book& val, 
       allocator_type allocator=allocator_type())
    {
        Json j(allocator);
        j.try_emplace("author", val.author);
        j.try_emplace("title", val.title);
        j.try_emplace("price", val.price);
        return j;
    }
};

} // namespace reflect
} // namespace jsoncons

TEST_CASE("json_conv_traits tests")
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
    
    SECTION("test 1")
    {
        auto j = jsoncons::json::parse(s);
        REQUIRE(j.is_array());
        REQUIRE(2 == j.size());
    
        auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,ns::book>::try_as(j[0]);
        ns::book& item{result.value()};

        std::cout << item.author << ", " 
                  << item.title << ", " 
                  << item.price << "\n";
    }
    
    SECTION("test 2")
    {
        auto j = jsoncons::json::parse(s);
        REQUIRE(j.is_array());
    
        std::cout << "is_json_conv_traits_declared: " << jsoncons::reflect::is_json_conv_traits_declared<std::vector<ns::book>>::value << "\n";

        std::cout << "is_compatible_array_type: " << jsoncons::reflect::is_json_conv_traits_declared<std::vector<ns::book>>::value << "\n";
        
        std::vector<ns::book> book_list = jsoncons::decode_json<std::vector<ns::book>>(s);

        ///auto result = jsoncons::reflect::json_conv_traits<jsoncons::json,std::vector<ns::book>>::try_as(j);

        /*auto book_list = result.value();
    
        std::cout << "(1)\n";
        for (const auto& item : book_list)
        {
            std::cout << item.author << ", " 
                      << item.title << ", " 
                      << item.price << "\n";
        }
    
        std::cout << "\n(2)\n";
        jsoncons::encode_json(book_list, std::cout, jsoncons::indenting::indent);
        std::cout << "\n\n";*/
    }
}



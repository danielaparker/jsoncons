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

template <typename T>
using cust_allocator = std::scoped_allocator_adaptor<mock_stateful_allocator<T>>;

namespace {
namespace ns {

    template <typename Alloc>
    struct book_all_m
    {
        using allocator_type = Alloc;

        using char_allocator_type = typename std::allocator_traits<typename Alloc::inner_allocator_type>:: template rebind_alloc<char>;
        using string_type = std::basic_string<char, std::char_traits<char>, char_allocator_type>;

        book_all_m(const Alloc alloc)
            : author(alloc), title(alloc)
        {
        }

        string_type author;
        string_type title;
        double price;
    };

} // namespace ns
} // namespace 
 
JSONCONS_ALL_MEMBER_TRAITS(ns::book_all_m<cust_allocator<char>>,author,title,price)

TEST_CASE("JSONCONS_ALL_MEMBER_TRAITS using allocator tests")
{
    SECTION("success")
    {
        std::string str = R"(
{
    "author" : "Haruki Murakami",  
    "title" : "Kafka on the Shore",
    "price" : 25.17
}
        )";

        cust_allocator<char> alloc(1);
        auto aset = make_alloc_set(alloc);
        auto r = decode_json<ns::book_all_m<cust_allocator<char>>>(aset, str);

        //REQUIRE(r);
    }
}

#endif

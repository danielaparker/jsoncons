// Copyright 2021 Daniel Parker
// Distributed under Boost license

#if defined(_MSC_VER)
#include "windows.h" // test no inadvertant macro expansions
#endif
#include <catch/catch.hpp>
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>
#include <common/sample_allocators.hpp>

using namespace jsoncons;

#if !(defined(__GNUC__) && (__GNUC__ == 4)) || !(defined(__GNUC__) && __GNUC_MINOR__ < 9)

TEST_CASE("jsonpath stateful allocator test")
{
    using my_json = basic_json<char,sorted_policy,FreelistAllocator<char>>;
    std::string input = R"(
{ "store": {
    "book": [ 
      { "category": "reference",
        "author": "Nigel Rees",
        "title": "Sayings of the Century",
        "price": 8.95
      },
      { "category": "fiction",
        "author": "Evelyn Waugh",
        "title": "Sword of Honour",
        "price": 12.99
      },
      { "category": "fiction",
        "author": "Herman Melville",
        "title": "Moby Dick",
        "isbn": "0-553-21311-3",
        "price": 8.99
      }
    ]
  }
}
)";

    SECTION("make_expression")
    {
        json_decoder<my_json,FreelistAllocator<char>> decoder(result_allocator_arg, FreelistAllocator<char>(1),
                                                              FreelistAllocator<char>(2));

        auto myAlloc = FreelistAllocator<char>(3);        

        basic_json_reader<char,string_source<char>,FreelistAllocator<char>> reader(input, decoder, myAlloc);
        reader.read();

        my_json j = decoder.get_result();

        jsoncons::string_view p{"$..book[?(@.category == 'fiction')].title"};
        auto expr = jsoncons::jsonpath::make_expression<my_json>(std::allocator_arg, myAlloc, p);   // this will not compile
        auto result = expr.evaluate(j);

        CHECK(result.size() == 2);
        CHECK(result[0].as_string_view() == "Sword of Honour");
        CHECK(result[1].as_string_view() == "Moby Dick");
    }
    SECTION("json_query")
    {
        json_decoder<my_json,FreelistAllocator<char>> decoder(result_allocator_arg, FreelistAllocator<char>(1),
                                                              FreelistAllocator<char>(2));

        auto myAlloc = FreelistAllocator<char>(3);        

        basic_json_reader<char,string_source<char>,FreelistAllocator<char>> reader(input, decoder, myAlloc);
        reader.read();

        my_json j = decoder.get_result();

        auto result = jsoncons::jsonpath::json_query(std::allocator_arg, myAlloc, j,"$..book[?(@.category == 'fiction')].title");

        CHECK(result.size() == 2);
        CHECK(result[0].as_string_view() == "Sword of Honour");
        CHECK(result[1].as_string_view() == "Moby Dick");
    }
}

#endif

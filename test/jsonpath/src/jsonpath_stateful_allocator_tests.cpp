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

#if !(defined(__GNUC__) && (__GNUC__ == 4)) && (defined(__GNUC__) && __GNUC_MINOR__ < 9)

TEST_CASE("jsonpath stateful allocator test")
{
    using my_json = basic_json<char,sorted_policy,FreelistAllocator<char>>;
    std::string input = R"(
[ 
  { 
      "author" : "Haruki Murakami",
      "title" : "Hard-Boiled Wonderland and the End of the World",
      "isbn" : "0679743464",
      "publisher" : "Vintage",
      "date" : "1993-03-02",
      "price": 18.90
  },
  { 
      "author" : "Graham Greene",
      "title" : "The Comedians",
      "isbn" : "0099478374",
      "publisher" : "Vintage Classics",
      "date" : "2005-09-21",
      "price": 15.74
  }
]
)";

    json_decoder<my_json,FreelistAllocator<char>> decoder(result_allocator_arg, FreelistAllocator<char>(1),
                                                          FreelistAllocator<char>(2));

    auto myAlloc = FreelistAllocator<char>(3);        

    basic_json_reader<char,string_source<char>,FreelistAllocator<char>> reader(input, decoder, myAlloc);
    reader.read();

    my_json j = decoder.get_result();
    std::cout << pretty_print(j) << "\n";

    //jsoncons::string_view p{"$.store.book[?(@.price < 10)].author"};
    //auto expr = jsoncons::jsonpath::make_expression<my_json>(std::allocator_arg, myAlloc, p);   // this will not compile
    //auto r = expr.evaluate(j);
}

#endif

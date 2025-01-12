// Copyright 2013-2025 Daniel Parker
// Distributed under Boost license

#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath.hpp>
#include <catch/catch.hpp>

namespace jsonpath = jsoncons::jsonpath;

std::string store = R"(
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
      },
      { "category": "fiction",
        "author": "J. R. R. Tolkien",
        "title": "The Lord of the Rings",
        "isbn": "0-395-19395-8",
        "price": 22.99
      }
    ]
  }
}
)";

TEST_CASE("csv function tests")
{
    SECTION("length")
    {
        auto root = jsoncons::json::parse(store);

        // The number of books
        auto result1 = jsonpath::json_query(root, "length($.store.book)");
        REQUIRE(1 == result1.size());
        REQUIRE(result1[0].is<std::size_t>());
        CHECK(4 == result1[0].as<std::size_t>());

        auto result2 = jsonpath::json_query(root, "length($..book)");
        REQUIRE(1 == result2.size());
        REQUIRE(result2[0].is<std::size_t>());
        //CHECK(4 == result2[0].as<std::size_t>());
    }
}



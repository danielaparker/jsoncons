// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

void jsonpointer_example1()
{
json root = json::parse(R"(
[
  { "category": "reference",
    "author": "Nigel Rees",
    "title": "Sayings of the Century",
    "price": 8.95
  },
  { "category": "fiction",
    "author": "Evelyn Waugh",
    "title": "Sword of Honour",
    "price": 12.99
  }
]
)");

    json result = jsonpointer::select(root, "/1/author");

    std::cout << result << std::endl;
}

void jsonpointer_examples()
{
    std::cout << "\njsonpointer examples\n\n";
    jsonpointer_example1();
    std::cout << std::endl;
}


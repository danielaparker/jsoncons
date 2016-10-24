// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <fstream>
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/json_query.hpp"

using namespace jsoncons;
using namespace jsoncons::jsonpath;

void json_query_examples()
{
    std::ifstream is("input/booklist.json");
    json booklist;
    is >> booklist;

    // The authors of books that are cheaper than $10
    json result1 = json_query(booklist, "$.store.book[?(@.price < 10)].author");
    std::cout << "(1) " << result1 << std::endl;

    // The number of books
    json result2 = json_query(booklist, "$..book.length");
    std::cout << "(2) " << result2 << std::endl;

    // The third book
    json result3 = json_query(booklist, "$..book[2]");
    std::cout << "(3)\n" << pretty_print(result3) << std::endl;

    // All books whose author's name starts with Evelyn
    json result4 = json_query(booklist, "$.store.book[?(@.author =~ /Evelyn.*?/)]");
    std::cout << "(4)\n" << pretty_print(result4) << std::endl;

    // The titles of all books that have isbn number
    json result5 = json_query(booklist, "$..book[?(@.isbn)].title");
    std::cout << "(5) " << result5 << std::endl;

    // All authors and titles of books
    json result6 = json_query(booklist, "$['store']['book']..['author','title']");
    std::cout << "(6)\n" << pretty_print(result6) << std::endl;
}

void jsonpath_examples()
{
    std::cout << "\nJsonPath examples\n\n";
    json_query_examples();
    std::cout << std::endl;
}


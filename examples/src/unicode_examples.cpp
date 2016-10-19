// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include <iostream>
#include "jsoncons/json.hpp"

using namespace jsoncons;

void read_and_write_escaped_unicode()
{
    std::string input = "[\"\\u8A73\\u7D30\\u95B2\\u89A7\\uD800\\uDC01\\u4E00\"]";
    json value = json::parse(input);
    output_format format;
    format.escape_all_non_ascii(true);
    std::string output = value.to_string(format);

    std::cout << "Input:" << std::endl;
    std::cout << input << std::endl;
    std::cout << std::endl;
    std::cout << "Output:" << std::endl;
    std::cout << output << std::endl;
}

void unicode_examples()
{
    std::cout << "\nUnicode examples\n\n";
    read_and_write_escaped_unicode();
    std::cout << std::endl;
}


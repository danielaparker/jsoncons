// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include "jsoncons/json.hpp"

using jsoncons::json;
using jsoncons::output_format;
using std::string;

void first_example()
{
    try
    {
        json books = json::parse_file("input/books.json");

        for (size_t i = 0; i < books.size(); ++i)
        {
            json& book = books[i];
            string author = book["author"].as_string();
            string title = book["title"].as_string();
            double price = book["price"].as_double();
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void second_example()
{
    try
    {
        json books = json::parse_file("input/books.json");

        for (size_t i = 0; i < books.size(); ++i)
        {
            json& book = books[i];
            string author = book["author"].as_string();
            string title = book["title"].as_string();
            string price = book.get("price","N/A").as_string();
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void third_example()
{
    try
    {
        json books = json::parse_file("input/books.json");

        output_format format;
        format.fixed_decimal_places(2);
        for (size_t i = 0; i < books.size(); ++i)
        {
            json& book = books[i];
            string author = book["author"].as_string();
            string title = book["title"].as_string();
            string price = book.get("price","N/A").as_string(format);
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

int main()
{
    first_example();
    second_example();
    third_example();
    return 0;
}



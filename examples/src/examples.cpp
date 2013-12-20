// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <string>
#include "jsoncons/json.hpp"
#include "csv_examples.h"

using jsoncons::json;
using jsoncons::pretty_print;
using jsoncons::output_format;
using std::string;

void array_examples();
void custom_data_examples();

void first_example_a()
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

void first_example_b()
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

void second_example_a()
{
    try
    {
        json books(json::an_array);

        {
            json book;
            book["title"] = "Kafka on the Shore";
            book["author"] = "Haruki Murakami";
            book["price"] = 25.17;
            books.add(std::move(book));
        }

        {
            json book;
            book["title"] = "Women: A Novel";
            book["author"] = "Charles Bukowski";
            book["price"] = 12.00;
            books.add(std::move(book));
        }

        {
            json book;
            book["title"] = "Cutter's Way";
            book["author"] = "Ivan Passer";
            books.add(std::move(book));
        }

        std::cout << pretty_print(books) << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

int main()
{
    first_example_a();
    first_example_b();

    second_example_a();

    array_examples();
    custom_data_examples();

    read_csv_file();
    write_csv_file();

    return 0;
}



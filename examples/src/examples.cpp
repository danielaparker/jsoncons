// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <stdexcept>
#include <string>
#include "jsoncons/json.hpp"
#include "csv_examples.h"

using jsoncons::json;
using jsoncons::json_deserializer;
using jsoncons::json_reader;
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

void first_example_c()
{
    try
    {
        json books = json::parse_file("input/books.json");

        output_format format;
        format.floatfield(std::ios::fixed);
        format.precision(2);

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

void second_example_a()
{
    try
    {
        json books = json::make_array();

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

void mulitple_json_objects()
{
    std::ifstream is("input/multiple-json-objects.json");
    if (!is.is_open())
    {
        throw std::runtime_error("Cannot open file");
    }

    json_deserializer handler;
    json_reader reader(is,handler);

    while (!reader.eof())
    {
        reader.read();
        if (!reader.eof())
        {
            json val = std::move(handler.root());
            std::cout << val << std::endl;
        }
    }
}

void more_examples()
{
	json image_sizing;  
	image_sizing["resolution"] = 144; 
	image_sizing["long_edge"] = 9.84;
	image_sizing["size_units"] =  "cm";
	image_sizing["resize_to_fit"] = true;
	std::cout << pretty_print(image_sizing) << std::endl;

    json image_formats = json::make_array();
    image_formats.add("JPEG");
    image_formats.add("PSD");
    image_formats.add("TIFF");
    image_formats.add("DNG");

    json file_export;
    file_export["image_formats"] = std::move(image_formats);
    file_export["image_sizing"] = std::move(image_sizing);
    std::cout << pretty_print(file_export) << std::endl;

	size_t n = 10, m = 3;
	std::vector<size_t> x(n,m);
	x[5] = 3;
}

int main()
{
    first_example_a();
    first_example_b();
    first_example_c();

    second_example_a();

    array_examples();
    custom_data_examples();

    read_csv_file();
    write_csv_file();

	more_examples();
    mulitple_json_objects();

    return 0;
}



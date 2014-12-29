// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <stdexcept>
#include <string>
#include "jsoncons/json.hpp"
#include "csv_examples.h"

using jsoncons::json;
using jsoncons::json_exception;
using jsoncons::json_deserializer;
using jsoncons::json_reader;
using jsoncons::pretty_print;
using jsoncons::output_format;
using std::string;

void array_examples();
void json_any_examples();
void wjson_examples();

void first_example_a()
{
    json books = json::parse_file("input/books.json");

    for (size_t i = 0; i < books.size(); ++i)
    {
        try
        {
            json& book = books[i];
            string author = book["author"].as<std::string>();
            string title = book["title"].as<std::string>();
            double price = book["price"].as<double>();
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}

void first_example_b()
{
    json books = json::parse_file("input/books.json");

    for (size_t i = 0; i < books.size(); ++i)
    {
        try
        {
            json& book = books[i];
            string author = book["author"].as<std::string>();
            string title = book["title"].as<std::string>();
            string price = book.get("price", "N/A").as<std::string>();
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
        catch (const json_exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}

void first_example_c()
{
    json books = json::parse_file("input/books.json");

    output_format format;
    format.floatfield(std::ios::fixed);
    format.precision(2);

    for (size_t i = 0; i < books.size(); ++i)
    {
        try
        {
            json& book = books[i];
            string author = book["author"].as<std::string>();
            string title = book["title"].as<std::string>();
            string price = book.get("price", "N/A").to_string(format);
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

}

void first_example_d()
{
    json books = json::parse_file("input/books.json");

    output_format format;
    format.floatfield(std::ios::fixed);
    format.precision(2);

    for (size_t i = 0; i < books.size(); ++i)
    {
        try
        {
            json& book = books[i];
            string author = book["author"].as<std::string>();
            string title = book["title"].as<std::string>();
            if (book.has_member("price") && book["price"].is_numeric())
            {
                double price = book["price"].as<double>();
                std::cout << author << ", " << title << ", " << price << std::endl;
            }
            else
            {
                std::cout << author << ", " << title << ", " << "n/a" << std::endl;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
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

void mulitple_json_objects()
{
    std::ifstream is("input/multiple-json-objects.json");
    if (!is.is_open())
    {
        throw std::runtime_error("Cannot open file");
    }

    json_deserializer handler;
    json_reader reader(is, handler);

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
	image_sizing["resize_to_fit"] = true;  // a boolean 
	image_sizing["resize_unit"] =  "pixels";  // a string
	image_sizing["resize_what"] =  "long_edge";  // a string
	image_sizing["dimension1"] = 9.84;  // a double
	image_sizing["dimension2"] = jsoncons::null_type();  // a null value
    std::cout << pretty_print(image_sizing) << std::endl;

    json image_formats(json::an_array);
    image_formats.add("JPEG");
    image_formats.add("PSD");
    image_formats.add("TIFF");
    image_formats.add("DNG");

    json file_export;
    file_export["image_formats"] = std::move(image_formats);
    file_export["image_sizing"] = std::move(image_sizing);
    std::cout << pretty_print(file_export) << std::endl;

    size_t n = 10, m = 3;
    std::vector<size_t> x(n, m);
    x[5] = 3;
}

void introspection_example()
{
    json val = json::parse_file("input/books.json");
    std::cout << std::boolalpha;
    std::cout << "Is this an object? " << val.is<json::object>() << ", or an array? " << val.is<json::array>() << std::endl;

    if (val.is<json::array>())
    {
        for (size_t i = 0; i < val.size(); ++i)
        {
            json& elem = val[i];
            std::cout << "Is element " << i << " an object? " << elem.is<json::object>() << std::endl;
            if (elem.is<json::object>())
            {
                for (auto it = elem.begin_members(); it != elem.end_members(); ++it){
                    std::cout << "Is member " << it->name() << " a string? " << it->value().is<std::string>() << ", or a double? " << it->value().is<double>() << ", or perhaps an int? " << it->value().is<int>() << std::endl;

                }
            }
        }
    }
}

void read_and_write_escaped_unicode()
{
    string input = "[\"\\u8A73\\u7D30\\u95B2\\u89A7\\uD800\\uDC01\\u4E00\"]";
    json value = json::parse_string(input);
    output_format format;
    format.escape_all_non_ascii(true);
    string output = value.to_string(format);

    std::cout << "Input:" << std::endl;
    std::cout << input << std::endl;
    std::cout << std::endl;
    std::cout << "Output:" << std::endl;
    std::cout << output << std::endl;
}

void parse_exception_example()
{
	string s = "[1,2,3,4,]";
    try 
	{
        jsoncons::json val = jsoncons::json::parse_string(s);
    } 
	catch(const jsoncons::json_parse_exception& e) 
	{
        std::cout << "Caught json_parse_exception with category " << e.code().category().name() 
			      << ", code " << e.code().value() 
                  << " and message " << e.what() << std::endl;
    }
}

int main()
{
    first_example_a();
    first_example_b();
    first_example_c();
    first_example_d();

    second_example_a();

    array_examples();
    json_any_examples();

    read_csv_file();
    write_csv_file();

    more_examples();
    mulitple_json_objects();

    introspection_example();

    wjson_examples();

    read_and_write_escaped_unicode();

	parse_exception_example();

    return 0;
}



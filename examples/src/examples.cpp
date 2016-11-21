// Copyright 2013 Daniel Parker
// Distributed under Boost license

#include <stdexcept>
#include <string>
#include <jsoncons/json.hpp>

using namespace jsoncons;

void basics_examples();
void json_filter_examples();
void array_examples();
void container_examples();
void wjson_examples();
void serialization_examples();
void type_extensibility_examples();
void ojson_examples();
void unicode_examples();
void csv_examples();
void jsonpath_examples();
void jsonx_examples();
void json_is_as_examples();

void first_example_a()
{
    json books = json::parse_file("input/books.json");

    for (size_t i = 0; i < books.size(); ++i)
    {
        try
        {
            json& book = books[i];
            std::string author = book["author"].as<std::string>();
            std::string title = book["title"].as<std::string>();
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
            std::string author = book["author"].as<std::string>();
            std::string title = book["title"].as<std::string>();
            std::string price = book.get("price", "N/A").as<std::string>();
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}

void first_example_c()
{
    json books = json::parse_file("input/books.json");

    serialization_options format;
    //format.floatfield(std::ios::fixed);
    format.precision(2);

    for (size_t i = 0; i < books.size(); ++i)
    {
        try
        {
            json& book = books[i];
            std::string author = book["author"].as<std::string>();
            std::string title = book["title"].as<std::string>();
            std::string price = book.get("price", "N/A").to_string(format);
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

    serialization_options format;
    //format.floatfield(std::ios::fixed);
    format.precision(2);

    for (size_t i = 0; i < books.size(); ++i)
    {
        try
        {
            json& book = books[i];
            std::string author = book["author"].as<std::string>();
            std::string title = book["title"].as<std::string>();
            if (book.has_key("price") && book["price"].is_number())
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

    json_decoder<json> decoder;
    json_reader reader(is, decoder);

    while (!reader.eof())
    {
        reader.read_next();
        if (!reader.eof())
        {
            json val = decoder.get_result();
            std::cout << val << std::endl;
        }
    }
}

void more_examples()
{
    json file_settings = json::object{
        {"Image Format", "JPEG"},
        {"Color Space", "sRGB"},
        { "Limit File Size", true},
        {"Limit File Size To", 10000}
    };

    json image_sizing;
    image_sizing.set("Resize To Fit",true);  // a boolean 
    image_sizing.set("Resize Unit", "pixels");  // a string
    image_sizing.set("Resize What", "long_edge");  // a string
    image_sizing.set("Dimension 1",9.84);  // a double
    image_sizing.set("Dimension 2",json::null());  // a null value
    std::cout << pretty_print(image_sizing) << std::endl;

    json image_formats = json::array{"JPEG","PSD","TIFF","DNG"};

    json color_spaces = json::array();
    color_spaces.add("sRGB");
    color_spaces.add("AdobeRGB");
    color_spaces.add("ProPhoto RGB");

    json file_export;
    file_export["File Format Options"]["Color Spaces"] = std::move(color_spaces);
    file_export["File Format Options"]["Image Formats"] = std::move(image_formats);
    file_export["File Settings"] = std::move(file_settings);
    file_export["Image Sizing"] = std::move(image_sizing);
    std::cout << pretty_print(file_export) << std::endl;

    size_t n = 10, m = 3;
    std::vector<size_t> x(n, m);
    x[5] = 3;
}

void parse_exception_example()
{
    std::string s = "[1,2,3,4,]";
    try 
    {
        jsoncons::json val = jsoncons::json::parse(s);
    } 
    catch(const jsoncons::parse_exception& e) 
    {
        std::cout << "Caught parse_exception with category " << e.code().category().name() 
                  << ", code " << e.code().value() 
                  << " and message " << e.what() << std::endl;
    }
}

int main()
{
    try
    {
        json_is_as_examples();
/*
        basics_examples();
        ojson_examples();

        first_example_a();
        first_example_b();
        first_example_c();
        first_example_d();

        second_example_a();

        array_examples();
        container_examples();

        csv_examples();

        more_examples();
        mulitple_json_objects();

        introspection_example();

        wjson_examples();

        unicode_examples();

        serialization_examples();

        parse_exception_example();

        type_extensibility_examples();

        json_filter_examples();

        jsonpath_examples();

        jsonx_examples();
*/
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}



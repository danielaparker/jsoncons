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
void type_extensibility_examples2();
void ojson_examples();
void unicode_examples();
void csv_examples();
void jsonpath_examples();
void json_is_as_examples();
void msgpack_examples();
void streaming_examples();

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
    format.precision(2);

    for (size_t i = 0; i < books.size(); ++i)
    {
        try
        {
            json& book = books[i];
            std::string author = book["author"].as<std::string>();
            std::string title = book["title"].as<std::string>();
            std::string price;
            book.get("price", "N/A").dump(price,format);
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
        json books = json::array();

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

void construction_in_code()
{
    // A null value
    json null_val = json::null();

    // A boolean value
    json flag(true);

    // A numeric value
    json number(10.5);

    // An object value with four members
    json obj;
    obj["first_name"] = "Jane";
    obj["last_name"] = "Roe";
    obj["events_attended"] = 10;
    obj["accept_waiver_of_liability"] = true;

    std::string first_name = obj["first_name"].as<std::string>();
    std::string last_name = obj.at("last_name").as<std::string>();
    int events_attended = obj["events_attended"].as<int>();
    bool accept_waiver_of_liability = obj["accept_waiver_of_liability"].as<bool>();

    // An array value with four elements
    json arr = json::array();
    arr.add(null_val);
    arr.add(flag);
    arr.add(number);
    arr.add(obj);

    serialization_options format;
    std::cout << pretty_print(arr) << std::endl;
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

void object_range_based_for_loop()
{
    json j = json::parse(R"(
{
    "category" : "Fiction",
    "title" : "Pulp",
    "author" : "Charles Bukowski",
    "date" : "2004-07-08",
    "price" : 22.48,
    "isbn" : "1852272007"  
}
)");

    for (const auto& kv : j.object_range())
    {
        std::cout << kv.key() << " => " << kv.value().as<std::string>() << std::endl;
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
    std::cout << pretty_print(image_sizing) << std::endl;

    json image_formats = json::array{"JPEG","PSD","TIFF","DNG"};

    json color_spaces = json::array();
    color_spaces.add("sRGB");
    color_spaces.add("AdobeRGB");
    color_spaces.add("ProPhoto RGB");

    json export_settings;
    export_settings["File Format Options"]["Color Spaces"] = std::move(color_spaces);
    export_settings["File Format Options"]["Image Formats"] = std::move(image_formats);
    export_settings["File Settings"] = std::move(file_settings);
    export_settings["Image Sizing"] = std::move(image_sizing);

    // Write to stream
    std::ofstream os("export_settings.json");
    os << export_settings;

    // Read from stream
    std::ifstream is("export_settings.json");
    json j = json::parse(is);

    // Pretty print
    std::cout << "(1)\n" << pretty_print(j) << "\n\n";

    // Get reference to object member
    const json& val = j["Image Sizing"];

    // Access member as double
    std::cout << "(2) " << "Dimension 1 = " << val["Dimension 1"].as<double>() << "\n\n";

    // Try access member with default
    std::cout << "(3) " << "Dimension 2 = " << val.get_with_default("Dimension 2",0.0) << "\n\n";

}

void parse_exception_example()
{
    std::string s = "[1,2,3,4,]";
    try 
    {
        jsoncons::json val = jsoncons::json::parse(s);
    } 
    catch(const jsoncons::parse_error& e) 
    {
        std::cout << "Caught parse_error with category " << e.code().category().name() 
                  << ", code " << e.code().value() 
                  << " and message " << e.what() << std::endl;
    }
}

void validation_example()
{
    std::string s = R"(
{
    "StartDate" : "2017-03-01",
    "MaturityDate" "2020-12-30"          
}
    )";
    std::stringstream is(s);

    json_reader reader(is);

    std::error_code ec;
    reader.read(ec);
    if (ec)
    {
        std::cout << ec.message() 
                  << " on line " << reader.line_number()
                  << " and column " << reader.column_number()
                  << std::endl;
    }
}

int main()
{
    try
    {
        object_range_based_for_loop();

        json_is_as_examples();

        basics_examples();
        ojson_examples();

        first_example_a();
        first_example_b();
        first_example_c();
        first_example_d();

        second_example_a();

        construction_in_code();

        array_examples();
        container_examples();

        csv_examples();

        more_examples();
        mulitple_json_objects();

        wjson_examples();

        unicode_examples();

        serialization_examples();

        parse_exception_example();

        type_extensibility_examples();

        type_extensibility_examples2();

        json_filter_examples();

        jsonpath_examples();

        msgpack_examples();

        streaming_examples();

        validation_example();
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}

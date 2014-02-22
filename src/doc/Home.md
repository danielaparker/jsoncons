The jsoncons library implements C++ classes for the construction of [JavaScript Object Notation](http://www.json.org) (JSON). 

The library is header-only: it consists solely of header files containing templates and inline functions, and requires no separately-compiled library binaries when linking. It has no dependence on other libraries.

jsoncons uses some features that are new to C++ 11, particularly move semantics, however, it has been written to be compatible with VC++ 10. It has been tested with MS Visual C++ 10 SP1, Intel C++ Studio XE 2013 and clang 3.3 and GCC 4.8. Note that SP1 is required for VC++ 10, it fixes compiler bugs with move semantics.

The code repository is on [github](https://github.com/danielaparker/jsoncons), releases are on [sourceforge](https://sourceforge.net/projects/jsoncons/?source=navbar).

To install the jsoncons library, download the zip file, unpack the release, under `src` find the directory `jsoncons`, and copy it to your `include` directory. If you wish to use extensions, copy the `jsoncons_ext` directory as well. 

All core jsoncons classes and functions are in namespace `jsoncons`. Extensions are in namespace `jsoncons_ext`.

### Features

* Implements parsing and serializing JSON text in UTF-8
* Supports event based JSON parsing and serializing with user defined input and output handlers
* Uses the standard C++ input/output streams library
* Guarantees basic exception safety (no leaks)
* Accepts and ignores single line comments that start with `//`, and multi line comments that start with `/*` and end with `*/`
* Supports optional escaping of the solidus (`/`) character
* Supports `Nan`, `Inf` and `-Inf` replacement
* Supports reading multiple json objects from a stream
* Supports optional escaping of non-ascii UTF-8 octets
* Supports storing custom data types in a json object, with specialized serialization
* Supports reading JSON values from CSV files

### Roadmap

[[Roadmap]]

### Tutorial

[[Basics]]

[[Arrays]]

[[Introspection]]

[[Extensibility]]

[[Custom data]]

[[Transforming JSON with filters]]

### Classes

[[json]]

[[json_serializer]]

[[output_format]]

[[json_reader]]

[[json_deserializer]]

### Extensions

#### csv

[[csv_reader]]

[[csv_serializer]]

## Examples

### First example

Here is a sample json file (`books.json`):

    [
        {
            "title" : "Kafka on the Shore",
            "author" : "Haruki Murakami",
            "price" : 25.17
        },
        {
            "title" : "Women: A Novel",
            "author" : "Charles Bukowski",
            "price" : 12.00
        },
        {
            "title" : "Cutter's Way",
            "author" : "Ivan Passer"
        }
    ]

This is how to read the document with `jsoncons`: 

    #include <string>
    #include "jsoncons/json.hpp"

    using jsoncons::json;
    using jsoncons::output_format;
    using std::string;

    try
    {
        json books = json::parse_file("input/books.json");

        for (size_t i = 0; i < books.size(); ++i)
        {
            json& book = books[i];
            string author = book["author"].as<string>();
            string title = book["title"].as<string>();
            double price = book["price"].as<double>();
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

The output is

    Haruki Murakami, Kafka on the Shore, 25.17
    Charles Bukowski, Women: A Novel, 12
    Member price not found.

Note the following:

- The assignment `json& book = books[i]` is to a reference, the reference identifies a [[json]] object owned by `books`. 
- The book "Cutter's Way" has no price, so `book["price"]` throws a [[json_exception]] ("Member price not found")

Instead of having the exception, let's use an accessor that evaluates to a string and substitutes "N/A" for the missing price: 

    try
    {
        json books = json::parse_file("input/books.json");

        for (size_t i = 0; i < books.size(); ++i)
        {
            json& book = books[i];
            string author = book["author"].as<string>();
            string title = book["title"].as<string>();
            string price = book.get("price","N/A").as<string>();
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

Now the output is

    Haruki Murakami, Kafka on the Shore, 25.17
    Charles Bukowski, Women: A Novel, 12.0
    Ivan Passer, Cutter's Way, N/A

By default, when formatting a floating-point value with `as<string>()`, `jsoncons` truncates trailing zeros but keeps one if immediately after a decimal point. Let's change the floating-point notation to 'fixed' and the number of decimal places to 2.

    try
    {
        json books = json::parse_file("input/books.json");

        output_format format;
        format.floatfield(std::ios::fixed);
        format.precision(2);

        for (size_t i = 0; i < books.size(); ++i)
        {
            json& book = books[i];
            string author = book["author"].as<string>();
            string title = book["title"].as<string>();
            string price = book.get("price","N/A").to_string(format);
            std::cout << author << ", " << title << ", " << price << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

This time the output is

    Haruki Murakami, Kafka on the Shore, 25.17
    Charles Bukowski, Women: A Novel, 12.00
    Ivan Passer, Cutter's Way, N/A

### Second example

This example shows how to create the `books.json` file in code.

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

The output is

    [
        {
            "author":"Haruki Murakami",
            "price":25.17,
            "title":"Kafka on the Shore"
        },
        {
            "author":"Charles Bukowski",
            "price":12.0,
            "title":"Women: A Novel"
        },
        {
            "author":"Ivan Passer",
            "title":"Cutter's Way"
        }
    ]

### User specified `Nan`, `Inf` and `-Inf` replacement

    json obj;
    obj["field1"] = std::sqrt(-1.0);
    obj["field2"] = 1.79e308*1000;
    obj["field3"] = -1.79e308*1000;

    output_format format;
    format.nan_replacement("null");        // default is "null"
    format.pos_inf_replacement("1e9999");  // default is "null"
    format.neg_inf_replacement("-1e9999"); // default is "null"

    obj.to_stream(std::cout,format);
    std::cout << std::endl;

The output is

    {"field1":null,"field2":1e9999,"field3":-1e9999}

### Reading multiple JSON objects from a stream

Here is a sample json file containing multiple json objects (`multiple-json-objects.json`):

    {"a":1,"b":2,"c":3}
    {"a":4,"b":5,"c":6}
    {"a":7,"b":8,"c":9}

and here is the code to read them: 

    std::ifstream is("input/multiple-json-objects.json");
    if (!is.is_open())
    {
        throw std::exception("Cannot open file");
    }

    json_deserializer handler;
    json_reader reader(handler);

    while (!reader.eof())
    {
        reader.read(is);
        if (!reader.eof())
        {
            json val = std::move(handler.root());
            std::cout << val << std::endl;
        }
    }

The output is

    {"a":1,"b":2,"c":3}
    {"a":4,"b":5,"c":6}
    {"a":7,"b":8,"c":9}

### Optional escaping of non-ASCII UTF8 octets 

    string inputStr("[\"\\u007F\\u07FF\\u0800\"]");
    std::cout << "Input:    " << inputStr << std::endl;

    json arr = json::parse_string(inputStr);
    std::string s = arr[0].as<string>();
    std::cout << "Hex dump: [";
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (i != 0)
            std::cout << " ";
        unsigned int u(s[i] >= 0 ? s[i] : 256 + s[i] );
        std::cout << "0x"  << std::hex<< std::setfill('0') << std::setw(2) << u;
    }
    std::cout << "]" << std::endl;
    
    std::ostringstream os;
    output_format format;
    format.escape_all_non_ascii(true);
    arr.to_stream(os,format);
    std::string outputStr = os.str();
    std::cout << "Output:   " << os.str() << std::endl;

    json arr2 = json::parse_string(outputStr);
    std::string s2 = arr2[0].as<string>();
    std::cout << "Hex dump: [";
    for (size_t i = 0; i < s2.size(); ++i)
    {
        if (i != 0)
            std::cout << " ";
        unsigned int u(s2[i] >= 0 ? s2[i] : 256 + s2[i] );
        std::cout << "0x"  << std::hex<< std::setfill('0') << std::setw(2) << u;
    }
    std::cout << "]" << std::endl;

The output is

    Input:    ["\u007F\u07FF\u0800"]
    Hex dump: [0x7f 0xdf 0xbf 0xe0 0xa0 0x80]
    Output:   ["?\u07FF\u0800"]
    Hex dump: [0x7f 0xdf 0xbf 0xe0 0xa0 0x80]

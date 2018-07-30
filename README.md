# JSONCONS

jsoncons is a C++, header-only library for constructing [JSON](http://www.json.org) and JSON-like
data formats (e.g. CBOR). It supports 

- Parsing JSON-like text or binary data into an unpacked representation
  that defines an interface for accessing and modifying that data.

- Serializing the unpacked representation into different JSON-like text or binary data.

- Converting from JSON-like text or binary data to C++ objects and back.

- Streaming JSON read and write events, somewhat analogously to SAX processing in the XML world. 

It is distributed under the [Boost Software License](http://www.boost.org/users/license.html).

jsoncons uses some features that are new to C++ 11, including [move semantics](http://thbecker.net/articles/rvalue_references/section_02.html) and the [AllocatorAwareContainer](http://en.cppreference.com/w/cpp/concept/AllocatorAwareContainer) concept. It has been tested with MS VC++ 2015, GCC 4.8, GCC 4.9, GCC 6.2.0 and recent versions of clang. 

## Get jsoncons

Download the [latest release](https://github.com/danielaparker/jsoncons/releases) and unpack the zip file. Copy the directory `include/jsoncons` to your `include` directory. If you wish to use extensions, copy `include/jsoncons_ext` as well. 

Or, download the latest code on [master](https://github.com/danielaparker/jsoncons/archive/master.zip).

## How to use it

- [Quick guide](http://danielaparker.github.io/jsoncons)
- [FAQ](doc/FAQ.md)
- [Reference](doc/Home.md)

As the `jsoncons` library has evolved, names have sometimes changed. To ease transition, jsoncons deprecates the old names but continues to support many of them. See the [deprecated list](doc/ref/deprecated.md) for the status of old names. The deprecated names can be suppressed by defining macro `JSONCONS_NO_DEPRECATED`, which is recommended for new code.

## Benchmarks

[json_benchmarks](https://github.com/danielaparker/json_benchmarks) provides some measurements about how `jsoncons` compares to other `json` libraries.

- [Performance benchmarks with text and integers](https://github.com/danielaparker/json_benchmarks/blob/master/report/performance.md)

- [Performance benchmarks with text and doubles](https://github.com/danielaparker/json_benchmarks/blob/master/report/performance_fp.md)

- [JSONTestSuite and JSON_checker test suites](https://danielaparker.github.io/json_benchmarks/) 

## Extensions

- [jsonpointer](doc/ref/jsonpointer/jsonpointer.md) implements the IETF standard [JavaScript Object Notation (JSON) Pointer](https://tools.ietf.org/html/rfc6901)
- [jsonpatch](doc/ref/jsonpatch/jsonpatch.md) implements the IETF standard [JavaScript Object Notation (JSON) Patch](https://tools.ietf.org/html/rfc6902)
- [jsonpath](doc/ref/jsonpath/jsonpath.md) implements [Stefan Goessner's JSONPath](http://goessner.net/articles/JsonPath/).  It also supports search and replace using JSONPath expressions.
- [cbor](doc/ref/cbor/cbor.md) implements decode from and encode to the IETF standard [Concise Binary Object Representation (CBOR)](http://cbor.io/). It also supports a set of operations for iterating over and accessing the nested data items of a packed CBOR value.
- [msgpack](doc/ref/msgpack/msgpack.md) implements decode from and encode to the [MessagePack](http://msgpack.org/index.html) binary serialization format.
- [csv](doc/ref/csv/csv.md) implements reading (writing) JSON values from (to) CSV files

Planned new features are listed on the [roadmap](doc/Roadmap.md)

## Playing around with CBOR, JSON, and CSV

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor_serializer.hpp>
#include <jsoncons_ext/cbor/cbor_view.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/csv/csv_serializer.hpp>

// For convenience
using namespace jsoncons;    

int main()
{
    // Construct some CBOR using the streaming API
    std::vector<uint8_t> b;
    cbor::cbor_bytes_serializer bserializer(b);
    bserializer.begin_document();
    bserializer.begin_array(); // indefinite length array
    bserializer.begin_array(3); // fixed length array
    bserializer.string_value("Toronto");
    bserializer.byte_string_value({'H','e','l','l','o'});
    bserializer.bignum_value("-18446744073709551617");
    bserializer.end_array();
    bserializer.end_array();
    bserializer.end_document();

    // Print bytes
    std::cout << "(1)\n";
    for (auto x : b)
    {
        std::cout << std::hex << (int)x;
    }
    std::cout << "\n\n";

    cbor::cbor_view bv = b; // a non-owning view of the CBOR bytes

    // Loop over the outer array elements
    std::cout << "(2)\n";
    for (cbor::cbor_view row : bv.array_range())
    {
        std::cout << row << "\n";
    }
    std::cout << "\n";

    // Get element at position /0/1 using jsonpointer::get on cbor_view (returns value)
    cbor::cbor_view val = jsonpointer::get(bv, "/0/1");
    std::cout << "(3) " << val.as<std::string>() << "\n\n";

    // Print JSON representation with default options
    std::cout << "(4)\n";
    std::cout << pretty_print(bv) << "\n\n";

    // Print JSON representation with different options
    json_serializing_options options;
    options.byte_string_format(byte_string_chars_format::base64)
           .bignum_format(bignum_chars_format::base64url);
    std::cout << "(5)\n";
    std::cout << pretty_print(bv, options) << "\n\n";

    // Unpack bytes into a json variant like structure, and add some more elements
    json j = cbor::decode_cbor<json>(bv);
    j[0].push_back(bignum("18446744073709551616"));
    j[0].insert(j[0].array_range().begin(),10.5);
    std::cout << "(6)\n";
    std::cout << pretty_print(j) << "\n\n";

    // Get element at position /0/0 using jsonpointer::get on json (returns reference)
    json& ref = jsonpointer::get(j, "/0/0");
    std::cout << "(7) " << ref.as<double>() << "\n\n";

    // Repack bytes
    std::vector<uint8_t> b2;
    cbor::encode_cbor(j, b2);
    std::cout << "(8)\n";
    cbor::cbor_view b2v = b2;
    std::cout << pretty_print(b2v) << "\n\n";

    // Serialize to CSV
    csv::csv_serializing_options csv_options;
    csv_options.column_names("A,B,C,D,E");

    std::string from_unpacked;
    csv::encode_csv(j, from_unpacked, csv_options);
    std::cout << "(9)\n";
    std::cout << from_unpacked << "\n\n";

    std::string from_packed;
    csv::encode_csv(b2v, from_packed, csv_options);
    std::cout << "(10)\n";
    std::cout << from_packed << "\n\n";
}

```
Output:
```
(1)
9f8367546f726f6e746f4548656c6c6fc349100000000ff

(2)
["Toronto","SGVsbG8","-18446744073709551617"]

(3) SGVsbG8

(4)
[
    ["Toronto","SGVsbG8","-18446744073709551617"]
]

(5)
[
    ["Toronto","SGVsbG8=","~AQAAAAAAAAAA"]
]

(6)
[
    [10.5,"Toronto","SGVsbG8","-18446744073709551617","18446744073709551616"]
]

(7) 10.5

(8)
[
    [10.5,"Toronto","SGVsbG8","-18446744073709551617","18446744073709551616"]
]

(9)
A,B,C,D,E
10.5,Toronto,SGVsbG8,-18446744073709551617,18446744073709551616


(10)
A,B,C,D,E
10.5,Toronto,SGVsbG8,-18446744073709551617,18446744073709551616
```

## About jsoncons::basic_json

The jsoncons library provides a `basic_json` class template, which is the generalization of a `json` value for different 
character types, different policies for ordering name-value pairs, etc. A `basic_json` provides an unpacked representation 
of JSON-like string or binary data formats, and defines an interface for accessing and modifying that data.

```c++
typedef basic_json<char,
                   ImplementationPolicy = sorted_policy,
                   Allocator = std::allocator<char>> json;
```
The library includes four instantiations of `basic_json`:

- [json](doc/ref/json.md) constructs a utf8 character json value that sorts name-value members alphabetically

- [ojson](doc/ref/ojson.md) constructs a utf8 character json value that preserves the original name-value insertion order

- [wjson](doc/ref/wjson.md) constructs a wide character json value that sorts name-value members alphabetically

- [wojson](doc/ref/wojson.md) constructs a wide character json value that preserves the original name-value insertion order

## More examples

### Constructing json

```c++
#include <iostream>
#include <fstream>
#include <jsoncons/json.hpp>

// For convenience
using jsoncons::json;

int main()
{
    json color_spaces = json::array();
    color_spaces.push_back("sRGB");
    color_spaces.push_back("AdobeRGB");
    color_spaces.push_back("ProPhoto RGB");

    json image_sizing; // empty object
    image_sizing["Resize To Fit"] = true; // a boolean 
    image_sizing["Resize Unit"] = "pixels"; // a string
    image_sizing["Resize What"] = "long_edge"; // a string
    image_sizing["Dimension 1"] = 9.84; // a double
    
    json export_settings;

    // create "File Format Options" as an object and put "Color Spaces" in it
    export_settings["File Format Options"]["Color Spaces"] = std::move(color_spaces); 

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
    std::cout << "(3) " << "Dimension 2 = " << val.get_with_default("Dimension 2",0.0) << "\n";
}
```
Output:
```json
(1)
{
    "File Format Options": {
        "Color Spaces": ["sRGB","AdobeRGB","ProPhoto RGB"]
    },
    "Image Sizing": {
        "Dimension 1": 9.84,
        "Resize To Fit": true,
        "Resize Unit": "pixels",
        "Resize What": "long_edge"
    }
}

(2) Dimension 1 = 9.84

(3) Dimension 2 = 0.0
```

### Convert unpacked json values to standard library types and back

```c++
std::vector<int> v{1, 2, 3, 4};
json j(v);
std::cout << "(1) "<< j << std::endl;
std::deque<int> d = j.as<std::deque<int>>();

std::map<std::string,int> m{{"one",1},{"two",2},{"three",3}};
json j(m);
std::cout << "(2) " << j << std::endl;
std::unordered_map<std::string,int> um = j.as<std::unordered_map<std::string,int>>();
```
Output:
```
(1) [1,2,3,4]

(2) {"one":1,"three":3,"two":2}
```

See [json_type_traits](doc/ref/json_type_traits.md)

### Convert unpacked `json` values to user defined types and back (also standard library containers of user defined types)

```c++
    struct book
    {
        std::string author;
        std::string title;
        double price;
    };

    namespace jsoncons
    {
        template<class Json>
        struct json_type_traits<Json, book>
        {
            // Implement static functions is, as and to_json 
        };
    }        

    book book1{"Haruki Murakami", "Kafka on the Shore", 25.17};
    book book2{"Charles Bukowski", "Women: A Novel", 12.0};

    std::vector<book> v{book1, book2};

    json j = v;

    std::list<book> l = j.as<std::list<book>>();
```

See [Type Extensibility](doc/Tutorials/Type%20Extensibility.md) for details.

### Convert JSON text to C++ objects, and back

The functions `decode_json` and `encode_json` convert JSON 
formatted strings to C++ objects and back. These functions attempt to 
perform the conversion by streaming json read and write events with the help of 'json_convert_traits', and if
that is not supported, fall back to using `json_type_traits`. `decode_json` 
and `encode_json` will work for all C++ classes that have `json_type_traits` defined.

```c++
#include <iostream>
#include <map>
#include <tuple>
#include <jsoncons/json.hpp>

using namespace jsoncons;

int main()
{
    typedef std::map<std::string,std::tuple<std::string,std::string,double>> employee_collection;

    employee_collection employees = 
    { 
        {"John Smith",{"Hourly","Software Engineer",10000}},
        {"Jane Doe",{"Commission","Sales",20000}}
    };

    std::string s;
    jsoncons::encode_json(employees, s, jsoncons::indenting::indent);
    std::cout << "(1)\n" << s << std::endl;
    auto employees2 = jsoncons::decode_json<employee_collection>(s);

    std::cout << "\n(2)\n";
    for (const auto& pair : employees2)
    {
        std::cout << pair.first << ": " << std::get<1>(pair.second) << std::endl;
    }
}
```
Output:
```
(1)
{
    "Jane Doe": ["Commission","Sales",20000.0],
    "John Smith": ["Hourly","Software Engineer",10000.0]
}

(2)
Jane Doe: Sales
John Smith: Software Engineer
```

`decode_json` and `encode_json` are supported for many standard library types, and for  
[user defined types](doc/Tutorials/Type%20Extensibility.md)

See [decode_json](doc/ref/decode_json.md) and [encode_json](doc/ref/encode_json.md) 

### Dump json fragments into a larger document

```c++
#include <jsoncons/json.hpp>

using namespace jsoncons;

int main()
{
    const json some_books = json::parse(R"(
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
        }
    ]
    )");

    const json more_books = json::parse(R"(
    [
        {
            "title" : "A Wild Sheep Chase: A Novel",
            "author" : "Haruki Murakami",
            "price" : 9.01
        },
        {
            "title" : "Cutter's Way",
            "author" : "Ivan Passer",
            "price" : 8.00
        }
    ]
    )");

    json_serializer serializer(std::cout, jsoncons::indenting::indent); // pretty print
    serializer.begin_json();
    serializer.begin_array();
    for (const auto& book : some_books.array_range())
    {
        book.dump_fragment(serializer);
    }
    for (const auto& book : more_books.array_range())
    {
        book.dump_fragment(serializer);
    }
    serializer.end_array();
    serializer.end_json();
}
```
Output:
```json
[
    {
        "author": "Haruki Murakami",
        "price": 25.17,
        "title": "Kafka on the Shore"
    },
    {
        "author": "Charles Bukowski",
        "price": 12.0,
        "title": "Women: A Novel"
    },
    {
        "author": "Haruki Murakami",
        "price": 9.01,
        "title": "A Wild Sheep Chase: A Novel"
    },
    {
        "author": "Ivan Passer",
        "price": 8.0,
        "title": "Cutter's Way"
    }
]
```

## Building the test suite and examples with CMake

[CMake](https://cmake.org/) is a cross-platform build tool that generates makefiles and solutions for the compiler environment of your choice. On Windows you can download a [Windows Installer package](https://cmake.org/download/). On Linux it is usually available as a package, e.g., on Ubuntu,
```
sudo apt-get install cmake
```

Instructions for building the test suite with CMake may be found in

    jsoncons/tests/build/cmake/README.txt

Instructions for building the examples with CMake may be found in

    jsoncons/examples/build/cmake/README.txt

## Supported compilers

| Compiler        | Operating System             |Notes
|-----------------|------------------------------|----------------
| VS 2015      | Windows 10       |
| g++-4.8      | Ubuntu           | `std::regex` isn't fully implemented in GCC 4.8., so `jsoncons_ext/jsonpath` regular expression filters aren't supported for that compiler. 
| g++-6        | Ubuntu           |
| g++-7        | Ubuntu           |
| g++-8        | Ubuntu           |
| clang-5.0    |
| clang-6.0    |

## Acknowledgements

Special thanks to our [contributors](https://github.com/danielaparker/jsoncons/blob/master/acknowledgements.txt)


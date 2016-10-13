# jsoncons: a C++ library for json construction

jsoncons is a C++ library for the construction of [JavaScript Object Notation (JSON)](http://www.json.org). It supports parsing a JSON file or string into a `json` value, building a `json` value in C++ code, and serializing a `json` value to a file or string. It supports converting to and from the standard library sequence and associative containers. It also provides an API for generating json read and write events in code, somewhat analogously to SAX processing in the XML world. Consult the wiki for the latest [documentation and tutorials](https://github.com/danielaparker/jsoncons/wiki) and [roadmap](https://github.com/danielaparker/jsoncons/wiki/Roadmap). 

jsoncons uses some features that are new to C++ 11, particularly [move semantics](http://thbecker.net/articles/rvalue_references/section_02.html) and the [AllocatorAwareContainer](http://en.cppreference.com/w/cpp/concept/AllocatorAwareContainer) concept. It has been tested with MS VC++ 2013, MS VC++ 2015, GCC 4.8, GCC 4.9, and recent versions of clang. Note that `std::regex` isn't fully implemented in GCC 4.8., so `jsoncons_ext/jsonpath` regular expression filters aren't supported for that compiler.

## Using the jsoncons library

The jsoncons library is header-only: it consists solely of header files containing templates and inline functions, and requires no separately-compiled library binaries when linking. It has no dependence on other libraries. 

To install the jsoncons library, download the zip file, extract the zipped files, look under `src` for the directory `jsoncons`, and copy it to your `include` directory. If you wish to use extensions, copy the `jsoncons_ext` directory as well. 

The jsoncons classes and functions are in namespace `jsoncons`. You need to include the header file
```c++ 
#include "jsoncons/json.hpp"
```
and, for convenience,

    using jsoncons::json;

### Reading JSON text from a file

Here is a sample file, `books.json`:
```c++
[
    {
        "title" : "Kafka on the Shore",
        "author" : "Haruki Murakami",
        "price" : 25.17
    },
    {
        "title" : "Women: A Novel",
        "author" : "Charles Bukowski",
        "price" : 12.0
    },
    {
        "title" : "Cutter's Way",
        "author" : "Ivan Passer"
    }
]
```
It consists of an array of book elements, each element is an object with members title, author, and price.

Read the JSON text into a `json` value,
```c++
std::ifstream is("books.json");
json books;
is >> books;
```
Loop through the book array elements, using a range-based for loop
```c++
for (const auto& book : books.elements())
{
    std::string author = book["author"].as<std::string>();
    std::string title = book["title"].as<std::string>();
    std::cout << author << ", " << title << std::endl;
}
```
or begin-end iterators
```c++
for (auto it = books.elements().begin(); 
     it != books.elements().end();
     ++it)
{
    std::string author = (*it)["author"].as<std::string>();
    std::string title = (*it)["title"].as<std::string>();
    std::cout << author << ", " << title << std::endl;
} 
```
or a traditional for loop
```c++
for (size_t i = 0; i < books.size(); ++i)
{
    json& book = books[i];
    std::string author = book["author"].as<std::string>();
    std::string title = book["title"].as<std::string>();
    std::cout << author << ", " << title << std::endl;
}
```
The output is
```
Haruki Murakami, Kafka on the Shore
Charles Bukowski, Women: A Novel
Ivan Passer, Cutter's Way
```

Loop through the members of the third book element, using a range-based for loop

```c++
for (const auto& member : books[2].members())
{
    std::cout << member.name() << "=" 
              << member.value() << std::endl;
}
```

or begin-end iterators:

```c++
for (auto it = books[2].members().begin(); 
     it != books[2].members().end();
     ++it)
{
    std::cout << (*it).name() << "=" 
              << (*it).value() << std::endl;
} 
```
The output is
```
author=Ivan Passer
title=Cutter's Way
```

Note that the third book, Cutter's Way, is missing a price.

You have a choice of object member accessors:

- `book["price"]` will throw `std::out_of_range` if there is no price
- `book.get_with_default("price","n/a")` will return the price converted to the default's data type, or "n/a" if there is no price.

So if you want to show "n/a" for the missing price, you can use this accessor
```c++
std::string price = book.get_with_default("price","n/a");
```
Or you can check if book has a member "price" with the method `count`, and output accordingly,
```c++
if (book.count("price") > 0)
{
    double price = book["price"].as<double>();
    std::cout << price;
}
else
{
    std::cout << "n/a";
}
```
### Constructing json values in C++

The default `json` constructor produces an empty json object. For example 
```c++
json image_sizing;
std::cout << image_sizing << std::endl;
```
produces
```json
{}
```
To construct a json object with members, take an empty json object and set some name-value pairs
```c++
image_sizing.set("Resize To Fit",true);  // a boolean 
image_sizing.set("Resize Unit", "pixels");  // a string
image_sizing.set("Resize What", "long_edge");  // a string
image_sizing.set("Dimension 1",9.84);  // a double
image_sizing.set("Dimension 2",json::null());  // a null value
```

Or, use an object initializer-list:
```c++
json file_settings = json::object{
    {"Image Format", "JPEG"},
    {"Color Space", "sRGB"},
    {"Limit File Size", true},
    {"Limit File Size To", 10000}
};
```

To construct a json array, initialize with the array type 
```c++
json color_spaces = json::array();
```
and add some elements
```c++
color_spaces.add("sRGB");
color_spaces.add("AdobeRGB");
color_spaces.add("ProPhoto RGB");
```

Or, use an array initializer-list:
```c++
json image_formats = json::array{"JPEG","PSD","TIFF","DNG"};
```

The `operator[]` provides another way for setting name-value pairs.
```c++
json file_export;
file_export["File Format Options"]["Color Spaces"] = 
    std::move(color_spaces);
file_export["File Format Options"]["Image Formats"] = 
    std::move(image_formats);
file_export["File Settings"] = std::move(file_settings);
file_export["Image Sizing"] = std::move(image_sizing);
```
Note that if 
```c++
file_export["File Format Options"]
```
doesn't exist, 
```c++
file_export["File Format Options"]["Color Spaces"] = 
    std::move(color_spaces)
```
creates `"File Format Options"` as an object and puts `"Color Spaces"` in it.

Serializing
```c++
std::cout << pretty_print(file_export) << std::endl;
```
produces
```json
{
    "File Format Options": {
        "Color Spaces": ["sRGB","AdobeRGB","ProPhoto RGB"],
        "Image Formats": ["JPEG","PSD","TIFF","DNG"]
    },
    "File Settings": {
        "Color Space": "sRGB",
        "Image Format": "JPEG",
        "Limit File Size": true,
        "Limit File Size To": 10000
    },
    "Image Sizing": {
        "Dimension 1": 9.84,
        "Dimension 2": null,
        "Resize To Fit": true,
        "Resize Unit": "pixels",
        "Resize What": "long_edge"
    }
}
```
### Converting to and from standard library containers

The jsoncons library supports converting to and from the standard library sequence and associative containers.

```c++
std::vector<int> v = {1,2,3,4};
json j(v);
std::cout << j << std::endl;
```
The output is
```json
[1,2,3,4]
```

```c++
json j = json::array{1,true,"last"};
auto d = j.as<std::deque<std::string>>();
for (auto x : d)
{
    std::cout << x << std::endl;
}
```
The output is
```
1
true
last
```

```c++
std::map<std::string,int> m = {{"one",1},{"two",2},{"three",3}};
json j(m);
std::cout << j << std::endl;
```
The output is
```json
{"one":1,"three":3,"two":2}
```

```c++
json j;
j["one"] = 1;
j["two"] = 2;
j["three"] = 3;

auto um = j.as<std::unordered_map<std::string,int>>();
for (const auto& x : um)
{
    std::cout << x.first << "=" << x.second << std::endl;
}
```
The output is
```
one=1
three=3
two=2
```

### Converting CSV files to json

Here is a sample CSV file (tasks.csv):
```
project_id, task_name, task_start, task_finish
4001,task1,01/01/2003,01/31/2003
4001,task2,02/01/2003,02/28/2003
4001,task3,03/01/2003,03/31/2003
4002,task1,04/01/2003,04/30/2003
4002,task2,05/01/2003,
```
You can read the `CSV` file into a `json` value with the `csv_reader`.
```c++
#include "jsoncons_ext/csv/csv_reader.hpp"

using jsoncons::csv::csv_parameters;
using jsoncons::csv::csv_reader;
using jsoncons::json_deserializer;

std::fstream is("tasks.csv");

json_deserializer handler;

csv_parameters params;
params.assume_header(true)
      .trim(true)
      .ignore_empty_values(true)
      .column_types({"integer","string","string","string"});

csv_reader reader(is,handler,params);
reader.read();
json val = handler.get_result();

std::cout << pretty_print(val) << std::endl;
```
The output is:
```json
[
    {
        "project_id":4001,
        "task_finish":"01/31/2003",
        "task_name":"task1",
        "task_start":"01/01/2003"
    },
    {
        "project_id":4001,
        "task_finish":"02/28/2003",
        "task_name":"task2",
        "task_start":"02/01/2003"
    },
    {
        "project_id":4001,
        "task_finish":"03/31/2003",
        "task_name":"task3",
        "task_start":"03/01/2003"
    },
    {
        "project_id":4002,
        "task_finish":"04/30/2003",
        "task_name":"task1",
        "task_start":"04/01/2003"
    },
    {
        "project_id":4002,
        "task_name":"task2",
        "task_start":"05/01/2003"
    }
]
```
There are a few things to note about the effect of the parameter settings.
- `assume_header` `true` tells the csv parser to parse the first line of the file for column names, which become object member names.
- `trim` `true` tells the parser to trim leading and trailing whitespace, in particular, to remove the leading whitespace in the column names.
- `ignore_empty_values` `true` causes the empty last value in the `task_finish` column to be omitted.
- The `column_types` setting specifies that column one ("project_id") contains integers and the remaining columns strings.

### Pretty print

The `pretty_print` function applies stylistic formatting to JSON text. For example

```c++
    json val;

    val["verts"] = json::array{1, 2, 3};
    val["normals"] = json::array{1, 0, 1};
    val["uvs"] = json::array{0, 0, 1, 1};

    std::cout << pretty_print(val) << std::endl;
```
produces

```json
{
    "normals": [1,0,1],
    "uvs": [0,0,1,1],
    "verts": [1,2,3]
}
```
By default, within objects, arrays of scalar values are displayed on the same line.

The `pretty_print` function takes an optional second parameter, [output_format](https://github.com/danielaparker/jsoncons/wiki/output_format), that allows custom formatting of output.
To display the array scalar values on a new line, set the `object_array_split_lines` property to `line_split_kind::new_line`. The code
```c++
output_format format;
format.object_array_split_lines(line_split_kind::new_line);
std::cout << pretty_print(val,format) << std::endl;
```
produces
```json
{
    "normals": [
        1,0,1
    ],
    "uvs": [
        0,0,1,1
    ],
    "verts": [
        1,2,3
    ]
}
```
To display the elements of array values on multiple lines, set the `object_array_split_lines` property to `line_split_kind::multi_line`. The code
```c++
output_format format;
format.object_array_split_lines(line_split_kind::multi_line);
std::cout << pretty_print(val,format) << std::endl;
```
produces
```json
{
    "normals": [
        1,
        0,
        1
    ],
    "uvs": [
        0,
        0,
        1,
        1
    ],
    "verts": [
        1,
        2,
        3
    ]
}
```

### JsonPath

[Stefan Goessner's JsonPath](http://goessner.net/articles/JsonPath/) is an XPATH inspired query language for selecting parts of a JSON structure.

Here is a sample JSON file (store.json):
```json
{ "store": {
    "book": [ 
      { "category": "reference",
        "author": "Nigel Rees",
        "title": "Sayings of the Century",
        "price": 8.95
      },
      { "category": "fiction",
        "author": "Evelyn Waugh",
        "title": "Sword of Honour",
        "price": 12.99
      },
      { "category": "fiction",
        "author": "Herman Melville",
        "title": "Moby Dick",
        "isbn": "0-553-21311-3",
        "price": 8.99
      },
      { "category": "fiction",
        "author": "J. R. R. Tolkien",
        "title": "The Lord of the Rings",
        "isbn": "0-395-19395-8",
        "price": 22.99
      }
    ]
  }
}
```
JsonPath examples:
```c++    
#include "jsoncons_ext/jsonpath/json_query.hpp"

using jsoncons::jsonpath::json_query;

json root = json::parse_file("store.json");

// (1) The authors of books that are cheaper than $10
json result = json_query(root,"$.store.book[?(@.price < 10)].author");
std::cout << result << std::endl;

// (2) The number of books
json result = json_query(root,"$..book.length");
std::cout << result << std::endl;

// (3) The third book
json result = json_query(root,"$..book[2]");
std::cout << pretty_print(result) << std::endl;

// (4) All books whose author's name starts with Evelyn
json result = json_query(root,"$.store.book[?(@.author =~ /Evelyn.*?/)]");
std::cout << pretty_print(result) << std::endl;

// (5) The titles of all books that have isbn number
json result = json_query(root,"$..book[?(@.isbn)]/title");
std::cout << result << std::endl;

// (6) All authors and titles of books
result = json_query(booklist, "$['store']['book']..['author','title']");
std::cout << pretty_print(result) << std::endl;
```
Result:
```json
(1) ["Nigel Rees","Herman Melville"]
(2) [4]
(3) [
        {
            "category": "fiction",
            "author": "Herman Melville",
            "title": "Moby Dick",
            "isbn": "0-553-21311-3",
            "price": 8.99
        }
    ]
(4) [
        {
            "category": "fiction",
            "author": "Evelyn Waugh",
            "title": "Sword of Honour",
            "price": 12.99
        }
    ]
(5) ["Moby Dick","The Lord of the Rings"] 
(6) 
[
    "Nigel Rees",
    "Sayings of the Century",
    "Evelyn Waugh",
    "Sword of Honour",
    "Herman Melville",
    "Moby Dick",
    "J. R. R. Tolkien",
    "The Lord of the Rings"
]
```
### About jsoncons::json

The [json](https://github.com/danielaparker/jsoncons/wiki/json) class is an instantiation of the `basic_json` class template that uses `char` as the character type
and sorts object members in alphabetically order.
```c++
typedef basic_json<char,
                   JsonTraits = json_traits<char>,
                   Allocator = std::allocator<char>> json;
```
If you prefer to retain the original insertion order, use [ojson](https://github.com/danielaparker/jsoncons/wiki/ojson) instead.

The library includes an instantiation for wide characters as well, [wjson](https://github.com/danielaparker/jsoncons/wiki/wjson)
```c++
typedef basic_json<wchar_t,
                   JsonTraits = json_traits<wchar_t>,
                   Allocator = std::allocator<wchar_t>> wjson;
```
If you prefer to retain the original insertion order, use [wojson](https://github.com/danielaparker/jsoncons/wiki/wojson) instead.

Note that the allocator type allows you to supply a custom allocator. For example, you can use the boost [fast_pool_allocator](http://www.boost.org/doc/libs/1_60_0/libs/pool/doc/html/boost/fast_pool_allocator.html):
```c++
#include <boost/pool/pool_alloc.hpp>
#include "jsoncons/json.hpp"

typedef jsoncons::basic_json<char, boost::fast_pool_allocator<char>> myjson;

myjson o;

o.set("FirstName","Joe");
o.set("LastName","Smith");
```
This results in a json value being constucted with all memory being allocated from the boost memory pool. (In this particular case there is no improvement in performance over `std::allocator`.)

Note that the underlying memory pool used by the `boost::fast_pool_allocator` is never freed. 

### Wide character support

jsoncons supports wide character strings and streams with `wjson` and `wjson_reader`. It supports `UTF16` encoding if `wchar_t` has size 2 (Windows) and `UTF32` encoding if `wchar_t` has size 4. You can construct a `wjson` value in exactly the same way as a `json` value, for instance:
```c++
using jsoncons::wjson;

wjson root;
root[L"field1"] = L"test";
root[L"field2"] = 3.9;
root[L"field3"] = true;

std::wcout << root << L"\n";
```
which prints
```c++
{"field1":"test","field2":3.9,"field3":true}
```
### ojson and wojson

The [ojson](https://github.com/danielaparker/jsoncons/wiki/ojson) ([wojson](https://github.com/danielaparker/jsoncons/wiki/wojson)) class is an instantiation of the `basic_json` class template that uses `char` (`wchar_t`) as the character type and keeps object members in their original order. 
```c++
ojson o = ojson::parse(R"(
{
    "street_number" : "100",
    "street_name" : "Queen St W",
    "city" : "Toronto",
    "country" : "Canada"
}
)");

std::cout << pretty_print(o) << std::endl;
```
The output is
```json
{
    "street_number": "100",
    "street_name": "Queen St W",
    "city": "Toronto",
    "country": "Canada"
}
```
Insert "postal_code" at end
```c++
o.set("postal_code", "M5H 2N2");

std::cout << pretty_print(o) << std::endl;
```
The output is
```json
{
    "street_number": "100",
    "street_name": "Queen St W",
    "city": "Toronto",
    "country": "Canada",
    "postal_code": "M5H 2N2"
}
```
Insert "province" before "country"
```c++
auto it = o.find("country");
o.set(it,"province","Ontario");

std::cout << pretty_print(o) << std::endl;
```
The output is
```json
{
    "street_number": "100",
    "street_name": "Queen St W",
    "city": "Toronto",
    "province": "Ontario",
    "country": "Canada",
    "postal_code": "M5H 2N2"
}
```

### Convert `json` to/from user defined type

In the json class, constructors, accessors and modifiers are templated, for example,
```c++
template <class T>
json(const T& val)

template<class T>
bool is() const

template<class T>
T as() const

template <class T>
basic_json& operator=(const T& val)

template <class T>
void add(const T& val)
```
The implementations of these functions and operators make use of the class template `json_type_traits`

This `json_type_traits` template is extensible, you, the user, can extend `json_type_traits` in the `jsoncons` namespace with your own types. 
For example, you can provide a specialization of `json_type_traits` for a `book` class, and then transfer book objects or
standard library collections of book objects to and from `json` values.

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
        static bool is(const Json& rhs) noexcept
        {
            return rhs.is_object() &&
                   rhs.count("author") == 1 && 
                   rhs.count("title") == 1 && 
                   rhs.count("price") == 1;
        }
        static book as(const Json& rhs)
        {
            book val;
            val.author = rhs["author"].as<std::string>();
            val.title = rhs["title"].as<std::string>();
            val.price = rhs["price"].as<double>();
            return val;
        }
        static Json to_json(const book& val)
        {
            Json j;
            j["author"] = val.author;
            j["title"] = val.title;
            j["price"] = val.price;
            return j;
        }
    };
};

int main()
{
    book book1{"Haruki Murakami", "Kafka on the Shore", 25.17};

    json j = book1;

    std::cout << "(1) " << std::boolalpha << j.is<book>() << "\n\n";

    std::cout << "(2) " << pretty_print(j) << "\n\n";

    book temp = j.as<book>();
    std::cout << "(3) " << temp.author << "," 
                        << temp.title << "," 
                        << temp.price << "\n\n";

    book book2{"Charles Bukowski", "Women: A Novel", 12.0};

    std::vector<book> book_array{book1, book2};

    json ja = book_array;

    std::cout << "(4) " << std::boolalpha 
                        << ja.is<std::vector<book>>() << "\n\n";

    std::cout << "(5)" << "\n";
    std::cout << pretty_print(ja) << "\n\n";

    auto book_list = ja.as<std::list<book>>();

    std::cout << "(6)" << std::endl;
    for (auto b : book_list)
    {
        std::cout << b.author << ", " 
                  << b.title << ", " 
                  << b.price << std::endl;
    }
}
``` 
The output is
```
(1) true

(2) {
    "author": "Haruki Murakami",
    "price": 25.17,
    "title": "Kafka on the Shore"
}

(3) Haruki Murakami,Kafka on the Shore,25.17

(4) true

(5)
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
    }
]

(6)
Haruki Murakami, Kafka on the Shore, 25.17
Charles Bukowski, Women: A Novel, 12
```

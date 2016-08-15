# jsoncons: a C++ library for json construction

jsoncons is a C++ library for the construction of [JavaScript Object Notation (JSON)](http://www.json.org). It supports parsing a JSON file or string into a `json` value, building a `json` value in C++ code, and serializing a `json` value to a file or string. It supports converting to and from the standard library sequence and associative containers. It also provides an API for generating json read and write events in code, somewhat analogously to SAX processing in the XML world. Consult the wiki for the latest [documentation and tutorials](https://github.com/danielaparker/jsoncons/wiki) and [roadmap](https://github.com/danielaparker/jsoncons/wiki/Roadmap). 

jsoncons uses some features that are new to C++ 11, particularly [move semantics](http://thbecker.net/articles/rvalue_references/section_02.html) and the [AllocatorAwareContainer](http://en.cppreference.com/w/cpp/concept/AllocatorAwareContainer) concept. It has been tested with MS VC++ 2013, MS VC++ 2015, GCC 4.8, GCC 4.9, and recent versions of clang. Note that `std::regex` isn't fully implemented in GCC 4.8., so `jsoncons_ext/jsonpath` regular expression filters aren't supported.

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
for (auto book : books.elements())
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
for (auto member : books[2].members())
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
- `book.get("price",default_value)` will return `default_value` if there is no price

So if you want to show "n/a" for the missing price, you can use this accessor
```c++
std::string price = book.get("price","n/a").as<std::string>();
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

To construct an empty json object, use the default constructor:
```c++
json image_sizing;
```
Serializing to standard out
```c++
std::cout << image_sizing << std::endl;
```
produces
```json
{}
```
Setting some name-value pairs,
```c++
image_sizing["resize_to_fit"] = true;  // a boolean 
image_sizing["resize_unit"]   = "pixels";  // a string
image_sizing["resize_what"]   = "long_edge";  // a string
image_sizing["dimension1"]    = 9.84;  // a double
image_sizing["dimension2"]    = json::null();  // a null
```
Serializing again, this time with pretty print,
```c++
std::cout << pretty_print(image_sizing) << std::endl;
```
produces
```json
{
    "dimension1":9.84,
    "dimension2":null,
    "resize_to_fit":true,
    "resize_unit":"pixels",
    "resize_what":"long_edge"
}
```
To construct a json array, use an initializer list:
```c++
json image_formats = json::array{"JPEG","PSD","TIFF","DNG"};
```
or initialize with the array type and use the `add` function:
```c++
json color_spaces{json::array()};
color_spaces.add("sRGB");
color_spaces.add("AdobeRGB");
color_spaces.add("ProPhoto RGB");
```
Combining these three 
```c++
json file_export;
file_export["image_sizing"] = std::move(image_sizing);
file_export["image_formats"] = std::move(image_formats);
file_export["color_spaces"] = std::move(color_spaces);
```
and serializing
```c++
std::cout << pretty_print(file_export) << std::endl;
```
produces
```json
{
    "color_spaces":
    ["sRGB","AdobeRGB","ProPhoto RGB"],
    "image_formats":
    ["JPEG","PSD","TIFF","DNG"],
    "image_sizing":
    {
        "dimension1":9.84,
        "dimension2":null,
        "resize_to_fit":true,
        "resize_unit":"pixels",
        "resize_what":"long_edge"
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
{"one": 1,"three": 3,"two": 2}
```

```c++
json j;
j["one"] = 1;
j["two"] = 2;
j["three"] = 3;

auto um = j.as<std::unordered_map<std::string,int>>();
for (auto x : um)
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
```
### About jsoncons::json

The `json` class is an instantiation of the `basic_json` class template that uses `char` as the character type
and sorts object members in alphabetically order.
```c++
typedef basic_json<char,
                   JsonTraits = json_traits<char>,
                   Allocator = std::allocator<char>> json;
```
If you prefer to retain the original insertion order, use `ojson` instead.

The library includes an instantiation for wide characters as well,
```c++
typedef basic_json<wchar_t,
                   JsonTraits = json_traits<wchar_t>,
                   Allocator = std::allocator<wchar_t>> wjson;
```
If you prefer to retain the original insertion order, use `wojson` instead.

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

The `ojson` (wojson) class is an instantiation of the `basic_json` class template that uses `char` (`wchar_t`) as the character type and keeps object members in their original order. 
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

### Type extensibility

In the json class, constructors, accessors and modifiers are templated, for example,
```c++
template <class T>
json(T val)

template<class T>
bool is() const

template<class T>
T as() const

template <class T>
basic_json& operator=(T val)

template <class T>
void add(T val)
```
The implementations of these functions and operators make use of the class template `json_type_traits`
```c++

template <class Json, class T, class Enable=void>
struct json_type_traits
{
    static const bool is_assignable = false;

    static bool is(const Json&)
    {
        return false;
    }

    static T as(const Json& rhs);

    static void assign(Json& lhs, T rhs);
};
```
This class template is extensible, you as a user can extend `json_type_traits` in the `jsoncons` namespace with your own types. 

You can, for example, extend `json_type_traits` to access and modify `json` structures with `boost::gregorian::date values`.

```c++
#include "jsoncons/json.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

namespace jsoncons 
{
    template <class Json>
    struct json_type_traits<Json,boost::gregorian::date>
    {
        static const bool is_assignable = true;

        static bool is(const Json& val) noexcept
        {
            if (!val.is_string())
            {
                return false;
            }
            std::string s = val.template as<std::string>();
            try
            {
                boost::gregorian::from_simple_string(s);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        static boost::gregorian::date as(const Json& val)
        {
            std::string s = val.template as<std::string>();
            return boost::gregorian::from_simple_string(s);
        }

        static void assign(Json& lhs, boost::gregorian::date val)
        {
            lhs.assign_string(to_iso_extended_string(val));
        }
    };
}
```
```c++
namespace my_ns
{
    using jsoncons::json;
    using boost::gregorian::date;

    json deal = json::parse(R"(
    {
        "Maturity":"2014-10-14",
        "ObservationDates": ["2014-02-14","2014-02-21"]
    }
    )");

    deal["ObservationDates"].add(date(2014,2,28));    

    date maturity = deal["Maturity"].as<date>();
    std::cout << "Maturity: " << maturity << std::endl << std::endl;

    std::cout << "Observation dates: " << std::endl << std::endl;

    for (auto observation_date: deal["ObservationDates"].elements())
    {
        std::cout << observation_date << std::endl;
    }
    std::cout << std::endl;
}
```
The output is
```
Maturity: 2014-Oct-14

Observation dates:

2014-Feb-14
2014-Feb-21
2014-Feb-28
``` 

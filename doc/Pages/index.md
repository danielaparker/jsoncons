# jsoncons: a C++ library for json construction

[Preliminaries](#A1)

[Reading JSON text from a file](#A2)

[Constructing json values in C++](#A3)

[Converting to and from standard library containers](#A4)

[Converting CSV files to json](#A5  )

[Pretty print](#A6)

[Filters](#A7)

[JSONPath](#A8)

[About jsoncons::json](#A9)

[Wide character support](#A10)

[ojson and wojson](#A11)

[Convert json to/from user defined type](#A12)

<div id="A1"/>
### Preliminaries

jsoncons is a C++, header-only library for constructing [JSON](http://www.json.org) and JSON-like
data formats (e.g. CBOR). It supports 

- Parsing JSON-like text or binary data into an unpacked representation
  that defines an interface for accessing and modifying that data.

- Serializing the unpacked representation into different JSON-like text or binary data.

- Converting from JSON-like text or binary data to C++ objects and back.

- Streaming JSON read and write events, somewhat analogously to SAX processing in the XML world. 

For more information, consult the latest [documentation](https://github.com/danielaparker/jsoncons/blob/master/doc/Home.md) and [roadmap](https://github.com/danielaparker/jsoncons/blob/master/doc/Roadmap.md). 

jsoncons uses some features that are new to C++ 11, particularly [move semantics](http://thbecker.net/articles/rvalue_references/section_02.html) and the [AllocatorAwareContainer](http://en.cppreference.com/w/cpp/concept/AllocatorAwareContainer) concept. It has been tested with MS VC++ 2015, GCC 4.8, GCC 4.9, and recent versions of clang. Note that `std::regex` isn't fully implemented in GCC 4.8., so `jsoncons_ext/jsonpath` regular expression filters aren't supported for that compiler.

The jsoncons library is header-only: it consists solely of header files containing templates and inline functions, and requires no separately-compiled library binaries when linking. It has no dependence on other libraries. 

To install the jsoncons library, download the zip file, extract the zipped files, and copy the directory `include/jsoncons` to your `include` directory. If you wish to use extensions, copy `include/jsoncons_ext` as well. 

The jsoncons classes and functions are in namespace `jsoncons`. You need to include the header file
```c++ 
#include <jsoncons/json.hpp>
```
and, for convenience,
```c++
using jsoncons::json;
```

<div id="A2"/>
### Reading JSON text from a file

Example file (`books.json`):
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
for (const auto& book : books.array_range())
{
    std::string author = book["author"].as<std::string>();
    std::string title = book["title"].as<std::string>();
    std::cout << author << ", " << title << std::endl;
}
```
or begin-end iterators
```c++
for (auto it = books.array_range().begin(); 
     it != books.array_range().end();
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
Output:
```
Haruki Murakami, Kafka on the Shore
Charles Bukowski, Women: A Novel
Ivan Passer, Cutter's Way
```

Loop through the members of the third book element, using a range-based for loop

```c++
for (const auto& member : books[2].object_range())
{
    std::cout << member.key() << "=" 
              << member.value() << std::endl;
}
```

or begin-end iterators:

```c++
for (auto it = books[2].object_range().begin(); 
     it != books[2].object_range().end();
     ++it)
{
    std::cout << (*it).key() << "=" 
              << (*it).value() << std::endl;
} 
```
Output:
```
author=Ivan Passer
title=Cutter's Way
```

Note that the third book, Cutter's Way, is missing a price.

You have a choice of object member accessors:

- `book["price"]` will throw `std::out_of_range` if there is no price
- `book.get_with_default("price",std::string("n/a"))` will return the price converted to the
   default's data type, `std::string`, or `"n/a"` if there is no price.

So if you want to show "n/a" for the missing price, you can use this accessor
```c++
std::string price = book.get_with_default("price","n/a");
```
Or you can check if book has a member "price" with the method `has_key`, and output accordingly,
```c++
if (book.has_key("price"))
{
    double price = book["price"].as<double>();
    std::cout << price;
}
else
{
    std::cout << "n/a";
}
```
<div id="A3"/>
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
image_sizing.insert_or_assign("Resize To Fit",true);  // a boolean 
image_sizing.insert_or_assign("Resize Unit", "pixels");  // a string
image_sizing.insert_or_assign("Resize What", "long_edge");  // a string
image_sizing.insert_or_assign("Dimension 1",9.84);  // a double
image_sizing.insert_or_assign("Dimension 2",json::null());  // a null value
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
color_spaces.push_back("sRGB");
color_spaces.push_back("AdobeRGB");
color_spaces.push_back("ProPhoto RGB");
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
Note that if `file_export["File Format Options"]` doesn't exist, 
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
<div id="A4"/>
### Converting to and from standard library containers

The jsoncons library supports converting to and from the standard library sequence and associative containers.

```c++
std::vector<int> v = {1,2,3,4};
json j(v);
std::cout << j << std::endl;
```
Output:
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
Output:
```
1
true
last
```

```c++
{% raw %}
std::map<std::string,int> m = {{"one",1},{"two",2},{"three",3}};
json j(m);
std::cout << j << std::endl;
{% endraw %}
```
Output:
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
Output:
```
one=1
three=3
two=2
```
<div id="A5"/>
### Converting CSV files to json

Example CSV file (tasks.csv):

```
project_id, task_name, task_start, task_finish
4001,task1,01/01/2003,01/31/2003
4001,task2,02/01/2003,02/28/2003
4001,task3,03/01/2003,03/31/2003
4002,task1,04/01/2003,04/30/2003
4002,task2,05/01/2003,
```

You can read the `CSV` file into a `json` value with the `decode_csv` function.

```c++
#include <fstream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_reader.hpp>
#include <jsoncons_ext/csv/csv_serializer.hpp>

using namespace jsoncons;
using namespace jsoncons::csv;

int main()
{
    std::ifstream is("input/tasks.csv");

    csv_serializing_options options;
    params.assume_header(true)
          .trim(true)
          .ignore_empty_values(true) 
          .column_types("integer,string,string,string");
    ojson tasks = decode_csv<ojson>(is, options);

    std::cout << "(1)\n" << pretty_print(tasks) << "\n\n";

    std::cout << "(2)\n";
    encode_csv(tasks, std::cout);
}
```
Output:
```json
(1)
[
    {
        "project_id": 4001,
        "task_name": "task1",
        "task_start": "01/01/2003",
        "task_finish": "01/31/2003"
    },
    {
        "project_id": 4001,
        "task_name": "task2",
        "task_start": "02/01/2003",
        "task_finish": "02/28/2003"
    },
    {
        "project_id": 4001,
        "task_name": "task3",
        "task_start": "03/01/2003",
        "task_finish": "03/31/2003"
    },
    {
        "project_id": 4002,
        "task_name": "task1",
        "task_start": "04/01/2003",
        "task_finish": "04/30/2003"
    },
    {
        "project_id": 4002,
        "task_name": "task2",
        "task_start": "05/01/2003"
    }
]
```
There are a few things to note about the effect of the parameter settings.
- `assume_header` `true` tells the csv parser to parse the first line of the file for column names, which become object member names.
- `trim` `true` tells the parser to trim leading and trailing whitespace, in particular, to remove the leading whitespace in the column names.
- `ignore_empty_values` `true` causes the empty last value in the `task_finish` column to be omitted.
- The `column_types` setting specifies that column one ("project_id") contains integers and the remaining columns strings.

<div id="A6"/>
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

The `pretty_print` function takes an optional second parameter, [json_serializing_options](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/json_serializing_options.md), that allows custom formatting of output.
To display the array scalar values on a new line, set the `object_array_split_lines` property to `line_split_kind::new_line`. The code
```c++
json_serializing_options options;
format.object_array_split_lines(line_split_kind::new_line);
std::cout << pretty_print(val,options) << std::endl;
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
json_serializing_options options;
format.object_array_split_lines(line_split_kind::multi_line);
std::cout << pretty_print(val,options) << std::endl;
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
<div id="A7"/>
### Filters

You can rename object member names with the built in filter [rename_object_member_filter](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/rename_object_member_filter.md)

```c++
#include <sstream>
#include <jsoncons/json.hpp>
#include <jsoncons/json_filter.hpp>

using namespace jsoncons;

int main()
{
    std::string s = R"({"first":1,"second":2,"fourth":3,"fifth":4})";    

    json_serializer serializer(std::cout);

    // Filters can be chained
    rename_object_member_filter filter2("fifth", "fourth", serializer);
    rename_object_member_filter filter1("fourth", "third", filter2);

    // A filter can be passed to any function that takes
    // a json_content_handler ...
    std::cout << "(1) ";
    std::istringstream is(s);
    json_reader reader(is, filter1);
    reader.read();
    std::cout << std::endl;

    // or a json_content_handler    
    std::cout << "(2) ";
    ojson j = ojson::parse(s);
    j.dump(filter1);
    std::cout << std::endl;
}
```
Output:
```json
(1) {"first":1,"second":2,"third":3,"fourth":4}
(2) {"first":1,"second":2,"third":3,"fourth":4}
```
Or define and use your own filters. See [json_filter](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/json_filter.md) for details.
<div id="A8"/>
### JSONPath

[Stefan Goessner's JSONPath](http://goessner.net/articles/JsonPath/) is an XPATH inspired query language for selecting parts of a JSON structure.

Example JSON file (booklist.json):
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
JSONPath examples:
```c++    
#include <jsoncons_ext/jsonpath/json_query.hpp>

using jsoncons::jsonpath::json_query;

std::ifstream is("./input/booklist.json");
json booklist = json::parse(is);

// The authors of books that are cheaper than $10
json result1 = json_query(booklist, "$.store.book[?(@.price < 10)].author");
std::cout << "(1) " << result1 << std::endl;

// The number of books
json result2 = json_query(booklist, "$..book.length");
std::cout << "(2) " << result2 << std::endl;

// The third book
json result3 = json_query(booklist, "$..book[2]");
std::cout << "(3)\n" << pretty_print(result3) << std::endl;

// All books whose author's name starts with Evelyn
json result4 = json_query(booklist, "$.store.book[?(@.author =~ /Evelyn.*?/)]");
std::cout << "(4)\n" << pretty_print(result4) << std::endl;

// The titles of all books that have isbn number
json result5 = json_query(booklist, "$..book[?(@.isbn)].title");
std::cout << "(5) " << result5 << std::endl;

// All authors and titles of books
json result6 = json_query(booklist, "$['store']['book']..['author','title']");
std::cout << "(6)\n" << pretty_print(result6) << std::endl;
```
Output:
```json
(1) ["Nigel Rees","Herman Melville"]
(2) [4]
(3)
[
    {
        "author": "Herman Melville",
        "category": "fiction",
        "isbn": "0-553-21311-3",
        "price": 8.99,
        "title": "Moby Dick"
    }
]
(4)
[
    {
        "author": "Evelyn Waugh",
        "category": "fiction",
        "price": 12.99,
        "title": "Sword of Honour"
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
<div id="A9"/>
### About jsoncons::json

The [json](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/json.md) class is an instantiation of the `basic_json` class template that uses `char` as the character type
and sorts object members in alphabetically order.
```c++
typedef basic_json<char,
                   ImplementationPolicy = sorted_policy,
                   Allocator = std::allocator<char>> json;
```
If you prefer to retain the original insertion order, use [ojson](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/ojson.md) instead.

The library includes an instantiation for wide characters as well, [wjson](https://github.com/danielaparker/jsoncons/blob/master/ref/doc/wjson.md)
```c++
typedef basic_json<wchar_t,
                   ImplementationPolicy = sorted_policy,
                   Allocator = std::allocator<wchar_t>> wjson;
```
If you prefer to retain the original insertion order, use [wojson](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/wojson.md) instead.

Note that the allocator type allows you to supply a custom allocator. For example, you can use the boost [fast_pool_allocator](http://www.boost.org/doc/libs/1_60_0/libs/pool/doc/html/boost/fast_pool_allocator.html):
```c++
#include <boost/pool/pool_alloc.hpp>
#include <jsoncons/json.hpp>

typedef jsoncons::basic_json<char, boost::fast_pool_allocator<char>> myjson;

myjson o;

o.insert_or_assign("FirstName","Joe");
o.insert_or_assign("LastName","Smith");
```
This results in a json value being constucted with all memory being allocated from the boost memory pool. (In this particular case there is no improvement in performance over `std::allocator`.)

Note that the underlying memory pool used by the `boost::fast_pool_allocator` is never freed. 

<div id="A10"/>
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
<div id="A11"/>
### ojson and wojson

The [ojson](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/ojson.md) ([wojson](https://github.com/danielaparker/jsoncons/blob/master/doc/ref/wojson.md)) class is an instantiation of the `basic_json` class template that uses `char` (`wchar_t`) as the character type and keeps object members in their original order. 
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
Output:
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
o.insert_or_assign("postal_code", "M5H 2N2");

std::cout << pretty_print(o) << std::endl;
```
Output:
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
o.insert_or_assign(it,"province","Ontario");

std::cout << pretty_print(o) << std::endl;
```
Output:
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

<div id="A12"/>
### Convert unpacked `json` value to user defined type and back

In the json class, constructors, accessors and modifiers are templated, for example,
```c++
template <class T>
json(const T& val)

template<class T, class... Args>
bool is(Args&&... args) const

template<class T, class... Args>
T as(Args&&... args) const

template <class T>
basic_json& operator=(const T& val)

template <class T>
void push_back(T&& val)

template <class T>
void insert_or_assign(const string_view_type& name, T&& val)
```
The implementations of these functions and operators make use of the class template `json_type_traits`

If you want to use the json constructor, `is<T>`, `as<T>`, `operator=`, `push_back`, `insert`, and `insert_or_assign` to access or modify with a new type, you need to show `json` how to interact with that type, by extending `json_type_traits` in the `jsoncons` namespace.

Note that the json::is<T>() and json::as<T>() functions accept template packs, which they forward to the `json_type_traits` `is` and `as` functions.
This allows user defined `json_type_traits` implementations to resolve, for instance, a name into a C++ object
looked up from a registry, as illustrated below.


```c++
#include <string>
#include <unordered_map>
#include <memory>
#include <jsoncons/json.hpp>

using namespace jsoncons;
using namespace jsoncons::literals;

class Employee
{
    std::string name_;
public:
    Employee(const std::string& name)
        : name_(name)
    {
    }
    virtual ~Employee() = default;
    const std::string& name() const
    {
        return name_;
    }
    virtual double calculatePay() const = 0;
};

class HourlyEmployee : public Employee
{
public:
    HourlyEmployee(const std::string& name)
        : Employee(name)
    {
    }
    double calculatePay() const override
    {
        return 10000;
    }
};

class CommissionedEmployee : public Employee
{
public:
    CommissionedEmployee(const std::string& name)
        : Employee(name)
    {
    }
    double calculatePay() const override
    {
        return 20000;
    }
};

class EmployeeRegistry
{
    typedef std::unordered_map<std::string,std::shared_ptr<Employee>> employee_map;
    employee_map employees_;
public:
    EmployeeRegistry()
    {
        employees_.try_emplace("John Smith",std::make_shared<HourlyEmployee>("John Smith"));
        employees_.try_emplace("Jane Doe",std::make_shared<CommissionedEmployee>("Jane Doe"));
    }

    bool contains(const std::string& name) const
    {
        return employees_.count(name) > 0; 
    }

    std::shared_ptr<Employee> get(const std::string& name) const
    {
        auto it = employees_.find(name);
        if (it == employees_.end())
        {
            throw std::runtime_error("Employee not found");
        }
        return it->second; 
    }
};

namespace jsoncons
{
    template<class Json>
    struct json_type_traits<Json, std::shared_ptr<Employee>>
    {
        static bool is(const Json& rhs, const EmployeeRegistry& registry) noexcept
        {
            return rhs.is_string() && registry.contains(rhs.as<std::string>());
        }
        static std::shared_ptr<Employee> as(const Json& rhs, 
                                            const EmployeeRegistry& registry)
        {
            return registry.get(rhs.as<std::string>());
        }
        static Json to_json(std::shared_ptr<Employee> val)
        {
            Json j(val->name());
            return j;
        }
    };
};

int main()
{
    json j = R"(
    {
        "EmployeeName" : "John Smith"
    }
    )"_json;

    EmployeeRegistry registry;

    std::shared_ptr<Employee> employee = j["EmployeeName"].as<std::shared_ptr<Employee>>(registry);

    std::cout << "(1) " << employee->name() << " => " 
              << employee->calculatePay() << std::endl;

    // j does not have a key "SalesRep", so get_with_default returns "Jane Doe"
    // The template parameter is explicitly specified as json, to return a json string
    // json::as is then applied to the returned json string  
    std::shared_ptr<Employee> salesRep = j.get_with_default<json>("SalesRep","Jane Doe")
                                          .as<std::shared_ptr<Employee>>(registry);

    std::cout << "(2) " << salesRep->name() << " => " 
              << salesRep->calculatePay() << std::endl;

    json j2;
    j2["EmployeeName"] = employee;
    j2["SalesRep"] = salesRep;

    std::cout << "(3)\n" << pretty_print(j2) << std::endl;
}
```

Output:

```json
(1) John Smith => 10000
(2) Jane Doe => 20000
(3)
{
    "EmployeeName": "John Smith",
    "SalesRep": "Jane Doe"
}
```



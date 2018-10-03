# Examples

### Parsing

[Parse JSON from a string](#A1)  
[Parse JSON from a file](#A2)  
[Validate JSON without incurring parse exceptions](#A3)  
[How to allow comments? How not to?](#A4)  
[Set a maximum nesting depth](#A5)  
[Prevent the alphabetic sort of the outputted JSON, retaining the original insertion order](#A6)  
[Parse a very large JSON file with json_stream_reader](#A7)  

### Serializing

[Serialize a json value to a string](#B1)  
[Serialize a json value to a stream](#B2)  
[Escape all non-ascii characters](#B3)  
[Replace the representation of NaN, Inf and -Inf when serializing. And when reading in again.](#B4)

### Constructing

[Construct a json object](#C1)  
[Construct a json array](#C2)  
[Insert a new value in an array at a specific position](#C3)  
[Create arrays of arrays of arrays of ...](#C4)  
[Merge two json objects](#C5)  

### Iterating

[Iterate over a json array](#D1)  
[Iterate over a json object](#D2)  

### Getters

[Use `string_view` to access the actual memory that's being used to hold a string](#E1)  
[Given a string in a `json` object that represents a decimal number, assign it to a double](#E2)  
[Look up a key, if found, return the value converted to type T, otherwise, return a default value of type T.](#E3)  
[Retrieve a value in a hierarchy of JSON objects](#E4)  

### Search and Replace

[Search for and repace an object member key](#F1)  
[Search for and replace a value](#F2)  

### Deserializing

<div id="A1"/> 

#### Parse JSON from a string

```
std::string s = R"({"first":1,"second":2,"fourth":3,"fifth":4})";    

json j = json::parse(s);
```

or

```c++
using namespace jsoncons::literals;

json j = R"(
{
    "StartDate" : "2017-03-01",
    "MaturityDate" : "2020-12-30"          
}
)"_json;
```

<div id="A2"/> 

#### Parse JSON from a file

```
std::ifstream is("myfile.json");    

json j = json::parse(is);
```

<div id="A3"/> 

#### Validate JSON without incurring parse exceptions
```c++
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
```
Output:
```
Expected name separator ':' on line 4 and column 20
```

<div id="A4"/> 

#### How to allow comments? How not to?

jsoncons, by default, accepts and ignores C-style comments

```c++
std::string s = R"(
{
    // Single line comments
    /*
        Multi line comments 
    */
}
)";

// Default
json j = json::parse(s);
std::cout << "(1) " << j << std::endl;

// Strict
try
{
    strict_parse_error_handler err_handler;
    json j = json::parse(s, err_handler);
}
catch (const parse_error& e)
{
    std::cout << "(2) " << e.what() << std::endl;
}
```
Output:
```
(1) {}
(2) Illegal comment at line 3 and column 10
```

<div id="A5"/> 

#### Set a maximum nesting depth

Like this,
```c++
std::string s = "[[[[[[[[[[[[[[[[[[[[[\"Too deep\"]]]]]]]]]]]]]]]]]]]]]";
try
{
    json_serializing_options options;
    options.max_nesting_depth(20);
    json j = json::parse(s, options);
}
catch (const parse_error& e)
{
     std::cout << e.what() << std::endl;
}
```
Output:
```
Maximum JSON depth exceeded at line 1 and column 21
```

<div id="A6"/> 

#### Prevent the alphabetic sort of the outputted JSON, retaining the original insertion order

Use `ojson` instead of `json` (or `wojson` instead of `wjson`) to retain the original insertion order. 

```c++
ojson j = ojson::parse(R"(
{
    "street_number" : "100",
    "street_name" : "Queen St W",
    "city" : "Toronto",
    "country" : "Canada"
}
)");
std::cout << "(1)\n" << pretty_print(j) << std::endl;

// Insert "postal_code" at end
j.insert_or_assign("postal_code", "M5H 2N2");
std::cout << "(2)\n" << pretty_print(j) << std::endl;

// Insert "province" before "country"
auto it = j.find("country");
j.insert_or_assign(it,"province","Ontario");
std::cout << "(3)\n" << pretty_print(j) << std::endl;
```
Output:
```
(1)
{
    "street_number": "100",
    "street_name": "Queen St W",
    "city": "Toronto",
    "country": "Canada"
}
(2)
{
    "street_number": "100",
    "street_name": "Queen St W",
    "city": "Toronto",
    "country": "Canada",
    "postal_code": "M5H 2N2"
}
(3)
{
    "street_number": "100",
    "street_name": "Queen St W",
    "city": "Toronto",
    "province": "Ontario",
    "country": "Canada",
    "postal_code": "M5H 2N2"
}
```

<div id="A7"/> 

### Parse a very large JSON file with json_stream_reader


A typical pull parsing application will repeatedly process the `current()` 
event and call `next()` to advance to the next event, until `done()` 
returns `true`.

The example JSON text, `book_catalog.json`, is used by the examples below.

```json
[ 
  { 
      "author" : "Haruki Murakami",
      "title" : "Hard-Boiled Wonderland and the End of the World",
      "isbn" : "0679743464",
      "publisher" : "Vintage",
      "date" : "1993-03-02",
      "price": 18.90
  },
  { 
      "author" : "Graham Greene",
      "title" : "The Comedians",
      "isbn" : "0099478374",
      "publisher" : "Vintage Classics",
      "date" : "2005-09-21",
      "price": 15.74
  }
]
```

#### Reading the JSON stream
```c++
std::ifstream is("book_catalog.json");

json_stream_reader reader(is);

for (; !reader.done(); reader.next())
{
    const auto& event = reader.current();
    switch (event.event_type())
    {
        case stream_event_type::begin_array:
            std::cout << "begin_array\n";
            break;
        case stream_event_type::end_array:
            std::cout << "end_array\n";
            break;
        case stream_event_type::begin_object:
            std::cout << "begin_object\n";
            break;
        case stream_event_type::end_object:
            std::cout << "end_object\n";
            break;
        case stream_event_type::name:
            // If underlying type is string, can return as string_view
            std::cout << "name: " << event.as<jsoncons::string_view>() << "\n";
            break;
        case stream_event_type::string_value:
            std::cout << "string_value: " << event.as<jsoncons::string_view>() << "\n";
            break;
        case stream_event_type::null_value:
            std::cout << "null_value: " << event.as<std::string>() << "\n";
            break;
        case stream_event_type::bool_value:
            std::cout << "bool_value: " << event.as<std::string>() << "\n";
            break;
        case stream_event_type::int64_value:
            std::cout << "int64_value: " << event.as<std::string>() << "\n";
            break;
        case stream_event_type::uint64_value:
            std::cout << "uint64_value: " << event.as<std::string>() << "\n";
            break;
        case stream_event_type::bignum_value:
            // Returned if 64 bit integer overflow
            std::cout << "bignum_value: " << event.as<std::string>() << "\n";
            break;
        case stream_event_type::double_value:
            // Return as string, could also use event.as<double>()
            std::cout << "double_value: " << event.as<std::string>() << "\n";
            break;
        default:
            std::cout << "Unhandled event type\n";
            break;
    }
}
```
Output:
```
begin_array
begin_object
name: author
string_value: Haruki Murakami
name: title
string_value: Hard-Boiled Wonderland and the End of the World
name: isbn
string_value: 0679743464
name: publisher
string_value: Vintage
name: date
string_value: 1993-03-02
name: price
double_value: 18.90
end_object
begin_object
name: author
string_value: Graham Greene
name: title
string_value: The Comedians
name: isbn
string_value: 0099478374
name: publisher
string_value: Vintage Classics
name: date
string_value: 2005-09-21
name: price
double_value: 15.74
end_object
end_array
```

#### Implementing a stream_filter

```c++
// A stream filter to filter out all events except name 
// and restrict name to "author"

class author_filter : public stream_filter
{
    bool accept_next_ = false;
public:
    bool accept(const stream_event& event) override
    {
        if (event.event_type()  == stream_event_type::name &&
            event.as<jsoncons::string_view>() == "author")
        {
            accept_next_ = true;
            return false;
        }
        else if (accept_next_)
        {
            accept_next_ = false;
            return true;
        }
        else
        {
            accept_next_ = false;
            return false;
        }
    }
};
```

#### Filtering the JSON stream

```c++
std::ifstream is("book_catalog.json");

author_filter filter;
json_stream_reader reader(is, filter);

for (; !reader.done(); reader.next())
{
    const auto& event = reader.current();
    switch (event.event_type())
    {
        case stream_event_type::string_value:
            std::cout << event.as<jsoncons::string_view>() << "\n";
            break;
    }
}
```
Output:
```
Haruki Murakami
Graham Greene
```

See [json_stream_reader](doc/ref/json_stream_reader.md) 

### Serializing

<div id="B1"/>

#### Serialize a json value to a string

```
std::string s;

j.dump(s); // compressed

j.dump(s, indenting::indent); // pretty print
```

<div id="B2"/>

#### Serialize a json value to a stream

```
j.dump(std::cout); // compressed

j.dump(std::cout, indenting::indent); // pretty print
```
or
```
std::cout << j << std::endl; // compressed

std::cout << pretty_print(j) << std::endl; // pretty print
```

<div id="B3"/>

#### Escape all non-ascii characters

```
json_serializing_options options;
options.escape_all_non_ascii(true);

j.dump(std::cout, options); // compressed

j.dump(std::cout, options, indenting::indent); // pretty print
```
or
```
std::cout << print(j, options) << std::endl; // compressed

std::cout << pretty_print(j, options) << std::endl; // pretty print
```

<div id="B4"/>

#### Replace the representation of NaN, Inf and -Inf when serializing. And when reading in again.

Set the serializing options for `nan_replacement`, `pos_inf_replacement` and `neg_inf_replacement` to distinct string values.

```c++
json j;
j["field1"] = std::sqrt(-1.0);
j["field2"] = 1.79e308 * 1000;
j["field3"] = -1.79e308 * 1000;

json_serializing_options options;
options.nan_replacement("\"NaN\"")
       .pos_inf_replacement("\"Inf\"")
       .neg_inf_replacement("\"-Inf\""); // defaults are null

std::ostringstream os;
os << pretty_print(j, options);

std::cout << "(1)\n" << os.str() << std::endl;

json j2 = json::parse(os.str(),options);

std::cout << "\n(2) " << j2["field1"].as<double>() << std::endl;
std::cout << "(3) " << j2["field2"].as<double>() << std::endl;
std::cout << "(4) " << j2["field3"].as<double>() << std::endl;

std::cout << "\n(5)\n" << pretty_print(j2,options) << std::endl;
```

Output:
```json
(1)
{
    "field1": "NaN",
    "field2": "Inf",
    "field3": "-Inf"
}

(2) nan
(3) inf
(4) -inf

(5)
{
    "field1": "NaN",
    "field2": "Inf",
    "field3": "-Inf"
}
```

### Constructing

<div id="C1"/>

#### Construct a json object

Start with an empty json object and insert some name-value pairs,

```c++
json image_sizing;
image_sizing.insert_or_assign("Resize To Fit",true);  // a boolean 
image_sizing.insert_or_assign("Resize Unit", "pixels");  // a string
image_sizing.insert_or_assign("Resize What", "long_edge");  // a string
image_sizing.insert_or_assign("Dimension 1",9.84);  // a double
image_sizing.insert_or_assign("Dimension 2",json::null());  // a null value
```

or use an object initializer-list,

```c++
json file_settings = json::object{
    {"Image Format", "JPEG"},
    {"Color Space", "sRGB"},
    {"Limit File Size", true},
    {"Limit File Size To", 10000}
};
```

<div id="C2"/>

#### Construct a json array

Insert values into a json array,

```c++
json color_spaces = json::array(); // an empty array
color_spaces.push_back("sRGB");
color_spaces.push_back("AdobeRGB");
color_spaces.push_back("ProPhoto RGB");
```

or use an array initializer-list,
```c++
json image_formats = json::array{"JPEG","PSD","TIFF","DNG"};
```

<div id="C3"/>

#### Insert a new value in an array at a specific position

```c++
json cities = json::array(); // an empty array
cities.push_back("Toronto");  
cities.push_back("Vancouver");
// Insert "Montreal" at beginning of array
cities.insert(cities.array_range().begin(),"Montreal");  

std::cout << cities << std::endl;
```
Output:
```
["Montreal","Toronto","Vancouver"]
```

<div id="C4"/>

#### Create arrays of arrays of arrays of ...

Like this:

```c++
json j = json::make_array<3>(4, 3, 2, 0.0);
double val = 1.0;
for (size_t i = 0; i < a.size(); ++i)
{
    for (size_t j = 0; j < j[i].size(); ++j)
    {
        for (size_t k = 0; k < j[i][j].size(); ++k)
        {
            j[i][j][k] = val;
            val += 1.0;
        }
    }
}
std::cout << pretty_print(j) << std::endl;
```
Output:
```json
[
    [
        [1.0,2.0],
        [3.0,4.0],
        [5.0,6.0]
    ],
    [
        [7.0,8.0],
        [9.0,10.0],
        [11.0,12.0]
    ],
    [
        [13.0,14.0],
        [15.0,16.0],
        [17.0,18.0]
    ],
    [
        [19.0,20.0],
        [21.0,22.0],
        [23.0,24.0]
    ]
]
```

<div id="C5"/>

#### Merge two json objects

[json::merge](ref/json/merge.md) inserts another json object's key-value pairs into a json object,
unless they already exist with an equivalent key.

[json::merge_or_update](ref/json/merge_or_update.md) inserts another json object's key-value pairs 
into a json object, or assigns them if they already exist.

The `merge` and `merge_or_update` functions perform only a one-level-deep shallow merge,
not a deep merge of nested objects.

```c++
json another = json::parse(R"(
{
    "a" : "2",
    "c" : [4,5,6]
}
)");

json j = json::parse(R"(
{
    "a" : "1",
    "b" : [1,2,3]
}
)");

j.merge(std::move(another));
std::cout << pretty_print(j) << std::endl;
```
Output:
```json
{
    "a": "1",
    "b": [1,2,3],
    "c": [4,5,6]
}
```

### Iterating

<div id="D1"/>

#### Iterate over a json array

```c++
json j = json::array{1,2,3,4};

for (auto val : j.array_range())
{
    std::cout << val << std::endl;
}
```

<div id="D2"/>

#### Iterate over a json object

```c++
json j = json::object{
    {"author", "Haruki Murakami"},
    {"title", "Kafka on the Shore"},
    {"price", 25.17}
};

for (const auto& member : j.object_range())
{
    std::cout << member.key() << "=" 
              << member.value() << std::endl;
}
```

### Getters

<div id="E1"/>

#### Use `string_view` to access the actual memory that's being used to hold a string

You can use `j.as<jsoncons::string_view>()`, e.g.
```c++
json j = json::parse("\"Hello World\"");
auto sv = j.as<jsoncons::string_view>();
```
`jsoncons::string_view` supports the member functions of `std::string_view`, including `data()` and `size()`. 

If your compiler supports `std::string_view`, you can also use `j.as<std::string_view>()`.

<div id="E2"/>

#### Given a string in a `json` object that represents a decimal number, assign it to a double

```c++
json j = json::object{
    {"price", "25.17"}
};

double price = j["price"].as<double>();
```

<div id="E3"/>

#### Look up a key, if found, return the value converted to type T, otherwise, return a default value of type T.
 
```c++
json j = json::object{
    {"price", "25.17"}
};

double price = j.get_with_default("price", 25.00); // returns 25.17

double sale_price = j.get_with_default("sale_price", 22.0); // returns 22.0
```

<div id="E4"/>
 
#### Retrieve a value in a hierarchy of JSON objects

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>

int main()
{
    json j = json::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum.example.com",
           "assertion": "is-good",
           "rated": "sk",
           "rating": 0.90
         }
       ]
    }
    )");

    // Using index or `at` accessors
    std::string result1 = j["reputons"][0]["rated"].as<std::string>();
    std::cout << "(1) " << result1 << std::endl;
    std::string result2 = j.at("reputons").at(0).at("rated").as<std::string>();
    std::cout << "(2) " << result2 << std::endl;

    // Using JSON Pointer
    std::string result3 = jsonpointer::get(j, "/reputons/0/rated").as<std::string>();
    std::cout << "(3) " << result3 << std::endl;

    // Using JSONPath
    json result4 = jsonpath::json_query(j, "$.reputons.0.rated");
    if (result4.size() > 0)
    {
        std::cout << "(4) " << result4[0].as<std::string>() << std::endl;
    }
    json result5 = jsonpath::json_query(j, "$..0.rated");
    if (result5.size() > 0)
    {
        std::cout << "(5) " << result5[0].as<std::string>() << std::endl;
    }
}
```

### Search and Replace
 
<div id="F1"/>

#### Search for and repace an object member key

You can rename object members with the built in filter [rename_object_member_filter](ref/rename_object_member_filter.md)

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

    // A filter can be passed to any function that takes a json_content_handler ...
    std::cout << "(1) ";
    std::istringstream is(s);
    json_reader reader(is, filter1);
    reader.read();
    std::cout << std::endl;

    // or 
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
 
<div id="F2"/>

#### Search for and replace a value

You can use [json_replace](ref/jsonpath/json_replace.md) in the `jsonpath` extension

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>

using namespace jsoncons;

int main()
{
    json j = json::parse(R"(
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
              }
            ]
          }
        }
    )");

    // Change the price of "Moby Dick" from $8.99 to $10
    jsonpath::json_replace(j,"$.store.book[?(@.isbn == '0-553-21311-3')].price",10.0);
    std::cout << pretty_print(booklist) << std::endl;
}


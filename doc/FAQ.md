## FAQ

### Deserialize

#### How can I parse JSON from a string?

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

#### How can I parse JSON from a file?

```
std::ifstream is("myfile.json");    

json j = json::parse(is);
```

#### How can I validate JSON without incurring parse exceptions?
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

#### What if I want to allow comments? What if I don't?

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

#### Can I set a maximum nesting depth?

Yes, like this,
```
try
{
    std::ifstream is("myfile.json");    
    json_serializing_options options;
    options.max_nesting_depth(20);
    json::parse(is, options);
}
catch (const parse_error& e)
{
     std::cout e.what() << std::endl;
}
```
### Serialize 

#### How can I serialize a json value to a string?

```
std::string s;

j.dump(s); // compressed

j.dump(s, indenting::indent); // pretty print
```

#### How can I serialize a json value to a stream?

```
j.dump(std::cout); // compressed

j.dump(std::cout, indenting::indent); // pretty print
```
or
```
std::cout << j << std::endl; // compressed

std::cout << pretty_print(j) << std::endl; // pretty print
```

### Access

#### How can I iterate over a json array?

```c++
json j = json::array{1,2,3,4};

for (auto val : j.array_range())
{
    std::cout << val << std::endl;
}
```

#### How can I iterate over a json object?

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

### Construct

#### How do I create arrays of arrays of arrays of ...

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

#### Is it possible to merge two json objects?

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

### Search and Replace

#### Is there a way to search for and repace an object member key?

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

#### Is there a way to search for and replace a value?

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


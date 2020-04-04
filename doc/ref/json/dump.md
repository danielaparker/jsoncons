### `jsoncons::basic_json::dump`

```c++
    template <class SAllocator=std::allocator<char_type>>
    void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s, 
              indenting line_indent = indenting::no_indent) const; // (1)

    template <class SAllocator=std::allocator<char_type>>
    void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s,
              const basic_json_encode_options<char_type>& options, 
              indenting line_indent = indenting::no_indent) const; // (2)

    void dump(std::basic_ostream<char_type>& os, 
              indenting line_indent = indenting::no_indent) const; // (3)

    void dump(std::basic_ostream<char_type>& os, 
              const basic_json_encode_options<char_type>& options, 
              indenting line_indent = indenting::no_indent) const; // (4)

    void dump(basic_json_visitor<char_type>& visitor) const; // (5)

    template <class SAllocator=std::allocator<char_type>>
    void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s,
              const basic_json_encode_options<char_type>& options, 
              indenting line_indent,
              std::error_code& ec) const; // (6)

    template <class SAllocator=std::allocator<char_type>>
    void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s, 
              indenting line_indent,
              std::error_code& ec) const; // (7)

    void dump(std::basic_ostream<char_type>& os, 
              const basic_json_encode_options<char_type>& options, 
              indenting line_indent,
              std::error_code& ec) const; // (8)

    void dump(std::basic_ostream<char_type>& os, 
              indenting line_indent,
              std::error_code& ec) const; // (9)

    void dump(basic_json_visitor<char_type>& visitor, 
              std::error_code& ec) const; // (10)
```

(1) Dumps a json value to a string using the specified [indenting](../indenting.md).

(2) Dumps a json value to a string using the specified [encoding options](../basic_json_options.md)
    and [indenting](../indenting.md).

(3) Dumps a json value to an output stream using the specified [indenting](../indenting.md).

(4) Dumps a json value to an output stream using the specified [encoding options](../basic_json_options.md)
    and [indenting](../indenting.md).

(5) Dumps a json value to the specified [visitor](../basic_json_visitor.md).

(6) - (10) Same as (1)-(5), except set `ec` on serialization errors. 

#### Exceptions

The overloads that do not take a `std::error_code&` parameter throw a
[ser_error](ser_error.md) on serialization errors, constructed with the error code as the error code argument
and line and column from the `context`. 

The overloads that take a `std::error_code&` parameter set it to the error code on serialization errors.

### Examples

#### Dump json value to csv file

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv.hpp>

using namespace jsoncons;

int main()
{
    const json books = json::parse(R"(
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
    )");

    csv_options options;
    options.column_names("author,title,price");

    csv_stream_encoder encoder(std::cout, options);

    books.dump(encoder);
}
```

Output:

```csv
author,title,price
Haruki Murakami,Kafka on the Shore,25.17
Charles Bukowski,Women: A Novel,12.0
Ivan Passer,Cutter's Way,
```

#### Dump json content into a larger document

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

    json_stream_encoder encoder(std::cout); // pretty print
    encoder.begin_array();
    for (const auto& book : some_books.array_range())
    {
        book.dump(encoder);
    }
    for (const auto& book : more_books.array_range())
    {
        book.dump(encoder);
    }
    encoder.end_array();
    encoder.flush();
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

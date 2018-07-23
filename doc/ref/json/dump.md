### `jsoncons::json::dump`, `jsoncons::json::dump_fragment`

```c++
template <class SAllocator>
void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s) const; // (1)

template <class SAllocator>
void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s, 
          indenting line_indent) const; // (2)

template <class SAllocator>
void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s, 
          const json_serializing_options& options) const; // (3)

template <class SAllocator>
void dump(std::basic_string<char_type,char_traits_type,SAllocator>& s,
          const basic_json_serializing_options<char_type>& options, 
          indenting line_indent) const; // (4)

void dump(std::ostream& os) const; // (5)

void dump(std::ostream<CharT> os, indenting line_indent) const; // (6)

void dump(std::ostream<CharT> os, const json_serializing_options& options) const; // (7)

void dump(std::ostream<CharT> os, const json_serializing_options& options, indenting line_indent) const; // (8)

void dump(basic_json_content_handler<char_type>& content_handler) const; // (9)

void dump_fragment(json_content_handler& handler) const; // (10)
```

(1) Dumps json value to string using default json_serializing_options.

(2) Dumps json value to string using default serialization options and the specified [indenting](../indenting.md). 

(3) Dumps json value to string using specified [json_serializing_options](../json_serializing_options.md). 

(4) Dumps json value to string using the specified [json_serializing_options](../json_serializing_options.md) and [indenting](../indenting.md). 

(5) Dumps json value to stream with default serialization options. 

(6) Dumps json value to stream using default serialization options and the specified [indenting](../indenting.md). 

(7) Dumps json value to stream using specified [json_serializing_options](../json_serializing_options.md). 

(8) Dumps json value to stream using the specified [json_serializing_options](../json_serializing_options.md) and [indenting](../indenting.md). 

(9) Calls `begin_document()` on [json_content_handler](../json_content_handler.md), emits json value to the [json_content_handler](../json_content_handler.md), and calls `end_document()` on [json_content_handler](../json_content_handler.md). 

(10) Emits json value to the [json_content_handler](../json_content_handler.md) (does not call `begin_document()` or `end_document()`.)

### Examples

#### Dump json value to csv file

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_serializer.hpp>

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

    csv_serializing_options options;
    params.column_names("author,title,price");

    csv_serializer serializer(std::cout, options);

    books.dump(serializer);
}
```

Output:

```csv
author,title,price
Haruki Murakami,Kafka on the Shore,25.17
Charles Bukowski,Women: A Novel,12.0
Ivan Passer,Cutter's Way,
```

#### Dump json fragments into a larger document

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

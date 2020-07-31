### `jsoncons::basic_json::dump`

```c++
template <Container>
void dump(Container& s,
          const basic_json_encode_options<char_type>& options 
              = basic_json_encode_options<char_type>()) const; // (1)

void dump(std::basic_ostream<char_type>& os, 
          const basic_json_encode_options<char_type>& options 
              = basic_json_encode_options<char_type>()) const; // (2)

template <Container>
void dump_pretty(Container& s,
                 const basic_json_encode_options<char_type>& options 
                     = basic_json_encode_options<char_type>()) const; // (3)

void dump_pretty(std::basic_ostream<char_type>& os, 
                 const basic_json_encode_options<char_type>& options 
                     = basic_json_encode_options<char_type>()) const; // (4)

void dump(basic_json_visitor<char_type>& visitor) const; // (5)

template <Container>
void dump(Container& s, indenting line_indent) const; // (6)

template <Container>
void dump(Container& s,
          const basic_json_encode_options<char_type>& options, 
          indenting line_indent) const; // (7)

void dump(std::basic_ostream<char_type>& os, indenting line_indent) const; // (13)

void dump(std::basic_ostream<char_type>& os, 
          const basic_json_encode_options<char_type>& options, 
          indenting line_indent) const; // (8)
)
```

(1) Dumps a json value to a character container with "minified" output.

(2) Dumps a json value to an output stream with "minified" output.

(3) Dumps a json value to a character container with prettified output including spaces and line breaks.

(4) Dumps a json value to an output stream with prettified output including spaces and line breaks.

(5) Dumps a json value to the specified [visitor](../basic_json_visitor.md).

(6) - (8) support the pre 0.155.0 `dump` overloads with the `jsoncons::indenting` parameter.
New code should use the `dump_pretty` overloads instead.

#### Exceptions

Throws [ser_error](ser_error.md) if there is a serialization error. 

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

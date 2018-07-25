### jsoncons::csv::encode_csv

Serializes a json value as CSV to a string or an output stream.

#### Header
```c++
#include <jsoncons_ext/csv/csv_serializer.hpp>

template <class Json>
void encode_csv(const Json& j, 
                std::basic_ostream<typename Json::char_type>& os); // (1)

template <class Json>
void encode_csv(const Json& j, 
                std::basic_ostream<typename Json::char_type>& os, 
                const basic_csv_serializing_options<typename Json::char_type>& options); // (2)

template <class Json>
void encode_csv(const Json& j, 
                std::basic_string<typename Json::char_type>& s); // (3)

template <class Json>
void encode_csv(const Json& j, 
                std::basic_string<typename Json::char_type>& s, 
                const basic_csv_serializing_options<typename Json::char_type>& options); // (4)
```

(1) Serializes json value as CSV to an output stream using default [serializing options](csv_serializing_options.md)

(2) Serializes json value as CSV to an output stream using specified [serializing options](csv_serializing_options.md)

(3) Serializes json value as CSV to a string using default [serializing options](csv_serializing_options.md)

(4) Serializes json value as CSV to a string using specified [serializing options](csv_serializing_options.md)

### Examples

#### Write a json value to a CSV output stream

```c++
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_serializer.hpp>

using namespace jsoncons;
using namespace jsoncons::csv;

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

    encode_csv(books, std::cout);
}
```
Output:
```json
author,price,title
Haruki Murakami,00,Kafka on the Shore
Charles Bukowski,00,Women: A Novel
Ivan Passer,,Cutter's Way
```



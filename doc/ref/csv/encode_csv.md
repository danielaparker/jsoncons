### jsoncons::csv::encode_csv

Serializes a json value as CSV to a string or an output stream.

#### Header
```c++
#include <jsoncons_ext/csv/csv_encoder.hpp>

template <class T, class CharT>
void encode_csv(const T& j, 
                std::basic_ostream<CharT>& os, 
                const basic_csv_options<CharT>& options = basic_csv_options<CharT>()); // (1)

template <class T,class CharT>
void encode_csv(const T& j, 
                std::basic_string<CharT>& s, 
                const basic_csv_options<CharT>& options = basic_csv_options<CharT>()); // (2)
```

(1) Serializes json value as CSV to an output stream using specified [serializing options](csv_options.md)

(2) Serializes json value as CSV to a string using specified [serializing options](csv_options.md)

### Examples

#### Write a json value to a CSV output stream

```c++
#include <iostream>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv_encoder.hpp>

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



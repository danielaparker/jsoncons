### jsoncons::csv::encode_csv

Serializes a json value to csv output.

#### Header
```c++
#include <jsoncons_ext/csv/csv_serializer.hpp>

template <class Json>
void encode_csv(const Json& j, 
                std::basic_ostream<typename Json::char_type>& os); // (1)

template <class Json>
void encode_csv(const Json& j, 
                std::basic_ostream<typename Json::char_type>& os, 
                const basic_csv_parameters<typename Json::char_type>& params); // (2)
```

(1) Encodes json value to csv using default [parameters](csv_parameters)

(2) Encodes json value to csv using specified [parameters](csv_parameters)

### Examples

#### Encode a json value to csv output

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



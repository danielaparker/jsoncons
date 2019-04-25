### ubjson extension

The ubjson extension implements decode from and encode to the [Universal Binary JSON Specification](http://ubjson.org/) data format.

[decode_ubjson](decode_ubjson.md)

[encode_ubjson](encode_ubjson.md)

[ubjson_encoder](ubjson_encoder.md)

#### jsoncons-ubjson mappings

jsoncons data item|jsoncons tag|UBJSON data item
--------------|------------------|---------------
null          |                  | null
bool          |                  | true or false
int64         |                  | uint8_t or integer
uint64        |                  | uint8_t or integer
double        |                  | float 32 or float 64
string        |                  | string
string        | big_integer      | high precision number type
string        | big_decimal      | high precision number type
byte_string   |                  | array of uint8_t
array         |                  | array 
object        |                  | object

### Examples

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/ubjson/ubjson.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>

using namespace jsoncons;

int main()
{
    ojson j1 = ojson::parse(R"(
    {
       "application": "hiking",
       "reputons": [
       {
           "rater": "HikingAsylum.example.com",
           "assertion": "strong-hiker",
           "rated": "Marilyn C",
           "rating": 0.90
         }
       ]
    }
    )");

    // Encode a basic_json value to a UBJSON value
    std::vector<uint8_t> data;
    ubjson::encode_ubjson(j1, data);

    // Decode a UBJSON value to a basic_json value
    ojson j2 = ubjson::decode_ubjson<ojson>(data);
    std::cout << "(1)\n" << pretty_print(j2) << "\n\n";

    // Accessing the data items 

    const ojson& reputons = j2["reputons"];

    std::cout << "(2)\n";
    for (auto element : reputons.array_range())
    {
        std::cout << element.at("rated").as<std::string>() << ", ";
        std::cout << element.at("rating").as<double>() << "\n";
    }
    std::cout << std::endl;

    // Get a UBJSON value for a nested data item with jsonpointer
    std::error_code ec;
    const auto& rated = jsonpointer::get(j2, "/reputons/0/rated", ec);
    if (!ec)
    {
        std::cout << "(3) " << rated.as_string() << "\n";
    }

    std::cout << std::endl;
}
```
Output:
```
(1)
{
    "application": "hiking",
    "reputons": [
        {
            "rater": "HikingAsylum.example.com",
            "assertion": "strong-hiker",
            "rated": "Marilyn C",
            "rating": 0.9
        }
    ]
}

(2)
Marilyn C, 0.9

(3) Marilyn C
```





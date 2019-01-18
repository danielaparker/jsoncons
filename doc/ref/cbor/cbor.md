### cbor extension

The cbor extension implements decode from and encode to the IETF standard [Concise Binary Object Representation (CBOR)](http://cbor.io/).
It supports decoding a packed CBOR value to an unpacked (json) value and
encoding an unpacked (json) value to a packed CBOR value.

[decode_cbor](decode_cbor.md)

[encode_cbor](encode_cbor.md)

[cbor_serializer](cbor_serializer.md)

#### jsoncons - CBOR mappings

jsoncons data item|jsoncons tag|CBOR data item|CBOR tag
--------------|------------------|---------------|--------
null          |                  | null |&#160;
null          | undefined        | undefined |&#160;
bool          |                  | true or false |&#160;
int64         |                  | unsigned or negative integer |&#160;
int64         | timestamp        | unsigned or negative integer | 1 (epoch-based date/time)
uint64        |                  | unsigned integer |&#160;
uint64        | timestamp        | unsigned integer | 1 (epoch-based date/time)
double        |                  | half-precision float, float, or double |&#160;
double        | timestamp        | double | 1 (epoch-based date/time)
string        |                  | string |&#160;
string        | big_integer      | byte string | 2 (positive bignum) or 2 (negative bignum)  
string        | big_decimal      | array | 4 (decimal fraction)
string        | date_time        | string | 0 (date/time string) 
string        | uri              | string | 32 (uri)
string        | base64url        | string | 33 (base64url)
string        | base64           | string | 34 (base64)
byte_string   |                  | byte string |&#160;
byte_string   | base64url        | byte string | 21 (Expected conversion to base64url encoding)
byte_string   | base64           | byte string | 22 (Expected conversion to base64 encoding)
byte_string   | base16           | byte string | 23 (Expected conversion to base16 encoding)
array         |                  | array |&#160;
object        |                  | map |&#160;

### Examples

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
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
           "assertion": "is-good",
           "rated": "sk",
           "rating": 0.90
         }
       ]
    }
    )");

    // Encoding an unpacked (json) value to a packed CBOR value
    std::vector<uint8_t> data;
    cbor::encode_cbor(j1, data);

    // Decoding a packed CBOR value to an unpacked (json) value
    ojson j2 = cbor::decode_cbor<ojson>(data);
    std::cout << "(1)\n" << pretty_print(j2) << "\n\n";

    // Accessing the data items 

    const ojson& reputons = j2["reputons"];

    std::cout << "(2)\n";
    for (auto element : reputons.array_range())
    {
        std::cout << element.at("rated").as_string() << ", ";
        std::cout << element.at("rating").as_double() << "\n";
    }
    std::cout << std::endl;

    // Querying a packed CBOR value for a nested data item with jsonpointer
    std::error_code ec;
    auto const& rated = jsonpointer::get(j2, "/reputons/0/rated", ec);
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
            "assertion": "is-good",
            "rated": "sk",
            "rating": 0.9
        }
    ]
}

(2)
sk, 0.9

(3) sk
```


## cbor extension

The cbor extension implements decode from and encode to the IETF standard [Concise Binary Object Representation (CBOR)](http://cbor.io/).
You can either parse into or serialize from a variant-like structure, [basic_json](../basic_json.md), or your own
data structures, using [json_type_traits](../json_type_traits.md).

[decode_cbor](decode_cbor.md)

[basic_cbor_cursor](cbor_cursor.md)

[encode_cbor](encode_cbor.md)

[basic_cbor_encoder](basic_cbor_encoder.md)

[cbor_options](cbor_options.md)

### Tag handling and extensions

All tags not explicitly mentioned below are ignored.

0 (standard date/time string)  
CBOR standard date/time strings are decoded into jsoncons strings tagged with `semantic_tag::datetime`.
jsoncons strings tagged with `semantic_tag::datetime` are encoded into CBOR standard date/time strings.

1 (epoch time)  
CBOR epoch times are decoded into jsoncons int64_t, uint64_t and double and tagged with `semantic_tag::timestamp`. 
jsoncons int64_t, uint64_t and double tagged with `semantic_tag::timestamp` are encoded into CBOR epoch time.

2,3 (positive and negative bignum)  
CBOR positive and negative bignums are decoded into jsoncons strings and tagged with `semantic_tag::bigint`.
jsoncons strings tagged with `semantic_tag::bigint` are encoded into CBOR positive or negative bignums.

4 (decimal fratction)  
CBOR decimal fractions are decoded into jsoncons strings tagged with `semantic_tag::bigdec`.
jsoncons strings tagged with `semantic_tag::bigdec` are encoded into CBOR decimal fractions.

5 (bigfloat)  
CBOR bigfloats are decoded into a jsoncons string that consists of the following parts

- (optional) minus sign
- 0x
- nonempty sequence of hexadecimal digits (defines mantissa)
- p followed with optional minus or plus sign and nonempty sequence of hexadecimal digits (defines base-2 exponent)

and tagged with `semantic_tag::bigfloat`. 

jsoncons strings that consist of the following parts

- (optional) plus or minus sign
- 0x or 0X
- nonempty sequence of hexadecimal digits optionally containing a decimal-point character
- (optional) p or P followed with optional minus or plus sign and nonempty sequence of decimal digits

and tagged with `semantic_tag::bigfloat` are encoded into CBOR bignums.

21, 22, 23 (byte string expected conversion is base64url, base64 or base16)  
CBOR byte strings tagged with 21, 22 and 23 are decoded into jsoncons byte strings tagged with
`semantic_tag::base64url`, `semantic_tag::base64` and `semantic_tag::base16`.
jsoncons byte strings tagged with `semantic_tag::base64url`, `semantic_tag::base64` and `semantic_tag::base16`
are encoded into CBOR byte strings tagged with 21, 22 and 23.

32 (URI)  
CBOR URI strings are decoded into jsoncons strings tagged with `semantic_tag::uri`.
jsoncons strings tagged with  `semantic_tag::uri` are encoded into CBOR URI strings.

33, 34 (UTF-8 string is base64url or base64)  
CBOR strings tagged with 33 and 34 are decoded into jsoncons strings tagged with `semantic_tag::base64url` and `semantic_tag::base64`.
jsoncons strings tagged with `semantic_tag::base64url` and `semantic_tag::base64` are encoded into CBOR strings tagged with 33 and 34.

256, 25 [stringref-namespace, stringref](http://cbor.schmorp.de/stringref)  
Tags 256 and 25 are automatically decoded when detected. They are encoded when CBOR option `pack_strings` is set to true.

### jsoncons - CBOR mappings

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
string        | bigint      | byte string | 2 (positive bignum) or 2 (negative bignum)  
string        | bigdec      | array | 4 (decimal fraction)
string        | bigfloat      | array | 5 (bigfloat)
string        | datetime        | string | 0 (date/time string) 
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

#### CBOR and basic_json

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
           "rater": "HikingAsylum",
           "assertion": "advanced",
           "rated": "Marilyn C",
           "rating": 0.90
         }
       ]
    }
    )");

    // Encode a basic_json value to a CBOR value
    std::vector<uint8_t> data;
    cbor::encode_cbor(j1, data);

    // Decode a CBOR value to a basic_json value
    ojson j2 = cbor::decode_cbor<ojson>(data);
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

    // Get a CBOR value for a nested data item with jsonpointer
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
            "rater": "HikingAsylum",
            "assertion": "advanced",
            "rated": "Marilyn C",
            "rating": 0.9
        }
    ]
}

(2)
Marilyn C, 0.9

(3) Marilyn C
```

#### Query CBOR with JSONPath
```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace jsoncons; // For convenience

int main()
{
    // Construct a json array of numbers
    json j = json::array();

    j.emplace_back(5.0);

    j.emplace_back(0.000071);

    j.emplace_back("-18446744073709551617",semantic_tag::bigint);

    j.emplace_back("1.23456789012345678901234567890", semantic_tag::bigdec);

    j.emplace_back("0x3p-1", semantic_tag::bigfloat);

    // Encode to JSON
    std::cout << "(1)\n";
    std::cout << pretty_print(j);
    std::cout << "\n\n";

    // as<std::string>() and as<double>()
    std::cout << "(2)\n";
    std::cout << std::dec << std::setprecision(15);
    for (const auto& item : j.array_range())
    {
        std::cout << item.as<std::string>() << ", " << item.as<double>() << "\n";
    }
    std::cout << "\n";

    // Encode to CBOR
    std::vector<uint8_t> v;
    cbor::encode_cbor(j,v);

    std::cout << "(3)\n";
    for (auto c : v)
    {
        std::cout << std::hex << std::setprecision(2) << std::setw(2)
                  << std::setfill('0') << static_cast<int>(c);
    }
    std::cout << "\n\n";
/*
    85 -- Array of length 5     
      fa -- float 
        40a00000 -- 5.0
      fb -- double 
        3f129cbab649d389 -- 0.000071
      c3 -- Tag 3 (negative bignum)
        49 -- Byte string value of length 9
          010000000000000000
      c4 -- Tag 4 (decimal fraction)
        82 -- Array of length 2
          38 -- Negative integer of length 1
            1c -- -29
          c2 -- Tag 2 (positive bignum)
            4d -- Byte string value of length 13
              018ee90ff6c373e0ee4e3f0ad2
      c5 -- Tag 5 (bigfloat)
        82 -- Array of length 2
          20 -- -1
          03 -- 3   
*/

    // Decode back to json
    json other = cbor::decode_cbor<json>(v);
    assert(other == j);

    // Query with JSONPath
    std::cout << "(4)\n";
    json result = jsonpath::json_query(other,"$[?(@ < 1.5)]");
    std::cout << pretty_print(result) << "\n\n";
```
Output:
```
(1)
[
    5.0,
    7.1e-05,
    "-18446744073709551617",
    "1.23456789012345678901234567890",
    [-1, 3]
]

(2)
5.0, 5
7.1e-05, 7.1e-05
-18446744073709551617, -1.84467440737096e+19
1.23456789012345678901234567890, 1.23456789012346
1.5, 1.5

(3)
85fa40a00000fb3f129cbab649d389c349010000000000000000c482381cc24d018ee90ff6c373e0ee4e3f0ad2c5822003

(4)
[
    7.1e-05,
    "-18446744073709551617",
    "1.23456789012345678901234567890"
]
```


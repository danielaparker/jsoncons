### jsoncons::cbor::decode_cbor

Decodes a [cbor](http://cbor.io/) binary serialization format to a json value.

#### Header
```c++
#include <jsoncons_ext/cbor/cbor.hpp>

template<class Json>
Json decode_cbor(const cbor_view& v); // (1)

template<class Json>
Json decode_cbor(std::basic_istream<typename Json::char_type>& is); // (2)
```

### Examples

#### Round trip (JSON to CBOR bytes back to JSON)

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;
using namespace jsoncons::cbor;

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

    std::vector<uint8_t> v;
    encode_cbor(j1, v);

    ojson j2 = decode_cbor<ojson>(v);
    std::cout << pretty_print(j2) << std::endl;
}
```
Output:
```json
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
```

#### Round trip (JSON to CBOR file back to JSON)

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;
using namespace jsoncons::cbor;

int main()
{
    json j = json::parse(R"(
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

    std::ofstream os;
    os.open("./output/store.cbor", std::ios::binary | std::ios::out);
    encode_cbor(j,os);

    std::vector<uint8_t> v;
    std::ifstream is;
    is.open("./output/store.cbor", std::ios::binary | std::ios::in);

    json j2 = decode_cbor<json>(is);

    std::cout << pretty_print(j2) << std::endl; 
}
```
Output:
```json
{
    "application": "hiking",
    "reputons": [
        {
            "assertion": "is-good",
            "rated": "sk",
            "rater": "HikingAsylum.example.com",
            "rating": 0.9
        }
    ]
}
```

#### Decode CBOR byte string

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;

int main()
{
    // byte string for five bytes
    std::vector<uint8_t> buf = {0x45,'H','e','l','l','o'};
    json j = cbor::decode_cbor<json>(buf);

    auto bs = j.as<byte_string>();

    // byte_string to ostream displays as hex
    std::cout << "(1) "<< bs << "\n\n";

    // byte string value to JSON text becomes base64url
    std::cout << "(2) " << j << std::endl;
}
```
Output:
```
(1) 0x480x650x6c0x6c0x6f

(2) "SGVsbG8_"
```

#### See also

- [byte_string](../byte_string.md)
- [cbor_view](cbor_view)
- [encode_cbor](encode_cbor.md) encodes a json value to the [cbor](http://cbor.io/) binary serialization format.



### jsoncons::cbor::decode_cbor

Decodes a [cbor](http://cbor.io/) binary serialization format to a json value.

#### Header
```c++
#include <jsoncons_ext/cbor/cbor.hpp>

template<class Json>
Json decode_cbor(cbor_view v)
```

### Examples

#### Round trip

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

    std::vector<uint8_t> v = encode_cbor(j1);

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

#### Decode CBOR byte string

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;

int main()
{
    // byte string for five bytes
    std::vector<uint8_t> bs = {0x45,'H','e','l','l','o'};
    json j = cbor::decode_cbor<json>(bs);

    std::vector<uint8_t> v = j.as<std::vector<uint8_t>>();
    std::cout << "(1) ";
    for (auto b : v)
    {
        std::cout << (char)b;
    }
    std::cout << std::endl;

    // byte string value to json becomes base64url
    std::cout << "(2) " << j << std::endl;
}
```
Output:
```
(1) Hello
(2) "SGVsbG8_"
```

#### See also

- [cbor_view](cbor_view)
- [encode_cbor](encode_cbor.md) encodes a json value to the [cbor](http://cbor.io/) binary serialization format.



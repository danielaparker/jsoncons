### jsoncons::cbor::decode_cbor

Decodes a [cbor](http://cbor.io/) binary serialization format to a json value.

#### Header
```c++
#include <jsoncons_ext/cbor/cbor.hpp>

template<class Json>
Json decode_cbor(cbor_view v)
```

### Examples

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


#### See also

- [cbor_view](cbor_view)
- [encode_cbor](encode_cbor.md) encodes a json value to the [cbor](http://cbor.io/) binary serialization format.



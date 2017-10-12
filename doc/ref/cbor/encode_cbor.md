### jsoncons::cbor::encode_cbor

Encodes a json value to the [cbor](http://cbor.io/) binary serialization format.

#### Header
```c++
#include <jsoncons_ext/cbor/cbor.hpp>

template<class Json>
std::vector<uint8_t> encode_cbor(const Json& jval)
```

#### See also

- [decode_cbor](decode_cbor) decodes a [cbor](http://cbor.io/) binary serialization format to a json value.

### Examples

#### cbor example

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;

int main()
{
    ojson j1;
    j1["zero"] = 0;
    j1["one"] = 1;
    j1["two"] = 2;
    j1["null"] = null_type();
    j1["true"] = true;
    j1["false"] = false;
    j1["max int64_t"] = (std::numeric_limits<int64_t>::max)();
    j1["max uint64_t"] = (std::numeric_limits<uint64_t>::max)();
    j1["min int64_t"] = (std::numeric_limits<int64_t>::min)();
    j1["max int32_t"] = (std::numeric_limits<int32_t>::max)();
    j1["max uint32_t"] = (std::numeric_limits<uint32_t>::max)();
    j1["min int32_t"] = (std::numeric_limits<int32_t>::min)();
    j1["max int16_t"] = (std::numeric_limits<int16_t>::max)();
    j1["max uint16_t"] = (std::numeric_limits<uint16_t>::max)();
    j1["min int16_t"] = (std::numeric_limits<int16_t>::min)();
    j1["max int8_t"] = (std::numeric_limits<int8_t>::max)();
    j1["max uint8_t"] = (std::numeric_limits<uint8_t>::max)();
    j1["min int8_t"] = (std::numeric_limits<int8_t>::min)();
    j1["max double"] = (std::numeric_limits<double>::max)();
    j1["min double"] = -(std::numeric_limits<double>::max)();
    j1["max float"] = (std::numeric_limits<float>::max)();
    j1["zero float"] = 0.0;
    j1["min float"] = -(std::numeric_limits<float>::max)();
    j1["Key too long for small string optimization"] = "String too long for small string optimization";

    std::vector<uint8_t> v = cbor::encode_cbor(j1);

    ojson j2 = cbor::decode_cbor<ojson>(v);

    std::cout << pretty_print(j2) << std::endl;
}
```
Output:
```json
{
    "zero": 0,
    "one": 1,
    "two": 2,
    "null": null,
    "true": true,
    "false": false,
    "max int64_t": 9223372036854775807,
    "max uint64_t": 18446744073709551615,
    "min int64_t": -9223372036854775808,
    "max int32_t": 2147483647,
    "max uint32_t": 4294967295,
    "min int32_t": -2147483648,
    "max int16_t": 32767,
    "max uint16_t": 65535,
    "min int16_t": -32768,
    "max int8_t": 127,
    "max uint8_t": 255,
    "min int8_t": -128,
    "max double": 1.79769313486232e+308,
    "min double": -1.79769313486232e+308,
    "max float": 3.40282346638529e+038,
    "zero float": 0.0,
    "min float": -3.40282346638529e+038,
    "Key too long for small string optimization": "String too long for small string optimization"
}
```

#### Encode CBOR byte string

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;

int main()
{
    // construct byte string value
    std::vector<uint8_t> v = {'H','e','l','l','o'};
    json j(v.data(), v.size());

    std::vector<uint8_t> bs = cbor::encode_cbor(j);
    std::cout << "(1) ";

    std::cout << std::hex << (int)bs[0];
    for (size_t i = 1; i < bs.size(); ++i)
    {
        std::cout << (char)bs[i];
    }
    std::cout << std::endl;

    // byte string value to json becomes base64url
    std::cout << "(2) " << j << std::endl;
}
```
Output:
```
(1) 45Hello
(2) "SGVsbG8_"
```


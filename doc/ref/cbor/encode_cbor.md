### jsoncons::cbor::encode_cbor

Encodes a json value to the [Concise Binary Object Representation](http://cbor.io/) data format.

#### Header
```c++
#include <jsoncons_ext/cbor/cbor.hpp>

template<class Json>
void encode_cbor(const Json& jval, std::vector<uint8_t>& buffer); // (1)

template<class Json>
void encode_cbor(const Json& j, std::basic_ostream<typename Json::char_type>& os); // (2)
```

(1) Writes json value in CBOR data format to buffer

(2) Writes json value in CBOR data format to binary output stream

#### See also

- [decode_cbor](decode_cbor) decodes a [Concise Binary Object Representation](http://cbor.io/) data format to a json value.

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
    j1["min int64_t"] = (std::numeric_limits<int64_t>::lowest)();
    j1["max int32_t"] = (std::numeric_limits<int32_t>::max)();
    j1["max uint32_t"] = (std::numeric_limits<uint32_t>::max)();
    j1["min int32_t"] = (std::numeric_limits<int32_t>::lowest)();
    j1["max int16_t"] = (std::numeric_limits<int16_t>::max)();
    j1["max uint16_t"] = (std::numeric_limits<uint16_t>::max)();
    j1["min int16_t"] = (std::numeric_limits<int16_t>::lowest)();
    j1["max int8_t"] = (std::numeric_limits<int8_t>::max)();
    j1["max uint8_t"] = (std::numeric_limits<uint8_t>::max)();
    j1["min int8_t"] = (std::numeric_limits<int8_t>::lowest)();
    j1["max double"] = (std::numeric_limits<double>::max)();
    j1["min double"] = (std::numeric_limits<double>::lowest)();
    j1["max float"] = (std::numeric_limits<float>::max)();
    j1["zero float"] = 0.0;
    j1["min float"] = (std::numeric_limits<float>::lowest)();
    j1["Key too long for small string optimization"] = "String too long for small string optimization";

    std::vector<uint8_t> v;
    cbor::encode_cbor(j1, v);

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
    json j(byte_string("Hello"));

    std::vector<uint8_t> buf;
    cbor::encode_cbor(j, buf);

    std::cout << std::hex << std::showbase << "(1) ";
    for (auto c : buf)
    {
        std::cout << (int)c;
    }
    std::cout << std::dec << "\n\n";

    json j2 = cbor::decode_cbor<json>(buf);
    std::cout << "(2) " << j2 << std::endl;
}
```
Output:
```
(1) 0x450x480x650x6c0x6c0x6f

(2) "SGVsbG8"
```

#### Encode byte string with encoding hint

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>

using namespace jsoncons;

int main()
{
    // construct byte string value
     json j1(byte_string("Hello"), semantic_tag_type::base64);

    std::vector<uint8_t> buf;
    cbor::encode_cbor(j1, buf);

    std::cout << std::hex << std::showbase << "(1) ";
    for (auto c : buf)
    {
        std::cout << (int)c;
    }
    std::cout << std::dec << "\n\n";

    json j2 = cbor::decode_cbor<json>(buf);
    std::cout << "(2) " << j2 << std::endl;
}
```
Output:
```
(1) 0xd60x450x480x650x6c0x6c0x6f

(2) "SGVsbG8="
```

#### See also

- [byte_string](../byte_string.md)
- [decode_cbor](decode_cbor.md) decodes a [Concise Binary Object Representation](http://cbor.io/) data format to a json value.


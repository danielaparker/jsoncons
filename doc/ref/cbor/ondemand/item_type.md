### jsoncons::cbor::ondemand::cbor_type

```cpp
#include <jsoncons_ext/cbor/cbor_ondemand.hpp>

enum class cbor_type : uint8_t
{
    unsigned_integer = 0,
    negative_integer = 0x20,
    byte_string      = 0x40,
    text_string      = 0x60,
    array            = 0x80,
    map              = 0xa0,
    boolean          = 0xf5,
    null             = 0xf6,
    undefined        = 0xf7,
    float16          = 0xf9,
    float32          = 0xfa,
    float64          = 0xfb,
    simple           = 0xe0,
};                   
```

The `cbor_type` enum generally matches the CBOR major type values, 
except that major type 7 is subdivided into several logical types.


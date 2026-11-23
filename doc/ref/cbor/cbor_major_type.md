### jsoncons::cbor::cbor_major_type

```cpp
#include <jsoncons_ext/cbor/cbor_common.hpp>

enum class cbor_major_type : uint8_t
{
    unsigned_integer = 0x00,
    negative_integer = 0x01,
    byte_string = 0x02,
    text_string = 0x03,
    array = 0x04,
    map = 0x05,   
    semantic_tag = 0x06,
    simple = 0x7
};
```


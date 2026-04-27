### jsoncons::staj_event_type

```cpp
#include <jsoncons/staj_event.hpp>

enum class staj_event_type
{
    string_value,             
    byte_string_value,
    null_value,
    bool_value,
    int64_value,
    uint64_value,                       (until 1.7.0)
    half_value,
    double_value,
    begin_array,
    end_array,
    begin_object,
    end_object,
    key
};

using staj_event_type = staj_events;    (since 1.7.0)
```
Indicates the type of a staj event.

Since 1.7.0, `staj_event_type` is aliased to [staj_events](staj_events.md),
which meets the requirements of a [BitMaskType](https://en.cppreference.com/w/cpp/named_req/BitmaskType.html),
and means that the bitwise operators operator&, operator|, operator^, operator~, operator&=, operator|=, and operator^= 
are defined for this type.


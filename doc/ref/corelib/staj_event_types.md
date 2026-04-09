### jsoncons::staj_event_type

```cpp
#include <jsoncons/staj_event.hpp>

enum class staj_event_types
{
    begin_array,
    end_array,
    begin_object,
    end_object,
    string_value,             
    byte_string_value,    (since 1.7.0)
    null_value,
    bool_value,
    int64_value,
    uint64_value,
    half_value,
    double_value,
    key
};
```

A [BitMaskType](https://en.cppreference.com/w/cpp/named_req/BitmaskType.html). 
that indicates the type of a staj event.

### jsoncons::staj_event_types

```cpp
#include <jsoncons/staj_event.hpp>

enum class staj_event_types
{
    string_value,             
    byte_string_value,
    null_value,
    bool_value,
    int64_value,
    uint64_value,          (since 1.7.0)
    half_value,
    double_value,
    begin_array,
    end_array,
    begin_object,
    end_object,
    key
};

using staj_event_type = staj_event_types;   // For backwards compatibility
```

A [BitMaskType](https://en.cppreference.com/w/cpp/named_req/BitmaskType.html). 
that indicates the type of a staj event.

### Example

```cpp
constexpr auto mask = staj_event_types::begin_array | staj_event_types::begin_object;
if ((event_type & mask) != staj_event_types{}) 
{
    /*...*/
}
```  


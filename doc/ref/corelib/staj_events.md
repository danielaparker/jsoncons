### jsoncons::staj_events

```cpp
#include <jsoncons/staj_event.hpp>

enum class staj_events   (since 1.7.0)
{
    string_value      = /*unspecified*/,             
    byte_string_value = /*unspecified*/,
    null_value        = /*unspecified*/,
    bool_value        = /*unspecified*/,
    int64_value       = /*unspecified*/,
    uint64_value      = /*unspecified*/,                       
    half_value        = /*unspecified*/,
    double_value      = /*unspecified*/,
    begin_array       = /*unspecified*/,
    end_array         = /*unspecified*/,
    begin_object      = /*unspecified*/,
    end_object        = /*unspecified*/,
    second            = /*unspecified*/,   (since 1.9.0)
    key = second | string_value            (since 1.9.0)
};
```
Represents a classification of streaming events for JSON-like formats.

`staj_events` satisfies the requirements of a
[BitMaskType](https://en.cppreference.com/w/cpp/named_req/BitmaskType.html). This 
allows us to write e.g.

```cpp
constexpr auto mask = staj_events::begin_array | staj_events::begin_object;

if ((event & mask) != staj_events{}) 
{
    // Process begin_array and begin_object_events
}
```  


### jsoncons::staj_events

```cpp
#include <jsoncons/staj_event.hpp>

enum class staj_events
{
    string_value,             
    byte_string_value,
    null_value,
    bool_value,
    int64_value,
    uint64_value,                       (since 1.7.0)
    half_value,
    double_value,
    begin_array,
    end_array,
    begin_object,
    end_object,
    key
};
```
Represents a classification of streaming events for JSON-like formats.

`staj_events` satisfies the requirements of a
[BitMaskType](https://en.cppreference.com/w/cpp/named_req/BitmaskType.html). This 
allows us to write e.g.

```cpp
constexpr auto mask = staj_events::begin_array | staj_events::begin_object;
if ((event_type & mask) != staj_events{}) 
{
    /*...*/
}
```  


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
    uint64_value,
    half_value,
    double_value,
    begin_array,
    end_array,
    begin_object,
    end_object,
    key
};
```
Indicates the type of a staj event.

Since 1.7.0, `staj_event_type` is defined as a
[BitMaskType](https://en.cppreference.com/w/cpp/named_req/BitmaskType.html). This 
allows us to write e.g.

```cpp
constexpr auto mask = staj_event_type::begin_array | staj_event_type::begin_object;
if ((event_type & mask) != staj_event_type{}) 
{
    /*...*/
}
```  


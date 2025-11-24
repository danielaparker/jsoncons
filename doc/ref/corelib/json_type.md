### jsoncons::json_type

```cpp
#include <jsoncons/json_type.hpp>

enum class json_type : uint8_t 
{
    null,                            // (since 1.5.0)
    boolean,                         // (since 1.5.0)
    int64,                           // (since 1.5.0)
    uint64,                          // (since 1.5.0)
    float16,                         // (since 1.5.0)
    float64,                         // (since 1.5.0)
    string,                          // (since 1.5.0)
    byte_string,                     // (since 1.5.0)
    array,                           // (since 1.5.0)
    object,                          // (since 1.5.0) 
    null_value = null,               // (dprecated in 1.5.0)
    bool_value = boolean,            // (dprecated in 1.5.0)
    int64_value = int64,             // (dprecated in 1.5.0)
    uint64_value = uint64,           // (dprecated in 1.5.0)
    half_value = float16,            // (dprecated in 1.5.0)
    double_value = float64,          // (dprecated in 1.5.0)
    string_value = string,           // (dprecated in 1.5.0)
    byte_string_value = byte_string, // (dprecated in 1.5.0)
    array_value = array,             // (dprecated in 1.5.0) 
    object_value = object            // (dprecated in 1.5.0)
};
```


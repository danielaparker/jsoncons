### jsoncons::stream_event_type

#### Header
```c++
#include <jsoncons/stream_reader.hpp>
```

```c++
enum class stream_event_type
{
    begin_object,
    end_object,
    begin_array,
    end_array,
    name,
    string_value,
    byte_string_value,
    bignum_value,
    int64_value,
    uint64_value,
    double_value,
    bool_value,
    null_value
};
```


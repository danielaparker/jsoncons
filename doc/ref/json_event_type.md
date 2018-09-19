### jsoncons::json_event_type

#### Header
```c++
#include <jsoncons/json_event_reader.hpp>
```

```c++
enum class json_event_type
{
    begin_document,
    end_document,
    begin_object,
    end_object,
    begin_array,
    end_array,
    name,
    string_value,
    bignum_value,
    int64_value,
    uint64_value,
    double_value,
    bool_value,
    null_value
};
```


### jsoncons::staj_event_type

__`jsoncons/staj_reader.hpp`__

```c++
enum class staj_event_type
{
    begin_array,
    end_array,
    begin_object,
    end_object,
    name,
    string_value,
    byte_string_value,
    null_value,
    bool_value,
    int64_value,
    uint64_value,
    half_value,
    double_value
};
```


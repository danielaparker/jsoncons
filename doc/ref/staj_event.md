### jsoncons::staj_event

```c++
#include <jsoncons/staj_reader.hpp>

typedef basic_staj_event<char> staj_event;
```

A JSON-like data event.

| Event type        | Sample data | Valid accessors |
|-------------------|------------------------|-----------------|
| begin_object      |                        | |            
| end_object        |                        | |
| begin_array       |                        | |
| end_array         |                        | |
| name              | "foo"                  | `get<std::string>()`, `get<jsoncons::string_view>`, `get<std::string_view>()` |
| string_value      | "1000"                 | `get<std::string>()`, `get<jsoncons::string_view>`, `get<std::string_view>()`, `get<int>()`, `get<unsigned>()` |
| byte_string_value | 0x660x6F0x6F           | `get<std::string>()`, `get<jsoncons::byte_string>()` |
| int64_value       | -1000                  | `get<std::string>()`, `get<int>()`, `get<long>`, `get<int64_t>()` |
| uint64_value      | 1000                   | `get<std::string>()`, `get<int>()`, `get<unsigned>()`, `get<int64_t>()`, `get<uint64_t>()` |
| double_value      | 125.72                 | `get<std::string>()`, `get<double>()` |
| bool_value        | true                   | `get<std::string>()`, `get<bool>()` |
| null_value        |                        | `get<std::string>()` |

#### Member functions

    staj_event_type event_type() const noexcept;
Returns a [staj_event_type](staj_event_type.md) for this event.

    semantic_tag tag() const noexcept;
Returns a [semantic_tag](semantic_tag.md) for this event.

    template <class T, class... Args>
    T get(Args&&... args) const;
Attempts to convert the json value to the template value type.


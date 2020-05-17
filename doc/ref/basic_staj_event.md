### jsoncons::basic_staj_event

```c++
#include <jsoncons/staj_cursor.hpp>

template<class CharT>
class basic_staj_event
```

A JSON-like parse event.

Typedefs for common character types are provided:

Type                |Definition
--------------------|------------------------------
staj_event     |`basic_staj_event<char>`
wstaj_event    |`basic_staj_event<wchar_t>`

The `event_type()` function returns the [event type](doc/ref/staj_event_type.md).
You can use the `get<T>()` function to access a string, number, or boolean value.

| Event type        | Sample data | Valid accessors |
|-------------------|------------------------|-----------------|
| begin_object      |                        | |            
| end_object        |                        | |
| begin_array       |                        | |
| end_array         |                        | |
| key               | "foo"                  | `get<std::string>()`<br>`get<jsoncons::string_view>`<br>`get<std::string_view>()` |
| string_value      | "1000"                 | `get<std::string>()`<br>`get<jsoncons::string_view>`<br>`get<std::string_view>()`<br>`get<int>()`<br>`get<unsigned>()` |
| byte_string_value | 0x660x6F0x6F           | `get<std::string>()`<br>`get<jsoncons::byte_string>()` |
| int64_value       | -1000                  | `get<std::string>()`<br>`get<int>()`<br>`get<long>`<br>`get<int64_t>()` |
| uint64_value      | 1000                   | `get<std::string>()`<br>`get<int>()`<br>`get<unsigned>()`<br>`get<int64_t>()`<br>`get<uint64_t>()` |
| half_value        | 1.5 (as double)        | `get<std::string>()`<br>`get<uint16_t>()`<br>`get<double>()` |
| double_value      | 125.72                 | `get<std::string>()`<br>`get<double>()` |
| bool_value        | true                   | `get<std::string>()`<br>`get<bool>()` |
| null_value        |                        | `get<std::string>()` |

#### Member functions

    staj_event_type event_type() const noexcept;
Returns a [staj_event_type](staj_event_type.md) for this event.

    semantic_tag tag() const noexcept;
Returns a [semantic_tag](semantic_tag.md) for this event.

    uint64_t ext_tag() const
If `tag()` == `semantic_tag::ext`, returns a format specific tag associated with a byte string value,
otherwise return 0. An example is a MessagePack `type` in the range 0-127 associated with the
MessagePack ext format family, or a CBOR tag preceeding a byte string. 

    template <class T, class... Args>
    T get() const;
Attempts to convert the json value to the template value type.

    template <class T, class... Args>
    T get(std::error_code& ec) const;
Attempts to convert the json value to the template value type.


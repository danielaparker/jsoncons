### jsoncons::stream_event

```c++
typedef basic_stream_event<char,std::allocator<char>> stream_event;
```

#### Header
```c++
#include <jsoncons/stream_reader.hpp>

A JSON-like data event.
```

#### Member functions

    stream_event_type event_type();
Returns a [stream_event_type](stream_event_type.md) for this event.

    template <class T, class... Args>
    bool is(Args&&... args) const noexcept;
Returns `true` if the json value is the same as type `T`, using 
[json_type_traits](json_type_traits.md) if necessary, otherwise `false`.

    template <class T, class... Args>
    T as(Args&&... args) const;
Attempts to convert the json value to the template value type,
using [json_type_traits](json_type_traits.md) if necessary.


### jsoncons::json_event

```c++
typedef basic_json_event<char,std::allocator<char>> json_event;
```

#### Header
```c++
#include <jsoncons/json_event_reader.hpp>

A json event.
```

#### Member functions

    json_event_type event_type();
    Returns a [json_event_type](json_event_type.md) for this event.

    template <class T, class... Args>
    bool is(Args&&... args) const noexcept;
    Returns `true` if the json value is the same as type `T`, using 
    [json_type_traits](json_type_traits.md) if necessary, otherwise `false`.

    template <class T, class... Args>
    T as(Args&&... args) const;
    Attempts to convert the json value to the template value type,
    using [json_type_traits](json_type_traits.md) if necessary.


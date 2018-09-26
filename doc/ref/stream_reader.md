### jsoncons::json_stream_reader

```c++
typedef basic_json_stream_reader<char,std::allocator<char>> stream_reader
```

#### Header
```c++
#include <jsoncons/stream_reader.hpp>
```

#### Destructor

    virtual ~basic_stream_reader() = default;

#### Member functions

    virtual bool done() const = 0;
Check if there are no more events.

    virtual const stream_event& current() const = 0;
Returns the current [stream_event](stream_event.md).

    virtual void next() = 0;
Get the next event. 

    virtual size_t line_number() const = 0;

    virtual size_t column_number() const = 0;


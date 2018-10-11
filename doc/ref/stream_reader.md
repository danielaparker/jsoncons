### jsoncons::stream_reader

```c++
typedef basic_stream_reader<char> stream_reader
```

#### Header
```c++
#include <jsoncons/stream_reader.hpp>
```

The `stream_reader` interface supports forward, read-only, access to JSON and JSON-like data formats.

The `stream_reader` is designed to iterate over stream events until `done()` returns `true`.
The `next()` function causes the reader to read the next stream event. The `current()` function
returns the current stream event. The data can be accessed using the [stream_event](stream_event.md) 
interface. When `next()` is called, copies of data accessed previously may be invalidated.

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


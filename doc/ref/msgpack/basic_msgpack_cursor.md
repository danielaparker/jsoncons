### jsoncons::msgpack::basic_msgpack_cursor

```c++
#include <jsoncons/msgpack_cursor.hpp>

template<
    class Src=jsoncons::binary_stream_source,
    class Allocator=std::allocator<char>>
class basic_msgpack_cursor;
```

A pull parser for reporting MSGPACK parse events. A typical application will 
repeatedly process the `current()` event and call the `next()`
function to advance to the next event, until `done()` returns `true`.

`basic_msgpack_cursor` is noncopyable and nonmoveable.

Typedefs for common sources are provided:

Type                |Definition
--------------------|------------------------------
msgpack_stream_cursor  |basic_msgpack_cursor<jsoncons::binary_stream_source>
msgpack_bytes_cursor   |basic_msgpack_cursor<jsoncons::bytes_source>

### Implemented interfaces

[staj_reader](staj_reader.md)

#### Constructors

    template <class Source>
    basic_msgpack_cursor(Source&& source); // (1)

    template <class Source>
    basic_msgpack_cursor(Source&& source,
                      std::function<bool(const staj_event&, const ser_context&)> filter); // (2)

    template <class Source>
    basic_msgpack_cursor(Source&& source, std::error_code& ec); // (3)

    template <class Source>
    basic_msgpack_cursor(Source&& source,
                      std::function<bool(const staj_event&, const ser_context&)> filter, 
                      std::error_code& ec); // (4)

Constructor3 (1)-(2) read from a buffer or stream source and throw a 
[ser_error](ser_error.md) if a parsing error is encountered while processing the initial event.

Constructor3 (3)-(4) read from a buffer or stream source and set `ec`
if a parsing error is encountered while processing the initial event.

Note: It is the programmer's responsibility to ensure that `basic_msgpack_cursor` does not outlive any source passed in the constuctor, 
as `basic_msgpack_cursor` holds pointers to but does not own these resources.

#### Parameters

`source` - a value from which a `source_type` is constructible. 

#### Member functions

    bool done() const override;
Checks if there are no more events.

    const staj_event& current() const override;
Returns the current [staj_event](staj_event.md).

    void read_to(json_content_handler& handler) override
Sends the parse events from the current event to the
matching completion event to the supplied [handler](json_content_handler.md)
E.g., if the current event is `begin_object`, sends the `begin_object`
event and all inbetween events until the matching `end_object` event.
If a parsing error is encountered, throws a [ser_error](ser_error.md).

    void read_to(json_content_handler& handler, std::error_code& ec) override
Sends the parse events from the current event to the
matching completion event to the supplied [handler](json_content_handler.md)
E.g., if the current event is `begin_object`, sends the `begin_object`
event and all inbetween events until the matching `end_object` event.
If a parsing error is encountered, sets `ec`.

    void next() override;
Advances to the next event. If a parsing error is encountered, throws a 
[ser_error](ser_error.md).

    void next(std::error_code& ec) override;
Advances to the next event. If a parsing error is encountered, sets `ec`.

    const ser_context& context() const override;
Returns the current [context](ser_context.md)



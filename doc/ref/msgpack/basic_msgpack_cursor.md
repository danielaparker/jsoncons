### jsoncons::msgpack::basic_msgpack_cursor

```cpp
#include <jsoncons_ext/msgpack/msgpack_cursor.hpp>

template<
    typename Source=jsoncons::binary_stream_source,
    typename Allocator=std::allocator<char>>
class basic_msgpack_cursor;
```

A pull parser for reporting MessagePack parse events. A typical application will 
repeatedly process the `current()` event and call the `next()`
function to advance to the next event, until `done()` returns **true**.
In addition, when positioned on a `begin_object` event, 
the `read_to` function can pull a complete object representing
the events from `begin_object` to `end_object`, 
and when positioned on a `begin_array` event, a complete array
representing the events from `begin_array` ro `end_array`.

`basic_msgpack_cursor` is noncopyable and nonmoveable.

Aliases for common sources are provided:

Type                |Definition
--------------------|------------------------------
msgpack_stream_cursor  |basic_msgpack_cursor<jsoncons::binary_stream_source>
msgpack_bytes_cursor   |basic_msgpack_cursor<jsoncons::bytes_source>

### Implemented interfaces

[staj_cursor](staj_cursor.md)

#### Constructors

    template <typename Sourceable>
    basic_msgpack_cursor(Sourceable&& source,
                         const msgpack_decode_options& options = msgpack_decode_options(),
                         const Allocator& alloc = Allocator()); (1)

    template <typename Sourceable>
    basic_msgpack_cursor(Sourceable&& source,
                         std::error_code& ec); (2)
    template <typename Sourceable>
    basic_msgpack_cursor(Sourceable&& source,
                         const msgpack_decode_options& options,
                         std::error_code& ec); (3)

    template <typename Sourceable>
    basic_msgpack_cursor(std::allocator_arg_t, const Allocator& alloc, 
                         Sourceable&& source,
                         const msgpack_decode_options& options,
                         std::error_code& ec); (4)

Constructor (1) reads from a buffer or stream source and throws a 
[ser_error](ser_error.md) if a parsing error is encountered while processing the initial event.

Constructors (2)-(4) read from a buffer or stream source and set `ec`
if a parsing error is encountered while processing the initial event.

Note: It is the programmer's responsibility to ensure that `basic_msgpack_cursor` does not outlive any source passed in the constuctor, 
as `basic_msgpack_cursor` holds a pointer to but does not own this object.

#### Parameters

`source` - a value from which a `source_type` is constructible. 

#### Member functions

##### staj_event input

    bool done() const final;
Check if there are no more events.

    void next() final;
Get the next event. If a parsing error is encountered, throws a [ser_error](../corelib/ser_error.md).

    void next(std::error_code& ec) final;
Get the next event. If a parsing error is encountered, sets `ec`.

    const staj_event& current() const final;
Returns the current [staj_event](../corelib/basic_staj_event.md).

    void read_to(json_visitor& visitor) final;
Sends the parse events from the current event to the
matching completion event to the supplied [visitor](../corelib/basic_json_visitor.md)
E.g., if the current event is `begin_object`, sends the `begin_object`
event and all inbetween events until the matching `end_object` event.
If a parsing error is encountered, throws a [ser_error](../corelib/ser_error.md).

    void read_to(json_visitor& visitor, std::error_code& ec) final;
Sends the parse events from the current event to the
matching completion event to the supplied [visitor](../corelib/basic_json_visitor.md)
E.g., if the current event is `begin_object`, sends the `begin_object`
event and all inbetween events until the matching `end_object` event.
If a parsing error is encountered, sets `ec`.

##### Miscellaneous

    const ser_context& context() const final;
Returns the current [context](../corelib/ser_context.md)

    void reset();
Reset cursor to read another value from the same source

    template <typename Sourceable>
    reset(Sourceable&& source)
Reset cursor to read new value from a new source

#### Non-member functions

    template <typename Source,typename Allocator>
    staj_filter_view operator|(basic_msgpack_cursor<Source,Allocator>& cursor, 
                               std::function<bool(const staj_event&, const ser_context&)> pred);

### Examples

#### Read non-string keys (since 1.9.0)

Since 1.9.0, jsoncons supports reading non-string keys using the cursor API.
Keys are reported by [staj_events](../corelib/basic_staj_events.md) 
that are ORed with the flag `staj_events::key_flag`.
For backwards compatibility, the enum value `staj_events::key` has been redefined as
```cpp
    staj_events::string_value | staj_events::key_flag
```
In addition, for the common case of CBOR or MessagePack unsigned integer keys,
the enum value `id` has been added to `sjaj_events` and defined as 
```cpp
   staj_events::uint64_value | staj_events::key_flag`
```

##### Read an unsigned integer key

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/msgpack/msgpack.hpp>
#include <iostream>

namespace msgpack = jsoncons::msgpack;

int main()
{
    std::vector<uint8_t> data = {
        0x92,                    // fixarray(2)
        0x82,                    // fixmap(2)
        0x01,                    // key = 1
        0xa3, 0x66, 0x6f, 0x6f,  // "foo"
        0x02,                    // key = 2
        0xa3, 0x62, 0x61, 0x72,  // "bar"
        0x81,                    // fixmap(1)
        0x03,                    // key = 3
        0xa3, 0x62, 0x61, 0x7a   // "baz"
    };

    msgpack::msgpack_bytes_cursor cursor{data};
    while (!cursor.done())
    {
        switch (cursor.current().event_type())
        {
            case jsoncons::staj_events::begin_array:
                std::cout << "begin array\n";
                break;
            case jsoncons::staj_events::end_array:
                std::cout << "end array\n";
                break;
            case jsoncons::staj_events::begin_object:
                std::cout << "  begin object\n";
                break;
            case jsoncons::staj_events::end_object:
                std::cout << "  end object\n";
                break;
            case jsoncons::staj_events::string_value:
                std::cout << "    value: " << cursor.current().get<std::string>() << "\n";
                break;
            case jsoncons::staj_events::id:
                std::cout << "    key: " << cursor.current().get<uint64_t>() << "\n";
                break;
        }
        cursor.next();
    }
    };

    cbor::cbor_bytes_cursor cursor{data};
    while (!cursor.done())
    {
        switch (cursor.current().event_type())
        {
            case jsoncons::staj_events::begin_array:
                std::cout << "begin array\n";
                break;
            case jsoncons::staj_events::end_array:
                std::cout << "end array\n";
                break;
            case jsoncons::staj_events::begin_object:
                std::cout << "  begin object\n";
                break;
            case jsoncons::staj_events::end_object:
                std::cout << "  end object\n";
                break;
            case jsoncons::staj_events::string_value:
                std::cout << "    value: " << cursor.current().get<std::string>() << "\n";
                break;
            case jsoncons::staj_events::id:
                std::cout << "    key: " << cursor.current().get<uint64_t>() << "\n";
                break;
        }
        cursor.next();
    }
}
```

Output:

```
begin array
  begin object
    key: 1
    value: foo
    key: 2
    value: bar
  end object
  begin object
    key: 3
    value: baz
  end object
end array
```


### See also

[staj_events](../corelib/basic_staj_events.md)  


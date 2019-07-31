### jsoncons::csv::basic_csv_cursor

```c++
#include <jsoncons/csv/csv_cursor.hpp>

template<
    class CharT,
    class Src=jsoncons::stream_source<CharT>,
    class Allocator=std::allocator<char>> basic_csv_cursor;
```

A pull parser for reporting CSV parse events. A typical application will 
repeatedly process the `current()` event and call the `next()`
function to advance to the next event, until `done()` returns `true`.

`basic_csv_cursor` is noncopyable and nonmoveable.

Typedefs for common character types are provided:

Type                |Definition
--------------------|------------------------------
csv_cursor     |basic_csv_cursor<char>
wcsv_cursor    |basic_csv_cursor<wchar_t>

### Implemented interfaces

[basic_staj_reader](../staj_reader.md)

#### Constructors

    template <class Source>
    basic_csv_cursor(Source&& source, 
                     const basic_csv_decode_options<CharT>& options = basic_csv_options<CharT>::get_default_options(),
                     std::function<bool(csv_errc,const ser_context&)> err_handler = default_csv_parsing()); // (1)

    template <class Source>
    basic_csv_cursor(Source&& source, 
                      std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                      const basic_csv_decode_options<CharT>& options = basic_csv_options<CharT>::get_default_options(),
                      std::function<bool(json_errc,const ser_context&)> err_handler = default_csv_parsing()); // (2)

Constructors (1)-(2) read from a character sequence or stream and throw a 
[ser_error](../ser_error.md) if a parsing error is encountered while processing the initial event.

    template <class Source>
    basic_csv_cursor(Source&& source, std::error_code& ec); // (3)

    template <class Source>
    basic_csv_cursor(Source&& source, 
                     const basic_csv_decode_options<CharT>& options,
                     std::error_code& ec); // (4)

    template <class Source>
    basic_csv_cursor(Source&& source,
                     std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                     std::error_code& ec); // (5)

    template <class Source>
    basic_csv_cursor(Source&& source, 
                     std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                     const basic_csv_decode_options<CharT>& options,
                     std::error_code& ec); // (6)

    template <class Source>
    basic_csv_cursor(Source&& source, 
                     std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                     const basic_csv_decode_options<CharT>& options,
                     std::error_code& ec); // (7)

    template <class Source>
    basic_csv_cursor(Source&& source, 
                     std::function<bool(const basic_staj_event<CharT>&, const ser_context&)> filter,
                     const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler,
                     std::error_code& ec); // (8)

Constructors (3)-(8) read from a character sequence or stream and set `ec`
if a parsing error is encountered while processing the initial event.

Note: It is the programmer's responsibility to ensure that `basic_csv_cursor` does not outlive any source, 
content handler, and error handler passed in the constuctor, as `basic_csv_cursor` holds pointers to but does not own these resources.

#### Parameters

`source` - a value from which a `jsoncons::basic_string_view<char_type>` is constructible, 
or a value from which a `source_type` is constructible. In the case that a `jsoncons::basic_string_view<char_type>` is constructible
from `source`, `source` is dispatched immediately to the parser. Otherwise, the `csv_cursor` reads from a `source_type` in chunks. 

#### Member functions

    bool done() const override;
Checks if there are no more events.

    const basic_staj_event& current() const override;
Returns the current [basic_staj_event](../staj_event.md).

    void read_to(json_content_handler& handler) override
Sends the parse events from the current event to the
matching completion event to the supplied [handler](../json_content_handler.md)
E.g., if the current event is `begin_object`, sends the `begin_object`
event and all inbetween events until the matching `end_object` event.
If a parsing error is encountered, throws a [ser_error](../ser_error.md).

    void read_to(basic_json_content_handler<char_type>& handler,
                std::error_code& ec) override
Sends the parse events from the current event to the
matching completion event to the supplied [handler](../json_content_handler.md)
E.g., if the current event is `begin_object`, sends the `begin_object`
event and all inbetween events until the matching `end_object` event.
If a parsing error is encountered, sets `ec`.

    void next() override;
Advances to the next event. If a parsing error is encountered, throws a 
[ser_error](../ser_error.md).

    void next(std::error_code& ec) override;
Advances to the next event. If a parsing error is encountered, sets `ec`.

    const ser_context& context() const override;
Returns the current [context](../ser_context.md)

### Examples

### See also

- [staj_reader](../staj_reader.md) 
- [staj_array_iterator](../staj_array_iterator.md) 
- [staj_object_iterator](../staj_object_iterator.md)


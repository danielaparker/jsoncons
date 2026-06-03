### jsoncons::cbor::basic_cbor_cursor

```cpp
#include <jsoncons_ext/cbor/cbor_cursor.hpp>

template<
    typename Source=jsoncons::binary_stream_source,
    typename Allocator=std::allocator<char>>
class basic_cbor_cursor;
```

A pull parser for reporting CBOR parse events. A typical application will 
repeatedly process the `current()` event and call the `next()`
function to advance to the next event, until `done()` returns **true**.
In addition, when positioned on a `begin_object` event, 
the `read_to` function can pull a complete object representing
the events from `begin_object` to `end_object`, 
and when positioned on a `begin_array` event, a complete array
representing the events from `begin_array` ro `end_array`.

`basic_cbor_cursor` is noncopyable and nonmoveable.

Aliases for common sources are provided:

Type                |Definition
--------------------|------------------------------
cbor_stream_cursor  |basic_cbor_cursor<jsoncons::binary_stream_source>
cbor_bytes_cursor   |basic_cbor_cursor<jsoncons::bytes_source>

### Implemented interfaces

[basic_staj_cursor](../corelib/staj_cursor.md)

#### Constructors

    basic_cbor_cursor(Sourceable&& source,
        const cbor_decode_options& options = cbor_decode_options(),   (1)
        const Allocator& alloc = Allocator()); 

    template <typename Sourceable>
    basic_cbor_cursor(Sourceable&& source, std::error_code& ec);      (2)

    template <typename Sourceable>
    basic_cbor_cursor(Sourceable&& source,                            (3)
        const cbor_decode_options& options,                          
        std::error_code& ec); 

    template <typename Sourceable>
    basic_cbor_cursor(std::allocator_arg_t, const Allocator& alloc, 
        Sourceable&& source,                                          (4)
        const cbor_decode_options& options,
        std::error_code& ec); 

Constructors (1) reads from a buffer or stream source and throws a 
[ser_error](../corelib/ser_error.md) if a parsing error is encountered while processing the initial event.

Constructors (2)-(4) read from a buffer or stream source and set `ec`
if a parsing error is encountered while processing the initial event.

Note: It is the programmer's responsibility to ensure that `basic_cbor_cursor` does not outlive any source passed in the constuctor, 
as `basic_cbor_cursor` holds a pointer to but does not own this object.

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

##### Typed Array input

    bool is_typed_array() const final;                         (since 1.8.0)

    typed_array_tags array_tag() const final;                  (since 1.8.0)
Returns a [tag](../corelib/typed_array_tags.md) that indicates the element type of the typed array.

    jsoncons::span<uint8_t> array_buffer() final;              (since 1.8.0)

    void to_end_array() final;                                 (since 1.8.0)

##### Multi-dimensional array input

    bool is_multi_dim() const final;                           (since 1.8.0)
Indicates whether an array is a multi-dimensional array.

    jsoncons::span<const std::size_t> extents() const final;   (since 1.8.0)
Indicates the number of elements along each dimension of the array.

    mdarray_order order() const final;                         (since 1.8.0)
Indicates whether the elements of a multi-dimensional array are
arranged in row-major or column-major order. Returns a [mdarray_order](../corelib/mdarray_order.md).

##### Miscellaneous

    const ser_context& context() const final;
Returns the current [context](../corelib/ser_context.md)

    void reset();
Reset cursor to read another value from the same source

    template <typename Sourceable>
    reset(Sourceable&& source)
Reset cursor to read new value from a new source

    uint64_t raw_tag() const;                                  (since 1.2.0)
Returns the CBOR tag associated with the current value

##### Inherited from [jsoncons::basic_staj_cursor](../corelib/staj_cursor.md)

    template <typename T>                                      (since 1.8.0)
    void read_typed_array(T& v);

##### Non-member functions

    template <typename Source,typename Allocator>
    staj_filter_view operator|(basic_cbor_cursor<Source,Allocator>& cursor, 
                               std::function<bool(const staj_event&, const ser_context&)> pred);

### Examples

Input JSON file `book_catalog.json`:

```json
[ 
  { 
      "author" : "Haruki Murakami",
      "title" : "Hard-Boiled Wonderland and the End of the World",
      "isbn" : "0679743464",
      "publisher" : "Vintage",
      "date" : "1993-03-02",
      "price": 18.90
  },
  { 
      "author" : "Graham Greene",
      "title" : "The Comedians",
      "isbn" : "0099478374",
      "publisher" : "Vintage Classics",
      "date" : "2005-09-21",
      "price": 15.74
  }
]
```

#### Read CBOR parse events 

```cpp
#include <jsoncons_ext/cbor/cbor_cursor.hpp>
#include <string>
#include <fstream>

using namespace jsoncons;

int main()
{
    std::ifstream is("book_catalog.json");

    cbor_cursor cursor(is);

    for (; !cursor.done(); cursor.next())
    {
        const auto& event = cursor.current();
        switch (event.event_type())
        {
            case staj_event_type::begin_array:
                std::cout << event.event_type() << "\n";
                break;
            case staj_event_type::end_array:
                std::cout << event.event_type() << "\n";
                break;
            case staj_event_type::begin_object:
                std::cout << event.event_type() << "\n";
                break;
            case staj_event_type::end_object:
                std::cout << event.event_type() << "\n";
                break;
            case staj_event_type::key:
                // Or std::string_view, if supported
                std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::string_value:
                // Or std::string_view, if supported
                std::cout << event.event_type() << ": " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::null_value:
                std::cout << event.event_type() << ": " << "\n";
                break;
            case staj_event_type::bool_value:
                std::cout << event.event_type() << ": " << std::boolalpha << event.get<bool>() << "\n";
                break;
            case staj_event_type::int64_value:
                std::cout << event.event_type() << ": " << event.get<int64_t>() << "\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << event.event_type() << ": " << event.get<uint64_t>() << "\n";
                break;
            case staj_event_type::half_value:
            case staj_event_type::double_value:
                std::cout << event.event_type() << ": " << event.get<double>() << "\n";
                break;
            default:
                std::cout << "Unhandled event type\n";
                break;
        }
    }
}
```
Output:
```
begin_array
begin_object
key: author
string_value: Haruki Murakami
key: title
string_value: Hard-Boiled Wonderland and the End of the World
key: isbn
string_value: 0679743464
key: publisher
string_value: Vintage
key: date
string_value: 1993-03-02
key: price
double_value: 19
end_object
begin_object
key: author
string_value: Graham Greene
key: title
string_value: The Comedians
key: isbn
string_value: 0099478374
key: publisher
string_value: Vintage Classics
key: date
string_value: 2005-09-21
key: price
double_value: 16
end_object
end_array
```

#### Filter CBOR parse events

```cpp
#include <jsoncons_ext/cbor/cbor_cursor.hpp>
#include <string>
#include <fstream>

using namespace jsoncons;

int main()
{
    bool author_next = false;
    auto filter = [&](const staj_event& event, const ser_context&) -> bool
    {
        if (event.event_type() == staj_event_type::key &&
            event.get<jsoncons::string_view>() == "author")
        {
            author_next = true;
            return false;
        }
        if (author_next)
        {
            author_next = false;
            return true;
        }
        return false;
    };

    std::ifstream is("book_catalog.json");

    cbor_cursor cursor(is);
    auto filtered_c = cursor | filter;

    for (; !filtered_c.done(); filtered_c.next())
    {
        const auto& event = filtered_c.current();
        switch (event.event_type())
        {
            case staj_event_type::string_value:
                std::cout << event.get<jsoncons::string_view>() << "\n";
                break;
        }
    }
}
```
Output:
```
Haruki Murakami
Graham Greene
```

### Typed Array examples (until 1.8.0)

#### Read a typed array

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>
#include <cassert>

namespace cbor = jsoncons::cbor;

struct my_cbor_visitor : public jsoncons::default_json_visitor
{
    std::vector<double> v;
private:
    bool visit_typed_array(const jsoncons::span<const double>&data,
        jsoncons::semantic_tag,
        const jsoncons::ser_context&,
        std::error_code&) override
    {
        v = std::vector<double>(data.begin(), data.end());
        return false;
    }
};

int main()
{
    std::vector<uint8_t> data = {
        0xd8, // Tag
        0x56, // Tag 86, float64, little endian, Typed Array
        0x58, 0x20, // Byte string value of length 32
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x40,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x40,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x40,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x40
    };

    cbor::cbor_bytes_cursor cursor(data);
    //assert(jsoncons::staj_event_type::begin_array == cursor.current().event_type()); 
    assert(jsoncons::staj_events::begin_array == cursor.current().event_type());       // (since 1.7.0)
    assert(cursor.is_typed_array());

    my_cbor_visitor visitor;
    cursor.read_to(visitor);
    for (auto item : visitor.v)
    {
        std::cout << item << "\n";
    }
    std::cout << "\n";
}
```
Output:
```
10
20
30
40
```

#### Navigating typed arrays with cursor - multi-dimensional row major with Typed Array 

This example is taken from [CBOR Tags for Typed Arrays](https://tools.ietf.org/html/rfc8746)

```cpp
#include <jsoncons_ext/cbor/cbor_cursor.hpp>

int main()
{
    const std::vector<uint8_t> input = {
      0xd8,0x28,  // Tag 40 (multi-dimensional row major array)
        0x82,     // array(2)
          0x82,   // array(2)
            0x02,    // unsigned(2) 1st Dimension
            0x03,    // unsigned(3) 2nd Dimension
        0xd8,0x41,     // Tag 65 (uint16 big endian Typed Array)
          0x4c,        // byte string(12)
            0x00,0x02, // unsigned(2)
            0x00,0x04, // unsigned(4)
            0x00,0x08, // unsigned(8)
            0x00,0x04, // unsigned(4)
            0x00,0x10, // unsigned(16)
            0x01,0x00  // unsigned(256)
    };

    cbor::cbor_bytes_cursor cursor(input);
    for (; !cursor.done(); cursor.next())
    {
        const auto& event = cursor.current();
        switch (event.event_type())
        {
            case staj_event_type::begin_array:
                std::cout << event.event_type() 
                          << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::end_array:
                std::cout << event.event_type() 
                          << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << event.event_type() 
                          << ": " << event.get<uint64_t>() << " " << "(" << event.tag() << ")\n";
                break;
            default:
                std::cout << "Unhandled event type " << event.event_type() 
                          << " " << "(" << event.tag() << ")\n";
                break;
        }
    }
}
```
Output:
```
begin_array (multi-dim-row-major)
begin_array (n/a)
uint64_value: 2 (n/a)
uint64_value: 3 (n/a)
end_array (n/a)
begin_array (n/a)
uint64_value: 2 (n/a)
uint64_value: 4 (n/a)
uint64_value: 8 (n/a)
uint64_value: 4 (n/a)
uint64_value: 10 (n/a)
uint64_value: 100 (n/a)
end_array (n/a)
end_array (n/a)
```

#### Navigating typed arrays with cursor - multi-dimensional column major with classical CBOR array

This example is taken from [CBOR Tags for Typed Arrays](https://tools.ietf.org/html/rfc8746)

```cpp
#include <jsoncons_ext/cbor/cbor_cursor.hpp>

int main()
{
    const std::vector<uint8_t> input = {
      0xd9,0x04,0x10,  // Tag 1040 (multi-dimensional column major array)
        0x82,     // array(2)
          0x82,   // array(2)
            0x02,    // unsigned(2) 1st Dimension
            0x03,    // unsigned(3) 2nd Dimension
          0x86,   // array(6)
            0x02,           // unsigned(2)   
            0x04,           // unsigned(4)   
            0x08,           // unsigned(8)   
            0x04,           // unsigned(4)   
            0x10,           // unsigned(16)  
            0x19,0x01,0x00  // unsigned(256) 
    };

    cbor::cbor_bytes_cursor cursor(input);
    for (; !cursor.done(); cursor.next())
    {
        const auto& event = cursor.current();
        switch (event.event_type())
        {
            case staj_event_type::begin_array:
                std::cout << event.event_type() 
                          << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::end_array:
                std::cout << event.event_type() 
                          << " " << "(" << event.tag() << ")\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << event.event_type() 
                          << ": " << event.get<uint64_t>() << " " << "(" << event.tag() << ")\n";
                break;
            default:
                std::cout << "Unhandled event type " << event.event_type() 
                          << " " << "(" << event.tag() << ")\n";
                break;
        }
    }
}
```
Output:
```
begin_array (multi-dim-column-major)
begin_array (n/a)
uint64_value: 2 (n/a)
uint64_value: 3 (n/a)
end_array (n/a)
begin_array (n/a)
uint64_value: 2 (n/a)
uint64_value: 4 (n/a)
uint64_value: 8 (n/a)
uint64_value: 4 (n/a)
uint64_value: 10 (n/a)
uint64_value: 100 (n/a)
end_array (n/a)
end_array (n/a)
```

### Typed Array examples (since 1.8.0)

#### Read a typed array

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>
#include <cassert>

namespace cbor = jsoncons::cbor;

int main()
{
    std::vector<uint8_t> data = {
        0xd8, // Tag
        0x56, // Tag 86, float64, little endian, Typed Array
        0x58, 0x20, // Byte string value of length 32
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x40,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x40,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x40,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x40
    };

    cbor::cbor_bytes_cursor cursor(data);
    assert(jsoncons::staj_events::begin_array == cursor.current().event_type()); 
    assert(cursor.is_typed_array());

    std::vector<double> v;
    cursor.read_typed_array(v);
    for (auto item : v)
    {
        std::cout << item << "\n";
    }
    std::cout << "\n";
}
```
Output:
```
10
20
30
40
```

#### Read a CBOR 3D Typed Array

```cpp
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>
#include <cassert>

namespace cbor = jsoncons::cbor;

int main() 
{
    // A 2 x 3 x 2 3D array
    std::vector<uint8_t> data = {0xD8, 0x28,  // tag(40) row major storage 
        0x82,                                 // array(2)
        0x83,                                 // dimensions array(3)
        0x02, 0x03, 0x02,                     // [2, 3, 2]
        0xD8, 0x40,                           // tag(64) uint8 typed array
        0x4C,                                 // bytes(12)
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
        0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C
    };

    // Read CBOR data to a json value
    jsoncons::json_decoder<jsoncons::json> decoder;
    cbor::cbor_bytes_reader reader(data, decoder);
    std::error_code ec;
    reader.read(ec);
    jsoncons::json result = decoder.get_result();
    std::cout << "(1) " << result << "\n\n";

    // Access CBOR data using a cursor
    cbor::cbor_bytes_cursor cursor(data);

    assert(jsoncons::staj_events::begin_array == cursor.current().event_type());
    assert(true == cursor.is_multi_dim());
    assert(true == cursor.is_typed_array());

    auto extents = cursor.extents();
    std::cout << "(2) ";
    for (std::size_t i = 0; i < extents.size(); ++i)
    {
        if (i > 0) std::cout << " x ";
        std::cout << extents[i];
    }
    std::cout << "\n\n";

    std::vector<int> v;
    cursor.read_typed_array(v);
    std::cout << "(3) [";
    for (std::size_t i = 0; i < v.size(); ++i)
    {
        if (i > 0) std::cout << ',';
        std::cout << v[i];
    }
    std::cout << "]\n\n";
    assert(jsoncons::staj_events::end_array == cursor.current().event_type());
}
```

Output:

```
(1) [[[1,2],[3,4],[5,6]],[[7,8],[9,10],[11,12]]]

(2) 2 x 3 x 2

(3) [1,2,3,4,5,6,7,8,9,10,11,12]
```

### See also

[staj_event](../corelib/basic_staj_event.md)  

[staj_array_iterator](../corelib/staj_array_iterator.md)  

[staj_object_iterator](../corelib/staj_object_iterator.md)  


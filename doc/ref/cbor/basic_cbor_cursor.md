### jsoncons::cbor::basic_cbor_cursor

__`jsoncons_ext/cbor/cbor_cursor.hpp`__

```c++
template<
    class Src=jsoncons::binary_stream_source,
    class Allocator=std::allocator<char>>
class basic_cbor_cursor;
```

A pull parser for reporting CBOR parse events. A typical application will 
repeatedly process the `current()` event and call the `next()`
function to advance to the next event, until `done()` returns `true`.

`basic_cbor_cursor` is noncopyable and nonmoveable.

Typedefs for common sources are provided:

Type                |Definition
--------------------|------------------------------
cbor_stream_cursor  |basic_cbor_cursor<jsoncons::binary_stream_source>
cbor_bytes_cursor   |basic_cbor_cursor<jsoncons::bytes_source>

### Implemented interfaces

[staj_reader](staj_reader.md)

#### Constructors

    template <class Source>
    basic_cbor_cursor(Source&& source); // (1)

    template <class Source>
    basic_cbor_cursor(Source&& source,
                      std::function<bool(const staj_event&, const ser_context&)> filter); // (2)

    template <class Source>
    basic_cbor_cursor(Source&& source, std::error_code& ec); // (3)

    template <class Source>
    basic_cbor_cursor(Source&& source,
                      std::function<bool(const staj_event&, const ser_context&)> filter, 
                      std::error_code& ec); // (4)

Constructor3 (1)-(2) read from a buffer or stream source and throw a 
[ser_error](ser_error.md) if a parsing error is encountered while processing the initial event.

Constructor3 (3)-(4) read from a buffer or stream source and set `ec`
if a parsing error is encountered while processing the initial event.

Note: It is the programmer's responsibility to ensure that `basic_cbor_cursor` does not outlive any source passed in the constuctor, 
as `basic_cbor_cursor` holds a pointer to but does not own this object.

#### Parameters

`source` - a value from which a `source_type` is constructible. 

#### Member functions

    bool done() const override;
Checks if there are no more events.

    const staj_event& current() const override;
Returns the current [staj_event](staj_event.md).

    void read(json_content_handler& handler) override
Feeds the current and succeeding [staj events](staj_event.md) through the provided
[handler](basic_json_content_handler.md), until the handler indicates
to stop. If a parsing error is encountered, throws a [ser_error](ser_error.md).

    void read(json_content_handler& handler, std::error_code& ec) override
Feeds the current and succeeding [staj events](staj_event.md) through the provided
[handler](basic_json_content_handler.md), until the handler indicates
to stop. If a parsing error is encountered, sets `ec`.

    void next() override;
Advances to the next event. If a parsing error is encountered, throws a 
[ser_error](ser_error.md).

    void next(std::error_code& ec) override;
Advances to the next event. If a parsing error is encountered, sets `ec`.

    const ser_context& context() const override;
Returns the current [context](ser_context.md)

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

```c++
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
                std::cout << "begin_array\n";
                break;
            case staj_event_type::end_array:
                std::cout << "end_array\n";
                break;
            case staj_event_type::begin_object:
                std::cout << "begin_object\n";
                break;
            case staj_event_type::end_object:
                std::cout << "end_object\n";
                break;
            case staj_event_type::name:
                // Or std::string_view, if supported
                std::cout << "name: " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::string_value:
                // Or std::string_view, if supported
                std::cout << "string_value: " << event.get<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::null_value:
                std::cout << "null_value: " << "\n";
                break;
            case staj_event_type::bool_value:
                std::cout << "bool_value: " << std::boolalpha << event.get<bool>() << "\n";
                break;
            case staj_event_type::int64_value:
                std::cout << "int64_value: " << event.get<int64_t>() << "\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << "uint64_value: " << event.get<uint64_t>() << "\n";
                break;
            case staj_event_type::half_value:
            case staj_event_type::double_value:
                std::cout << "double_value: " << event.get<double>() << "\n";
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
name: author
string_value: Haruki Murakami
name: title
string_value: Hard-Boiled Wonderland and the End of the World
name: isbn
string_value: 0679743464
name: publisher
string_value: Vintage
name: date
string_value: 1993-03-02
name: price
double_value: 19
end_object
begin_object
name: author
string_value: Graham Greene
name: title
string_value: The Comedians
name: isbn
string_value: 0099478374
name: publisher
string_value: Vintage Classics
name: date
string_value: 2005-09-21
name: price
double_value: 16
end_object
end_array
```

#### Filter CBOR parse events

```c++
#include <jsoncons_ext/cbor/cbor_cursor.hpp>
#include <string>
#include <fstream>

using namespace jsoncons;

struct author_filter 
{
    bool accept_next_ = false;

    bool operator()(const staj_event& event, const ser_context&) 
    {
        if (event.event_type()  == staj_event_type::name &&
            event.get<jsoncons::string_view>() == "author")
        {
            accept_next_ = true;
            return false;
        }
        else if (accept_next_)
        {
            accept_next_ = false;
            return true;
        }
        else
        {
            accept_next_ = false;
            return false;
        }
    }
};

int main()
{
    std::ifstream is("book_catalog.json");

    author_filter filter;
    cbor_cursor cursor(is, filter);

    for (; !cursor.done(); cursor.next())
    {
        const auto& event = cursor.current();
        switch (event.event_type())
        {
            case staj_event_type::string_value:
                std::cout << event.as<jsoncons::string_view>() << "\n";
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

### Typed Array examples

#### Read a typed array

```c++
#include <jsoncons/json.hpp>
#include <jsoncons_ext/cbor/cbor.hpp>
#include <iostream>
#include <iomanip>
#include <cassert>

struct my_cbor_content_handler : public default_json_content_handler
{
    std::vector<double> v;
private:
    bool do_typed_array(const span<const double>& data,  
                        semantic_tag,
                        const ser_context&,
                        std::error_code&) override
    {
        v = std::vector<double>(data.begin(),data.end());
        return false;
    }
};

int main()
{
    std::vector<double> v{10.0,20.0,30.0,40.0};

    std::vector<uint8_t> buffer;
    cbor::cbor_options options;
    options.enable_typed_arrays(true);
    cbor::encode_cbor(v, buffer, options);

    std::cout << "(1)\n";
    std::cout << byte_string_view(buffer.data(),buffer.size()) << "\n\n";
/*
    0xd8, // Tag
        0x56, // Tag 86, float64, little endian, Typed Array
    0x58,0x20, // Byte string value of length 32 
        0x00,0x00,0x00,0x00,0x00,0x00,0x24,0x40,
        0x00,0x00,0x00,0x00,0x00,0x00,0x34,0x40, 
        0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x40, 
        0x00,0x00,0x00,0x00,0x00,0x00,0x44,0x40
*/

    cbor::cbor_bytes_cursor cursor(buffer);
    assert(cursor.current().event_type() == staj_event_type::begin_array);
    assert(cursor.is_typed_array());

    my_cbor_content_handler handler;
    cursor.read(handler);
    std::cout << "(2)\n";
    for (auto item : handler.v)
    {
        std::cout << item << "\n";
    }
    std::cout << "\n";
}
```
Output:
```
(1)
d8 56 58 20 00 00 00 00 00 00 24 40 00 00 00 00 00 00 34 40 00 00 00 00 00 00 3e 40 00 00 00 00 00 00 44 40

(2)
10
20
30
40
```

#### Navigating Typed Arrays with cursor - multi-dimensional row major with typed array

This example is taken from [CBOR Tags for Typed Arrays](https://tools.ietf.org/html/draft-ietf-cbor-array-tags-08)

```c++
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

#### Navigating Typed Arrays with cursor - multi-dimensional column major with classical CBOR array

This example is taken from [CBOR Tags for Typed Arrays](https://tools.ietf.org/html/draft-ietf-cbor-array-tags-08)

```c++
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

### See also

- [staj_reader](staj_reader.md) 
- [staj_array_iterator](staj_array_iterator.md) 
- [staj_object_iterator](staj_object_iterator.md)


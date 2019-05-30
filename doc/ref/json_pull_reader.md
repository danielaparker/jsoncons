### jsoncons::json_cursor

```c++
typedef basic_json_cursor<char,std::allocator<char>> json_cursor
```

A pull parser for parsing json events. A typical application will 
repeatedly process the `current()` event and call the `next()`
function to advance to the next event, until `done()` returns `true`.

`json_cursor` is noncopyable and nonmoveable.

#### Header
```c++
#include <jsoncons/json_cursor.hpp>
```

### Implemented interfaces

[staj_reader](staj_reader.md)

#### Constructors

    json_cursor(std::istream& is); // (1)

    json_cursor(std::istream& is,
                     staj_filter& filter); // (2)

    json_cursor(std::istream& is, 
                     const json_decode_options& options); // (3)

    json_cursor(std::istream& is,
                     staj_filter& filter, 
                     const json_decode_options& options); // (4)

    json_cursor(std::istream& is, 
                     parse_error_handler& err_handler); // (5)

    json_cursor(std::istream& is,
                     staj_filter& filter, 
                     parse_error_handler& err_handler); // (6)

    json_cursor(std::istream& is, 
                     const json_decode_options& options,
                     parse_error_handler& err_handler); // (7)

Constructors (1)-(7) read from a `std::istream` and throw a 
[ser_error](ser_error.md) if a parsing error is encountered 
while processing the initial event.

(1) Constructs a `json_cursor` that reads from an input stream `is` of 
JSON text, uses default [json_decode_options](json_decode_options.md)
and a default [parse_error_handler](parse_error_handler.md).

(2) Constructs a `json_cursor` that reads from an input stream `is` of 
JSON text, applies a [staj_filter](staj_filter.md) to the events, uses default [json_decode_options](json_decode_options.md)
and a default [parse_error_handler](parse_error_handler.md).

(3) Constructs a `json_cursor` that reads from an input stream `is` of JSON text, 
uses the specified [json_decode_options](json_decode_options.md)
and a default [parse_error_handler](parse_error_handler.md).

(4) Constructs a `json_cursor` that reads from an input stream `is` of JSON text, 
applies a [staj_filter](staj_filter.md) to the events, 
uses the specified [json_decode_options](json_decode_options.md)
and a default [parse_error_handler](parse_error_handler.md).

(5) Constructs a `json_cursor` that reads from an input stream `is` of JSON text, 
uses default [json_decode_options](json_decode_options.md)
and a specified [parse_error_handler](parse_error_handler.md).

(6) Constructs a `json_cursor` that reads from an input stream `is` of JSON text, 
applies a [staj_filter](staj_filter.md) to the events, 
uses default [json_decode_options](json_decode_options.md)
and a specified [parse_error_handler](parse_error_handler.md).

(7) Constructs a `json_cursor` that reads from an input stream `is` of JSON text, 
applies a [staj_filter](staj_filter.md), 
uses the specified [json_decode_options](json_decode_options.md)
and a specified [parse_error_handler](parse_error_handler.md).

    json_cursor(std::istream& is,
                     std::error_code& ec); // (8)

    json_cursor(std::istream& is,
                     staj_filter& filter,
                     std::error_code& ec); // (9)

    json_cursor(std::istream& is, 
                     const json_decode_options& options,
                     std::error_code& ec); // (10)

    json_cursor(std::istream& is,
                     staj_filter& filter, 
                     const json_decode_options& options,
                     std::error_code& ec); // (11)

    json_cursor(std::istream& is, 
                     parse_error_handler& err_handler,
                     std::error_code& ec); // (12)

    json_cursor(std::istream& is,
                     staj_filter& filter, 
                     parse_error_handler& err_handler,
                     std::error_code& ec); // (13)

    json_cursor(std::istream& is, 
                     const json_decode_options& options,
                     parse_error_handler& err_handler,
                     std::error_code& ec); // (14)

Constructors (8)-(14) read from a `std::istream` and set `ec`
if a parsing error is encountered while processing the initial event.

    json_cursor(string_view_type& s); // (15)

    json_cursor(string_view_type& s, 
                     staj_filter& filter); // (16)

    json_cursor(string_view_type& s, 
                     const json_decode_options& options); // (17)

    json_cursor(string_view_type& s,
                     staj_filter& filter, 
                     const json_decode_options& options); // (18)

    json_cursor(string_view_type& s, 
                     parse_error_handler& err_handler); // (19)

    json_cursor(string_view_type& s,
                     staj_filter& filter, 
                     parse_error_handler& err_handler); // (20)

    json_cursor(string_view_type& s, 
                     const json_decode_options& options,
                     parse_error_handler& err_handler); // (21)

Constructors (15)-(21) read from a `string_view_type` and throw a 
[ser_error](ser_error.md) if a parsing error s encountered 
while processing the initial event.

    json_cursor(std::string_view_type& s,
                     std::error_code& ec); // (22)

    json_cursor(std::string_view_type& s,
                     staj_filter& filter,
                     std::error_code& ec); // (23)

    json_cursor(std::string_view_type& s, 
                     const json_decode_options& options,
                     std::error_code& ec); // (24)

    json_cursor(std::string_view_type& s,
                     staj_filter& filter, 
                     const json_decode_options& options,
                     std::error_code& ec); // (25)

    json_cursor(std::string_view_type& s, 
                     parse_error_handler& err_handler,
                     std::error_code& ec); // (26)

    json_cursor(std::string_view_type& s,
                     staj_filter& filter, 
                     parse_error_handler& err_handler,
                     std::error_code& ec); // (27)

    json_cursor(std::string_view_type& s, 
                     const json_decode_options& options,
                     parse_error_handler& err_handler,
                     std::error_code& ec); // (28)

Constructors (22)-(28) read from a `string_view_type` and set `ec` 
if a parsing error is encountered while processing the initial event.

Note: It is the programmer's responsibility to ensure that `json_cursor` does not outlive an error handler passed in the constuctor.

#### Member functions

    bool done() const override;
Checks if there are no more events.

    const staj_event& current() const override;
Returns the current [staj_event](staj_event.md).

    void accept(json_content_handler& handler) override
Sends the parse events from the current event to the
matching completion event to the supplied [handler](json_content_handler.md)
E.g., if the current event is `begin_object`, sends the `begin_object`
event and all inbetween events until the matching `end_object` event.
If a parsing error is encountered, throws a [ser_error](ser_error.md).

    void accept(json_content_handler& handler,
                std::error_code& ec) override
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

### Examples

The example JSON text, `book_catalog.json`, is used by the examples below.

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

#### Reading a JSON stream

```c++
#include <jsoncons/json_cursor.hpp>
#include <string>
#include <fstream>

using namespace jsoncons;

int main()
{
    std::ifstream is("book_catalog.json");

    json_cursor reader(is);

    for (; !reader.done(); reader.next())
    {
        const auto& event = reader.current();
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
                // If underlying type is string, can return as string_view
                std::cout << "name: " << event.as<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::string_value:
                std::cout << "string_value: " << event.as<jsoncons::string_view>() << "\n";
                break;
            case staj_event_type::null_value:
                std::cout << "null_value: " << event.as<std::string>() << "\n";
                break;
            case staj_event_type::bool_value:
                std::cout << "bool_value: " << event.as<std::string>() << "\n";
                // or std::cout << "bool_value: " << event.as<bool>() << "\n";
                break;
            case staj_event_type::int64_value:
                std::cout << "int64_value: " << event.as<std::string>() << "\n";
                // or std::cout << "int64_value: " << event.as<int64_t>() << "\n";
                break;
            case staj_event_type::uint64_value:
                std::cout << "uint64_value: " << event.as<std::string>() << "\n";
                // or std::cout << "int64_value: " << event.as<uint64_t>() << "\n";
                break;
            case staj_event_type::double_value:
                std::cout << "double_value: " << event.as<std::string>() << "\n";
                // or std::cout << "double_value: " << event.as<double>() << "\n";
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
double_value: 18.90
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
double_value: 15.74
end_object
end_array
```

#### Filtering a JSON stream

```c++
#include <jsoncons/json_cursor.hpp>
#include <string>
#include <fstream>

using namespace jsoncons;

class author_filter : public staj_filter
{
    bool accept_next_ = false;
public:
    bool accept(const staj_event& event, const ser_context&) override
    {
        if (event.event_type()  == staj_event_type::name &&
            event.as<jsoncons::string_view>() == "author")
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
    json_cursor reader(is, filter);

    for (; !reader.done(); reader.next())
    {
        const auto& event = reader.current();
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

#### See also

- [staj_array_iterator](staj_array_iterator.md) 
- [staj_object_iterator](staj_object_iterator.md)


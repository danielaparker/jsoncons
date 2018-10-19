### jsoncons::json_stream_reader

```c++
typedef basic_json_stream_reader<char,std::allocator<char>> json_stream_reader
```

A pull parser for parsing json events. A typical application will 
repeatedly process the `current()` event and call the `next()`
function to advance to the next event, until `done()` returns `true`.

`json_stream_reader` is noncopyable and nonmoveable.

#### Header
```c++
#include <jsoncons/json_stream_reader.hpp>
```

### Implemented interfaces

[stream_reader](stream_reader.md)

#### Constructors

    basic_json_stream_reader(std::istream& is); // (1)

    basic_json_stream_reader(std::istream& is,
                             stream_filter& filter); // (2)

    basic_json_stream_reader(std::istream& is, 
                             const json_read_options& options); // (3)

    basic_json_stream_reader(std::istream& is,
                             stream_filter& filter, 
                             const json_read_options& options); // (4)

    json_stream_reader(std::istream& is, 
                       parse_error_handler& err_handler); // (5)

    json_stream_reader(std::istream& is,
                       stream_filter& filter, 
                       parse_error_handler& err_handler); // (6)

    json_stream_reader(std::istream& is, 
                       const json_read_options& options,
                       parse_error_handler& err_handler); // (7)

(1) Constructs a `json_reader` that reads from an input stream `is` of 
JSON text, uses default [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(2) Constructs a `json_reader` that reads from an input stream `is` of 
JSON text, applies a [stream_filter](stream_filter.md) to the events, uses default [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(3) Constructs a `json_reader` that reads from an input stream `is` of JSON text, 
uses the specified [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(4) Constructs a `json_reader` that reads from an input stream `is` of JSON text, 
applies a [stream_filter](stream_filter.md) to the events, 
uses the specified [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(5) Constructs a `json_reader` that reads from an input stream `is` of JSON text, 
uses default [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

(6) Constructs a `json_reader` that reads from an input stream `is` of JSON text, 
applies a [stream_filter](stream_filter.md) to the events, 
uses default [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

(7) Constructs a `json_reader` that reads from an input stream `is` of JSON text, 
applies a [stream_filter](stream_filter.md), 
uses the specified [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

Note: It is the programmer's responsibility to ensure that `json_stream_reader` does not outlive any input stream, and error handler passed in the constuctor.

#### Member functions

    bool done() const override;
Checks if there are no more events.

    const stream_event& current() const override;
Returns the current [stream_event](stream_event.md).

    void next() override;
Advances to the next event. 

    const serializing_context& context() const override;
Returns the current [context](serializing_context.md)

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
#include <jsoncons/json_stream_reader.hpp>
#include <string>
#include <fstream>

using namespace jsoncons;

int main()
{
    std::ifstream is("book_catalog.json");

    json_stream_reader reader(is);

    for (; !reader.done(); reader.next())
    {
        const auto& event = reader.current();
        switch (event.event_type())
        {
            case stream_event_type::begin_array:
                std::cout << "begin_array\n";
                break;
            case stream_event_type::end_array:
                std::cout << "end_array\n";
                break;
            case stream_event_type::begin_object:
                std::cout << "begin_object\n";
                break;
            case stream_event_type::end_object:
                std::cout << "end_object\n";
                break;
            case stream_event_type::name:
                // If underlying type is string, can return as string_view
                std::cout << "name: " << event.as<jsoncons::string_view>() << "\n";
                break;
            case stream_event_type::string_value:
                std::cout << "string_value: " << event.as<jsoncons::string_view>() << "\n";
                break;
            case stream_event_type::null_value:
                std::cout << "null_value: " << event.as<std::string>() << "\n";
                break;
            case stream_event_type::bool_value:
                std::cout << "bool_value: " << event.as<std::string>() << "\n";
                // or std::cout << "bool_value: " << event.as<bool>() << "\n";
                break;
            case stream_event_type::int64_value:
                std::cout << "int64_value: " << event.as<std::string>() << "\n";
                // or std::cout << "int64_value: " << event.as<int64_t>() << "\n";
                break;
            case stream_event_type::uint64_value:
                std::cout << "uint64_value: " << event.as<std::string>() << "\n";
                // or std::cout << "int64_value: " << event.as<uint64_t>() << "\n";
                break;
            case stream_event_type::double_value:
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
#include <jsoncons/json_stream_reader.hpp>
#include <string>
#include <fstream>

using namespace jsoncons;

class author_filter : public stream_filter
{
    bool accept_next_ = false;
public:
    bool accept(const stream_event& event, const serializing_context&) override
    {
        if (event.event_type()  == stream_event_type::name &&
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
    json_stream_reader reader(is, filter);

    for (; !reader.done(); reader.next())
    {
        const auto& event = reader.current();
        switch (event.event_type())
        {
            case stream_event_type::string_value:
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


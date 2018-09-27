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

### Examples

#### Using a json_stream_reader

```c++
#include <jsoncons/json_stream_reader.hpp>
#include <string>
#include <sstream>

using namespace jsoncons;

int main()
{
    std::string s = R"(
    [
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55              
        },
        {
            "enrollmentNo" : 101,
            "firstName" : "Catherine",
            "lastName" : "Smith",
            "mark" : 95              
        },
        {
            "enrollmentNo" : 102,
            "firstName" : "William",
            "lastName" : "Skeleton",
            "mark" : 60              
        }
    ]
    )";

    std::istringstream is(s);

    json_stream_reader reader(is);

    for (; !reader.done(); reader.next())
    {
        const auto& event = reader.current();
        switch (event.event_type())
        {
            case stream_event_type::name:
                // Returned data is string, so can use as<jsoncons::string_view>()>()
                std::cout << event.as<jsoncons::string_view>() << ": ";
                break;
            case stream_event_type::string_value:
                // Can use as<std::string_view>() if your compiler supports it
                std::cout << event.as<jsoncons::string_view>() << "\n";
                break;
            case stream_event_type::int64_value:
            case stream_event_type::uint64_value:
                // Converts integer value to std::string
                std::cout << event.as<std::string>() << "\n";
                break;
        }
    }
}
```
Output:
```
enrollmentNo: 100
firstName: Tom
lastName: Cochrane
mark: 55
enrollmentNo: 101
firstName: Catherine
lastName: Smith
mark: 95
enrollmentNo: 102
firstName: William
lastName: Skeleton
mark: 60
```

#### Using a json_stream_reader with a stream_filter

```c++
#include <jsoncons/json_stream_reader.hpp>
#include <string>
#include <sstream>

using namespace jsoncons;

class first_name_filter : public stream_filter
{
    bool accept_next_ = false;
public:
    bool accept(const stream_event& event) override
    {
        if (event.event_type()  == stream_event_type::name &&
            event.as<jsoncons::string_view>() == "firstName")
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
    std::string s = R"(
    [
        {
            "enrollmentNo" : 100,
            "firstName" : "Tom",
            "lastName" : "Cochrane",
            "mark" : 55              
        },
        {
            "enrollmentNo" : 101,
            "firstName" : "Catherine",
            "lastName" : "Smith",
            "mark" : 95              
        },
        {
            "enrollmentNo" : 102,
            "firstName" : "William",
            "lastName" : "Skeleton",
            "mark" : 60              
        }
    ]
    )";

    std::istringstream is(s);

    first_name_filter filter;
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
Tom
Catherine
William
```


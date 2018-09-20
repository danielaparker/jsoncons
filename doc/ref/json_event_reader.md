### jsoncons::json_event_reader

```c++
typedef basic_json_event_reader<char,std::allocator<char>> json_event_reader
```

`json_event_reader` is noncopyable and nonmoveable.

#### Header
```c++
#include <jsoncons/json_event_reader.hpp>
```

A pull parser for parsing json events.

#### Constructors

    basic_json_event_reader(std::istream& is); // (1)

    basic_json_event_reader(std::istream& is, 
                            const json_read_options& options); // (2)

    json_event_reader(std::istream& is, 
                      parse_error_handler& err_handler); // (3)

    json_reader(std::istream& is, 
                const json_read_options& options,
                parse_error_handler& err_handler); // (4)

(1) Constructs a `json_reader` that reads from an input stream `is` of 
JSON text, uses default [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(2) Constructs a `json_reader` that reads from an input stream `is` of JSON text, 
uses the specified [json_read_options](json_read_options)
and a default [parse_error_handler](parse_error_handler.md).

(3) Constructs a `json_reader` that reads from an input stream `is` of JSON text, 
uses default [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

(4) Constructs a `json_reader` that reads from an input stream `is` of JSON text, 
uses the specified [json_read_options](json_read_options)
and a specified [parse_error_handler](parse_error_handler.md).

Note: It is the programmer's responsibility to ensure that `json_event_reader` does not outlive any input stream, and error handler passed in the constuctor.

#### Member functions

    bool done() const;
Check if there are no more events.

    const json_event& current() const;
Returns the current [json_event](json_event.md).

    void next();
Get the next event. 

### Examples

#### Using json_event_reader

```c++
#include <jsoncons/json_event_reader.hpp>
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

    json_event_reader event_reader(is);

    for (; !event_reader.done(); event_reader.next())
    {
        const auto& event = event_reader.current();
        switch (event.event_type())
        {
            case json_event_type::name:
                std::cout << event.as<std::string>() << ": ";
                break;
            case json_event_type::string_value:
                std::cout << event.as<std::string>() << "\n";
                break;
            case json_event_type::int64_value:
            case json_event_type::uint64_value:
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


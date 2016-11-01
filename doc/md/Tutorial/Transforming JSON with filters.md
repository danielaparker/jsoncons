Suppose you have a JSON address book file (`address-book.json`) that looks like this
```json
{
    "address-book" : 
    [
        {
            "name":"Jane Roe",
            "email":"jane.roe@example.com"
        },
        {
             "name":"John",
             "email" : "john.doe@example.com"
         }
    ]
}
```

Now suppose you want to break the name into a first name and last name, and report a warning when `name` does not contain a space or tab separator. 

You can achieve the desired result by subclassing the [json_filter](json_filter) class, overriding the default methods for receiving name and string value events, and passing modified events on to the parent [json_input_handler](json_input_handler) (which in this example will forward them to a [json_serializer](json_serializer).) 
```c++
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/json_reader.hpp>

using namespace jsoncons;

class name_fix_up_filter : public json_filter
{
public:
    name_fix_up_filter(json_output_handler& handler)
        : json_filter(handler)
    {
    }

private:
    void do_name(const char* p, size_t length) override
    {
        member_name_ = std::string(p, length);
        if (member_name_ != "name")
        {
            output_handler().name(p, length);
        }
    }

    void do_string_value(const char* p, size_t length) override
    {
        if (member_name_ == "name")
        {
            std::string value(p, length);
            size_t end_first = value.find_first_of(" \t");
            size_t start_last = value.find_first_not_of(" \t", end_first);
            output_handler().name("first-name");
            std::string first = value.substr(0, end_first);
            output_handler().value(first);
            if (start_last != std::string::npos)
            {
                output_handler().name("last-name");
                std::string last = value.substr(start_last);
                output_handler().value(last);
            }
        }
        else
        {
            output_handler().value(p, length);
        }
    }

    std::string member_name_;
};
```
In your code you will pass `name_fix_up_filter` to the constructor of [json_reader](json_reader), and call `read_next`
```c++
    std::string in_file = "input/address-book.json";
    std::string out_file = "output/address-book-new.json";
    std::ifstream is(in_file, std::ofstream::binary);
    std::ofstream os(out_file);

    json_serializer serializer(os, true);
    name_fix_up_filter filter(serializer);
    json_reader reader(is, filter);
    reader.read_next();
```
The output is a address book file (`address-book-new.json`) with the required change
```json
    {
        "address-book":
        [
            {
                "first-name":"Jane",
                "last-name":"Roe",
                "email":"jane.roe@example.com"
            },
            {
                "first-name":"John",
                "email":"john.doe@example.com"
            }
        ]
    }
```


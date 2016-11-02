### Rename object member names

You can rename object member names with the built in filter `rename_name_filter`

```c++
#include "jsoncons/json.hpp"
#include "jsoncons/json_filter.hpp"

using namespace jsoncons;

int main()
{
    ojson j = ojson::parse(R"({"first":1,"second":2,"fourth":3})");

    ojson_serializer serializer(std::cout);

    rename_name_filter filter("fourth","third",serializer);
    j.write(filter);
}
```
Output:
```json
{"first":1,"second":2,"third":3}
```

### Fix up names in an address book JSON file

Example address book file (`address-book.json`):
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

Suppose you want to break the name into a first name and last name, and report a warning when `name` does not contain a space or tab separated part. 

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
            this->output_handler().name(p, length);
        }
    }

    void do_string_value(const char* p, size_t length) override
    {
        if (member_name_ == "name")
        {
            std::string value(p, length);
            size_t end_first = value.find_first_of(" \t");
            size_t start_last = value.find_first_not_of(" \t", end_first);
            this->output_handler().name("first-name");
            std::string first = value.substr(0, end_first);
            this->output_handler().value(first);
            if (start_last != std::string::npos)
            {
                this->output_handler().name("last-name");
                std::string last = value.substr(start_last);
                this->output_handler().value(last);
            }
            else
            {
                std::cerr << "Incomplete name \"" << value
                   << "\" at line " << this->context().line_number()
                   << " and column " << this->context().column_number() << std::endl;
            }
        }
        else
        {
            this->output_handler().value(p, length);
        }
    }

    std::string member_name_;
};
```
In your code you will pass `name_fix_up_filter` to the constructor of [json_reader](json_reader), and call `read_next`
```c++
std::string in_file = "input/address-book.json";
std::string out_file = "output/new-address-book1.json";
std::ifstream is(in_file);
std::ofstream os(out_file);

json_serializer serializer(os, true);
name_fix_up_filter filter(serializer);
json_reader reader(is, filter);
reader.read_next();
```
Output:
(1) An address book file (`address-book-new.json`) with name fixes
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
(2) Warning messages
```
Incomplete name "John" at line 9 and column 26 
```
### Fix up names in an address book `json` value

```c++
    std::string in_file = "input/address-book.json";
    std::string out_file = "output/new-address-book2.json";
    std::ifstream is(in_file);
    std::ofstream os(out_file);

    json j;
    is >> j;

    json_serializer serializer(os, true);
    name_fix_up_filter filter(serializer);
    j.write(filter);
```
Output:
(1) An address book file (`address-book-new.json`) with name fixes
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
(2) Warning messages
``` 
Incomplete name "John" at line 0 and column 0
```
Note that when filtering `json` events written from a `json` value to an output handler, information about the location of tokens in the original file has been lost. 


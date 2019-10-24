### jsoncons::basic_json_content_filter

```c++
#include <jsoncons/json_content_filter.hpp>

template <
    class CharT
> class basic_json_content_filter
```

Defines an interface for filtering JSON events. 

`basic_json_content_filter` is noncopyable and nonmoveable.

![json_content_filter](./diagrams/json_content_filter.png)

Typedefs for common character types are provided:

Type                |Definition
--------------------|------------------------------
json_content_filter    |`basic_json_content_filter<char>`
wjson_content_filter   |`basic_json_content_filter<wchar_t>`

#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`char_type`|CharT
`string_view_type`|A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [std::string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if C++ 17 is detected or if `JSONCONS_HAS_STRING_VIEW` is defined, otherwise proxied.  

#### Constructors

    basic_json_content_filter(basic_json_content_handler<char_type>& handler)
All JSON events that pass through the `basic_json_content_filter` go to the specified [handler](basic_json_content_handler.md).
You must ensure that the `handler` exists as long as does `basic_json_content_filter`, as `basic_json_content_filter` holds a pointer to but does not own this object.

#### Accessors

    basic_json_content_handler<char_type>& to_handler()
Returns a reference to the JSON handler that sends json events to the destination handler. 

#### Inherited from [jsoncons::basic_json_content_handler](basic_json_content_handler.md)

    bool begin_object(semantic_tag tag=semantic_tag::none,
                      const ser_context& context=null_ser_context()); // (1)

    bool begin_object(size_t length, 
                      semantic_tag tag=semantic_tag::none, 
                      const ser_context& context = null_ser_context()); // (2)

    bool end_object(const ser_context& context = null_ser_context()); // (3)

    bool begin_array(semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (4)

    bool begin_array(size_t length, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (5)

    bool end_array(const ser_context& context=null_ser_context()); // (6)

    bool name(const string_view_type& name, 
              const ser_context& context=null_ser_context()); // (7)

    bool null_value(semantic_tag tag = semantic_tag::none,
                    const ser_context& context=null_ser_context()); // (8) 

    bool bool_value(bool value, 
                    semantic_tag tag = semantic_tag::none,
                    const ser_context& context=null_ser_context()); // (9) 

    bool string_value(const string_view_type& value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()); // (10) 

    bool byte_string_value(const byte_string_view& b, 
                           semantic_tag tag=semantic_tag::none, 
                           const ser_context& context=null_ser_context()); // (11)

    bool uint64_value(uint64_t value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()); // (12)

    bool int64_value(int64_t value, 
                     semantic_tag tag = semantic_tag::none, 
                     const ser_context& context=null_ser_context()); // (13)

    bool half_value(uint16_t value, 
                    semantic_tag tag = semantic_tag::none, 
                    const ser_context& context=null_ser_context()); // (14)

    bool double_value(double value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()); // (15)

    bool begin_object(semantic_tag tag,
                      const ser_context& context,
                      std::error_code& ec); // (16)

    bool begin_object(size_t length, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (17)

    bool end_object(const ser_context& context, 
                    std::error_code& ec); // (18)

    bool begin_array(semantic_tag tag, 
                     const ser_context& context, 
                     std::error_code& ec); // (19)

    bool begin_array(size_t length, 
                     semantic_tag tag, 
                     const ser_context& context, 
                     std::error_code& ec); // (20)

    bool end_array(const ser_context& context, 
                   std::error_code& ec); // (21)

    bool name(const string_view_type& name, 
              const ser_context& context, 
              std::error_code& ec); // (22)

    bool null_value(semantic_tag tag,
                    const ser_context& context,
                    std::error_code& ec); // (23) 

    bool bool_value(bool value, 
                    semantic_tag tag,
                    const ser_context& context,
                    std::error_code& ec); // (24) 

    bool string_value(const string_view_type& value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (25) 

    bool byte_string_value(const byte_string_view& b, 
                           semantic_tag tag, 
                           const ser_context& context,
                           std::error_code& ec); // (26)

    bool uint64_value(uint64_t value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (27)

    bool int64_value(int64_t value, 
                     semantic_tag tag, 
                     const ser_context& context,
                     std::error_code& ec); // (28)

    bool half_value(uint16_t value, 
                    semantic_tag tag, 
                    const ser_context& context,
                    std::error_code& ec); // (29)

    bool double_value(double value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (30)

(1) Indicates the begining of an object of indefinite length.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(2) Indicates the begining of an object of known length. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(3) Indicates the end of an object.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(4) Indicates the beginning of an indefinite length array. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(5) Indicates the beginning of an array of known length. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(6) Indicates the end of an array.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(7) Writes the name part of an object name-value pair.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(8) Writes a null value. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(9) Writes a boolean value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(10) Writes a text string value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(11) Writes a byte string value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(12) Writes a non-negative integer value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(13) Writes a signed integer value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(14) Writes a half precision floating point value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(15) Writes a double precision floating point value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(16)-(30) Same as (1)-(15), except sets `ec` and returns `false` on parse errors.

    void flush()
Flushes whatever is buffered to the destination.

### See also

- [basic_json_content_handler](basic_json_content_handler.md)

### Examples

#### Rename object member names with the built in filter [rename_object_member_filter](rename_object_member_filter.md)

```c++
#include <sstream>
#include <jsoncons/json.hpp>
#include <jsoncons/json_content_filter.hpp>

using namespace jsoncons;

int main()
{
    std::string s = R"({"first":1,"second":2,"fourth":3,"fifth":4})";    

    json_stream_encoder encoder(std::cout);

    // Filters can be chained
    rename_object_member_filter filter2("fifth", "fourth", encoder);
    rename_object_member_filter filter1("fourth", "third", filter2);

    // A filter can be passed to any function that takes
    // a json_content_handler ...
    std::cout << "(1) ";
    std::istringstream is(s);
    json_reader reader(is, filter1);
    reader.read();
    std::cout << std::endl;

    // or a json_content_handler    
    std::cout << "(2) ";
    ojson j = ojson::parse(s);
    j.dump(filter1);
    std::cout << std::endl;
}
```
Output:
```json
(1) {"first":1,"second":2,"third":3,"fourth":4}
(2) {"first":1,"second":2,"third":3,"fourth":4}
```

#### Fix up names in an address book JSON file

Input JSON file `address-book.json`:

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

You can achieve the desired result by subclassing the [basic_json_content_filter](basic_json_content_filter.md) class, overriding the default methods for receiving name and string value events, and passing modified events on to the parent [json_content_handler](basic_json_content_handler.md) (which in this example will forward them to a [basic_json_encoder](basic_json_encoder.md).) 
```c++
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_content_filter.hpp>
#include <jsoncons/json_reader.hpp>

using namespace jsoncons;


class name_fix_up_filter : public json_content_filter
{
    std::string member_name_;

public:
    name_fix_up_filter(json_content_handler& handler)
        : json_content_filter(handler)
    {
    }

private:
    bool do_name(const string_view_type& name, 
                 const ser_context& context,
                 std::error_code&) override
    {
        member_name_ = name;
        if (member_name_ != "name")
        {
            this->to_handler().write_name(name, context);
        }
        return true;
    }

    bool do_string_value(const string_view_type& s, 
                         const ser_context& context,
                         std::error_code&) override
    {
        if (member_name_ == "name")
        {
            size_t end_first = val.find_first_of(" \t");
            size_t start_last = val.find_first_not_of(" \t", end_first);
            this->to_handler().write_name("first-name", context);
            string_view_type first = val.substr(0, end_first);
            this->to_handler().value(first, context);
            if (start_last != string_view_type::npos)
            {
                this->to_handler().write_name("last-name", context);
                string_view_type last = val.substr(start_last);
                this->to_handler().value(last, context);
            }
            else
            {
                std::cerr << "Incomplete name \"" << s
                   << "\" at line " << context.line()
                   << " and column " << context.column() << std::endl;
            }
        }
        else
        {
            this->to_handler().value(s, context);
        }
        return true;
    }
};
```
Configure a [rename_object_member_filter](rename_object_member_filter.md) to emit json events to a [basic_json_encoder](basic_json_encoder.md). 
```c++
std::ofstream os("output/new-address-book.json");
json_encoder encoder(os, jsoncons::indenting::indent);
name_fix_up_filter filter(encoder);
```
Parse the input and send the json events into the filter ...
```c++
std::cout << "(1) ";
std::ifstream is("input/address-book.json");
json_reader reader(is, filter);
reader.read();
std:: << "\n";
```
or read into a json value and write to the filter
```c++
std::cout << "(2) ";
json j;
is >> j;
j.dump(filter);
std:: << "\n";
```
Output:
```
(1) Incomplete name "John" at line 9 and column 26 
(2) Incomplete name "John" at line 0 and column 0
```
Note that when filtering `json` events written from a `json` value to an output handler, contexual line and column information in the original file has been lost. 
```

The output JSON file `address-book-new.json` with name fixes is

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


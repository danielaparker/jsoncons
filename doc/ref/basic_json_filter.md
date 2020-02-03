### jsoncons::basic_json_filter

__`jsoncons/json_filter.hpp`__

```c++
template <
    class CharT
> class basic_json_filter
```

Defines an interface for filtering JSON events. 

`basic_json_filter` is noncopyable and nonmoveable.

![json_filter](./diagrams/json_filter.png)

Typedefs for common character types are provided:

Type                |Definition
--------------------|------------------------------
json_filter    |`basic_json_filter<char>`
wjson_filter   |`basic_json_filter<wchar_t>`

#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`char_type`|CharT
`string_view_type`|A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [std::string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if C++ 17 is detected or if `JSONCONS_HAS_STD_STRING_VIEW` is defined, otherwise proxied.  

#### Constructors

    basic_json_filter(basic_json_content_handler<char_type>& handler)
All JSON events that pass through the `basic_json_filter` go to the specified [handler](basic_json_content_handler.md).
You must ensure that the `handler` exists as long as does `basic_json_filter`, as `basic_json_filter` holds a pointer to but does not own this object.

#### Accessors

    basic_json_content_handler<char_type>& to_handler()
Returns a reference to the JSON handler that sends json events to the destination handler. 

### Inherited from [jsoncons::basic_json_content_handler](basic_json_content_handler.md)

#### Public member functions

    void flush(); // (1)

    bool begin_object(semantic_tag tag=semantic_tag::none,
                      const ser_context& context=null_ser_context()); // (2)

    bool begin_object(std::size_t length, 
                      semantic_tag tag=semantic_tag::none, 
                      const ser_context& context = null_ser_context()); // (3)

    bool end_object(const ser_context& context = null_ser_context()); // (4)

    bool begin_array(semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (5)

    bool begin_array(std::size_t length, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (6)

    bool end_array(const ser_context& context=null_ser_context()); // (7)

    bool name(const string_view_type& name, 
              const ser_context& context=null_ser_context()); // (8)

    bool null_value(semantic_tag tag = semantic_tag::none,
                    const ser_context& context=null_ser_context()); // (9) 

    bool bool_value(bool value, 
                    semantic_tag tag = semantic_tag::none,
                    const ser_context& context=null_ser_context()); // (10) 

    bool string_value(const string_view_type& value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()); // (11) 

    bool byte_string_value(const byte_string_view& b, 
                           semantic_tag tag=semantic_tag::none, 
                           const ser_context& context=null_ser_context()); // (12)

    bool uint64_value(uint64_t value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()); // (13)

    bool int64_value(int64_t value, 
                     semantic_tag tag = semantic_tag::none, 
                     const ser_context& context=null_ser_context()); // (14)

    bool half_value(uint16_t value, 
                    semantic_tag tag = semantic_tag::none, 
                    const ser_context& context=null_ser_context()); // (15)

    bool double_value(double value, 
                      semantic_tag tag = semantic_tag::none, 
                      const ser_context& context=null_ser_context()); // (16)

    bool begin_object(semantic_tag tag,
                      const ser_context& context,
                      std::error_code& ec); // (17)

    bool begin_object(std::size_t length, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (18)

    bool end_object(const ser_context& context, 
                    std::error_code& ec); // (19)

    bool begin_array(semantic_tag tag, 
                     const ser_context& context, 
                     std::error_code& ec); // (20)

    bool begin_array(std::size_t length, 
                     semantic_tag tag, 
                     const ser_context& context, 
                     std::error_code& ec); // (21)

    bool end_array(const ser_context& context, 
                   std::error_code& ec); // (22)

    bool name(const string_view_type& name, 
              const ser_context& context, 
              std::error_code& ec); // (23)

    bool null_value(semantic_tag tag,
                    const ser_context& context,
                    std::error_code& ec); // (24) 

    bool bool_value(bool value, 
                    semantic_tag tag,
                    const ser_context& context,
                    std::error_code& ec); // (25) 

    bool string_value(const string_view_type& value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (26) 

    bool byte_string_value(const byte_string_view& b, 
                           semantic_tag tag, 
                           const ser_context& context,
                           std::error_code& ec); // (27)

    bool uint64_value(uint64_t value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (28)

    bool int64_value(int64_t value, 
                     semantic_tag tag, 
                     const ser_context& context,
                     std::error_code& ec); // (29)

    bool half_value(uint16_t value, 
                    semantic_tag tag, 
                    const ser_context& context,
                    std::error_code& ec); // (30)

    bool double_value(double value, 
                      semantic_tag tag, 
                      const ser_context& context,
                      std::error_code& ec); // (31)

   bool typed_array(const span<const uint8_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (32)

    bool typed_array(const span<const uint16_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (33)

    bool typed_array(const span<const uint32_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (34)

    bool typed_array(const span<const uint64_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (35)

    bool typed_array(const span<const int8_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (36)

    bool typed_array(const span<const int16_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (37)

    bool typed_array(const span<const int32_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (38)

    bool typed_array(const span<const int64_t>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (39)

    bool typed_array(half_arg_t, const span<const uint16_t>& data,
                     semantic_tag tag = semantic_tag::none,
                     const ser_context& context = null_ser_context()); // (40)

    bool typed_array(const span<const float>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (41)

    bool typed_array(const span<const double>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (42)

    bool typed_array(const span<const float128_type>& data, 
                     semantic_tag tag=semantic_tag::none,
                     const ser_context& context=null_ser_context()); // (43)

    bool begin_multi_dim(const span<const size_t>& shape,
                         semantic_tag tag,
                         const ser_context& context); // (44) 

    bool end_multi_dim(const ser_context& context=null_ser_context()); // (45) 

    bool typed_array(const span<const uint8_t>& v, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (46)

    bool typed_array(const span<const uint16_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (47)

    bool typed_array(const span<const uint32_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (48)

    bool typed_array(const span<const uint64_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (49)

    bool typed_array(const span<const int8_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (50)

    bool typed_array(const span<const int16_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (51)

    bool typed_array(const span<const int32_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (52)

    bool typed_array(const span<const int64_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (53)

    bool typed_array(half_arg_t arg, const span<const uint16_t>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (54)

    bool typed_array(const span<const float>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (55)

    bool typed_array(const span<const double>& data, 
                     semantic_tag tag,
                     const ser_context& context, 
                     std::error_code& ec); // (56)

    bool begin_multi_dim(const span<const size_t>& shape,
                         semantic_tag tag,
                         const ser_context& context, 
                         std::error_code& ec); // (57)

    bool end_multi_dim(const ser_context& context,
                       std::error_code& ec); // (58) 

(1) Flushes whatever is buffered to the destination.

(2) Indicates the begining of an object of indefinite length.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(3) Indicates the begining of an object of known length. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(4) Indicates the end of an object.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(5) Indicates the beginning of an indefinite length array. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(6) Indicates the beginning of an array of known length. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(7) Indicates the end of an array.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(8) Writes the name part of an object name-value pair.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(9) Writes a null value. 
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(10) Writes a boolean value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(11) Writes a text string value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(12) Writes a byte string value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(13) Writes a non-negative integer value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(14) Writes a signed integer value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(15) Writes a half precision floating point value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(16) Writes a double precision floating point value.
Returns `true` if the consumer wishes to receive more events, `false` otherwise.
Throws a [ser_error](ser_error.md) on parse errors. 

(17)-(31) Same as (2)-(16), except sets `ec` and returns `false` on parse errors.

#### Private virtual functions

    virtual void do_flush() = 0; // (1)

    virtual bool do_begin_object(semantic_tag tag, 
                                 const ser_context& context, 
                                 std::error_code& ec) = 0; // (2)

    virtual bool do_begin_object(std::size_t length, 
                                 semantic_tag tag, 
                                 const ser_context& context, 
                                 std::error_code& ec); // (3)

    virtual bool do_end_object(const ser_context& context, 
                               std::error_code& ec) = 0; // (4)

    virtual bool do_begin_array(semantic_tag tag, 
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (5)

    virtual bool do_begin_array(std::size_t length, 
                                semantic_tag tag, 
                                const ser_context& context, 
                                std::error_code& ec); // (6)

    virtual bool do_end_array(const ser_context& context, 
                              std::error_code& ec) = 0; // (7)

    virtual bool do_name(const string_view_type& name, 
                         const ser_context& context, 
                         std::error_code&) = 0; // (8)

    virtual bool do_null_value(semantic_tag tag, 
                               const ser_context& context, 
                               std::error_code& ec) = 0; // (9)

    virtual bool do_bool_value(bool value, 
                               semantic_tag tag, 
                               const ser_context& context, 
                               std::error_code&) = 0; // (10)

    virtual bool do_string_value(const string_view_type& value, 
                                 semantic_tag tag, 
                                 const ser_context& context, 
                                 std::error_code& ec) = 0; // (11)

    virtual bool do_byte_string_value(const byte_string_view& value, 
                                      semantic_tag tag, 
                                      const ser_context& context,
                                      std::error_code& ec) = 0; // (12)

    virtual bool do_uint64_value(uint64_t value, 
                                 semantic_tag tag, 
                                 const ser_context& context,
                                 std::error_code& ec) = 0; // (13)

    virtual bool do_int64_value(int64_t value, 
                                semantic_tag tag,
                                const ser_context& context,
                                std::error_code& ec) = 0; // (14)

    virtual bool do_half_value(uint16_t value, 
                               semantic_tag tag,
                               const ser_context& context,
                               std::error_code& ec) = 0; // (15)

    virtual bool do_double_value(double value, 
                                 semantic_tag tag,
                                 const ser_context& context,
                                 std::error_code& ec) = 0; // (16)

    virtual bool do_typed_array(const span<const uint8_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (17)

    virtual bool do_typed_array(const span<const uint16_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (18)

    virtual bool do_typed_array(const span<const uint32_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (19)

    virtual bool do_typed_array(const span<const uint64_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (20)

    virtual bool do_typed_array(const span<const int8_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (21)

    virtual bool do_typed_array(const span<const int16_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (22)

    virtual bool do_typed_array(const span<const int32_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (23)

    virtual bool do_typed_array(const span<const int64_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (24)

    virtual bool do_typed_array(half_arg_t, 
                                const span<const uint16_t>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (25)

    virtual bool do_typed_array(const span<const float>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (26)

    virtual bool do_typed_array(const span<const double>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (27)

    virtual bool do_typed_array(const span<const float128_type>& data, 
                                semantic_tag tag,
                                const ser_context& context, 
                                std::error_code& ec) = 0; // (28)

    virtual bool do_begin_multi_dim(const span<const size_t>& shape,
                                    semantic_tag tag,
                                    const ser_context& context, 
                                    std::error_code& ec) = 0; // (29)

    virtual bool do_end_multi_dim(const ser_context& context,
                                  std::error_code& ec) = 0; // (30)

(1) Allows producers of json events to flush any buffered data.

(2) Handles the beginning of an object of indefinite length.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(3) Handles the beginning of an object of known length.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(4) Handles the end of an object.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(5) Handles the beginning of an array of indefinite length.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(6) Handles the beginning of an array of known length.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(7) Handles the end of an array.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(8) Handles the name part of an object name-value pair.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(9) Handles a null value.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(10) Handles a boolean value. 
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(11) Handles a string value.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(12) Handles a byte string value.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(13) Handles a non-negative integer value.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(14) Handles a signed integer value.
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(15) Handles a half precision floating point value. 
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

(16) Handles a double precision floating point value. 
Returns `true` if the producer should generate more events, `false` otherwise.
Sets `ec` and returns `false` on parse errors. 

#### Parameters

`tag` - a jsoncons semantic tag
`context` - parse context information including line and column number
`ec` - a parse error code

#### Exceptions

The overloads that do not take a `std::error_code&` parameter throw a
[ser_error](ser_error.md) on parse errors, constructed with the error code as the error code argument
and line and column from the `context`. 

The overloads that take a `std::error_code&` parameter set it to the error code and return `false` on parse errors.

### See also

- [basic_json_content_handler](basic_json_content_handler.md)

### Examples

#### Rename object member names with the built in filter [rename_object_member_filter](rename_object_member_filter.md)

```c++
#include <sstream>
#include <jsoncons/json.hpp>
#include <jsoncons/json_filter.hpp>

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

You can achieve the desired result by subclassing the [basic_json_filter](basic_json_filter.md) class, overriding the default methods for receiving name and string value events, and passing modified events on to the parent [json_content_handler](basic_json_content_handler.md) (which in this example will forward them to a [basic_json_encoder](basic_json_encoder.md).) 
```c++
#include <jsoncons/json_encoder.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/json_reader.hpp>

using namespace jsoncons;


class name_fix_up_filter : public json_filter
{
    std::string member_name_;

public:
    name_fix_up_filter(json_content_handler& handler)
        : json_filter(handler)
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
            this->to_handler().name(name, context);
        }
        return true;
    }

    bool do_string_value(const string_view_type& s, 
                         const ser_context& context,
                         std::error_code&) override
    {
        if (member_name_ == "name")
        {
            std::size_t end_first = val.find_first_of(" \t");
            std::size_t start_last = val.find_first_not_of(" \t", end_first);
            this->to_handler().name("first-name", context);
            string_view_type first = val.substr(0, end_first);
            this->to_handler().value(first, context);
            if (start_last != string_view_type::npos)
            {
                this->to_handler().name("last-name", context);
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
json_stream_encoder encoder(os);
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


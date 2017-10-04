### jsoncons::json_output_handler

```c++
typedef basic_json_output_handler<char> json_output_handler
```

The `json_output_handler` class is an instantiation of the `basic_json_output_handler` class template that uses `char` as the character type. It is a pure virtual class that defines an interface for a JSON writer.

#### Header
```c++
#include <jsoncons/json_output_handler.hpp>
```
#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`string_view_type`|A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if `JSONCONS_HAS_STRING_VIEW` is defined in `jsoncons_config.hpp`, otherwise proxied.  

#### Public interface methods

    void begin_json()
Output begin JSON document event. Uses `do_begin_json`.

    void end_json()
Output end JSON document event. Uses `do_end_json`.

    void begin_object()
Output begin JSON object event. Uses `do_begin_object`.

    void end_object()
Output end JSON object event. Uses `do_end_object`.

    void begin_array()
Output begin JSON array event. Uses `do_begin_array`.

    void end_array()
Output end JSON array event. Uses `do_end_array`.

    void name(string_view_type name)
Output name event for a name-value pair. Uses `do_name`.

    void string_value(string_view_type value) 
Output character data event. Uses `do_string_value`.

    void integer_value(int64_t value) 
Output signed integer event. Uses `do_integer_value`.

    void uinteger_value(uint64_t value) 
Output non-negative integer event. Uses `do_uinteger_value`.

    void double_value(double value) 
Output floating point value with default precision (`std::numeric_limits<double>::digits10`.) Uses `do_double_value`.

    void double_value(double value, uint8_t precision) 
Output floating point value with specified precision. Uses `do_double_value`.

    void bool_value(bool value) 
Output boolean value event. Uses `do_bool_value`.

    void null_value() 
Output null value event. Uses `do_null_value`.

#### Private implementation methods

    virtual void do_begin_json()
Receive begin JSON document event

    virtual void do_end_json()
Receive end JSON document event

    virtual void do_end_object()
Receive end JSON object event

    virtual void do_begin_array()
Receive begin JSON array event

    virtual void do_end_array()
Receive end JSON array event

    virtual void do_name(string_view_type name)
Receive name event for name-value pair

    virtual void do_string_value(string_view_type value)
Receive character data event

    virtual void do_integer_value(int64_t value)
Receive signed integer value event

    virtual void do_uinteger_value(uint64_t value)
Receive non-negative integer value event

    virtual void do_double_value(double value, uint8_t precision)
Receive floating point value event

    virtual void do_bool_value(bool value)
Receive a boolean value event

    virtual void do_null_value()
Receive a `null` value event


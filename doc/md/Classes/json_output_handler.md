```c++
jsoncons::json_output_handler

typedef basic_json_output_handler<char> json_output_handler
```

The `json_output_handler` class is an instantiation of the `basic_json_output_handler` class template that uses `char` as the character type. It is a pure virtual class that defines an interface for a JSON writer.

### Header
```c++
#include <jsoncons/json_output_handler.hpp>
```
### Member types

Member type                         |Definition
------------------------------------|------------------------------
`string_view_type`|A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 `std:string_view` if `JSONCONS_HAS_STRING_VIEW` is defined in `jsoncons_config.hpp`, otherwise proxied.  

### Public interface methods

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

    void name(const std::string& value)
Output name event for a name-value pair. Uses `do_name`.

    void name(const CharT* p, size_t length)
Output name event for a name-value pair. Uses `do_name`.

    void value(const CharT* p)
Send a null terminated string event.

    void value(const std::string& value)
Output character data event. Uses `do_string_value`.

    void value(long long value)
Output signed integer event. Uses `do_integer_value`.

    void value(unsigned long long value)
Output non-negative integer event. Uses `do_uinteger_value`.

    void value(double value, uint8_t precision = 0)
Output floating point value event. Uses `do_double_value`.

    void value(bool value)
Output boolean value event. Uses `do_bool_value`.

    void value(null_type)
Output null value event. Uses `do_null_value`.

### Private implementation methods

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


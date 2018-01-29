### jsoncons::json_input_handler

```c++
typedef basic_json_input_handler<char> json_input_handler

The `json_input_handler` class is an instantiation of the `basic_json_input_handler` class template that uses `char` as the character type. It defines an interface for a listener to JSON events.

#### Header
```c++
#include <jsoncons/json_input_handler.hpp>
```
#### Member types

Member type                         |Definition
------------------------------------|------------------------------
`string_view_type`|A non-owning view of a string, holds a pointer to character data and length. Supports conversion to and from strings. Will be typedefed to the C++ 17 [string view](http://en.cppreference.com/w/cpp/string/basic_string_view) if `JSONCONS_HAS_STRING_VIEW` is defined in `jsoncons_config.hpp`, otherwise proxied.  

#### Public interface methods

    void begin_json()
Send begin JSON document event. Uses `do_begin_json`.

    void end_json()
Send end JSON document event. Uses `do_end_json`.

    void begin_object(const parsing_context& context)
Send begin JSON object event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. Uses `do_begin_object`.

    void end_object(const parsing_context& context)
Send end JSON object event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. Uses `do_end_object`.

    void begin_array(const parsing_context& context)
Send begin JSON array event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. Uses `do_begin_array`.

    void end_array(const parsing_context& context)
Send end JSON array event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. Uses `do_end_array`.

    void name(const string_view_type& name, const parsing_context& context)
Send name event for a name-value pair. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter.  Uses `do_name`.

    void string_value(const string_view_type& value, const parsing_context& context) 
Send character data event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. Uses `do_string_value`.

    void integer_value(int64_t value, const parsing_context& context)
Send signed integer event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. Uses `do_integer_value`.

    void uinteger_value(uint64_t value, const parsing_context& context)
Send non-negative integer event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. Uses `do_uinteger_value`.

    void double_value(double value, const parsing_context& context)
Send floating point value with default precision (`std::numeric_limits<double>::digits10`.) Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. Uses `do_double_value`.

    void double_value(double value, uint8_t precision, const parsing_context& context)
Send floating point value with specified precision. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. Uses `do_double_value`.

    void double_value(double value, uint8_t precision, uint8_t decimal_places, const parsing_context& context)
Send floating point value with specified precision. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. Uses `do_double_value`.

    void bool_value(bool value, const parsing_context& context) 
Send boolean value. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. Uses `do_bool_value`.

    void null_value(const parsing_context& context) 
Send null value. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. Uses `do_null_value`.

#### Private virtual implementation methods

    virtual void do_begin_json() = 0;
Receive begin JSON document event

    virtual void do_end_json() = 0;
Receive end JSON document event

    virtual void do_begin_object(const parsing_context& context) = 0;
Receive begin JSON object event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. 

    virtual void do_end_object(const parsing_context& context) = 0;
Receive end JSON object event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. 

    virtual void do_begin_array(const parsing_context& context) = 0;
Receive begin JSON array event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. 

    virtual void do_end_array(const parsing_context& context) = 0;
Receive end JSON array event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. 

    virtual void do_name(const string_view_type& name, 
                         const parsing_context& context) = 0;
Receive name event for name-value pair. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter.  

    virtual void do_string_value(const string_view_type& val, 
                                 const parsing_context& context) = 0;
Receive character data event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. 

    virtual void do_integer_value(int64_t value, const parsing_context& context) = 0;
Receive signed integer value. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. 

    virtual void do_uinteger_value(uint64_t value, 
                                   const parsing_context& context) = 0;
Receive non-negative integer value. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. 

    virtual void do_double_value(double value, uint8_t precision, uint8_t decimal_places, const parsing_context& context) = 0;
Receive floating point value. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. 

    virtual void do_bool_value(bool value, const parsing_context& context) = 0;
Receive boolean value. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. 

    virtual void do_null_value(const parsing_context& context) = 0;
Receive null value. Contextual information including
line and column information is provided in the [parsing_context](parsing_context.md) parameter. 


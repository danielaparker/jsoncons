    jsoncons::json_output_handler

    typedef basic_json_output_handler<char> json_output_handler

The `json_output_handler` class is an instantiation of the `basic_json_output_handler` class template that uses `char` as the character type. It is a pure virtual class that defines an interface for a JSON writer.

### Header

    #include "jsoncons/json_output_handler.hpp"

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

    void name(const Char* p, size_t length)
Output name event for a name-value pair. Uses `do_name`.

    void value(const Char* p)
Send a null terminated string event.

    void value(const std::string& value)
Output character data event. Uses `do_string_value`.

    void value(long long value)
Output signed integer event. Uses `do_longlong_value`.

    void value(unsigned long long value)
Output non-negative integer event. Uses `do_ulonglong_value`.

    void value(double value)
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

    virtual void do_name(const char* p, size_t length)
Receive name event for name-value pair

    virtual void do_string_value(const char* p, size_t length)
Receive character data event

    virtual void do_longlong_value(long long value)
Receive signed integer value event

    virtual void do_ulonglong_value(unsigned long long value)
Receive non-negative integer value event

    virtual void do_double_value(double value)
Receive floating point value event

    virtual void do_bool_value(bool value)
Receive a boolean value event

    virtual void do_null_value()
Receive a `null` value event


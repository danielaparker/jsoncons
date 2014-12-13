    jsoncons::json_output_handler

    typedef basic_json_output_handler<char> json_output_handler

The `json_output_handler` class is an instantiation of the `basic_json_output_handler` class template that uses `char` as the character type. It is a pure virtual class that defines an interface for a JSON writer.

### Header

    #include "jsoncons/json_output_handler.hpp"

### Overloaded methods

    void name(const std::string& name)
Send name event for a name value pair

    void value(const std::string& value)
Send character data event

    void value(null_type)
Send `null` value event

    void value(int value)
    void value(long value)
    void value(long long value)
Send signed integer value event

    void value(unsigned int value)
    void value(unsigned long value)
    void value(unsigned long long value)
Send unsigned integer value event

    void value(float value)
    void value(double value)
Send floating point value event

    void value(bool value)
Send boolean value event

### Implementing methods

    virtual void begin_json()
Receive begin JSON document event

    virtual void end_json()
Receive end JSON document event

    virtual void end_object()
Receive end JSON object event

    virtual void begin_array()
Receive begin JSON array event

    virtual void end_array()
Receive end JSON array event

    virtual void write_name(const char* p, size_t length)
Receive name event for name-value pair

    virtual void write_null()
Receive a `null` value event

    virtual void write_string(const char* p, size_t length)
Receive character data event

    virtual void write_double(double value)
Receive floating point value event

    virtual void write_longlong(long long value)
Receive signed integer value event

    virtual void write_ulonglong(unsigned long long value)
Receive non-negative integer value event

    virtual void write_bool(bool value)
Receive a boolean value event


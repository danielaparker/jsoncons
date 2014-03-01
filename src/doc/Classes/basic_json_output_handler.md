    jsoncons::basic_json_output_handler

    template <typename Char>
    class basic_json_output_handler

    typedef basic_json_output_handler<char> json_output_handler;
    typedef basic_json_output_handler<wchar_t> wjson_output_handler;

### Header

    #include "jsoncons/json_output_handler.hpp"

### Overloaded methods

    void value(const std::basic_string<Char>& value)
Send notification of a `string` value

    void value(jsoncons::null_type)
Send notification of a `null` value

    void value(float value)
Send notification of a float number

    void value(double value)
Send notification of a double number

    void value(int value)
Send notification of an int number

    void value(unsigned int value)
Send notification of an unsigned int number

    void value(long value)
Send notification of a long number

    void value(unsigned long value)
Send notification of an unsigned long number

    void value(long long value)
Send notification of a long long number

    void value(unsigned long long value)
Send notification of an unsigned long long number

    void value(bool value)
Send notification of a boolean value

### Virtual methods

    virtual void begin_json()
Receive notification of the start of a JSON document

    virtual void end_json()
Receive notification of the end of a JSON document

    virtual void begin_object()
Receive notification of the start of a JSON object

    virtual void end_object()
Receive notification of the end of a JSON object

    virtual void begin_array()
Receive notification of the start of a JSON array

    virtual void end_array()
Receive notification of the end of a JSON object

    virtual void name(const std::basic_string<Char>& name)
Receive notification of the `name` of a name value pair

    virtual void null_value()
Receive notification of `null`

    virtual void string_value(const std::basic_string<Char>& value)
Receive notification of a `string` value

    virtual void double_value(double value)
Receive notification of a floating point number

    virtual void longlong_value(long long value)
Receive notification of a signed integer

    virtual void ulonglong_value(unsigned long long value)
Receive notification of a non-negative integer

    virtual void bool_value(bool value)
Receive notification of a boolean value


    jsoncons::basic_json_input_handler

    template <typename Char>
    class basic_json_input_handler

    typedef basic_json_input_handler<char> json_input_handler
    typedef basic_json_input_handler<wchar_t> wjson_input_handler

### Header

    #include "jsoncons/json_input_handler.hpp"

### Overloaded methods

    void value(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context)
Send notification of a string value. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    void value(null_type, const basic_parsing_context<Char>& context)
Send notification of a null value. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    void value(float value, const basic_parsing_context<Char>& context)
Send notification of a float number. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    void value(double value, const basic_parsing_context<Char>& context)
Send notification of a double number. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    void value(int value, const basic_parsing_context<Char>& context)
Send notification of an int number. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    void value(unsigned int value, const basic_parsing_context<Char>& context)
Send notification of an unsigned int number. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    void value(long value, const basic_parsing_context<Char>& context)
Send notification of a long number. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    void value(unsigned long value, const basic_parsing_context<Char>& context)
Send notification of an unsigned long number. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    void value(long long value, const basic_parsing_context<Char>& context)
Send notification of a long long number. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    void value(unsigned long long value, const basic_parsing_context<Char>& context)
Send notification of an unsigned long long number. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    void value(bool value, const basic_parsing_context<Char>& context)
Send notification of a boolean value. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

### Virtual methods

    virtual void begin_json()
Receive notification of the start of JSON text

    virtual void end_json()
Receive notification of the end of JSON text

    virtual void begin_object(const basic_parsing_context<Char>& context)
Receive notification of the start of a JSON object. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    virtual void end_object(const basic_parsing_context<Char>& context)
Receive notification of the end of a JSON object. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    virtual void begin_array(const basic_parsing_context<Char>& context)
Receive notification of the start of a JSON array. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    virtual void end_array(const basic_parsing_context<Char>& context)
Receive notification of the end of a JSON array. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    virtual void name(const std::basic_string<Char>& name, const basic_parsing_context<Char>& context)
Receive notification of the `name` part of a name-value pair. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter.  

    virtual void null_value(const basic_parsing_context<Char>& context)
Receive notification of a null value. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    virtual void null_value(const basic_parsing_context<Char>& context)
Receive notification of a null value. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    virtual void string_value(const std::basic_string<Char>& value, const basic_parsing_context<Char>& context)
Receive notification of a string value. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    virtual void double_value(double value, const basic_parsing_context<Char>& context)
Receive notification of a floating point number. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    virtual void longlong_value(long long value, const basic_parsing_context<Char>& context)
Receive notification of a signed integer. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    virtual void ulonglong_value(unsigned long long value, const basic_parsing_context<Char>& context)
Receive notification of a non-negative integer. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    virtual void bool_value(bool value, const basic_parsing_context<Char>& context)
Receive notification of a boolean value. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 


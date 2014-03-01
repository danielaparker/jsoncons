    jsoncons::basic_json_filter

    template <typename Char>
    class basic_json_filter

    typedef basic_json_filter<char> json_filter
    typedef basic_json_filter<wchar_t> wjson_filter

### Header

    #include "jsoncons/json_filter.hpp"

### Base classes

[basic_json_input_handler<Char>](basic_json_input_handler)

### Constructors

    basic_json_filter(basic_json_input_handler<Char>& parent)

    basic_json_filter(basic_json_output_handler<Char>& output_handler)

### Implementing methods

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

    virtual void name(const std::string& name, const basic_parsing_context<Char>& context)
Receive notification of the `name` part of a name-value pair. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter.  

    virtual void null_value(const basic_parsing_context<Char>& context)
Receive notification of a null value. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    virtual void null_value(const basic_parsing_context<Char>& context)
Receive notification of a null value. Contextual information including
line and column information is provided in the [basic_parsing_context<Char>](basic_parsing_context) parameter. 

    virtual void string_value(const std::string& value, const basic_parsing_context<Char>& context)
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

### Accessors

    basic_json_input_handler<Char>& parent()
Returns a reference to the parent input handler. 

    jsoncons::json_filter

    typedef basic_json_filter<char> json_filter

The `json_filter` class is an instantiation of the `basic_json_filter` class template that uses `char` as the character type.

### Header

    #include "jsoncons/json_filter.hpp"

### Base classes

[json_input_handler](json_input_handler)

### Constructors

    json_filter(json_input_handler& parent)

    json_filter(json_output_handler& output_handler)

### Implementing methods

    virtual void begin_json()
Receive begin JSON document event

    virtual void end_json()
Receive end JSON document event

    virtual void begin_object(const parsing_context& context)
Receive begin JSON object event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void end_object(const parsing_context& context)
Receive end JSON object event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void begin_array(const parsing_context& context)
Receive begin JSON array event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void end_array(const parsing_context& context)
Receive end JSON array event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void write_name(const char* p, size_t length, 
                            const parsing_context& context)
Receive name event for name-value pair. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter.  

    virtual void write_string(const char* p, size_t length, 
                              const parsing_context& context)
Receive character data event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void write_longlong(long long value, const parsing_context& context)
Receive signed integer value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void write_ulonglong(unsigned long long value, 
                                 const parsing_context& context)
Receive non-negative integer value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void write_double(double value, const parsing_context& context)
Receive floating point value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void write_bool(bool value, const parsing_context& context)
Receive boolean value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void write_null(const parsing_context& context)
Receive null value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

### Accessors

    json_input_handler& parent()
Returns a reference to the parent input handler. 

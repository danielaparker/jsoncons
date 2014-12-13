    jsoncons::json_input_handler

    typedef basic_json_input_handler<char> json_input_handler

The `json_input_handler` class is an instantiation of the `basic_json_input_handler` class template that uses `char` as the character type. It defines an interface for a listener to JSON events.

### Header

    #include "jsoncons/json_input_handler.hpp"

### Overloaded methods

    void name(const std::string& value, const parsing_context& context)
Send name event for a name-value pair. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter.  

    void value(const std::string& value, const parsing_context& context)
Send character data event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void value(float value, const parsing_context& context)
    void value(double value, const parsing_context& context)
Send floating point value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void value(int value, const parsing_context& context)
    void value(long value, const parsing_context& context)
    void value(long long value, const parsing_context& context)
Send signed integer event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void value(unsigned int value, const parsing_context& context)
    void value(unsigned long value, const parsing_context& context)
    void value(unsigned long long value, const parsing_context& context)
Send non-negative integer event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void value(bool value, const parsing_context& context)
Send boolean value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void value(null_type, const parsing_context& context)
Send null value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

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


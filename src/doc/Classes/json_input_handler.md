    jsoncons::json_input_handler

    typedef basic_json_input_handler<char> json_input_handler

The `json_input_handler` class is an instantiation of the `basic_json_input_handler` class template that uses `char` as the character type. It defines an interface for a listener to JSON events.

### Header

    #include "jsoncons/json_input_handler.hpp"

### Overloaded methods

    void begin_json()
Receive begin JSON document event

    void end_json()
Receive end JSON document event

    void begin_object(const parsing_context& context)
Receive begin JSON object event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void end_object(const parsing_context& context)
Receive end JSON object event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void begin_array(const parsing_context& context)
Receive begin JSON array event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void end_array(const parsing_context& context)
Receive end JSON array event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void name(const std::string& value, const parsing_context& context)
Send name event for a name-value pair. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter.  

    void name(const Char* p, size_t length, const basic_parsing_context<Char>& context)
Send name event for a name-value pair. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter.  

    void string_value(const std::string& value, const parsing_context& context)
Send character data event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void string_value(const Char* p, size_t length, const basic_parsing_context<Char>& context)
Send character data event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void longlong_value(long long value, const parsing_context& context)
Send signed integer event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void ulonglong_value(unsigned long long value, const parsing_context& context)
Send non-negative integer event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void double_value(double value, const parsing_context& context)
Send floating point value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void bool_value(bool value, const parsing_context& context)
Send boolean value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    void null_value(null_type, const parsing_context& context)
Send null value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

### Implementing methods

    virtual void do_begin_json()
Receive begin JSON document event

    virtual void do_end_json()
Receive end JSON document event

    virtual void do_begin_object(const parsing_context& context)
Receive begin JSON object event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void do_end_object(const parsing_context& context)
Receive end JSON object event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void do_begin_array(const parsing_context& context)
Receive begin JSON array event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void do_end_array(const parsing_context& context)
Receive end JSON array event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void do_name(const char* p, size_t length, 
                         const parsing_context& context)
Receive name event for name-value pair. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter.  

    virtual void do_string_value(const char* p, size_t length, 
                                 const parsing_context& context)
Receive character data event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void do_longlong_value(long long value, const parsing_context& context)
Receive signed integer value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void do_ulonglong_value(unsigned long long value, 
                                    const parsing_context& context)
Receive non-negative integer value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void do_double_value(double value, const parsing_context& context)
Receive floating point value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void do_bool_value(bool value, const parsing_context& context)
Receive boolean value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 

    virtual void do_null_value(const parsing_context& context)
Receive null value event. Contextual information including
line and column information is provided in the [parsing_context](parsing_context) parameter. 


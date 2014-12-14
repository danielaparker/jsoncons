    jsoncons::json_deserializer

    typedef basic_json_deserializer<char,std::allocator<void>> json_deserializer

The `json_deserializer` class is an instantiation of the `basic_json_deserializer` class template that uses `char` as the character type.

### Header

    #include "jsoncons/json_deserializer.hpp"

### Base classes

[json_input_handler](json_input_handler)

### Constructors

    json_deserializer()

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

### Accessors

    json& root()
Returns a reference to the root of the json value read from the stream

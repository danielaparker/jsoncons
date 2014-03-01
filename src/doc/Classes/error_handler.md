    jsoncons::error_handler

    typedef basic_error_handler<char> error_handler

When parsing JSON text with [json_reader](json_reader), if you need to implement
cutomized error handling, you must implement this pure virtual class
and pass it in the constructor of [json_reader](json_reader). The `read` method 
will then report all warnings and errors through this interface.

### Header

    #include "jsoncons/error_handler.hpp"

### Destructor

    virtual ~error_handler()

### Member functions

    virtual void warning(const std::string& error_code,
                         const std::string& message,
                         const parsing_context& context) throw (json_parse_exception) = 0
Receive notification of a warning. An [error_code](JSON parser error codes) indicates the type of error. Contextual information including
line and column information is provided in the [context](parsing_context) parameter.

    virtual void error(const std::string& error_code,
                       const std::string& message,
                       const parsing_context& context) throw (json_parse_exception) = 0
Receive notification of a content error, possibly recoverable. An [error_code](JSON parser error codes) indicates the type of error. Contextual information including
line and column information is provided in the [context](parsing_context) parameter. 
    
    virtual void fatal_error(const std::string& error_code,
                             const std::string& message,
                             const parsing_context& context) throw (json_parse_exception) = 0
Receive notification of a well-formedness error, not recoverable. An [error_code](JSON parser error codes) indicates the type of error. Contextual information including
line and column information is provided in the [context](parsing_context) parameter. 

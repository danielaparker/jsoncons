    jsoncons::default_error_handler

### Header

    #include "jsoncons/error_handler.hpp"

### Base class

[[error_handler]]    

### Member functions

    virtual void warning(const std::string& error_code,
                         const std::string& message,
                         const parsing_context& context) throw (json_parse_exception)
Does nothing

    virtual void error(const std::string& error_code,
                       const std::string& message,
                       const parsing_context& context) throw (json_parse_exception)
Throws a [[json_parse_exception]] with the message and the line 
number and column number taken from the [context](parsing_context) parameter.
    
    virtual void fatal_error(const std::string& error_code,
                             const std::string& message,
                             const parsing_context& context) throw (json_parse_exception)
Throws a [[json_parse_exception]] with the message and the line 
number and column number taken from the [context](parsing_context) parameter.


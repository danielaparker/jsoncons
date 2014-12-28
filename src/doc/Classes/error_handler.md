    jsoncons::parse_error_handler

    typedef basic_parse_error_handler<char> parse_error_handler

When parsing JSON text with [json_reader](json_reader), if you need to implement
cutomized error handling, you must implement this pure virtual class
and pass it in the constructor of [json_reader](json_reader). The `read` method 
will then report all warnings and errors through this interface.

### Header

    #include "jsoncons/error_handler.hpp"

### Destructor

    virtual ~parse_error_handler()

### Public interface methods

    void warning(std::error_code ec,
                 const parse_context& context) throw (json_parse_exception) = 0
Send a warning event. An [error_code](JSON%20parser%20error%20codes) indicates the type of error. Contextual information including
line and column information is provided in the [context](parse_context) parameter. Uses `do_warning`.

    void error(std::error_code ec,
               const parse_context& context) throw (json_parse_exception) = 0
Send a error event, possibly recoverable. An [error_code](JSON%20parser%20error%20codes) indicates the type of error. Contextual information including
line and column information is provided in the [context](parse_context) parameter.  Uses `do_error`.

### Private virtual implementation methods

    virtual void do_warning(std::error_code ec,
                            const parse_context& context) throw (json_parse_exception) = 0
Receive a warning event. An [error_code](JSON%20parser%20error%20codes) indicates the type of error. Contextual information including
line and column information is provided in the [context](parse_context) parameter.

    virtual void do_error(std::error_code ec,
                          const parse_context& context) throw (json_parse_exception) = 0
Receive a error event, possibly recoverable. An [error_code](JSON%20parser%20error%20codes) indicates the type of error. Contextual information including
line and column information is provided in the [context](parse_context) parameter. 
    


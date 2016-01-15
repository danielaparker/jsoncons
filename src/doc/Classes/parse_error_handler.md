    jsoncons::parse_error_handler

    typedef basic_parse_error_handler<char> parse_error_handler

When parsing JSON text with [json_reader](json_reader), if you need to implement
customized error handling, you must implement this abstract class
and pass it in the constructor of [json_reader](json_reader). The `read` method 
will then report all warnings and errors through this interface.

### Header

    #include "jsoncons/json_error_handler.hpp"

### Destructor

    virtual ~json_error_handler()

### Public interface methods

    void warning(std::error_code ec,
                 const parsing_context& context) throw (parse_exception) = 0
Send a warning event. An [error_code](json_error_category) indicates the type of error. Contextual information including
line and column information is provided in the [context](parsing_context) parameter. Uses `do_warning`.

    void error(std::error_code ec,
               const parsing_context& context) throw (parse_exception) = 0
Send an error event, possibly recoverable. An [error_code](json_error_category) indicates the type of error. Contextual information including
line and column information is provided in the [context](parsing_context) parameter.  Calls `do_error`.

    void fatal_error(std::error_code ec,
               const parsing_context& context) throw (parse_exception) = 0
Send an unrecoverable error. An [error_code](json_error_category) indicates the type of error. Contextual information including
line and column information is provided in the [context](parsing_context) parameter.  Calls `do_fatal_error` and throws a [parse_exception](parse_exception).

### Private virtual implementation methods

    virtual void do_warning(std::error_code ec,
                            const parsing_context& context) throw (parse_exception) = 0
Receive a warning event. An [error_code](json_error_category) indicates the type of error. Contextual information including
line and column information is provided in the [context](parsing_context) parameter.

    virtual void do_error(std::error_code ec,
                          const parsing_context& context) throw (parse_exception) = 0
Receive an error event, possibly recoverable. An [error_code](json_error_category) indicates the type of error. Contextual information including
line and column information is provided in the [context](parsing_context) parameter. 

    virtual void do_fatal_error(std::error_code ec,
                                const parsing_context& context) throw (parse_exception) = 0
Receives a non recoverable error. An [error_code](json_error_category) indicates the type of error. Contextual information including
line and column information is provided in the [context](parsing_context) parameter. 
    


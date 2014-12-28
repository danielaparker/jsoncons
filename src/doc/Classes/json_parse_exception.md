    jsoncons::json_parse_exception

### Header

    #include "jsoncons/json_exception.hpp"

### Base class

    [json_exception](json_exception)

### Constructors

    json_parse_exception(std::error_code ec,
                         unsigned long line,
                         unsigned long column)

    json_parse_exception(const json_parse_exception& other)

### Member functions

    const std::error_code error_code() const
Returns a message for this exception

    unsigned long line_number() const
Returns the line number to the end of the text where the exception occurred.
Line numbers start at 1.

    unsigned long column_number() const
Returns the column number to the end of the text where the exception occurred.
Column numbers start at 1.

    const char* what() const
Constructs an error message, including line and column position


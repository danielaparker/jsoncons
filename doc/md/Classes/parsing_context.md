    jsoncons::parsing_context

    typedef basic_parsing_context<char> parsing_context

Provides contextual information for the element being parsed. This information may be used for error reporting.

### Header

    #include "jsoncons/parse_error_handler.hpp"

### Public interface methods

    size_t line_number() const
Returns the line number for the text being parsed.
Line numbers start at 1.

    size_t column_number() const 
Returns the column number to the end of the text being parsed.
Column numbers start at 1.

    char current_char() const
Returns the current character in the input stream
    
### Private virtual implementation methods
    
    virtual size_t do_line_number() const = 0

    virtual size_t do_column_number() const = 0

    virtual char do_current_char() const = 0
    
### Deprecated methods

    char last_char() const 
Returns the last character consumed by the parser



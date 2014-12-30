    jsoncons::parsing_context

    typedef basic_parsing_context<char> parsing_context

Provides contextual information for the element being parsed. This information may be used for error reporting.

### Header

    #include "jsoncons/parse_error_handler.hpp"

### Public interface methods

    unsigned long line_number() const
Returns the line number to the end of the text being parsed.
Line numbers start at 1.

    unsigned long column_number() const 
Returns the column number to the end of the text being parsed.
Column numbers start at 1.

    size_t minimum_structure_capacity() const 
Provides a hint to the [json_input_handler](json_input_handler) for how many elements to reserve for a json array or object

    char last_char() const 
Returns the last character consumed by the parser
 
    bool eof() const
Returns `true` if the parser has reached end-of-file 
    
### Private virtual implementation methods
    
    virtual unsigned long do_line_number() const = 0

    virtual unsigned long do_column_number() const = 0

    virtual bool do_eof() const = 0

    virtual size_t do_minimum_structure_capacity() const = 0

    virtual char do_last_char() const = 0
    



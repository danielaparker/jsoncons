    jsoncons::parse_context

    typedef basic_parse_context<char> parse_context

Provides contextual information for the element being parsed. This information may be used for error reporting.

### Header

    #include "jsoncons/error_handler.hpp"

### Member functions

    virtual unsigned long line_number() const = 0
Returns the line number to the end of the text being parsed.
Line numbers start at 1.

    virtual unsigned long column_number() const = 0
Returns the column number to the end of the text being parsed.
Column numbers start at 1.

    virtual size_t minimum_structure_capacity() const = 0
Provides a hint to the [json_output_handler](json_output_handler) for how many elements to reserve for a json array or object

    virtual const std::string& buffer() const = 0;
Returns a reference to a string buffer of text 



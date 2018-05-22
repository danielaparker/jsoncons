### jsoncons::serializing_context

```c++
class serializing_context;
```

Provides contextual information for serializing and deserializing JSON and JSON-like data formats. This information may be used for error reporting.

#### Header

    #include <jsoncons/parse_error_handler.hpp>

#### Public interface methods

    size_t line_number() const
Returns the line number for the text being parsed.
Line numbers start at 1.

    size_t column_number() const 
Returns the column number to the end of the text being parsed.
Column numbers start at 1.
    
#### Private virtual implementation methods
    
    virtual size_t do_line_number() const = 0

    virtual size_t do_column_number() const = 0
    



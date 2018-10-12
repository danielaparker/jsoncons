### jsoncons::serializing_context

```c++
class serializing_context;
```

Provides contextual information for serializing and deserializing JSON and JSON-like data formats. This information may be used for error reporting.

#### Header

    #include <jsoncons/parse_error_handler.hpp>

    virtual size_t line_number() const = 0;
Returns the line number for the text being parsed.
Line numbers start at 1.

    virtual size_t column_number() const = 0; 
Returns the column number to the end of the text being parsed.
Column numbers start at 1.
    



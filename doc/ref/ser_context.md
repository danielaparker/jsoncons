### jsoncons::ser_context

```c++
#include <jsoncons/ser_context.hpp>

class ser_context;
```

Provides contextual information for serializing and deserializing JSON and JSON-like data formats. 
This information may be used for error reporting.

    virtual size_t line() const;
Returns the line number for the text being parsed.
Line numbers (if available) start at 1. The default implementation returns 0.

    virtual size_t column() const; 
Returns the column number to the end of the text being parsed.
Column numbers (if available) start at 1. The default implementation returns 0.

    virtual size_t position() const; 
`position()` is defined for all JSON parse events, and indicates the position 
of the character at the beginning of the event, e.g. '[' for an array, '{' for 
an object, and '"' for a string. Not currently supported for non-JSON (e.g. CSV) parsers.   



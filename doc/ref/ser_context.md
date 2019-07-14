### jsoncons::ser_context

```c++
#include <jsoncons/ser_context.hpp>

class ser_context;
```

Provides contextual information for serializing and deserializing JSON and JSON-like data formats. This information may be used for error reporting.

    virtual size_t line() const = 0;
Returns the line number for the text being parsed.
Line numbers start at 1.

    virtual size_t column() const = 0; 
Returns the column number to the end of the text being parsed.
Column numbers start at 1.
    



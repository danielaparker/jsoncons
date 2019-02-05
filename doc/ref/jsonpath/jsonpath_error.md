### jsoncons::jsonpath_error

#### Header

    #include <jsoncons_ext/jsonpath/jsonpath_error.hpp>

![jsonpath_error](./diagrams/jsonpath_error.png)

#### Constructors

    jsonpath_error(std::error_code ec);

    jsonpath_error(std::error_code ec, size_t line, size_t column);

    jsonpath_error(const jsonpath_error& other);

#### Member functions

    const std::error_code code() const
Returns a message for this exception

    size_t line_number() const
Returns the line number to the end of the text where the exception occurred.
Line numbers start at 1.

    size_t column_number() const
Returns the column number to the end of the text where the exception occurred.
Column numbers start at 1.

    const char* what() const
Constructs an error message, including line and column position

### Example



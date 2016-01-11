    jsoncons::json_parse_exception

### Header

    #include "jsoncons/parse_error_handler.hpp"

### Base class

[json_exception](json_exception)

### Constructors

    json_parse_exception(std::error_code ec,
                         size_t line,
                         size_t column)

    json_parse_exception(const json_parse_exception& other)

### Member functions

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

    #include "jsoncons/json.hpp"

    using jsoncons::json;

    int main()
    {
    	string s = "[1,2,3,4,]";
        try 
    	{
            jsoncons::json val = jsoncons::json::parse(s);
        } 
    	catch(const jsoncons::json_parse_exception& e) 
    	{
            std::cout << "Caught json_parse_exception with category " 
                      << e.code().category().name() 
    			      << ", code " << e.code().value() 
                      << " and message " << e.what() << std::endl;
        }
    }


Output:

    Caught json_parse_exception with category json_input, code 1 and message Unexpected value separator ',' at line 1 and column 10

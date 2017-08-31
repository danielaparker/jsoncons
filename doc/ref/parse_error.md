### jsoncons::parse_error

#### Header

    #include <jsoncons/parse_error_handler.hpp>

### Base class

std::exception

#### Constructors

    parse_error(std::error_code ec,
                    size_t line,
                    size_t column)

    parse_error(const parse_error& other)

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

    #include <jsoncons/json.hpp>

    using jsoncons::json;

    int main()
    {
        string s = "[1,2,3,4,]";
        try 
        {
            jsoncons::json val = jsoncons::json::parse(s);
        } 
        catch(const jsoncons::parse_error& e) 
        {
            std::cout << "Caught parse_error with category " 
                      << e.code().category().name() 
                              << ", code " << e.code().value() 
                      << " and message " << e.what() << std::endl;
        }
    }


Output:

    Caught parse_error with category json_input, code 1 and message Unexpected value separator ',' at line 1 and column 10

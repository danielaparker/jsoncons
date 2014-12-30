    jsoncons::default_parse_error_handler

    typedef default_basic_parse_error_handler<char> default_parse_error_handler

### Header

    #include "jsoncons/parse_error_handler.hpp"

### Base class

[parse_error_handler](parse_error_handler)  
  
### Static memebr functions
  
    static parse_error_handler& instance()   
Returns a singleton instance of a `default_parse_error_handler`       

### Private virtual implementation methods

    virtual void do_warning(const std::string& error_code,
                            std::error_code ec,
                            const parsing_context& context) throw (json_parse_exception)
Does nothing

    virtual void do_error(const std::string& error_code,
                          std::error_code ec,
                          const parsing_context& context) throw (json_parse_exception)
Throws a [json_parse_exception](json_parse_exception) with the message and the line 
number and column number taken from the [context](parsing_context) parameter.
    


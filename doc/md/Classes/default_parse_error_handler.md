    jsoncons::default_parse_error_handler

    typedef basic_default_parse_error_handler<char> default_parse_error_handler

### Header

    #include <jsoncons/parse_error_handler.hpp>

### Base class

[parse_error_handler](parse_error_handler)  
  
### Static memebr functions
  
    static parse_error_handler& instance()   
Returns a singleton instance of a `default_parse_error_handler`       

### Private virtual implementation methods

     bool do_error(std::error_code ec,
                   const parsing_context& context) override
Returns `true`
    


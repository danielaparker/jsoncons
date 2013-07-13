#ifndef JSONCONS_ERROR_HANDLER_HPP
#define JSONCONS_ERROR_HANDLER_HPP

#include "jsoncons/json_exception.hpp"

namespace jsoncons {

class error_handler
{
public:
    virtual ~error_handler()
    {
    }

    virtual void warning(const json_parse_exception& e) throw (json_parse_exception) = 0;

    virtual void content_error(const json_parse_exception& e) throw (json_parse_exception) = 0;
    
    virtual void fatal_error(const json_parse_exception& e) throw (json_parse_exception) = 0;
};

class default_error_handler : public error_handler
{
public:
    virtual void warning(const json_parse_exception& e) throw (json_parse_exception) 
    {
        // Do nothing
    }
    virtual void content_error(const json_parse_exception& e) throw (json_parse_exception)
    {
        throw e;
    }
    virtual void fatal_error(const json_parse_exception& e) throw (json_parse_exception) 
    {
        throw e;
    }
};
}
#endif

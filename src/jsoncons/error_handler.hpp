#ifndef JSONCONS_ERROR_HANDLER_HPP
#define JSONCONS_ERROR_HANDLER_HPP

#include "jsoncons/json_exception.hpp"

namespace jsoncons {

template<class Char>
class basic_parsing_context
{
public:
    virtual unsigned long line_number() const = 0;
    virtual unsigned long column_number() const = 0;
};

typedef basic_parsing_context<char> parsing_context;

template <class Char>
class basic_error_handler
{
public:
    virtual ~basic_error_handler()
    {
    }

    virtual void warning(const std::string& message, 
                         const basic_parsing_context<Char>& context) throw (json_parse_exception) = 0;

    virtual void content_error(const std::string& message, 
                               const basic_parsing_context<Char>& context) throw (json_parse_exception) = 0;
    
    virtual void fatal_error(const std::string& message, 
                             const basic_parsing_context<Char>& context) throw (json_parse_exception) = 0;
};

template <class Char>
class basic_default_error_handler : public basic_error_handler<Char>
{
public:
    virtual void warning(const std::string& message, 
                         const basic_parsing_context<Char>& context) throw (json_parse_exception)
    {
        // Do nothing
    }
    virtual void content_error(const std::string& message, 
                               const basic_parsing_context<Char>& context) throw (json_parse_exception)
    {
        throw json_parse_exception(message,context.line_number(),context.column_number());
    }
    virtual void fatal_error(const std::string& message, 
                             const basic_parsing_context<Char>& context) throw (json_parse_exception) 
    {
        throw json_parse_exception(message,context.line_number(),context.column_number());
    }
};

typedef basic_error_handler<char> error_handler;
typedef basic_default_error_handler<char> default_error_handler;

}
#endif

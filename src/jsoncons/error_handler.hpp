// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_ERROR_HANDLER_HPP
#define JSONCONS_ERROR_HANDLER_HPP

#include "jsoncons/jsoncons.hpp"

// Error codes
//
// JPE001 Informational
// JPE101 Unexpected end of file
// JPE102 Unexpected value separator
// JPE103 Unexpected end of object
// JPE104 Unexpected end of array
// JPE105 Unrecognized value
// JPE106 Expected name-value separator
// JPE201 Illegal character
// JPE202 Invalid codepoint
// JPE203 Invalid number

namespace jsoncons {

class json_parse_exception : public json_exception
{
public:
    json_parse_exception(std::string message,
                         unsigned long line,
                         unsigned long column)
        : message_(message),
          line_number_(line),
          column_number_(column)
    {
    }
    json_parse_exception(const json_parse_exception& other)
        : message_(other.message_),
          line_number_(other.line_number_),
          column_number_(other.column_number_)
    {
    }
    const char* what() const JSONCONS_NOEXCEPT
    {
        std::ostringstream os;
        os << message_ << " on line " << line_number_ << " at column " << column_number_;
        const_cast<std::string&>(message_) = os.str();
        return message_.c_str();
    }

    const std::string& message() const
    {
        return message_;
    }

    unsigned long line_number() const
    {
        return line_number_;
    }

    unsigned long column_number() const
    {
        return column_number_;
    }
private:
    std::string message_;
    std::string buffer_;
    unsigned long line_number_;
    unsigned long column_number_;
};

template<typename Char>
class basic_parsing_context
{
public:
    virtual ~basic_parsing_context() {}
    virtual unsigned long line_number() const = 0;
    virtual unsigned long column_number() const = 0;
    virtual size_t minimum_structure_capacity() const = 0;
    virtual const std::basic_string<Char>& buffer() const = 0;
};

typedef basic_parsing_context<char> parsing_context;
typedef basic_parsing_context<wchar_t> wparsing_context;

template <typename Char>
class basic_error_handler
{
public:
    virtual ~basic_error_handler()
    {
    }

    virtual void warning(const std::string& error_code,
                         const std::string& message,
                         const basic_parsing_context<Char>& context) throw (json_parse_exception) = 0;

    virtual void error(const std::string& error_code,
                               const std::string& message,
                               const basic_parsing_context<Char>& context) throw (json_parse_exception) = 0;

    virtual void fatal_error(const std::string& error_code,
                             const std::string& message,
                             const basic_parsing_context<Char>& context) throw (json_parse_exception) = 0;
};

template <typename Char>
class default_basic_error_handler : public basic_error_handler<Char>
{
public:
    virtual void warning(const std::string& error_code,
                         const std::string& message,
                         const basic_parsing_context<Char>& context) throw (json_parse_exception)
    {
        // Do nothing
    }
    virtual void error(const std::string& error_code,
                       const std::string& message,
                       const basic_parsing_context<Char>& context) throw (json_parse_exception)
    {
        throw json_parse_exception(message,context.line_number(),context.column_number());
    }
    virtual void fatal_error(const std::string& error_code,
                             const std::string& message,
                             const basic_parsing_context<Char>& context) throw (json_parse_exception)
    {
        throw json_parse_exception(message,context.line_number(),context.column_number());
    }
};

typedef basic_error_handler<char> error_handler;
typedef basic_error_handler<wchar_t> werror_handler;

typedef default_basic_error_handler<char> default_error_handler;
typedef default_basic_error_handler<wchar_t> wdefault_error_handler;

typedef basic_parsing_context<char> parsing_context;
typedef basic_parsing_context<wchar_t> wparsing_context;

}
#endif

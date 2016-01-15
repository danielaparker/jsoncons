/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_PARSE_ERROR_HANDLER_HPP
#define JSONCONS_PARSE_ERROR_HANDLER_HPP

#include "jsoncons/jsoncons.hpp"
#include <system_error>

namespace jsoncons {

class parse_exception : public std::exception, public virtual json_exception
{
public:
    parse_exception(std::error_code ec,
                         size_t line,
                         size_t column)
        : error_code_(ec),
          line_number_(line),
          column_number_(column)
    {
    }
    parse_exception(const parse_exception& other)
        : error_code_(other.error_code_),
          line_number_(other.line_number_),
          column_number_(other.column_number_)
    {
    }
    const char* what() const JSONCONS_NOEXCEPT
    {
        std::ostringstream os;
        os << error_code_.message() << " at line " << line_number_ << " and column " << column_number_;
        const_cast<std::string&>(buffer_) = os.str();
        return buffer_.c_str();
    }

    const std::error_code code() const
    {
        return error_code_;
    }

    size_t line_number() const
    {
        return line_number_;
    }

    size_t column_number() const
    {
        return column_number_;
    }
private:
    std::error_code error_code_;
    std::string buffer_;
    size_t line_number_;
    size_t column_number_;
};

typedef parse_exception json_parse_exception;

template<typename Char>
class basic_parsing_context
{
public:
    virtual ~basic_parsing_context() {}

    size_t line_number() const
    {
        return do_line_number();
    }
    size_t column_number() const 
    {
        return do_column_number();
    }
    Char current_char() const
    {
        return do_current_char();
    }

    // Deprecated
    Char last_char() const
    {
        return do_current_char();
    }

private:
    virtual size_t do_line_number() const = 0;
    virtual size_t do_column_number() const = 0;
    virtual Char do_current_char() const = 0;
};

typedef basic_parsing_context<char> parsing_context;
typedef basic_parsing_context<wchar_t> wparsing_context;

template <typename Char>
class basic_parse_error_handler
{
public:
    virtual ~basic_parse_error_handler()
    {
    }

    void warning(std::error_code ec,
                 const basic_parsing_context<Char>& context) throw (parse_exception) 
    {
        do_warning(ec,context);
    }

    void error(std::error_code ec,
               const basic_parsing_context<Char>& context) throw (parse_exception) 
    {
        do_error(ec,context);
    }

    void fatal_error(std::error_code ec,
                     const basic_parsing_context<Char>& context) throw (parse_exception) 
    {
        do_fatal_error(ec,context);
        throw parse_exception(ec,context.line_number(),context.column_number());
    }

private:
    virtual void do_warning(std::error_code,
                            const basic_parsing_context<Char>& context) throw (parse_exception) = 0;

    virtual void do_error(std::error_code,
                          const basic_parsing_context<Char>& context) throw (parse_exception) = 0;

    virtual void do_fatal_error(std::error_code,
                                const basic_parsing_context<Char>& context) throw (parse_exception)
    {
    }
};

template <typename Char>
class basic_default_parse_error_handler : public basic_parse_error_handler<Char>
{
public:
    static basic_parse_error_handler<Char>& instance()
    {
        static basic_default_parse_error_handler<Char> instance;
        return instance;
    }
private:
    virtual void do_warning(std::error_code,
                            const basic_parsing_context<Char>& context) throw (parse_exception) 
    {
    }

    virtual void do_error(std::error_code ec,
                          const basic_parsing_context<Char>& context) throw (parse_exception)
    {
        throw parse_exception(ec,context.line_number(),context.column_number());
    }
};

typedef basic_parse_error_handler<char> parse_error_handler;
typedef basic_parse_error_handler<wchar_t> wparse_error_handler;

typedef basic_default_parse_error_handler<char> default_parse_error_handler;
typedef basic_default_parse_error_handler<wchar_t> wdefault_parse_error_handler;

typedef basic_parsing_context<char> parsing_context;
typedef basic_parsing_context<wchar_t> wparsing_context;

}
#endif

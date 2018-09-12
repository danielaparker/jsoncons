/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_PARSE_ERROR_HANDLER_HPP
#define JSONCONS_PARSE_ERROR_HANDLER_HPP

#include <system_error>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_error_category.hpp>
#include <jsoncons/serializing_context.hpp>

namespace jsoncons {

class parse_error : public std::exception, public virtual json_exception
{
public:
    parse_error()
        : line_number_(0),
          column_number_(0)
    {
    }
    parse_error(std::error_code ec,
                size_t line,
                size_t column)
        : error_code_(ec),
          line_number_(line),
          column_number_(column)
    {
    }
    parse_error(const parse_error& other)
        : error_code_(other.error_code_),
          line_number_(other.line_number_),
          column_number_(other.column_number_)
    {
    }

    const char* what() const JSONCONS_NOEXCEPT override
    {
        try
        {
            std::ostringstream os;
            os << error_code_.message() << " at line " << line_number_ << " and column " << column_number_;
            const_cast<std::string&>(buffer_) = os.str();
            return buffer_.c_str();
        }
        catch (...)
        {
            return "";
        }
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

    parse_error& operator=(const parse_error& e)
    {
        error_code_ = e.error_code_;
        line_number_ = e.line_number_;
        column_number_ = e.column_number_;
        return *this;
    }
private:
    std::error_code error_code_;
    std::string buffer_;
    size_t line_number_;
    size_t column_number_;
};

#if !defined(JSONCONS_NO_DEPRECATED)
typedef parse_error json_parse_exception;
typedef parse_error parse_exception;
#endif

class parse_error_handler
{
public:
    virtual ~parse_error_handler()
    {
    }

    bool error(std::error_code ec,
               const serializing_context& context) JSONCONS_NOEXCEPT 
    {
        return do_error(ec,context);
    }

    void fatal_error(std::error_code ec,
                     const serializing_context& context) JSONCONS_NOEXCEPT 
    {
        do_fatal_error(ec,context);
    }

private:
    virtual bool do_error(std::error_code,
                          const serializing_context& context) JSONCONS_NOEXCEPT = 0;

    virtual void do_fatal_error(std::error_code,
                                const serializing_context&) JSONCONS_NOEXCEPT
    {
    }
};

class default_parse_error_handler : public parse_error_handler
{
private:
    bool do_error(std::error_code code,
                  const serializing_context&) JSONCONS_NOEXCEPT override
    {
        static const std::error_code illegal_comment = make_error_code(json_parse_errc::illegal_comment);

        if (code == illegal_comment)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
};

class strict_parse_error_handler : public parse_error_handler
{
private:
    bool do_error(std::error_code, const serializing_context&) JSONCONS_NOEXCEPT override
    {
        return true;
    }
};

}
#endif

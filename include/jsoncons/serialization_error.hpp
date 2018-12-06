/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_SERIALIZATION_ERROR_HPP
#define JSONCONS_SERIALIZATION_ERROR_HPP

#include <system_error>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_error.hpp>
#include <jsoncons/serializing_context.hpp>

namespace jsoncons {

class serialization_error : public std::exception, public virtual json_exception
{
public:
    serialization_error()
        : line_number_(0),
          column_number_(0)
    {
    }
    serialization_error(std::error_code ec,
                size_t line,
                size_t column)
        : error_code_(ec),
          line_number_(line),
          column_number_(column)
    {
    }
    serialization_error(const serialization_error& other)
        : error_code_(other.error_code_),
          line_number_(other.line_number_),
          column_number_(other.column_number_)
    {
    }

    const char* what() const noexcept override
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

    serialization_error& operator=(const serialization_error& e)
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

}
#endif

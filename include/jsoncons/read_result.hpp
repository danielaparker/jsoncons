/// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS_READ_RESULT_HPP    
#define JSONCONS_READ_RESULT_HPP    

#include <system_error>
#include <type_traits>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>

namespace jsoncons {

class read_error
{
    std::error_code ec_{};
    std::string message_arg_;
    std::size_t line_{};
    std::size_t column_{};
    
public:
    read_error(std::error_code ec, std::size_t line, std::size_t column)
        : ec_{ec}, line_{line}, column_{column}
    {
    }
    
    read_error(std::error_code ec, const std::string& message_arg, std::size_t line, std::size_t column)
        : ec_{ec}, message_arg_(message_arg), line_{line}, column_{column}
    {
    }

    read_error(const read_error& other) = default;

    read_error(read_error&& other) = default;

    read_error& operator=(const read_error& other) = default;

    read_error& operator=(read_error&& other) = default;
    
    const std::error_code& code() const noexcept
    {
        return ec_;
    }
    const std::string& message_arg() const noexcept
    {
        return message_arg_;
    }
    std::size_t line() const noexcept
    {
        return line_;
    }
    std::size_t column() const noexcept
    {
        return column_;
    }

    std::string message() const
    {
        std::string str(message_arg_);
        if (!str.empty())
        {
            str.append(": ");
        }
        str.append(ec_.message());
        if (line_ != 0 && column_ != 0)
        {
            str.append(" at line ");
            str.append(std::to_string(line_));
            str.append(" and column ");
            str.append(std::to_string(column_));
        }
        else if (column_ != 0)
        {
            str.append(" at position ");
            str.append(std::to_string(column_));
        }
        return str;
    }
};

inline
std::ostream& operator<<(std::ostream& os, const read_error& err)
{
    os << err.message();
    return os;
}

template <typename T>
using read_result = expected<T,read_error>;

} // namespace jsoncons

#endif // JSONCONS_READ_RESULT_HPP

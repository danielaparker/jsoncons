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
};

inline
std::string to_string(const read_error& err)
{
    std::string str(err.message_arg());
    if (!str.empty())
    {
        str.append(": ");
    }
    str.append(err.code().message());
    if (err.line() != 0 && err.column() != 0)
    {
        str.append(" at line ");
        str.append(std::to_string(err.line()));
        str.append(" and column ");
        str.append(std::to_string(err.column()));
    }
    else if (err.column() != 0)
    {
        str.append(" at position ");
        str.append(std::to_string(err.column()));
    }
    return str;
}

inline
std::ostream& operator<<(std::ostream& os, const read_error& err)
{
    os << to_string(err);
    return os;
}

template <typename T>
using read_result = result<T,read_error>;

} // jsoncons

#endif // JSONCONS_READ_RESULT_HPP

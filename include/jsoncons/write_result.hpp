/// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS_WRITE_RESULT_HPP    
#define JSONCONS_WRITE_RESULT_HPP    

#include <system_error>
#include <type_traits>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/conv_error.hpp>

namespace jsoncons {

class write_error
{
    std::error_code ec_;
    std::string message_arg_;
public:
    write_error(std::error_code ec)
        : ec_(ec)
    {
    }
    write_error(std::error_code ec, const jsoncons::string_view& message_arg)
        : ec_(ec), message_arg_(message_arg)
    {
    }

    write_error(const write_error& other) = default;

    write_error(write_error&& other) = default;

    write_error& operator=(const write_error& other) = default;

    write_error& operator=(write_error&& other) = default;
    
    std::error_code code() const
    {
        return ec_;
    }
    
    const std::string& message_arg() const 
    {
        return message_arg_;
    }
};

inline
std::string to_string(const write_error& err)
{
    std::string str{err.message_arg()};
    if (!str.empty())
    {
        str.append(": ");
    }
    str.append(err.code().message());
    return str;
}

inline
std::ostream& operator<<(std::ostream& os, const write_error& err)
{
    os << to_string(err);
    return os;
}

template <typename T>
using write_result = result<T,write_error>;

} // jsoncons

#endif // JSONCONS_WRITE_RESULT_HPP

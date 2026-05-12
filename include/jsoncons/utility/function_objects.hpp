// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_UTILITY_FUNCTION_OBJECTS_HPP
#define JSONCONS_UTILITY_FUNCTION_OBJECTS_HPP

#include <functional> 
#include <jsoncons/config/compiler_support.hpp>

namespace jsoncons {

struct identity
{
    template< typename T>
    constexpr T&& operator()(T&& val) const noexcept
    {
        return std::forward<T>(val);
    }
};

struct always_true
{
    template< typename T>
    constexpr bool operator()(const T&) const noexcept
    {
        return true;
    }
};

struct decode_half
{
    double operator()(uint16_t val) const 
    {
        return binary::decode_half(val);
    }
};

} // namespace jsoncons

#endif // JSONCONS_UTILITY_FUNCTION_OBJECTS_HPP

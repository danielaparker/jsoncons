/// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons2 for latest version

#ifndef JSONCONS_NONSTD_BIT_HPP    
#define JSONCONS_NONSTD_BIT_HPP    

#include <system_error>
#include <type_traits>
#include <jsoncons/nonstd/compiler_support.hpp>
#include <cassert>

namespace jsoncons {
namespace nonstd {
    
template <typename T>
typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value,int>::type
countl_zero(T x) 
{
    if (x == 0) return std::numeric_limits<T>::digits;
#if defined(__GNUC__) || defined(__clang__)
    if (sizeof(T) <= sizeof(unsigned int)) {
        return __builtin_clz(x) - (sizeof(unsigned int) - sizeof(T)) * 8;
    }
    else if (sizeof(T) <= sizeof(unsigned long long)) {
        return __builtin_clzll(x) - (sizeof(unsigned long long) - sizeof(T)) * 8;
    }
#endif
    int bits = std::numeric_limits<T>::digits;
    T mask = static_cast<T>(1) << (bits - 1);
    int leading_zeros = 0;
    while ((x & mask) == 0 && leading_zeros < bits) {
        leading_zeros++;
        x <<= 1;
    }
    return leading_zeros;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value,int>::type
bit_width(T x) noexcept 
{
    if (x == 0) return 0;
    return std::numeric_limits<T>::digits - countl_zero(x);
}  

} // namespace nonstd
} // namespace jsoncons

#endif // JSONCONS_NONSTD_BIT_HPP

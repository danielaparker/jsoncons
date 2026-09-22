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
int countl_zero(T x) 
{
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value, "Type must be unsigned integer");

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
int countr_zero(T value) {
    static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value, "Type must be unsigned integer");
    
    if (value == 0) {
        return sizeof(T) * 8; // Or std::numeric_limits<T>::digits
    }
    
    #if defined(__GNUC__) || defined(__clang__)
        if (sizeof(T) <= sizeof(unsigned int)) {
            return __builtin_ctz(static_cast<unsigned int>(value));
        } else if (sizeof(T) <= sizeof(unsigned long)) {
            return __builtin_ctzl(static_cast<unsigned long>(value));
        } else {
            return __builtin_ctzll(static_cast<unsigned long long>(value));
        }
    #elif defined(_MSC_VER)
        unsigned long index;
        #if defined(_M_X64) || defined(_M_ARM64)
            if (sizeof(T) == 8) {
                _BitScanForward64(&index, static_cast<unsigned __int64>(value));
                return static_cast<int>(index);
            }
        #endif
        _BitScanForward(&index, static_cast<unsigned long>(value));
        return static_cast<int>(index);
    #else
        int count = 0;
        while ((value & 1) == 0) {
            count++;
            value >>= 1;
        }
        return count;
    #endif
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

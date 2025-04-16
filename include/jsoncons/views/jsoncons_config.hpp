// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS2_CONFIG_JSONCONS2_CONFIG_HPP
#define JSONCONS2_CONFIG_JSONCONS2_CONFIG_HPP

#include <jsoncons/views/compiler_support.hpp>
#include <stdexcept>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring> // std::memcpy
#include <limits>
#include <memory>
#include <bit>
#include <type_traits>
#include <bit>

// The definitions below follow the definitions in compiler_support_p.h, https://github.com/01org/tinycbor
// MIT license

#ifdef __F16C__
#  include <immintrin.h>
#endif

#ifndef __has_builtin
#  define __has_builtin(x)  0
#endif

#if defined(__GNUC__)
#if (__GNUC__ * 100 + __GNUC_MINOR__ >= 403) || (__has_builtin(__builtin_bswap64) && __has_builtin(__builtin_bswap32))
#  define JSONCONS2_BYTE_SWAP_64 __builtin_bswap64
#  define JSONCONS2_BYTE_SWAP_32 __builtin_bswap32
#    ifdef __INTEL_COMPILER
#      define JSONCONS2_BYTE_SWAP_16 _bswap16
#    elif (__GNUC__ * 100 + __GNUC_MINOR__ >= 608) || __has_builtin(__builtin_bswap16)
#      define JSONCONS2_BYTE_SWAP_16    __builtin_bswap16
#  endif
#endif
#elif defined(__sun)
#  include <sys/byteorder.h>
#elif defined(_MSC_VER)
// MSVC, which implies sizeof(long) == 4 
#  define JSONCONS2_BYTE_SWAP_64       _byteswap_uint64
#  define JSONCONS2_BYTE_SWAP_32       _byteswap_ulong
#  define JSONCONS2_BYTE_SWAP_16       _byteswap_ushort
#endif

#define JSONCONS2_EXPAND(X) X    
#define JSONCONS2_QUOTE(Prefix, A) JSONCONS2_EXPAND(Prefix ## #A)
#define JSONCONS2_WIDEN(A) JSONCONS2_EXPAND(L ## A)

namespace jsoncons {
namespace binary {

    struct uint128_holder
    {
        uint64_t lo;
        uint64_t hi;
    };

    JSONCONS2_FORCEINLINE static bool add_check_overflow(std::size_t v1, std::size_t v2, std::size_t *r)
    {
    #if ((defined(__GNUC__) && (__GNUC__ >= 5)) && !defined(__INTEL_COMPILER)) || __has_builtin(__builtin_add_overflow)
        return __builtin_add_overflow(v1, v2, r);
    #else
        // unsigned additions are well-defined 
        *r = v1 + v2;
        return v1 > v1 + v2;
    #endif
    }

    #if defined(__apple_build_version__) && ((__clang_major__ < 8) || ((__clang_major__ == 8) && (__clang_minor__ < 1)))
    #define APPLE_MISSING_INTRINSICS 1
    #endif

    JSONCONS2_FORCEINLINE 
    uint16_t encode_half(double val)
    {
    #if defined(__F16C__) && !defined(APPLE_MISSING_INTRINSICS)
        return _cvtss_sh((float)val, 3);
    #else
        uint64_t v;
        std::memcpy(&v, &val, sizeof(v));
        int64_t sign = static_cast<int64_t>(v >> 63 << 15);
        int64_t exp = (v >> 52) & 0x7ff;
        int64_t mant = v << 12 >> 12 >> (53-11);    /* keep only the 11 most significant bits of the mantissa */
        exp -= 1023;
        if (exp == 1024) {
            /* infinity or NaN */
            exp = 16;
            mant >>= 1;
        } else if (exp >= 16) {
            /* overflow, as largest number */
            exp = 15;
            mant = 1023;
        } else if (exp >= -14) {
            /* regular normal */
        } else if (exp >= -24) {
            /* subnormal */
            mant |= 1024;
            mant >>= -(exp + 14);
            exp = -15;
        } else {
            /* underflow, make zero */
            return 0;
        }

        /* safe cast here as bit operations above guarantee not to overflow */
        return static_cast<uint16_t>(sign | ((exp + 15) << 10) | mant);
    #endif
    }

    /* this function was copied & adapted from RFC 7049 Appendix D */
    JSONCONS2_FORCEINLINE 
    double decode_half(uint16_t half)
    {
    #if defined(__F16C__) && !defined(APPLE_MISSING_INTRINSICS)
        return _cvtsh_ss(half);
    #else
        int64_t exp = (half >> 10) & 0x1f;
        int64_t mant = half & 0x3ff;
        double val;
        if (exp == 0) 
        {
            val = ldexp(static_cast<double>(mant), -24);
        }
        else if (exp != 31) 
        {
            val = ldexp(static_cast<double>(mant) + 1024.0, static_cast<int>(exp - 25));
        } 
        else
        {
            val = mant == 0 ? std::numeric_limits<double>::infinity() : std::nan("");
        }
        return half & 0x8000 ? -val : val;
    #endif
    }

    // byte_swap

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint8_t),T>::type
    byte_swap(T val)
    {
        return val;
    }

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint16_t),T>::type
    byte_swap(T val)
    {
    #if defined(JSONCONS2_BYTE_SWAP_16)
        return JSONCONS2_BYTE_SWAP_16(val);
    #else
        return (static_cast<uint16_t>(val) >> 8) | (static_cast<uint16_t>(val) << 8);
    #endif
    }
     
    template <typename T>
    typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint32_t),T>::type
    byte_swap(T val)
    {
    #if defined(JSONCONS2_BYTE_SWAP_32)
        return JSONCONS2_BYTE_SWAP_32(val);
    #else
        uint32_t tmp = ((static_cast<uint32_t>(val) << 8) & 0xff00ff00) | ((static_cast<uint32_t>(val) >> 8) & 0xff00ff);
        return (tmp << 16) | (tmp >> 16);
    #endif
    }

    template <typename T>
    typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint64_t),T>::type
    byte_swap(T val)
    {
    #if defined(JSONCONS2_BYTE_SWAP_64)
        return JSONCONS2_BYTE_SWAP_64(val);
    #else
        uint64_t tmp = ((static_cast<uint64_t>(val) & 0x00000000ffffffffull) << 32) | ((static_cast<uint64_t>(val) & 0xffffffff00000000ull) >> 32);
        tmp = ((tmp & 0x0000ffff0000ffffull) << 16) | ((tmp & 0xffff0000ffff0000ull) >> 16);
        return ((tmp & 0x00ff00ff00ff00ffull) << 8)  | ((tmp & 0xff00ff00ff00ff00ull) >> 8);
    #endif
    }

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value && sizeof(T) == sizeof(uint32_t),T>::type
    byte_swap(T val)
    {
        uint32_t x;
        std::memcpy(&x,&val,sizeof(uint32_t));
        uint32_t y = byte_swap(x);
        T val2;
        std::memcpy(&val2,&y,sizeof(uint32_t));
        return val2;
    }

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value && sizeof(T) == sizeof(uint64_t),T>::type
    byte_swap(T val)
    {
        uint64_t x;
        std::memcpy(&x,&val,sizeof(uint64_t));
        uint64_t y = byte_swap(x);
        T val2;
        std::memcpy(&val2,&y,sizeof(uint64_t));
        return val2;
    }

    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value && sizeof(T) == 2*sizeof(uint64_t),T>::type
    byte_swap(T val)
    {
        uint128_holder x;
        uint8_t buf[2*sizeof(uint64_t)];
        std::memcpy(buf,&val,2*sizeof(uint64_t));
        std::memcpy(&x.lo,buf,sizeof(uint64_t));
        std::memcpy(&x.hi,buf+sizeof(uint64_t),sizeof(uint64_t));

        uint128_holder y;
        y.lo = byte_swap(x.hi);
        y.hi = byte_swap(x.lo);

        T val2;
        std::memcpy(&val2,&y,2*sizeof(uint64_t));

        return val2;
    }

    // native_to_big

    template <typename T, typename OutputIt, typename Endian=std::endian>
    typename std::enable_if<Endian::native == Endian::big,void>::type
    native_to_big(T val, OutputIt d_first)
    {
        uint8_t buf[sizeof(T)];
        std::memcpy(buf, &val, sizeof(T));
        for (auto item : buf)
        {
            *d_first++ = item;
        }
    }

    template <typename T, typename OutputIt, typename Endian=std::endian>
    typename std::enable_if<Endian::native == Endian::little,void>::type
    native_to_big(T val, OutputIt d_first)
    {
        T val2 = byte_swap(val);
        uint8_t buf[sizeof(T)];
        std::memcpy(buf, &val2, sizeof(T));
        for (auto item : buf)
        {
            *d_first++ = item;
        }
    }

    // native_to_little

    template <typename T, typename OutputIt, typename Endian = std::endian>
    typename std::enable_if<Endian::native == Endian::little,void>::type
    native_to_little(T val, OutputIt d_first)
    {
        uint8_t buf[sizeof(T)];
        std::memcpy(buf, &val, sizeof(T));
        for (auto item : buf)
        {
            *d_first++ = item;
        }
    }

    template <typename T, typename OutputIt, typename Endian=std::endian>
    typename std::enable_if<Endian::native == Endian::big, void>::type
    native_to_little(T val, OutputIt d_first)
    {
        T val2 = byte_swap(val);
        uint8_t buf[sizeof(T)];
        std::memcpy(buf, &val2, sizeof(T));
        for (auto item : buf)
        {
            *d_first++ = item;
        }
    }

    // big_to_native

    template <typename T,class Endian=std::endian>
    typename std::enable_if<Endian::native == Endian::big,T>::type
    big_to_native(const uint8_t* first, std::size_t count)
    {
        if (sizeof(T) > count)
        {
            return T{};
        }
        T val;
        std::memcpy(&val,first,sizeof(T));
        return val;
    }

    template <typename T,class Endian=std::endian>
    typename std::enable_if<Endian::native == Endian::little,T>::type
    big_to_native(const uint8_t* first, std::size_t count)
    {
        if (sizeof(T) > count)
        {
            return T{};
        }
        T val;
        std::memcpy(&val,first,sizeof(T));
        return byte_swap(val);
    }

    // little_to_native

    template <typename T,class Endian=std::endian>
    typename std::enable_if<Endian::native == Endian::little,T>::type
    little_to_native(const uint8_t* first, std::size_t count)
    {
        if (sizeof(T) > count)
        {
            return T{};
        }
        T val;
        std::memcpy(&val,first,sizeof(T));
        return val;
    }

    template <typename T,class Endian=std::endian>
    typename std::enable_if<Endian::native == Endian::big,T>::type
    little_to_native(const uint8_t* first, std::size_t count)
    {
        if (sizeof(T) > count)
        {
            return T{};
        }
        T val;
        std::memcpy(&val,first,sizeof(T));
        return byte_swap(val);
    }

} // binary
} // jsoncons

namespace jsoncons {

    class assertion_error : public std::runtime_error
    {
    public:
        assertion_error(const std::string& s) noexcept
            : std::runtime_error(s)
        {
        }
        const char* what() const noexcept override
        {
            return std::runtime_error::what();
        }
    };

} // namespace jsoncons

// allow to disable exceptions
#if !defined(JSONCONS2_NO_EXCEPTIONS)
    #define JSONCONS2_THROW(exception) throw exception
    #define JSONCONS2_RETHROW throw
    #define JSONCONS2_TRY try
    #define JSONCONS2_CATCH(exception) catch(exception)
#else
    #define JSONCONS2_THROW(exception) std::terminate()
    #define JSONCONS2_RETHROW std::terminate()
    #define JSONCONS2_TRY if (true)
    #define JSONCONS2_CATCH(exception) if (false)
#endif

#define JSONCONS2_STR2(x)  #x
#define JSONCONS2_STR(x)  JSONCONS2_STR2(x)

#ifdef _DEBUG
#define JSONCONS2_ASSERT(x) if (!(x)) { \
    JSONCONS2_THROW(jsoncons::assertion_error("assertion '" #x "' failed at " __FILE__ ":" \
            JSONCONS2_STR(__LINE__))); }
#else
#define JSONCONS2_ASSERT(x) if (!(x)) { \
    JSONCONS2_THROW(jsoncons::assertion_error("assertion '" #x "' failed at  <> :" \
            JSONCONS2_STR( 0 ))); }
#endif // _DEBUG

#endif // JSONCONS2_CONFIG_JSONCONS2_CONFIG_HPP



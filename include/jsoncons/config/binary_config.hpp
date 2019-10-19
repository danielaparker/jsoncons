// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CONFIG_BINARY_CONFIG_HPP
#define JSONCONS_CONFIG_BINARY_CONFIG_HPP

#include <cfloat>
#include <cstddef>
#include <cstdint>
#include <cstring> // std::memcpy
#include <memory>
#include <type_traits> // std::enable_if
#include <jsoncons/json_exception.hpp>

#if defined(__apple_build_version__) && ((__clang_major__ < 8) || ((__clang_major__ == 8) && (__clang_minor__ < 1)))
#define APPLE_MISSING_INTRINSICS 1
#endif

// The definitions below follow the definitions in compiler_support_p.h, https://github.com/01org/tinycbor
// MIT license

#ifdef __F16C__
#  include <immintrin.h>
#endif

#ifndef __has_builtin
#  define __has_builtin(x)  0
#endif

#if (defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 403)) || \
    (__has_builtin(__builtin_bswap64) && __has_builtin(__builtin_bswap32))
#  define JSONCONS_BYTE_SWAP_64 __builtin_bswap64
#  define JSONCONS_BYTE_SWAP_32 __builtin_bswap32
#    ifdef __INTEL_COMPILER
#      define JSONCONS_BYTE_SWAP_16 _bswap16
#    elif (__GNUC__ * 100 + __GNUC_MINOR__ >= 608) || __has_builtin(__builtin_bswap16)
#      define JSONCONS_BYTE_SWAP_16    __builtin_bswap16
#    else
#      define JSONCONS_BYTE_SWAP_16(x) (((uint16_t)x >> 8) | ((uint16_t)x << 8))
#    endif
#elif defined(_MSC_VER)
// MSVC, which implies sizeof(long) == 4 
#  define JSONCONS_BYTE_SWAP_64       _byteswap_uint64
#  define JSONCONS_BYTE_SWAP_32        _byteswap_ulong
#  define JSONCONS_BYTE_SWAP_16        _byteswap_ushort
#else
#      error "swap undefined"
#endif

namespace jsoncons { namespace detail { 

enum class endian
{
#if defined(_MSC_VER) 
// MSVC, which implies Windows, which implies little-endian
     little = 0,
     big    = 1,
     native = little
#elif defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__) && defined(__BYTE_ORDER__) 
     little = __ORDER_LITTLE_ENDIAN__,
     big    = __ORDER_BIG_ENDIAN__,
     native = __BYTE_ORDER__
#else
#error "Unable to determine byte order!"
#endif
};

struct A
{
    static constexpr bool is_little_endian = endian::native == endian::little;
};

struct uint128_holder
{
    uint64_t lo;
    uint64_t hi;
};

class read_nbytes_failed : public std::invalid_argument, public virtual json_exception
{
public:
    explicit read_nbytes_failed(size_t count) noexcept
        : std::invalid_argument("")
    {
        buffer_.append("Failed attempting to read ");
        buffer_.append(std::to_string(count));
        buffer_.append(" bytes from vector");
    }
    ~read_nbytes_failed() noexcept
    {
    }
    const char* what() const noexcept override
    {
        return buffer_.c_str();
    }
private:
    std::string buffer_;
};

namespace detail {

static inline bool add_check_overflow(size_t v1, size_t v2, size_t *r)
{
#if ((defined(__GNUC__) && (__GNUC__ >= 5)) && !defined(__INTEL_COMPILER)) || __has_builtin(__builtin_add_overflow)
    return __builtin_add_overflow(v1, v2, r);
#else
    // unsigned additions are well-defined 
    *r = v1 + v2;
    return v1 > v1 + v2;
#endif
}

}

inline 
uint16_t encode_half(double val)
{
#if defined(__F16C__) && !defined(APPLE_MISSING_INTRINSICS)
    return _cvtss_sh((float)val, 3);
#else
    uint64_t v;
    std::memcpy(&v, &val, sizeof(v));
    int sign = v >> 63 << 15;
    int exp = (v >> 52) & 0x7ff;
    int mant = v << 12 >> 12 >> (53-11);    /* keep only the 11 most significant bits of the mantissa */
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
    return (uint16_t)(sign | ((exp + 15) << 10) | mant);
#endif
}

/* this function was copied & adapted from RFC 7049 Appendix D */
inline 
double decode_half(uint16_t half)
{
#if defined(__F16C__) && !defined(APPLE_MISSING_INTRINSICS)
    return _cvtsh_ss(half);
#else
    int exp = (half >> 10) & 0x1f;
    int mant = half & 0x3ff;
    double val;
    if (exp == 0) 
    {
        val = ldexp((double)mant, -24);
    }
    else if (exp != 31) 
    {
        val = ldexp(mant + 1024.0, exp - 25);
    } 
    else
    {
        val = mant == 0 ? INFINITY : NAN;
    }
    return half & 0x8000 ? -val : val;
#endif
}

// byte_swap

template<class T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint8_t),T>::type
byte_swap(T val)
{
    return val;
}

template<class T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint16_t),T>::type
byte_swap(T val)
{
    return JSONCONS_BYTE_SWAP_16(val);
}
 
template<class T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint32_t),T>::type
byte_swap(T val)
{
    return JSONCONS_BYTE_SWAP_32(val);
}

template<class T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint64_t),T>::type
byte_swap(T val)
{
    return JSONCONS_BYTE_SWAP_64(val);
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value && sizeof(T) == sizeof(uint32_t),T>::type
byte_swap(T val)
{
    uint32_t x;
    memcpy(&x,&val,sizeof(uint32_t));
    uint32_t y = JSONCONS_BYTE_SWAP_32(x);
    T val2;
    memcpy(&val2,&y,sizeof(uint32_t));
    return val2;
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value && sizeof(T) == sizeof(uint64_t),T>::type
byte_swap(T val)
{
    uint64_t x;
    memcpy(&x,&val,sizeof(uint64_t));
    uint64_t y = JSONCONS_BYTE_SWAP_64(x);
    T val2;
    memcpy(&val2,&y,sizeof(uint64_t));
    return val2;
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value && sizeof(T) == 2*sizeof(uint64_t),T>::type
byte_swap(T val)
{
    uint128_holder x;
    uint8_t buf[2*sizeof(uint64_t)];
    memcpy(buf,&val,2*sizeof(uint64_t));
    std::memcpy(&x.lo,buf,sizeof(uint64_t));
    std::memcpy(&x.hi,buf+sizeof(uint64_t),sizeof(uint64_t));

    uint128_holder y;
    y.lo = JSONCONS_BYTE_SWAP_64(x.hi);
    y.hi = JSONCONS_BYTE_SWAP_64(x.lo);

    T val2;
    memcpy(&val2,&y,2*sizeof(uint64_t));

    return val2;
}

// native_to_big

template<typename T, class OutputIt, class Endian=endian>
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

template<typename T, class OutputIt, class Endian=endian>
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

template<typename T, class OutputIt, class Endian = endian>
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

template<typename T, class OutputIt, class Endian=endian>
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

template<class T,class Endian=endian>
typename std::enable_if<Endian::native == Endian::big,T>::type
big_to_native(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    if (first + sizeof(T) > last)
    {
        *endp = first;
        return 0;
    }
    *endp = first + sizeof(T);
    T val;
    std::memcpy(&val,first,sizeof(T));
    return val;
}

template<class T,class Endian=endian>
typename std::enable_if<Endian::native == Endian::little,T>::type
big_to_native(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    if (first + sizeof(T) > last)
    {
        *endp = first;
        return 0;
    }
    *endp = first + sizeof(T);
    T val;
    std::memcpy(&val,first,sizeof(T));
    return byte_swap(val);
}

// little_to_native

template<class T,class Endian=endian>
typename std::enable_if<Endian::native == Endian::little,T>::type
little_to_native(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    if (first + sizeof(T) > last)
    {
        *endp = first;
        return 0;
    }
    *endp = first + sizeof(T);
    T val;
    std::memcpy(&val,first,sizeof(T));
    return val;
}

template<class T,class Endian=endian>
typename std::enable_if<Endian::native == Endian::big,T>::type
little_to_native(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    if (first + sizeof(T) > last)
    {
        *endp = first;
        return 0;
    }
    *endp = first + sizeof(T);
    T val;
    std::memcpy(&val,first,sizeof(T));
    return byte_swap(val);
}

}}

#endif

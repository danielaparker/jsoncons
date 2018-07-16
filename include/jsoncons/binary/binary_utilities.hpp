// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_BINARY_BINARY_UTILITIES_HPP
#define JSONCONS_BINARY_BINARY_UTILITIES_HPP

#include <assert.h>
#include <cfloat>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <istream>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>

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
#  if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#    define JSONCONS_BINARY_TO_BE64     __builtin_bswap64
#    define JSONCONS_BINARY_FROM_BE64     __builtin_bswap64
#    define JSONCONS_BINARY_TO_BE32      __builtin_bswap32
#    define JSONCONS_BINARY_FROM_BE32      __builtin_bswap32
#    ifdef __INTEL_COMPILER
#      define JSONCONS_BINARY_TO_BE16    _bswap16
#      define JSONCONS_BINARY_FROM_BE16    _bswap16
#    elif (__GNUC__ * 100 + __GNUC_MINOR__ >= 608) || __has_builtin(__builtin_bswap16)
#      define JSONCONS_BINARY_TO_BE16    __builtin_bswap16
#      define JSONCONS_BINARY_FROM_BE16    __builtin_bswap16
#    else
#      define JSONCONS_BINARY_TO_BE16(x) (((uint16_t)x >> 8) | ((uint16_t)x << 8))
#      define JSONCONS_BINARY_FROM_BE16    JSONCONS_BINARY_TO_BE16
#    endif
#  else
#    define JSONCONS_BINARY_TO_BE64
#    define JSONCONS_BINARY_FROM_BE64
#    define JSONCONS_BINARY_TO_BE32
#    define JSONCONS_BINARY_FROM_BE32
#    define JSONCONS_BINARY_TO_BE16
#    define JSONCONS_BINARY_FROM_BE16
#  endif
#elif defined(__sun)
#  include <sys/byteorder.h>
#elif defined(_MSC_VER)
/* MSVC, which implies Windows, which implies little-endian and sizeof(long) == 4 */
#  define JSONCONS_BINARY_TO_BE64       _byteswap_uint64
#  define JSONCONS_BINARY_FROM_BE64       _byteswap_uint64
#  define JSONCONS_BINARY_TO_BE32        _byteswap_ulong
#  define JSONCONS_BINARY_FROM_BE32        _byteswap_ulong
#  define JSONCONS_BINARY_TO_BE16        _byteswap_ushort
#  define JSONCONS_BINARY_FROM_BE16        _byteswap_ushort
#endif
#ifndef JSONCONS_BINARY_TO_BE16
#  include <arpa/inet.h>
#  define JSONCONS_BINARY_TO_BE16        ntohs
#  define JSONCONS_BINARY_FROM_BE16        htons
#endif
#ifndef JSONCONS_BINARY_TO_BE32
#  include <arpa/inet.h>
#  define JSONCONS_BINARY_TO_BE32        ntohl
#  define JSONCONS_BINARY_FROM_BE32        htonl
#endif
#ifndef JSONCONS_BINARY_TO_BE64
#  define JSONCONS_BINARY_TO_BE64       ntohll
#  define JSONCONS_BINARY_FROM_BE64       htonll
/* ntohll isn't usually defined */
#  ifndef ntohll
#    if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#      define ntohll
#      define htonll
#    elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#      define ntohll(x)       ((ntohl((uint32_t)(x)) * UINT64_C(0x100000000)) + (ntohl((x) >> 32)))
#      define htonll          ntohll
#    else
#      error "Unable to determine byte order!"
#    endif
#  endif
#endif

namespace jsoncons { namespace binary { 
  
class read_nbytes_failed : public std::invalid_argument, public virtual json_exception
{
public:
    explicit read_nbytes_failed(size_t count) JSONCONS_NOEXCEPT
        : std::invalid_argument("")
    {
        buffer_.append("Failed attempting to read ");
        buffer_.append(std::to_string(count));
        buffer_.append(" bytes from vector");
    }
    ~read_nbytes_failed() JSONCONS_NOEXCEPT
    {
    }
    const char* what() const JSONCONS_NOEXCEPT override
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
#ifdef __F16C__
    return _cvtss_sh(val, 3);
#else
    uint64_t v;
    memcpy(&v, &val, sizeof(v));
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
#ifdef __F16C__
    return _cvtsh_ss(half);
#else
    int exp = (half >> 10) & 0x1f;
    int mant = half & 0x3ff;
    double val;
    if (exp == 0) 
    {
        val = ldexp(mant, -24);
    }
    else if (exp != 31) 
    {
        val = ldexp(mant + 1024, exp - 25);
    } 
    else
    {
        val = mant == 0 ? INFINITY : NAN;
    }
    return half & 0x8000 ? -val : val;
#endif
}

// to_big_endian

template<typename T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint8_t),void>::type
to_big_endian(T val, std::vector<uint8_t>& v)
{
    v.push_back(static_cast<uint8_t>((val) & 0xff));
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value && 
sizeof(T) == sizeof(uint16_t),void>::type
to_big_endian(T val, std::vector<uint8_t>& v)
{
    T x = JSONCONS_BINARY_FROM_BE16(val);

    uint8_t where[sizeof(T)];
    memcpy(where, &x, sizeof(T));

    for (auto e : where)
    {
        v.push_back(e);
    }
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value && 
sizeof(T) == sizeof(uint32_t),void>::type
to_big_endian(T val, std::vector<uint8_t>& v)
{
    T x = JSONCONS_BINARY_FROM_BE32(val);

    uint8_t where[sizeof(T)];
    memcpy(where, &x, sizeof(T));

    for (auto e : where)
    {
        v.push_back(e);
    }
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value && 
sizeof(T) == sizeof(uint64_t),void>::type
to_big_endian(T val, std::vector<uint8_t>& v)
{
    T x = JSONCONS_BINARY_FROM_BE64(val);

    uint8_t where[sizeof(T)];
    memcpy(where, &x, sizeof(T));

    for (auto e : where)
    {
        v.push_back(e);
    }
}

inline
void to_big_endian(float val, std::vector<uint8_t>& v)
{
    uint32_t where;
    std::memcpy(&where,&val,sizeof(val));
    to_big_endian(where, v);
}

inline
void to_big_endian(double val, std::vector<uint8_t>& v)
{
    uint64_t where;
    std::memcpy(&where,&val,sizeof(val));
    to_big_endian(where, v);
}

// from_big_endian

template<class T>
typename std::enable_if<std::is_integral<T>::value && 
sizeof(T) == sizeof(uint8_t),T>::type
from_big_endian(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    if (first + sizeof(T) > last)
    {
        *endp = first;
        return 0;
    }
    else
    {
        *endp = first + sizeof(T);
        return static_cast<T>(*(first));
    }
}

template<class T>
typename std::enable_if<std::is_integral<T>::value && 
sizeof(T) == sizeof(uint16_t),T>::type
from_big_endian(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    if (first + sizeof(T) > last)
    {
        *endp = first;
        return 0;
    }
    else
    {
        *endp = first + sizeof(T);
        T val;
        std::memcpy(&val,first,sizeof(T));
        return JSONCONS_BINARY_TO_BE16(val);
    }
}

template<class T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint32_t),T>::type
from_big_endian(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    if (first + sizeof(T) > last)
    {
        *endp = first;
        return 0;
    }
    else
    {
        *endp = first + sizeof(T);
        T val;
        std::memcpy(&val,first,sizeof(T));
        return JSONCONS_BINARY_TO_BE32(val);
    }
}

template<class T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint64_t),T>::type
from_big_endian(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    if (first + sizeof(T) > last)
    {
        *endp = first;
        return 0;
    }
    else
    {
        *endp = first + sizeof(T);
        T val;
        std::memcpy(&val,first,sizeof(T));
        return JSONCONS_BINARY_TO_BE64(val);
    }
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value && 
sizeof(T) == sizeof(uint32_t),T>::type
from_big_endian(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    uint32_t data = from_big_endian<uint32_t>(first,last,endp);
    T val;
    std::memcpy(&val,&data,sizeof(T));
    return val;
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value && 
sizeof(T) == sizeof(uint64_t),T>::type
from_big_endian(const uint8_t* first, const uint8_t* last, const uint8_t** endp)
{
    uint64_t data = from_big_endian<uint64_t>(first,last,endp);
    T val;
    std::memcpy(&val,&data,sizeof(T));
    return val;
}

}}

#endif

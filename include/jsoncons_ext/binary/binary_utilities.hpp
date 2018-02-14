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
    /* unsigned additions are well-defined */
    *r = v1 + v2;
    return v1 > v1 + v2;
#endif
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
    to_big_endian(*reinterpret_cast<uint32_t*>(&val), v);
}

inline
void to_big_endian(double val, std::vector<uint8_t>& v)
{
    to_big_endian(*reinterpret_cast<uint64_t*>(&val), v);
}

// from_big_endian

template<class T>
typename std::enable_if<std::is_integral<T>::value && 
sizeof(T) == sizeof(uint8_t),T>::type
from_big_endian(const uint8_t* it, const uint8_t* end)
{
    if (it + sizeof(T) > end)
    {
        JSONCONS_THROW(read_nbytes_failed(sizeof(T)));
    }
    return static_cast<T>(*(it));
}

template<class T>
typename std::enable_if<std::is_integral<T>::value && 
sizeof(T) == sizeof(uint16_t),T>::type
from_big_endian(const uint8_t* it, const uint8_t* end)
{
    if (it + sizeof(T) > end)
    {
        JSONCONS_THROW(read_nbytes_failed(sizeof(T)));
    }
    return JSONCONS_BINARY_TO_BE16(*reinterpret_cast<const uint16_t*>(it));
}

template<class T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint32_t),T>::type
from_big_endian(const uint8_t* it, const uint8_t* end)
{
    if (it + sizeof(T) > end)
    {
        JSONCONS_THROW(read_nbytes_failed(sizeof(T)));
    }
    return JSONCONS_BINARY_TO_BE32(*reinterpret_cast<const uint32_t*>(it));
}

template<class T>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) == sizeof(uint64_t),T>::type
from_big_endian(const uint8_t* it, const uint8_t* end)
{
    if (it + sizeof(T) > end)
    {
        JSONCONS_THROW(read_nbytes_failed(sizeof(T)));
    }
    return JSONCONS_BINARY_TO_BE64(*reinterpret_cast<const uint64_t*>(it));
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value && 
sizeof(T) == sizeof(uint32_t),T>::type
from_big_endian(const uint8_t* it, const uint8_t* end)
{
    uint32_t data = from_big_endian<uint32_t>(it,end);
    return *reinterpret_cast<T*>(&data);
}

template<class T>
typename std::enable_if<std::is_floating_point<T>::value && 
sizeof(T) == sizeof(uint64_t),T>::type
from_big_endian(const uint8_t* it, const uint8_t* end)
{
    uint64_t data = from_big_endian<uint64_t>(it,end);
    return *reinterpret_cast<T*>(&data);
}

static const std::string base64_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                           "abcdefghijklmnopqrstuvwxyz"
                                           "0123456789+/";

inline 
static bool is_base64(uint8_t c) 
{
    return isalnum(c) || c == '+' || c == '/';
}

template <class InputIt>
std::string encode_base64(InputIt first, InputIt last)
{
    std::string result;
    unsigned char a3[3];
    unsigned char a4[4];
    int i = 0;
    int j = 0;

    while (first != last)
    {
        a3[i++] = *first++;
        if (i == 3)
        {
            a4[0] = (a3[0] & 0xfc) >> 2;
            a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
            a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
            a4[3] = a3[2] & 0x3f;

            for (i = 0; i < 4; i++) 
            {
                result.push_back(base64_alphabet[a4[i]]);
            }
            i = 0;
        }
    }

    if (i > 0)
    {
        for (j = i; j < 3; ++j) 
        {
            a3[j] = '\0';
        }

        a4[0] = (a3[0] & 0xfc) >> 2;
        a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
        a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; ++j) 
        {
            result.push_back(base64_alphabet[a4[j]]);
        }

        while (i++ < 3) 
        {
            result.push_back('=');
        }
    }

    return result; 
}

inline
std::string encode_base64(const std::string& s)
{
    return encode_base64(s.begin(), s.end());
}

inline
std::string decode_base64(const std::string& base64_string)
{
    std::string result;
    size_t buflen = base64_string.size();
    uint8_t a4[4], a3[3];
    uint8_t i = 0;
    uint8_t j = 0;

    auto first = base64_string.begin();
    auto last = base64_string.end();

    while (first != last && *first != '=')
    {
        JSONCONS_ASSERT(is_base64(*first));

        a4[i++] = *first++; 
        if (i == 4)
        {
            for (i = 0; i < 4; ++i) 
            {
                a4[i] = static_cast<uint8_t>(base64_alphabet.find(a4[i]));
            }

            a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
            a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
            a3[2] = ((a4[2] & 0x3) << 6) +   a4[3];

            for (i = 0; i < 3; i++) 
            {
                result.push_back(a3[i]);
            }
            i = 0;
        }
    }

    if (i > 0)
    {
        for (j = 0; j < i; ++j) 
        {
            a4[j] = static_cast<uint8_t>(base64_alphabet.find(a4[j]));
        }

        a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
        a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);

        for (j = 0; j < i - 1; ++j) 
        {
            result.push_back(a3[j]);
        }
    }

    return result;
}

}

}}

#endif

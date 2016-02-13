// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONCONS_CONFIG_HPP
#define JSONCONS_JSONCONS_CONFIG_HPP

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <cstdarg>
#include <limits> // std::numeric_limits

// Uncomment the following line to suppress deprecated names (recommended for new code)
// #define JSONCONS_NO_DEPRECATED

#define JSONCONS_NO_MACRO_EXP

namespace jsoncons
{

// Follow boost

#if defined (__clang__)
#if defined(_GLIBCXX_USE_NOEXCEPT)
#define JSONCONS_NOEXCEPT _GLIBCXX_USE_NOEXCEPT
#else
#define JSONCONS_NOEXCEPT noexcept
#endif
#elif defined(__GNUC__)
#define JSONCONS_NOEXCEPT _GLIBCXX_USE_NOEXCEPT
#elif defined(_MSC_VER)
#if _MSC_VER >= 1900
#define JSONCONS_NOEXCEPT noexcept
#else
#define JSONCONS_NOEXCEPT
#endif
#else
#define JSONCONS_NOEXCEPT
#endif

#if defined(_MSC_VER)
#define JSONCONS_HAS_FOPEN_S
#define JSONCONS_HAS_WCSTOMBS_S
#if _MSC_VER < 1800 // VS2013
#define JSONCONS_NO_RAW_STRING_LITERALS
#define JSONCONS_NO_FOR_RANGE
#endif
#if _MSC_VER >= 1900
#define JSONCONS_ALIGNOF alignof
#else
#define JSONCONS_ALIGNOF __alignof
#endif
#else
#define JSONCONS_ALIGNOF alignof
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
inline bool is_nan(double x) { return _isnan(x) != 0; }
inline bool is_inf(double x)
{
    return !_finite(x) && !_isnan(x);
}
inline bool is_pos_inf(double x)
{
    return is_inf(x) && x > 0;
}
inline bool is_neg_inf(double x)
{
    return is_inf(x) && x < 0;
}

inline
int c99_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    int count = -1;

    if (size != 0) count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
    if (count == -1) count = _vscprintf(format, ap);

    return count;
}

inline
int c99_snprintf(char *str, size_t size, const char *format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(str, size, format, ap);
    va_end(ap);

    return count;
}
#else
inline bool is_nan(double x)
{ return std::isnan( x ); }
inline bool is_pos_inf(double x)
{return std::isinf(x) && x > 0;}
inline bool is_neg_inf(double x)
{return  std::isinf(x) && x > 0;}

#if __cplusplus >= 201103L
#define c99_snprintf snprintf
#else
#define c99_snprintf std::snprintf
#endif

#endif

}

#endif

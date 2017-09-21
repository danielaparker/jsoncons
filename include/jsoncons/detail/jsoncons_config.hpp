// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONCONS_DETAIL_CONFIG_HPP
#define JSONCONS_JSONCONS_DETAIL_CONFIG_HPP

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
//#define JSONCONS_NO_DEPRECATED

#if defined(__GNUC__) || defined(__clang__)
#define JSONCONS_LIKELY(x) __builtin_expect(!!(x), 1)
#define JSONCONS_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define JSONCONS_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define JSONCONS_LIKELY(x) x
#define JSONCONS_UNLIKELY(x) x
#define JSONCONS_UNREACHABLE() __assume(0)
#else
#define JSONCONS_LIKELY(x) x
#define JSONCONS_UNLIKELY(x) x
#define JSONCONS_UNREACHABLE() do {} while (0)
#endif

#if defined(__GNUC__) && !defined(__INTEL_COMPILER) && !defined(__clang__) && \
    (__GNUC__ * 100 + __GNUC_MINOR__ >= 404)
#  pragma GCC optimize("-ffunction-sections")
#endif

namespace jsoncons
{

#if _MSC_VER > 1800 // _MSC_VER == 1800 -> MS Visual Studio 2013
#else
#define JSONCONS_NO_CONSTEXPR
#endif

#if defined(_MSC_VER)
#if _MSC_VER >= 1900
#define JSONCONS_HAS_USER_DEFINED_LITERALS
#endif
#else
#define JSONCONS_HAS_USER_DEFINED_LITERALS
#endif

//#define JSONCONS_HAS_STRING_VIEW

#if defined(ANDROID) || defined(__ANDROID__)
#define JSONCONS_HAS_STRTOD_L
#endif

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
#define JSONCONS_HAS__ECVT_S
#define JSONCONS_HAS_FOPEN_S
#define JSONCONS_HAS_WCSTOMBS_S
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
#if __cplusplus >= 201103L
#define c99_snprintf snprintf
#else
#define c99_snprintf std::snprintf
#endif

#endif

#define JSONCONS_DEFINE_LITERAL( name, lit ) \
template< class Ch > Ch const*const name(); \
template<> inline char const * const name<char>() { return lit; } \
template<> inline wchar_t const*const name<wchar_t>() { return L ## lit; } 

}

#endif

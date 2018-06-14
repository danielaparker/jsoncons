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
#include <exception>
#include <limits> // std::numeric_limits

// Uncomment the following line to suppress deprecated names (recommended for new code)
//#define JSONCONS_NO_DEPRECATED

// The definitions below follow the definitions in compiler_support_p.h, https://github.com/01org/tinycbor
// MIT license

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

namespace jsoncons
{

#if _MSC_VER > 1800 // _MSC_VER == 1800 -> MS Visual Studio 2013
#else
#define JSONCONS_NO_CONSTEXPR
#endif

#define JSONCONS_NO_TO_CHARS

#if defined(_MSC_VER)
#if _MSC_VER >= 1900
#define JSONCONS_HAS_USER_DEFINED_LITERALS
#endif
#else
#define JSONCONS_HAS_USER_DEFINED_LITERALS
#endif

//#define JSONCONS_HAS_STRING_VIEW

#if defined(ANDROID) || defined(__ANDROID__)
#define JSONCONS_HAS_STRTOLD_L
#if __ANDROID_API__ >= 21
#else
#define JSONCONS_NO_LOCALECONV
#endif
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
#define JSONCONS_HAS_MSC__STRTOD_L
#define JSONCONS_HAS__ECVT_S
#define JSONCONS_HAS_FOPEN_S
#if _MSC_VER >= 1900
#define JSONCONS_ALIGNOF alignof
#else
#define JSONCONS_ALIGNOF __alignof
#endif
#else
#define JSONCONS_ALIGNOF alignof
#endif

#define JSONCONS_DEFINE_LITERAL( name, lit ) \
template< class Ch > Ch const* name(); \
template<> inline char const * name<char>() { return lit; } \
template<> inline wchar_t const* name<wchar_t>() { return L ## lit; }

}

#endif

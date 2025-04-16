// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS2_CONFIG_COMPILER_SUPPORT_HPP
#define JSONCONS2_CONFIG_COMPILER_SUPPORT_HPP

#if defined (__clang__)
#define JSONCONS2_CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#endif

/*==============================================================================
 * Compiler Macros
 *============================================================================*/

/** compiler version (MSVC) */
#ifdef _MSC_VER
#   define YYJSON_MSC_VER _MSC_VER
#else
#   define YYJSON_MSC_VER 0
#endif

/** compiler version (GCC) */
#ifdef __GNUC__
#   define YYJSON_GCC_VER __GNUC__
#   if defined(__GNUC_PATCHLEVEL__)
#       define yyjson_gcc_available(major, minor, patch) \
            ((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) \
            >= (major * 10000 + minor * 100 + patch))
#   else
#       define yyjson_gcc_available(major, minor, patch) \
            ((__GNUC__ * 10000 + __GNUC_MINOR__ * 100) \
            >= (major * 10000 + minor * 100 + patch))
#   endif
#else
#   define YYJSON_GCC_VER 0
#   define yyjson_gcc_available(major, minor, patch) 0
#endif

/** real gcc check */
#if !defined(__clang__) && !defined(__INTEL_COMPILER) && !defined(__ICC) && \
    defined(__GNUC__)
#   define YYJSON_IS_REAL_GCC 1
#else
#   define YYJSON_IS_REAL_GCC 0
#endif

/** C version (STDC) */
#if defined(__STDC__) && (__STDC__ >= 1) && defined(__STDC_VERSION__)
#   define YYJSON_STDC_VER __STDC_VERSION__
#else
#   define YYJSON_STDC_VER 0
#endif

/** C++ version */
#if defined(__cplusplus)
#   define YYJSON_CPP_VER __cplusplus
#else
#   define YYJSON_CPP_VER 0
#endif

/** compiler builtin check (since gcc 10.0, clang 2.6, icc 2021) */
#ifndef yyjson_has_builtin
#   ifdef __has_builtin
#       define yyjson_has_builtin(x) __has_builtin(x)
#   else
#       define yyjson_has_builtin(x) 0
#   endif
#endif

/** compiler attribute check (since gcc 5.0, clang 2.9, icc 17) */
#ifndef yyjson_has_attribute
#   ifdef __has_attribute
#       define yyjson_has_attribute(x) __has_attribute(x)
#   else
#       define yyjson_has_attribute(x) 0
#   endif
#endif

/** compiler feature check (since clang 2.6, icc 17) */
#ifndef yyjson_has_feature
#   ifdef __has_feature
#       define yyjson_has_feature(x) __has_feature(x)
#   else
#       define yyjson_has_feature(x) 0
#   endif
#endif

/** include check (since gcc 5.0, clang 2.7, icc 16, msvc 2017 15.3) */
#ifndef yyjson_has_include
#   ifdef __has_include
#       define yyjson_has_include(x) __has_include(x)
#   else
#       define yyjson_has_include(x) 0
#   endif
#endif

/** noinline for compiler */
#ifndef yyjson_noinline
#   if YYJSON_MSC_VER >= 1400
#       define yyjson_noinline __declspec(noinline)
#   elif yyjson_has_attribute(noinline) || YYJSON_GCC_VER >= 4
#       define yyjson_noinline __attribute__((noinline))
#   else
#       define yyjson_noinline
#   endif
#endif

/** align for compiler */
#ifndef yyjson_align
#   if YYJSON_MSC_VER >= 1300
#       define yyjson_align(x) __declspec(align(x))
#   elif yyjson_has_attribute(aligned) || defined(__GNUC__)
#       define yyjson_align(x) __attribute__((aligned(x)))
#   elif YYJSON_CPP_VER >= 201103L
#       define yyjson_align(x) alignas(x)
#   else
#       define yyjson_align(x)
#   endif
#endif

/** compile-time constant check for compiler */
#ifndef yyjson_constant_p
#   if yyjson_has_builtin(__builtin_constant_p) || (YYJSON_GCC_VER >= 3)
#       define YYJSON_HAS_CONSTANT_P 1
#       define yyjson_constant_p(value) __builtin_constant_p(value)
#   else
#       define YYJSON_HAS_CONSTANT_P 0
#       define yyjson_constant_p(value) 0
#   endif
#endif

/** deprecate warning */
#ifndef yyjson_deprecated
#   if YYJSON_MSC_VER >= 1400
#       define yyjson_deprecated(msg) __declspec(deprecated(msg))
#   elif yyjson_has_feature(attribute_deprecated_with_message) || \
        (YYJSON_GCC_VER > 4 || (YYJSON_GCC_VER == 4 && __GNUC_MINOR__ >= 5))
#       define yyjson_deprecated(msg) __attribute__((deprecated(msg)))
#   elif YYJSON_GCC_VER >= 3
#       define yyjson_deprecated(msg) __attribute__((deprecated))
#   else
#       define yyjson_deprecated(msg)
#   endif
#endif

/** function export */
#ifndef yyjson_api
#   if defined(_WIN32)
#       if defined(YYJSON_EXPORTS) && YYJSON_EXPORTS
#           define yyjson_api __declspec(dllexport)
#       elif defined(YYJSON_IMPORTS) && YYJSON_IMPORTS
#           define yyjson_api __declspec(dllimport)
#       else
#           define yyjson_api
#       endif
#   elif yyjson_has_attribute(visibility) || YYJSON_GCC_VER >= 4
#       define yyjson_api __attribute__((visibility("default")))
#   else
#       define yyjson_api
#   endif
#endif

/** stdint (C89 compatible) */
#if (defined(YYJSON_HAS_STDINT_H) && YYJSON_HAS_STDINT_H) || \
    YYJSON_MSC_VER >= 1600 || YYJSON_STDC_VER >= 199901L || \
    defined(_STDINT_H) || defined(_STDINT_H_) || \
    defined(__CLANG_STDINT_H) || defined(_STDINT_H_INCLUDED) || \
    yyjson_has_include(<stdint.h>)
#   include <stdint.h>
#elif defined(_MSC_VER)
#   if _MSC_VER < 1300
        typedef signed char         int8_t;
        typedef signed short        int16_t;
        typedef signed int          int32_t;
        typedef unsigned char       uint8_t;
        typedef unsigned short      uint16_t;
        typedef unsigned int        uint32_t;
        typedef signed __int64      int64_t;
        typedef unsigned __int64    uint64_t;
#   else
        typedef signed __int8       int8_t;
        typedef signed __int16      int16_t;
        typedef signed __int32      int32_t;
        typedef unsigned __int8     uint8_t;
        typedef unsigned __int16    uint16_t;
        typedef unsigned __int32    uint32_t;
        typedef signed __int64      int64_t;
        typedef unsigned __int64    uint64_t;
#   endif
#else
#   if UCHAR_MAX == 0xFFU
        typedef signed char     int8_t;
        typedef unsigned char   uint8_t;
#   else
#       error cannot find 8-bit integer type
#   endif
#   if USHRT_MAX == 0xFFFFU
        typedef unsigned short  uint16_t;
        typedef signed short    int16_t;
#   elif UINT_MAX == 0xFFFFU
        typedef unsigned int    uint16_t;
        typedef signed int      int16_t;
#   else
#       error cannot find 16-bit integer type
#   endif
#   if UINT_MAX == 0xFFFFFFFFUL
        typedef unsigned int    uint32_t;
        typedef signed int      int32_t;
#   elif ULONG_MAX == 0xFFFFFFFFUL
        typedef unsigned long   uint32_t;
        typedef signed long     int32_t;
#   elif USHRT_MAX == 0xFFFFFFFFUL
        typedef unsigned short  uint32_t;
        typedef signed short    int32_t;
#   else
#       error cannot find 32-bit integer type
#   endif
#   if defined(__INT64_TYPE__) && defined(__UINT64_TYPE__)
        typedef __INT64_TYPE__  int64_t;
        typedef __UINT64_TYPE__ uint64_t;
#   elif defined(__GNUC__) || defined(__clang__)
#       if !defined(_SYS_TYPES_H) && !defined(__int8_t_defined)
        __extension__ typedef long long             int64_t;
#       endif
        __extension__ typedef unsigned long long    uint64_t;
#   elif defined(_LONG_LONG) || defined(__MWERKS__) || defined(_CRAYC) || \
        defined(__SUNPRO_C) || defined(__SUNPRO_CC)
        typedef long long           int64_t;
        typedef unsigned long long  uint64_t;
#   elif (defined(__BORLANDC__) && __BORLANDC__ > 0x460) || \
        defined(__WATCOM_INT64__) || defined (__alpha) || defined (__DECC)
        typedef __int64             int64_t;
        typedef unsigned __int64    uint64_t;
#   else
#       error cannot find 64-bit integer type
#   endif
#endif

/** stdbool (C89 compatible) */
#if (defined(YYJSON_HAS_STDBOOL_H) && YYJSON_HAS_STDBOOL_H) || \
    (yyjson_has_include(<stdbool.h>) && !defined(__STRICT_ANSI__)) || \
    YYJSON_MSC_VER >= 1800 || YYJSON_STDC_VER >= 199901L
#   include <stdbool.h>
#elif !defined(__bool_true_false_are_defined)
#   define __bool_true_false_are_defined 1
#   if defined(__cplusplus)
#       if defined(__GNUC__) && !defined(__STRICT_ANSI__)
#           define _Bool bool
#           if __cplusplus < 201103L
#               define bool bool
#               define false false
#               define true true
#           endif
#       endif
#   else
#       define bool unsigned char
#       define true 1
#       define false 0
#   endif
#endif

/** char bit check */
#if defined(CHAR_BIT)
#   if CHAR_BIT != 8
#       error non 8-bit char is not supported
#   endif
#endif

/**
 Microsoft Visual C++ 6.0 doesn't support converting number from uint64_t to double:
 error C2520: conversion from unsigned __int64 to double not implemented.
 */
#ifndef YYJSON_U64_TO_F64_NO_IMPL
#   if (0 < YYJSON_MSC_VER) && (YYJSON_MSC_VER <= 1200)
#       define YYJSON_U64_TO_F64_NO_IMPL 1
#   else
#       define YYJSON_U64_TO_F64_NO_IMPL 0
#   endif
#endif

#if defined(__clang__)
#  define JSONCONS2_FALLTHROUGH [[clang::fallthrough]]
#elif defined(__GNUC__) && ((__GNUC__ >= 7))
#  define JSONCONS2_FALLTHROUGH __attribute__((fallthrough))
#elif defined (__GNUC__)
#  define JSONCONS2_FALLTHROUGH // FALLTHRU
#else
#  define JSONCONS2_FALLTHROUGH
#endif

#if defined(__GNUC__) || defined(__clang__)
#define JSONCONS2_LIKELY(x) __builtin_expect(!!(x), 1)
#define JSONCONS2_UNLIKELY(x) __builtin_expect(!!(x), 0)
#define JSONCONS2_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#define JSONCONS2_LIKELY(x) x
#define JSONCONS2_UNLIKELY(x) x
#define JSONCONS2_UNREACHABLE() __assume(0)
#else
#define JSONCONS2_LIKELY(x) x
#define JSONCONS2_UNLIKELY(x) x
#define JSONCONS2_UNREACHABLE() do {} while (0)
#endif

// Deprecated symbols markup
#if (defined(__cplusplus) && __cplusplus >= 201402L)
#define JSONCONS2_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#endif

#if !defined(JSONCONS2_DEPRECATED_MSG) && defined(__GNUC__) && defined(__has_extension)
#if __has_extension(attribute_deprecated_with_message)
#define JSONCONS2_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#endif
#endif

#if !defined(JSONCONS2_DEPRECATED_MSG) && defined(_MSC_VER)
#if (_MSC_VER) >= 1920
#define JSONCONS2_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#else
#define JSONCONS2_DEPRECATED_MSG(msg) __declspec(deprecated(msg))
#endif
#endif

// Following boost/atomic/detail/config.hpp
#if !defined(JSONCONS2_DEPRECATED_MSG) && (\
    (defined(__GNUC__) && ((__GNUC__ + 0) * 100 + (__GNUC_MINOR__ + 0)) >= 405) ||\
    (defined(__SUNPRO_CC) && (__SUNPRO_CC + 0) >= 0x5130))
    #define JSONCONS2_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
#endif

#if !defined(JSONCONS2_DEPRECATED_MSG) && defined(__clang__) && defined(__has_extension)
    #if __has_extension(attribute_deprecated_with_message)
        #define JSONCONS2_DEPRECATED_MSG(msg) __attribute__((deprecated(msg)))
    #else
        #define JSONCONS2_DEPRECATED_MSG(msg) __attribute__((deprecated))
    #endif
#endif

#if !defined(JSONCONS2_DEPRECATED_MSG)
#define JSONCONS2_DEPRECATED_MSG(msg)
#endif

#if !defined(JSONCONS2_NO_EXCEPTIONS)

#if defined(__GNUC__) && !defined(__EXCEPTIONS)
# define JSONCONS2_NO_EXCEPTIONS 1
#elif defined(_MSC_VER)
#if defined(_HAS_EXCEPTIONS) && _HAS_EXCEPTIONS == 0
# define JSONCONS2_NO_EXCEPTIONS 1
#elif !defined(_CPPUNWIND)
# define JSONCONS2_NO_EXCEPTIONS 1
#endif
#endif
#endif

#ifndef JSONCONS2_FORCEINLINE
# ifdef _MSC_VER
#  define JSONCONS2_FORCEINLINE __forceinline
# elif defined(__GNUC__) || defined(__clang__)
#  define JSONCONS2_FORCEINLINE inline __attribute__((always_inline))
# else
#  define JSONCONS2_FORCEINLINE inline
# endif
#endif

#endif



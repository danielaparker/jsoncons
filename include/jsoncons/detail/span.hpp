//
// span for C++98 and later.
// Based on http://wg21.link/p0122r7
// For more information see https://github.com/martinmoene/span-lite
//
// Copyright 2018-2019 Martin Moene
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef JSONCONS_NONSTD_SPAN_HPP_INCLUDED
#define JSONCONS_NONSTD_SPAN_HPP_INCLUDED

#define jsoncons_span_lite_MAJOR  0
#define jsoncons_span_lite_MINOR  6
#define jsoncons_span_lite_PATCH  0

#define jsoncons_span_lite_VERSION  jsoncons_span_STRINGIFY(jsoncons_span_lite_MAJOR) "." jsoncons_span_STRINGIFY(jsoncons_span_lite_MINOR) "." jsoncons_span_STRINGIFY(jsoncons_span_lite_PATCH)

#define jsoncons_span_STRINGIFY(  x )  jsoncons_span_STRINGIFY_( x )
#define jsoncons_span_STRINGIFY_( x )  #x

// span configuration:

#define jsoncons_span_SPAN_DEFAULT  0
#define jsoncons_span_SPAN_NONSTD   1
#define jsoncons_span_SPAN_STD      2

#ifndef  jsoncons_span_CONFIG_SELECT_SPAN
# define jsoncons_span_CONFIG_SELECT_SPAN  ( jsoncons_span_HAVE_STD_SPAN ? jsoncons_span_SPAN_STD : jsoncons_span_SPAN_NONSTD )
#endif

#define jsoncons_span_CONFIG_INDEX_TYPE size_t
#ifndef  jsoncons_span_CONFIG_INDEX_TYPE
# define jsoncons_span_CONFIG_INDEX_TYPE  std::ptrdiff_t
#endif

// span configuration (features):

#ifndef  jsoncons_span_FEATURE_WITH_CONTAINER
#ifdef   jsoncons_span_FEATURE_WITH_CONTAINER_TO_STD
# define jsoncons_span_FEATURE_WITH_CONTAINER  jsoncons_span_IN_STD( jsoncons_span_FEATURE_WITH_CONTAINER_TO_STD )
#else
# define jsoncons_span_FEATURE_WITH_CONTAINER  0
#endif
#endif

#ifndef  jsoncons_span_FEATURE_CONSTRUCTION_FROM_STDARRAY_ELEMENT_TYPE
# define jsoncons_span_FEATURE_CONSTRUCTION_FROM_STDARRAY_ELEMENT_TYPE  0
#endif

#ifndef  jsoncons_span_FEATURE_MEMBER_AT
# define jsoncons_span_FEATURE_MEMBER_AT  0
#endif

#ifndef  jsoncons_span_FEATURE_MEMBER_BACK_FRONT
# define jsoncons_span_FEATURE_MEMBER_BACK_FRONT  1
#endif

#ifndef  jsoncons_span_FEATURE_MEMBER_CALL_OPERATOR
# define jsoncons_span_FEATURE_MEMBER_CALL_OPERATOR  0
#endif

#ifndef  jsoncons_span_FEATURE_MEMBER_SWAP
# define jsoncons_span_FEATURE_MEMBER_SWAP  0
#endif

#ifndef  jsoncons_span_FEATURE_NON_MEMBER_FIRST_LAST_SUB
# define jsoncons_span_FEATURE_NON_MEMBER_FIRST_LAST_SUB  0
#endif

#ifndef  jsoncons_span_FEATURE_COMPARISON
# define jsoncons_span_FEATURE_COMPARISON  1  // Note: C++20 does not provide comparison
#endif

#ifndef  jsoncons_span_FEATURE_SAME
# define jsoncons_span_FEATURE_SAME  0
#endif

#ifndef  jsoncons_span_FEATURE_MAKE_SPAN
#ifdef   jsoncons_span_FEATURE_MAKE_SPAN_TO_STD
# define jsoncons_span_FEATURE_MAKE_SPAN  jsoncons_span_IN_STD( jsoncons_span_FEATURE_MAKE_SPAN_TO_STD )
#else
# define jsoncons_span_FEATURE_MAKE_SPAN  0
#endif
#endif

#ifndef  jsoncons_span_FEATURE_BYTE_SPAN
# define jsoncons_span_FEATURE_BYTE_SPAN  0
#endif

// Control presence of exception handling (try and auto discover):

#ifndef jsoncons_span_CONFIG_NO_EXCEPTIONS
# if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
#  define jsoncons_span_CONFIG_NO_EXCEPTIONS  0
# else
#  define jsoncons_span_CONFIG_NO_EXCEPTIONS  1
#  undef  jsoncons_span_CONFIG_CONTRACT_VIOLATION_THROWS
#  undef  jsoncons_span_CONFIG_CONTRACT_VIOLATION_TERMINATES
#  define jsoncons_span_CONFIG_CONTRACT_VIOLATION_THROWS  0
#  define jsoncons_span_CONFIG_CONTRACT_VIOLATION_TERMINATES  1
# endif
#endif

// Control pre- and postcondition violation behaviour:

#if    defined( jsoncons_span_CONFIG_CONTRACT_LEVEL_ON )
# define        jsoncons_span_CONFIG_CONTRACT_LEVEL_MASK  0x11
#elif  defined( jsoncons_span_CONFIG_CONTRACT_LEVEL_OFF )
# define        jsoncons_span_CONFIG_CONTRACT_LEVEL_MASK  0x00
#elif  defined( jsoncons_span_CONFIG_CONTRACT_LEVEL_EXPECTS_ONLY )
# define        jsoncons_span_CONFIG_CONTRACT_LEVEL_MASK  0x01
#elif  defined( jsoncons_span_CONFIG_CONTRACT_LEVEL_ENSURES_ONLY )
# define        jsoncons_span_CONFIG_CONTRACT_LEVEL_MASK  0x10
#else
# define        jsoncons_span_CONFIG_CONTRACT_LEVEL_MASK  0x11
#endif

#if    defined( jsoncons_span_CONFIG_CONTRACT_VIOLATION_THROWS )
# define        jsoncons_span_CONFIG_CONTRACT_VIOLATION_THROWS_V  jsoncons_span_CONFIG_CONTRACT_VIOLATION_THROWS
#else
# define        jsoncons_span_CONFIG_CONTRACT_VIOLATION_THROWS_V  0
#endif

#if    defined( jsoncons_span_CONFIG_CONTRACT_VIOLATION_THROWS     ) && jsoncons_span_CONFIG_CONTRACT_VIOLATION_THROWS && \
       defined( jsoncons_span_CONFIG_CONTRACT_VIOLATION_TERMINATES ) && jsoncons_span_CONFIG_CONTRACT_VIOLATION_TERMINATES
# error Please define none or one of jsoncons_span_CONFIG_CONTRACT_VIOLATION_THROWS and jsoncons_span_CONFIG_CONTRACT_VIOLATION_TERMINATES to 1, but not both.
#endif

// C++ language version detection (C++20 is speculative):
// Note: VC14.0/1900 (VS2015) lacks too much from C++14.

#ifndef   jsoncons_span_CPLUSPLUS
# if defined(_MSVC_LANG ) && !defined(__clang__)
#  define jsoncons_span_CPLUSPLUS  (_MSC_VER == 1900 ? 201103L : _MSVC_LANG )
# else
#  define jsoncons_span_CPLUSPLUS  __cplusplus
# endif
#endif

#define jsoncons_span_CPP98_OR_GREATER  ( jsoncons_span_CPLUSPLUS >= 199711L )
#define jsoncons_span_CPP11_OR_GREATER  ( jsoncons_span_CPLUSPLUS >= 201103L )
#define jsoncons_span_CPP14_OR_GREATER  ( jsoncons_span_CPLUSPLUS >= 201402L )
#define jsoncons_span_CPP17_OR_GREATER  ( jsoncons_span_CPLUSPLUS >= 201703L )
#define jsoncons_span_CPP20_OR_GREATER  ( jsoncons_span_CPLUSPLUS >= 202000L )

// C++ language version (represent 98 as 3):

#define jsoncons_span_CPLUSPLUS_V  ( jsoncons_span_CPLUSPLUS / 100 - (jsoncons_span_CPLUSPLUS > 200000 ? 2000 : 1994) )

#define jsoncons_span_IN_STD( v )  ( ((v) == 98 ? 3 : (v)) >= jsoncons_span_CPLUSPLUS_V )

#define jsoncons_span_CONFIG(         feature )  ( jsoncons_span_CONFIG_##feature )
#define jsoncons_span_FEATURE(        feature )  ( jsoncons_span_FEATURE_##feature )
#define jsoncons_span_FEATURE_TO_STD( feature )  ( jsoncons_span_IN_STD( jsoncons_span_FEATURE( feature##_TO_STD ) ) )

// Use C++20 std::span if available and requested:

#if jsoncons_span_CPP20_OR_GREATER && defined(__has_include )
# if __has_include( <span> )
#  define jsoncons_span_HAVE_STD_SPAN  1
# else
#  define jsoncons_span_HAVE_STD_SPAN  0
# endif
#else
# define  jsoncons_span_HAVE_STD_SPAN  0
#endif

#define  jsoncons_span_USES_STD_SPAN  ( (jsoncons_span_CONFIG_SELECT_SPAN == jsoncons_span_SPAN_STD) || ((jsoncons_span_CONFIG_SELECT_SPAN == jsoncons_span_SPAN_DEFAULT) && jsoncons_span_HAVE_STD_SPAN) )

//
// Use C++20 std::span:
//

#if jsoncons_span_USES_STD_SPAN

#include <span>

namespace nonstd {

using std::span;

// Note: C++20 does not provide comparison
// using std::operator==;
// using std::operator!=;
// using std::operator<;
// using std::operator<=;
// using std::operator>;
// using std::operator>=;
}  // namespace nonstd

#else  // jsoncons_span_USES_STD_SPAN

#include <algorithm>

// Compiler versions:
//
// MSVC++ 6.0  _MSC_VER == 1200 (Visual Studio 6.0)
// MSVC++ 7.0  _MSC_VER == 1300 (Visual Studio .NET 2002)
// MSVC++ 7.1  _MSC_VER == 1310 (Visual Studio .NET 2003)
// MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
// MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
// MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
// MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
// MSVC++ 12.0 _MSC_VER == 1800 (Visual Studio 2013)
// MSVC++ 14.0 _MSC_VER == 1900 (Visual Studio 2015)
// MSVC++ 14.1 _MSC_VER >= 1910 (Visual Studio 2017)

#if defined(_MSC_VER ) && !defined(__clang__)
# define jsoncons_span_COMPILER_MSVC_VER      (_MSC_VER )
# define jsoncons_span_COMPILER_MSVC_VERSION  (_MSC_VER / 10 - 10 * ( 5 + (_MSC_VER < 1900 ) ) )
#else
# define jsoncons_span_COMPILER_MSVC_VER      0
# define jsoncons_span_COMPILER_MSVC_VERSION  0
#endif

#define jsoncons_span_COMPILER_VERSION( major, minor, patch )  ( 10 * ( 10 * (major) + (minor) ) + (patch) )

#if defined(__clang__)
# define jsoncons_span_COMPILER_CLANG_VERSION  jsoncons_span_COMPILER_VERSION(__clang_major__, __clang_minor__, __clang_patchlevel__)
#else
# define jsoncons_span_COMPILER_CLANG_VERSION  0
#endif

#if defined(__GNUC__) && !defined(__clang__)
# define jsoncons_span_COMPILER_GNUC_VERSION  jsoncons_span_COMPILER_VERSION(__GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__)
#else
# define jsoncons_span_COMPILER_GNUC_VERSION  0
#endif

// half-open range [lo..hi):
#define jsoncons_span_BETWEEN( v, lo, hi )  ( (lo) <= (v) && (v) < (hi) )

// Compiler warning suppression:

#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wundef"
# define jsoncons_span_RESTORE_WARNINGS()   _Pragma( "clang diagnostic pop" )

#elif defined __GNUC__
# pragma GCC   diagnostic push
# pragma GCC   diagnostic ignored "-Wundef"
# define jsoncons_span_RESTORE_WARNINGS()   _Pragma( "GCC diagnostic pop" )

#elif jsoncons_span_COMPILER_MSVC_VERSION >= 140
# define jsoncons_span_DISABLE_MSVC_WARNINGS(codes)  __pragma(warning(push))  __pragma(warning(disable: codes))
# define jsoncons_span_RESTORE_WARNINGS()            __pragma(warning(pop ))

// Suppress the following MSVC GSL warnings:
// - C26439, gsl::f.6 : special function 'function' can be declared 'noexcept'
// - C26440, gsl::f.6 : function 'function' can be declared 'noexcept'
// - C26472, gsl::t.1 : don't use a static_cast for arithmetic conversions;
//                      use brace initialization, gsl::narrow_cast or gsl::narrow
// - C26473: gsl::t.1 : don't cast between pointer types where the source type and the target type are the same
// - C26481: gsl::b.1 : don't use pointer arithmetic. Use span instead
// - C26490: gsl::t.1 : don't use reinterpret_cast

jsoncons_span_DISABLE_MSVC_WARNINGS( 26439 26440 26472 26473 26481 26490 )

#else
# define jsoncons_span_RESTORE_WARNINGS()  /*empty*/
#endif

// Presence of language and library features:

#define jsoncons_span_HAVE( feature )  ( jsoncons_span_HAVE_##feature )

#ifdef _HAS_CPP0X
# define jsoncons_span_HAS_CPP0X  _HAS_CPP0X
#else
# define jsoncons_span_HAS_CPP0X  0
#endif

#define jsoncons_span_CPP11_80   (jsoncons_span_CPP11_OR_GREATER || jsoncons_span_COMPILER_MSVC_VER >= 1400)
#define jsoncons_span_CPP11_90   (jsoncons_span_CPP11_OR_GREATER || jsoncons_span_COMPILER_MSVC_VER >= 1500)
#define jsoncons_span_CPP11_100  (jsoncons_span_CPP11_OR_GREATER || jsoncons_span_COMPILER_MSVC_VER >= 1600)
#define jsoncons_span_CPP11_110  (jsoncons_span_CPP11_OR_GREATER || jsoncons_span_COMPILER_MSVC_VER >= 1700)
#define jsoncons_span_CPP11_120  (jsoncons_span_CPP11_OR_GREATER || jsoncons_span_COMPILER_MSVC_VER >= 1800)
#define jsoncons_span_CPP11_140  (jsoncons_span_CPP11_OR_GREATER || jsoncons_span_COMPILER_MSVC_VER >= 1900)

#define jsoncons_span_CPP14_000  (jsoncons_span_CPP14_OR_GREATER)
#define jsoncons_span_CPP14_120  (jsoncons_span_CPP14_OR_GREATER || jsoncons_span_COMPILER_MSVC_VER >= 1800)
#define jsoncons_span_CPP14_140  (jsoncons_span_CPP14_OR_GREATER || jsoncons_span_COMPILER_MSVC_VER >= 1900)

#define jsoncons_span_CPP17_000  (jsoncons_span_CPP17_OR_GREATER)

// Presence of C++11 language features:

#define jsoncons_span_HAVE_ALIAS_TEMPLATE            jsoncons_span_CPP11_140
#define jsoncons_span_HAVE_AUTO                      jsoncons_span_CPP11_100
#define jsoncons_span_HAVE_CONSTEXPR_11              jsoncons_span_CPP11_140
#define jsoncons_span_HAVE_DEFAULT_FUNCTION_TEMPLATE_ARG  jsoncons_span_CPP11_120
#define jsoncons_span_HAVE_EXPLICIT_CONVERSION       jsoncons_span_CPP11_140
#define jsoncons_span_HAVE_INITIALIZER_LIST          jsoncons_span_CPP11_120
#define jsoncons_span_HAVE_IS_DEFAULT                jsoncons_span_CPP11_140
#define jsoncons_span_HAVE_IS_DELETE                 jsoncons_span_CPP11_140
#define jsoncons_span_HAVE_NOEXCEPT                  jsoncons_span_CPP11_140
#define jsoncons_span_HAVE_NULLPTR                   jsoncons_span_CPP11_100
#define jsoncons_span_HAVE_STATIC_ASSERT             jsoncons_span_CPP11_100

// Presence of C++14 language features:

#define jsoncons_span_HAVE_CONSTEXPR_14              jsoncons_span_CPP14_000

// Presence of C++17 language features:

#define jsoncons_span_HAVE_DEPRECATED                jsoncons_span_CPP17_000
#define jsoncons_span_HAVE_NODISCARD                 jsoncons_span_CPP17_000
#define jsoncons_span_HAVE_NORETURN                  jsoncons_span_CPP17_000

// MSVC: template parameter deduction guides since Visual Studio 2017 v15.7

#define jsoncons_span_HAVE_DEDUCTION_GUIDES         (jsoncons_span_CPP17_OR_GREATER && ! jsoncons_span_BETWEEN( jsoncons_span_COMPILER_MSVC_VERSION, 1, 999 ))

// Presence of C++ library features:

#define jsoncons_span_HAVE_ADDRESSOF                 jsoncons_span_CPP17_000
#define jsoncons_span_HAVE_ARRAY                     jsoncons_span_CPP11_110
#define jsoncons_span_HAVE_BYTE                      jsoncons_span_CPP17_000
#define jsoncons_span_HAVE_CONDITIONAL               jsoncons_span_CPP11_120
#define jsoncons_span_HAVE_CONTAINER_DATA_METHOD    (jsoncons_span_CPP11_140 || ( jsoncons_span_COMPILER_MSVC_VERSION >= 90 && jsoncons_span_HAS_CPP0X ))
#define jsoncons_span_HAVE_DATA                      jsoncons_span_CPP17_000
#define jsoncons_span_HAVE_LONGLONG                  jsoncons_span_CPP11_80
#define jsoncons_span_HAVE_REMOVE_CONST              jsoncons_span_CPP11_110
#define jsoncons_span_HAVE_SNPRINTF                  jsoncons_span_CPP11_140
#define jsoncons_span_HAVE_TYPE_TRAITS               jsoncons_span_CPP11_90

// Presence of byte-lite:

#ifdef NONSTD_BYTE_LITE_HPP
# define jsoncons_span_HAVE_NONSTD_BYTE  1
#else
# define jsoncons_span_HAVE_NONSTD_BYTE  0
#endif

// C++ feature usage:

#if jsoncons_span_HAVE_ADDRESSOF
# define jsoncons_span_ADDRESSOF(x)  std::addressof(x)
#else
# define jsoncons_span_ADDRESSOF(x)  (&x)
#endif

#if jsoncons_span_HAVE_CONSTEXPR_11
# define jsoncons_span_constexpr constexpr
#else
# define jsoncons_span_constexpr /*jsoncons_span_constexpr*/
#endif

#if jsoncons_span_HAVE_CONSTEXPR_14
# define jsoncons_span_constexpr14 constexpr
#else
# define jsoncons_span_constexpr14 /*jsoncons_span_constexpr*/
#endif

#if jsoncons_span_HAVE_EXPLICIT_CONVERSION
# define jsoncons_span_explicit explicit
#else
# define jsoncons_span_explicit /*explicit*/
#endif

#if jsoncons_span_HAVE_IS_DELETE
# define jsoncons_span_is_delete = delete
#else
# define jsoncons_span_is_delete
#endif

#if jsoncons_span_HAVE_IS_DELETE
# define jsoncons_span_is_delete_access public
#else
# define jsoncons_span_is_delete_access private
#endif

#if jsoncons_span_HAVE_NOEXCEPT && ! jsoncons_span_CONFIG_CONTRACT_VIOLATION_THROWS_V
# define jsoncons_span_noexcept noexcept
#else
# define jsoncons_span_noexcept /*noexcept*/
#endif

#if jsoncons_span_HAVE_NULLPTR
# define jsoncons_span_nullptr nullptr
#else
# define jsoncons_span_nullptr NULL
#endif

#if jsoncons_span_HAVE_DEPRECATED
# define jsoncons_span_deprecated(msg) [[deprecated(msg)]]
#else
# define jsoncons_span_deprecated(msg) /*[[deprecated]]*/
#endif

#if jsoncons_span_HAVE_NODISCARD
# define jsoncons_span_nodiscard [[nodiscard]]
#else
# define jsoncons_span_nodiscard /*[[nodiscard]]*/
#endif

#if jsoncons_span_HAVE_NORETURN
# define jsoncons_span_noreturn [[noreturn]]
#else
# define jsoncons_span_noreturn /*[[noreturn]]*/
#endif

// Other features:

#define jsoncons_span_HAVE_CONSTRAINED_SPAN_CONTAINER_CTOR  jsoncons_span_HAVE_DEFAULT_FUNCTION_TEMPLATE_ARG

// Additional includes:

#if jsoncons_span_HAVE( ADDRESSOF )
# include <memory>
#endif

#if jsoncons_span_HAVE( ARRAY )
# include <array>
#endif

#if jsoncons_span_HAVE( BYTE )
# include <cstddef>
#endif

#if jsoncons_span_HAVE( DATA )
# include <iterator> // for std::data(), std::size()
#endif

#if jsoncons_span_HAVE( TYPE_TRAITS )
# include <type_traits>
#endif

#if ! jsoncons_span_HAVE( CONSTRAINED_SPAN_CONTAINER_CTOR )
# include <vector>
#endif

#if jsoncons_span_FEATURE( MEMBER_AT ) > 1
# include <cstdio>
#endif

#if ! jsoncons_span_CONFIG( NO_EXCEPTIONS )
# include <stdexcept>
#endif

// Contract violation

#define jsoncons_span_ELIDE_CONTRACT_EXPECTS  ( 0 == ( jsoncons_span_CONFIG_CONTRACT_LEVEL_MASK & 0x01 ) )
#define jsoncons_span_ELIDE_CONTRACT_ENSURES  ( 0 == ( jsoncons_span_CONFIG_CONTRACT_LEVEL_MASK & 0x10 ) )

#if jsoncons_span_ELIDE_CONTRACT_EXPECTS
# define jsoncons_span_constexpr_exp    jsoncons_span_constexpr
# define jsoncons_span_EXPECTS( cond )  /* Expect elided */
#else
# define jsoncons_span_constexpr_exp    jsoncons_span_constexpr14
# define jsoncons_span_EXPECTS( cond )  jsoncons_span_CONTRACT_CHECK( "Precondition", cond )
#endif

#if jsoncons_span_ELIDE_CONTRACT_ENSURES
# define jsoncons_span_constexpr_ens    jsoncons_span_constexpr
# define jsoncons_span_ENSURES( cond )  /* Ensures elided */
#else
# define jsoncons_span_constexpr_ens    jsoncons_span_constexpr14
# define jsoncons_span_ENSURES( cond )  jsoncons_span_CONTRACT_CHECK( "Postcondition", cond )
#endif

#define jsoncons_span_CONTRACT_CHECK( type, cond ) \
    cond ? static_cast< void >( 0 ) \
         : nonstd::jsoncons_span_lite::detail::report_contract_violation( jsoncons_span_LOCATION( __FILE__, __LINE__ ) ": " type " violation." )

#ifdef __GNUG__
# define jsoncons_span_LOCATION( file, line )  file ":" jsoncons_span_STRINGIFY( line )
#else
# define jsoncons_span_LOCATION( file, line )  file "(" jsoncons_span_STRINGIFY( line ) ")"
#endif

// Method enabling

#define jsoncons_span_REQUIRES_0(VA) \
    template< bool B = (VA), typename std::enable_if<B, int>::type = 0 >

#define jsoncons_span_REQUIRES_T(VA) \
    , typename = typename std::enable_if< (VA), nonstd::jsoncons_span_lite::detail::enabler >::type

#define jsoncons_span_REQUIRES_R(R, VA) \
    typename std::enable_if< (VA), R>::type

#define jsoncons_span_REQUIRES_A(VA) \
    , typename std::enable_if< (VA), void*>::type = nullptr

namespace nonstd {
namespace jsoncons_span_lite {

// [views.constants], constants

typedef jsoncons_span_CONFIG_INDEX_TYPE index_t;

typedef std::ptrdiff_t extent_t;

jsoncons_span_constexpr const extent_t dynamic_extent = -1;

template< class T, extent_t Extent = dynamic_extent >
class span;

// Tag to select span constructor taking a container (prevent ms-gsl warning C26426):

struct with_container_t { jsoncons_span_constexpr with_container_t() jsoncons_span_noexcept {} };
const  jsoncons_span_constexpr   with_container_t with_container;

// C++11 emulation:

namespace std11 {

#if jsoncons_span_HAVE( REMOVE_CONST )

using std::remove_cv;
using std::remove_const;
using std::remove_volatile;

#else

template< class T > struct remove_const            { typedef T type; };
template< class T > struct remove_const< T const > { typedef T type; };

template< class T > struct remove_volatile               { typedef T type; };
template< class T > struct remove_volatile< T volatile > { typedef T type; };

template< class T >
struct remove_cv
{
    typedef typename std11::remove_volatile< typename std11::remove_const< T >::type >::type type;
};

#endif  // jsoncons_span_HAVE( REMOVE_CONST )

#if jsoncons_span_HAVE( TYPE_TRAITS )

using std::is_same;
using std::integral_constant;
using std::true_type;
using std::false_type;

#else

template< class T, T v > struct integral_constant { enum { value = v }; };
typedef integral_constant< bool, true  > true_type;
typedef integral_constant< bool, false > false_type;

template< class T, class U > struct is_same : false_type{};
template< class T          > struct is_same<T, T> : true_type{};

#endif

} // namespace std11

// C++17 emulation:

namespace std17 {

template< bool v > struct bool_constant : std11::integral_constant<bool, v>{};

#if jsoncons_span_CPP11_120

template< class...>
using void_t = void;

#endif

#if jsoncons_span_HAVE( DATA )

using std::data;
using std::size;

#elif jsoncons_span_HAVE_CONSTRAINED_SPAN_CONTAINER_CTOR

template< typename T, size_t N >
inline jsoncons_span_constexpr auto size( const T(&)[N] ) jsoncons_span_noexcept -> size_t
{
    return N;
}

template< typename C >
inline jsoncons_span_constexpr auto size( C const & cont ) -> decltype( cont.size() )
{
    return cont.size();
}

template< typename T, size_t N >
inline jsoncons_span_constexpr auto data( T(&arr)[N] ) jsoncons_span_noexcept -> T*
{
    return &arr[0];
}

template< typename C >
inline jsoncons_span_constexpr auto data( C & cont ) -> decltype( cont.data() )
{
    return cont.data();
}

template< typename C >
inline jsoncons_span_constexpr auto data( C const & cont ) -> decltype( cont.data() )
{
    return cont.data();
}

template< typename E >
inline jsoncons_span_constexpr auto data( std::initializer_list<E> il ) jsoncons_span_noexcept -> E const *
{
    return il.begin();
}

#endif // jsoncons_span_HAVE( DATA )

#if jsoncons_span_HAVE( BYTE )
using std::byte;
#elif jsoncons_span_HAVE( NONSTD_BYTE )
using nonstd::byte;
#endif

} // namespace std17

// Implementation details:

namespace detail {

/*enum*/ struct enabler{};

#if jsoncons_span_HAVE( TYPE_TRAITS )

template< class Q >
struct is_jsoncons_span_oracle : std::false_type{};

template< class T, std::ptrdiff_t Extent >
struct is_jsoncons_span_oracle< span<T, Extent> > : std::true_type{};

template< class Q >
struct is_span : is_jsoncons_span_oracle< typename std::remove_cv<Q>::type >{};

template< class Q >
struct is_std_array_oracle : std::false_type{};

#if jsoncons_span_HAVE( ARRAY )

template< class T, std::size_t Extent >
struct is_std_array_oracle< std::array<T, Extent> > : std::true_type{};

#endif

template< class Q >
struct is_std_array : is_std_array_oracle< typename std::remove_cv<Q>::type >{};

template< class Q >
struct is_array : std::false_type {};

template< class T >
struct is_array<T[]> : std::true_type {};

template< class T, std::size_t N >
struct is_array<T[N]> : std::true_type {};

#if jsoncons_span_CPP11_140 && ! jsoncons_span_BETWEEN( jsoncons_span_COMPILER_GNUC_VERSION, 1, 500 )

template< class, class = void >
struct has_size_and_data : std::false_type{};

template< class C >
struct has_size_and_data
<
    C, std17::void_t<
        decltype( std17::size(std::declval<C>()) ),
        decltype( std17::data(std::declval<C>()) ) >
> : std::true_type{};

template< class, class, class = void >
struct is_compatible_element : std::false_type {};

template< class C, class E >
struct is_compatible_element
<
    C, E, std17::void_t<
        decltype( std17::data(std::declval<C>()) ) >
> : std::is_convertible< typename std::remove_pointer<decltype( std17::data( std::declval<C&>() ) )>::type(*)[], E(*)[] >{};

template< class C >
struct is_container : std17::bool_constant
<
    ! is_span< C >::value
    && ! is_array< C >::value
    && ! is_std_array< C >::value
    &&   has_size_and_data< C >::value
>{};

template< class C, class E >
struct is_compatible_container : std17::bool_constant
<
    is_container<C>::value
    && is_compatible_element<C,E>::value
>{};

#else // jsoncons_span_CPP11_140

template<
    class C, class E
        jsoncons_span_REQUIRES_T((
            ! is_span< C >::value
            && ! is_array< C >::value
            && ! is_std_array< C >::value
            && ( std::is_convertible< typename std::remove_pointer<decltype( std17::data( std::declval<C&>() ) )>::type(*)[], E(*)[] >::value)
        //  &&   has_size_and_data< C >::value
        ))
        , class = decltype( std17::size(std::declval<C>()) )
        , class = decltype( std17::data(std::declval<C>()) )
>
struct is_compatible_container : std::true_type{};

#endif // jsoncons_span_CPP11_140

#endif // jsoncons_span_HAVE( TYPE_TRAITS )

#if ! jsoncons_span_CONFIG( NO_EXCEPTIONS )
#if   jsoncons_span_FEATURE( MEMBER_AT ) > 1

// format index and size:

#if defined(__clang__)
# pragma clang diagnostic ignored "-Wlong-long"
#elif defined __GNUC__
# pragma GCC   diagnostic ignored "-Wformat=ll"
# pragma GCC   diagnostic ignored "-Wlong-long"
#endif

inline void throw_out_of_range( index_t idx, index_t size )
{
    const char fmt[] = "span::at(): index '%lli' is out of range [0..%lli)";
    char buffer[ 2 * 20 + sizeof fmt ];
    sprintf( buffer, fmt, static_cast<long long>(idx), static_cast<long long>(size) );

    throw std::out_of_range( buffer );
}

#else // MEMBER_AT

inline void throw_out_of_range( index_t /*idx*/, index_t /*size*/ )
{
    throw std::out_of_range( "span::at(): index outside span" );
}
#endif  // MEMBER_AT
#endif  // NO_EXCEPTIONS

#if jsoncons_span_CONFIG( CONTRACT_VIOLATION_THROWS_V )

struct contract_violation : std::logic_error
{
    explicit contract_violation( char const * const message )
        : std::logic_error( message )
    {}
};

inline void report_contract_violation( char const * msg )
{
    throw contract_violation( msg );
}

#else // jsoncons_span_CONFIG( CONTRACT_VIOLATION_THROWS_V )

jsoncons_span_noreturn inline void report_contract_violation( char const * /*msg*/ ) jsoncons_span_noexcept
{
    std::terminate();
}

#endif // jsoncons_span_CONFIG( CONTRACT_VIOLATION_THROWS_V )

}  // namespace detail

// Prevent signed-unsigned mismatch:

#define jsoncons_span_sizeof(T)  static_cast<extent_t>( sizeof(T) )

template< class T >
inline jsoncons_span_constexpr index_t to_size( T size )
{
    return static_cast<index_t>( size );
}

//
// [views.span] - A view over a contiguous, single-dimension sequence of objects
//
template< class T, extent_t Extent /*= dynamic_extent*/ >
class span
{
public:
    // constants and types

    typedef T element_type;
    typedef typename std11::remove_cv< T >::type value_type;

    typedef T &       reference;
    typedef T *       pointer;
    typedef T const * const_pointer;
    typedef T const & const_reference;

    typedef index_t   index_type;
    typedef extent_t  extent_type;

    typedef pointer        iterator;
    typedef const_pointer  const_iterator;

    typedef std::ptrdiff_t difference_type;

    typedef std::reverse_iterator< iterator >       reverse_iterator;
    typedef std::reverse_iterator< const_iterator > const_reverse_iterator;

//    static constexpr extent_type extent = Extent;
    enum { extent = Extent };

    // 26.7.3.2 Constructors, copy, and assignment [span.cons]

#if jsoncons_span_HAVE( DEFAULT_FUNCTION_TEMPLATE_ARG )
    jsoncons_span_REQUIRES_0(( Extent <= 0 ))
#endif
    jsoncons_span_constexpr span() jsoncons_span_noexcept
        : data_( jsoncons_span_nullptr )
        , size_( 0 )
    {
        // jsoncons_span_EXPECTS( data() == jsoncons_span_nullptr );
        // jsoncons_span_EXPECTS( size() == 0 );
    }

    jsoncons_span_constexpr_exp span( pointer ptr, index_type count )
        : data_( ptr )
        , size_( count )
    {
        jsoncons_span_EXPECTS(
            ( ptr == jsoncons_span_nullptr && count == 0 ) ||
            ( ptr != jsoncons_span_nullptr && count >= 0 )
        );
    }

    jsoncons_span_constexpr_exp span( pointer firstElem, pointer lastElem )
        : data_( firstElem )
        , size_( to_size( std::distance( firstElem, lastElem ) ) )
    {
        jsoncons_span_EXPECTS(
            std::distance( firstElem, lastElem ) >= 0
        );
    }

    template< size_t N
#if jsoncons_span_HAVE( DEFAULT_FUNCTION_TEMPLATE_ARG )
        jsoncons_span_REQUIRES_T((
            (Extent == dynamic_extent || Extent == static_cast<extent_t>(N))
            && std::is_convertible< value_type(*)[], element_type(*)[] >::value
        ))
#endif
    >
    jsoncons_span_constexpr span( element_type ( &arr )[ N ] ) jsoncons_span_noexcept
        : data_( jsoncons_span_ADDRESSOF( arr[0] ) )
        , size_( N  )
    {}

#if jsoncons_span_HAVE( ARRAY )

    template< size_t N
# if jsoncons_span_HAVE( DEFAULT_FUNCTION_TEMPLATE_ARG )
        jsoncons_span_REQUIRES_T((
            (Extent == dynamic_extent || Extent == static_cast<extent_t>(N))
            && std::is_convertible< value_type(*)[], element_type(*)[] >::value
        ))
# endif
    >
# if jsoncons_span_FEATURE( CONSTRUCTION_FROM_STDARRAY_ELEMENT_TYPE )
        jsoncons_span_constexpr span( std::array< element_type, N > & arr ) jsoncons_span_noexcept
# else
        jsoncons_span_constexpr span( std::array< value_type, N > & arr ) jsoncons_span_noexcept
# endif
        : data_( jsoncons_span_ADDRESSOF( arr[0] ) )
        , size_( to_size( arr.size() ) )
    {}

    template< size_t N
# if jsoncons_span_HAVE( DEFAULT_FUNCTION_TEMPLATE_ARG )
        jsoncons_span_REQUIRES_T((
            (Extent == dynamic_extent || Extent == static_cast<extent_t>(N))
            && std::is_convertible< value_type(*)[], element_type(*)[] >::value
        ))
# endif
    >
    jsoncons_span_constexpr span( std::array< value_type, N> const & arr ) jsoncons_span_noexcept
        : data_( jsoncons_span_ADDRESSOF( arr[0] ) )
        , size_( to_size( arr.size() ) )
    {}

#endif // jsoncons_span_HAVE( ARRAY )

#if jsoncons_span_HAVE( CONSTRAINED_SPAN_CONTAINER_CTOR )
    template< class Container
        jsoncons_span_REQUIRES_T((
            detail::is_compatible_container< Container, element_type >::value
        ))
    >
    jsoncons_span_constexpr span( Container & cont )
        : data_( std17::data( cont ) )
        , size_( to_size( std17::size( cont ) ) )
    {}

    template< class Container
        jsoncons_span_REQUIRES_T((
            std::is_const< element_type >::value
            && detail::is_compatible_container< Container, element_type >::value
        ))
    >
    jsoncons_span_constexpr span( Container const & cont )
        : data_( std17::data( cont ) )
        , size_( to_size( std17::size( cont ) ) )
    {}

#endif // jsoncons_span_HAVE( CONSTRAINED_SPAN_CONTAINER_CTOR )

#if jsoncons_span_FEATURE( WITH_CONTAINER )

    template< class Container >
    jsoncons_span_constexpr span( with_container_t, Container & cont )
        : data_( cont.size() == 0 ? jsoncons_span_nullptr : jsoncons_span_ADDRESSOF( cont[0] ) )
        , size_( to_size( cont.size() ) )
    {}

    template< class Container >
    jsoncons_span_constexpr span( with_container_t, Container const & cont )
        : data_( cont.size() == 0 ? jsoncons_span_nullptr : const_cast<pointer>( jsoncons_span_ADDRESSOF( cont[0] ) ) )
        , size_( to_size( cont.size() ) )
    {}
#endif

#if jsoncons_span_HAVE( IS_DEFAULT )
    jsoncons_span_constexpr span( span const & other ) jsoncons_span_noexcept = default;

    ~span() jsoncons_span_noexcept = default;

    jsoncons_span_constexpr14 span & operator=( span const & other ) jsoncons_span_noexcept = default;
#else
    jsoncons_span_constexpr span( span const & other ) jsoncons_span_noexcept
        : data_( other.data_ )
        , size_( other.size_ )
    {}

    ~span() jsoncons_span_noexcept
    {}

    jsoncons_span_constexpr14 span & operator=( span const & other ) jsoncons_span_noexcept
    {
        data_ = other.data_;
        size_ = other.size_;

        return *this;
    }
#endif

    template< class OtherElementType, extent_type OtherExtent
#if jsoncons_span_HAVE( DEFAULT_FUNCTION_TEMPLATE_ARG )
        jsoncons_span_REQUIRES_T((
            (Extent == dynamic_extent || Extent == OtherExtent)
            && std::is_convertible<OtherElementType(*)[], element_type(*)[]>::value
        ))
#endif
    >
    jsoncons_span_constexpr_exp span( span<OtherElementType, OtherExtent> const & other ) jsoncons_span_noexcept
        : data_( reinterpret_cast<pointer>( other.data() ) )
        , size_( other.size() )
    {
        jsoncons_span_EXPECTS( OtherExtent == dynamic_extent || other.size() == to_size(OtherExtent) );
    }

    // 26.7.3.3 Subviews [span.sub]

    template< extent_type Count >
    jsoncons_span_constexpr_exp span< element_type, Count >
    first() const
    {
        jsoncons_span_EXPECTS( 0 <= Count && Count <= size() );

        return span< element_type, Count >( data(), Count );
    }

    template< extent_type Count >
    jsoncons_span_constexpr_exp span< element_type, Count >
    last() const
    {
        jsoncons_span_EXPECTS( 0 <= Count && Count <= size() );

        return span< element_type, Count >( data() + (size() - Count), Count );
    }

#if jsoncons_span_HAVE( DEFAULT_FUNCTION_TEMPLATE_ARG )
    template< index_type Offset, extent_type Count = dynamic_extent >
#else
    template< index_type Offset, extent_type Count /*= dynamic_extent*/ >
#endif
    jsoncons_span_constexpr_exp span< element_type, Count >
    subspan() const
    {
        jsoncons_span_EXPECTS(
            ( 0 <= Offset && Offset <= size() ) &&
            ( Count == dynamic_extent || (0 <= Count && Count + Offset <= size()) )
        );

        return span< element_type, Count >(
            data() + Offset, Count != dynamic_extent ? Count : (Extent != dynamic_extent ? Extent - Offset : size() - Offset) );
    }

    jsoncons_span_constexpr_exp span< element_type, dynamic_extent >
    first( index_type count ) const
    {
        jsoncons_span_EXPECTS( 0 <= count && count <= size() );

        return span< element_type, dynamic_extent >( data(), count );
    }

    jsoncons_span_constexpr_exp span< element_type, dynamic_extent >
    last( index_type count ) const
    {
        jsoncons_span_EXPECTS( 0 <= count && count <= size() );

        return span< element_type, dynamic_extent >( data() + ( size() - count ), count );
    }

    jsoncons_span_constexpr_exp span< element_type, dynamic_extent >
    subspan( index_type offset, index_type count = static_cast<index_type>(dynamic_extent) ) const
    {
        jsoncons_span_EXPECTS(
            ( ( 0 <= offset  && offset <= size() ) ) &&
            ( count == static_cast<index_type>(dynamic_extent) || ( 0 <= count && offset + count <= size() ) )
        );

        return span< element_type, dynamic_extent >(
            data() + offset, count == static_cast<index_type>(dynamic_extent) ? size() - offset : count );
    }

    // 26.7.3.4 Observers [span.obs]

    jsoncons_span_constexpr index_type size() const jsoncons_span_noexcept
    {
        return size_;
    }

    jsoncons_span_constexpr std::ptrdiff_t ssize() const jsoncons_span_noexcept
    {
        return static_cast<std::ptrdiff_t>( size_ );
    }

    jsoncons_span_constexpr index_type size_bytes() const jsoncons_span_noexcept
    {
        return size() * to_size( sizeof( element_type ) );
    }

    jsoncons_span_nodiscard jsoncons_span_constexpr bool empty() const jsoncons_span_noexcept
    {
        return size() == 0;
    }

    // 26.7.3.5 Element access [span.elem]

    jsoncons_span_constexpr_exp reference operator[]( index_type idx ) const
    {
        jsoncons_span_EXPECTS( 0 <= idx && idx < size() );

        return *( data() + idx );
    }

#if jsoncons_span_FEATURE( MEMBER_CALL_OPERATOR )
    jsoncons_span_deprecated("replace operator() with operator[]")

    jsoncons_span_constexpr_exp reference operator()( index_type idx ) const
    {
        jsoncons_span_EXPECTS( 0 <= idx && idx < size() );

        return *( data() + idx );
    }
#endif

#if jsoncons_span_FEATURE( MEMBER_AT )
    jsoncons_span_constexpr14 reference at( index_type idx ) const
    {
#if jsoncons_span_CONFIG( NO_EXCEPTIONS )
        return this->operator[]( idx );
#else
        if ( idx < 0 || size() <= idx )
        {
            detail::throw_out_of_range( idx, size() );
        }
        return *( data() + idx );
#endif
    }
#endif

    jsoncons_span_constexpr pointer data() const jsoncons_span_noexcept
    {
        return data_;
    }

#if jsoncons_span_FEATURE( MEMBER_BACK_FRONT )

    jsoncons_span_constexpr_exp reference front() const jsoncons_span_noexcept
    {
        jsoncons_span_EXPECTS( ! empty() );

        return *data();
    }

    jsoncons_span_constexpr_exp reference back() const jsoncons_span_noexcept
    {
        jsoncons_span_EXPECTS( ! empty() );

        return *( data() + size() - 1 );
    }

#endif

    // xx.x.x.x Modifiers [span.modifiers]

#if jsoncons_span_FEATURE( MEMBER_SWAP )

    jsoncons_span_constexpr14 void swap( span & other ) jsoncons_span_noexcept
    {
        using std::swap;
        swap( data_, other.data_ );
        swap( size_, other.size_ );
    }
#endif

    // 26.7.3.6 Iterator support [span.iterators]

    jsoncons_span_constexpr iterator begin() const jsoncons_span_noexcept
    {
#if jsoncons_span_CPP11_OR_GREATER
        return { data() };
#else
        return iterator( data() );
#endif
    }

    jsoncons_span_constexpr iterator end() const jsoncons_span_noexcept
    {
#if jsoncons_span_CPP11_OR_GREATER
        return { data() + size() };
#else
        return iterator( data() + size() );
#endif
    }

    jsoncons_span_constexpr const_iterator cbegin() const jsoncons_span_noexcept
    {
#if jsoncons_span_CPP11_OR_GREATER
        return { data() };
#else
        return const_iterator( data() );
#endif
    }

    jsoncons_span_constexpr const_iterator cend() const jsoncons_span_noexcept
    {
#if jsoncons_span_CPP11_OR_GREATER
        return { data() + size() };
#else
        return const_iterator( data() + size() );
#endif
    }

    jsoncons_span_constexpr reverse_iterator rbegin() const jsoncons_span_noexcept
    {
        return reverse_iterator( end() );
    }

    jsoncons_span_constexpr reverse_iterator rend() const jsoncons_span_noexcept
    {
        return reverse_iterator( begin() );
    }

    jsoncons_span_constexpr const_reverse_iterator crbegin() const jsoncons_span_noexcept
    {
        return const_reverse_iterator ( cend() );
    }

    jsoncons_span_constexpr const_reverse_iterator crend() const jsoncons_span_noexcept
    {
        return const_reverse_iterator( cbegin() );
    }

private:
    pointer    data_;
    index_type size_;
};

// class template argument deduction guides:

#if jsoncons_span_HAVE( DEDUCTION_GUIDES )   // jsoncons_span_CPP17_OR_GREATER

template< class T, size_t N >
span( T (&)[N] ) -> span<T, static_cast<extent_t>(N)>;

template< class T, size_t N >
span( std::array<T, N> & ) -> span<T, static_cast<extent_t>(N)>;

template< class T, size_t N >
span( std::array<T, N> const & ) -> span<const T, static_cast<extent_t>(N)>;

template< class Container >
span( Container& ) -> span<typename Container::value_type>;

template< class Container >
span( Container const & ) -> span<const typename Container::value_type>;

#endif // jsoncons_span_HAVE( DEDUCTION_GUIDES )

// 26.7.3.7 Comparison operators [span.comparison]

#if jsoncons_span_FEATURE( COMPARISON )
#if jsoncons_span_FEATURE( SAME )

template< class T1, extent_t E1, class T2, extent_t E2  >
inline jsoncons_span_constexpr bool same( span<T1,E1> const & l, span<T2,E2> const & r ) jsoncons_span_noexcept
{
    return std11::is_same<T1, T2>::value
        && l.size() == r.size()
        && static_cast<void const*>( l.data() ) == r.data();
}

#endif

template< class T1, extent_t E1, class T2, extent_t E2  >
inline jsoncons_span_constexpr bool operator==( span<T1,E1> const & l, span<T2,E2> const & r )
{
    return
#if jsoncons_span_FEATURE( SAME )
        same( l, r ) ||
#endif
        ( l.size() == r.size() && std::equal( l.begin(), l.end(), r.begin() ) );
}

template< class T1, extent_t E1, class T2, extent_t E2  >
inline jsoncons_span_constexpr bool operator<( span<T1,E1> const & l, span<T2,E2> const & r )
{
    return std::lexicographical_compare( l.begin(), l.end(), r.begin(), r.end() );
}

template< class T1, extent_t E1, class T2, extent_t E2  >
inline jsoncons_span_constexpr bool operator!=( span<T1,E1> const & l, span<T2,E2> const & r )
{
    return !( l == r );
}

template< class T1, extent_t E1, class T2, extent_t E2  >
inline jsoncons_span_constexpr bool operator<=( span<T1,E1> const & l, span<T2,E2> const & r )
{
    return !( r < l );
}

template< class T1, extent_t E1, class T2, extent_t E2  >
inline jsoncons_span_constexpr bool operator>( span<T1,E1> const & l, span<T2,E2> const & r )
{
    return ( r < l );
}

template< class T1, extent_t E1, class T2, extent_t E2  >
inline jsoncons_span_constexpr bool operator>=( span<T1,E1> const & l, span<T2,E2> const & r )
{
    return !( l < r );
}

#endif // jsoncons_span_FEATURE( COMPARISON )

// 26.7.2.6 views of object representation [span.objectrep]

#if jsoncons_span_HAVE( BYTE ) || jsoncons_span_HAVE( NONSTD_BYTE )

template< class T, extent_t Extent >
inline jsoncons_span_constexpr span< const std17::byte, ( (Extent == dynamic_extent) ? dynamic_extent : (jsoncons_span_sizeof(T) * Extent) ) >
as_bytes( span<T,Extent> spn ) jsoncons_span_noexcept
{
#if 0
    return { reinterpret_cast< std17::byte const * >( spn.data() ), spn.size_bytes() };
#else
    return span< const std17::byte, ( (Extent == dynamic_extent) ? dynamic_extent : (jsoncons_span_sizeof(T) * Extent) ) >(
        reinterpret_cast< std17::byte const * >( spn.data() ), spn.size_bytes() );  // NOLINT
#endif
}

template< class T, extent_t Extent >
inline jsoncons_span_constexpr span< std17::byte, ( (Extent == dynamic_extent) ? dynamic_extent : (jsoncons_span_sizeof(T) * Extent) ) >
as_writeable_bytes( span<T,Extent> spn ) jsoncons_span_noexcept
{
#if 0
    return { reinterpret_cast< std17::byte * >( spn.data() ), spn.size_bytes() };
#else
    return span< std17::byte, ( (Extent == dynamic_extent) ? dynamic_extent : (jsoncons_span_sizeof(T) * Extent) ) >(
        reinterpret_cast< std17::byte * >( spn.data() ), spn.size_bytes() );  // NOLINT
#endif
}

#endif // jsoncons_span_HAVE( BYTE ) || jsoncons_span_HAVE( NONSTD_BYTE )

// extensions: non-member views:
// this feature implies the presence of make_span()

#if jsoncons_span_FEATURE( NON_MEMBER_FIRST_LAST_SUB ) && jsoncons_span_CPP11_120

template< extent_t Count, class T >
jsoncons_span_constexpr auto
first( T & t ) -> decltype( make_span(t).template first<Count>() )
{
    return make_span( t ).template first<Count>();
}

template< class T >
jsoncons_span_constexpr auto
first( T & t, index_t count ) -> decltype( make_span(t).first(count) )
{
    return make_span( t ).first( count );
}

template< extent_t Count, class T >
jsoncons_span_constexpr auto
last( T & t ) -> decltype( make_span(t).template last<Count>() )
{
    return make_span(t).template last<Count>();
}

template< class T >
jsoncons_span_constexpr auto
last( T & t, extent_t count ) -> decltype( make_span(t).last(count) )
{
    return make_span( t ).last( count );
}

template< index_t Offset, extent_t Count = dynamic_extent, class T >
jsoncons_span_constexpr auto
subspan( T & t ) -> decltype( make_span(t).template subspan<Offset, Count>() )
{
    return make_span( t ).template subspan<Offset, Count>();
}

template< class T >
jsoncons_span_constexpr auto
subspan( T & t, index_t offset, extent_t count = dynamic_extent ) -> decltype( make_span(t).subspan(offset, count) )
{
    return make_span( t ).subspan( offset, count );
}

#endif // jsoncons_span_FEATURE( NON_MEMBER_FIRST_LAST_SUB )

// 27.8 Container and view access [iterator.container]

template< class T, extent_t Extent /*= dynamic_extent*/ >
jsoncons_span_constexpr std::size_t size( span<T,Extent> const & spn )
{
    return static_cast<std::size_t>( spn.size() );
}

template< class T, extent_t Extent /*= dynamic_extent*/ >
jsoncons_span_constexpr std::ptrdiff_t ssize( span<T,Extent> const & spn )
{
    return static_cast<std::ptrdiff_t>( spn.size() );
}

}  // namespace jsoncons_span_lite
}  // namespace nonstd

// make available in nonstd:

namespace nonstd {

using jsoncons_span_lite::dynamic_extent;

using jsoncons_span_lite::span;

using jsoncons_span_lite::with_container;

#if jsoncons_span_FEATURE( COMPARISON )
#if jsoncons_span_FEATURE( SAME )
using jsoncons_span_lite::same;
#endif

using jsoncons_span_lite::operator==;
using jsoncons_span_lite::operator!=;
using jsoncons_span_lite::operator<;
using jsoncons_span_lite::operator<=;
using jsoncons_span_lite::operator>;
using jsoncons_span_lite::operator>=;
#endif

#if jsoncons_span_HAVE( BYTE )
using jsoncons_span_lite::as_bytes;
using jsoncons_span_lite::as_writeable_bytes;
#endif

using jsoncons_span_lite::size;
using jsoncons_span_lite::ssize;

}  // namespace nonstd

#endif  // jsoncons_span_USES_STD_SPAN

// make_span() [span-lite extension]:

#if jsoncons_span_FEATURE( MAKE_SPAN ) || jsoncons_span_FEATURE( NON_MEMBER_FIRST_LAST_SUB )

namespace nonstd {
namespace jsoncons_span_lite {

template< class T >
inline jsoncons_span_constexpr span<T>
make_span( T * ptr, index_t count ) jsoncons_span_noexcept
{
    return span<T>( ptr, count );
}

template< class T >
inline jsoncons_span_constexpr span<T>
make_span( T * first, T * last ) jsoncons_span_noexcept
{
    return span<T>( first, last );
}

template< class T, size_t N >
inline jsoncons_span_constexpr span<T, static_cast<extent_t>(N)>
make_span( T ( &arr )[ N ] ) jsoncons_span_noexcept
{
    return span<T, static_cast<extent_t>(N)>( &arr[ 0 ], N );
}

#if jsoncons_span_USES_STD_SPAN || jsoncons_span_HAVE( ARRAY )

template< class T, size_t N >
inline jsoncons_span_constexpr span<T, static_cast<extent_t>(N)>
make_span( std::array< T, N > & arr ) jsoncons_span_noexcept
{
    return span<T, static_cast<extent_t>(N)>( arr );
}

template< class T, size_t N >
inline jsoncons_span_constexpr span< const T, static_cast<extent_t>(N) >
make_span( std::array< T, N > const & arr ) jsoncons_span_noexcept
{
    return span<const T, static_cast<extent_t>(N)>( arr );
}

#endif // jsoncons_span_HAVE( ARRAY )

#if jsoncons_span_USES_STD_SPAN || ( jsoncons_span_HAVE( CONSTRAINED_SPAN_CONTAINER_CTOR ) && jsoncons_span_HAVE( AUTO ) )

template< class Container, class EP = decltype( std17::data(std::declval<Container&>())) >
inline jsoncons_span_constexpr auto
make_span( Container & cont ) jsoncons_span_noexcept -> span< typename std::remove_pointer<EP>::type >
{
    return span< typename std::remove_pointer<EP>::type >( cont );
}

template< class Container, class EP = decltype( std17::data(std::declval<Container&>())) >
inline jsoncons_span_constexpr auto
make_span( Container const & cont ) jsoncons_span_noexcept -> span< const typename std::remove_pointer<EP>::type >
{
    return span< const typename std::remove_pointer<EP>::type >( cont );
}

#else

template< class T, class Allocator >
inline jsoncons_span_constexpr span<T>
make_span( std::vector<T, Allocator> & cont ) jsoncons_span_noexcept
{
    return span<T>( with_container, cont );
}

template< class T, class Allocator >
inline jsoncons_span_constexpr span<const T>
make_span( std::vector<T, Allocator> const & cont ) jsoncons_span_noexcept
{
    return span<const T>( with_container, cont );
}

#endif // jsoncons_span_USES_STD_SPAN || ( ... )

#if ! jsoncons_span_USES_STD_SPAN && jsoncons_span_FEATURE( WITH_CONTAINER )

template< class Container >
inline jsoncons_span_constexpr span<typename Container::value_type>
make_span( with_container_t, Container & cont ) jsoncons_span_noexcept
{
    return span< typename Container::value_type >( with_container, cont );
}

template< class Container >
inline jsoncons_span_constexpr span<const typename Container::value_type>
make_span( with_container_t, Container const & cont ) jsoncons_span_noexcept
{
    return span< const typename Container::value_type >( with_container, cont );
}

#endif // ! jsoncons_span_USES_STD_SPAN && jsoncons_span_FEATURE( WITH_CONTAINER )


}  // namespace jsoncons_span_lite
}  // namespace nonstd

// make available in nonstd:

namespace nonstd {
using jsoncons_span_lite::make_span;
}  // namespace nonstd

#endif // #if jsoncons_span_FEATURE_TO_STD( MAKE_SPAN )

#if jsoncons_span_CPP11_OR_GREATER && jsoncons_span_FEATURE( BYTE_SPAN ) && ( jsoncons_span_HAVE( BYTE ) || jsoncons_span_HAVE( NONSTD_BYTE ) )

namespace nonstd {
namespace jsoncons_span_lite {

template< class T >
inline jsoncons_span_constexpr auto
byte_span( T & t ) jsoncons_span_noexcept -> span< std17::byte, jsoncons_span_sizeof(T) >
{
    return span< std17::byte, jsoncons_span_sizeof(t) >( reinterpret_cast< std17::byte * >( &t ), jsoncons_span_sizeof(T) );
}

template< class T >
inline jsoncons_span_constexpr auto
byte_span( T const & t ) jsoncons_span_noexcept -> span< const std17::byte, jsoncons_span_sizeof(T) >
{
    return span< const std17::byte, jsoncons_span_sizeof(t) >( reinterpret_cast< std17::byte const * >( &t ), jsoncons_span_sizeof(T) );
}

}  // namespace jsoncons_span_lite
}  // namespace nonstd

// make available in nonstd:

namespace nonstd {
using jsoncons_span_lite::byte_span;
}  // namespace nonstd

#endif // jsoncons_span_FEATURE( BYTE_SPAN )

#if ! jsoncons_span_USES_STD_SPAN
jsoncons_span_RESTORE_WARNINGS()
#endif  // jsoncons_span_USES_STD_SPAN

#endif  // NONSTD_SPAN_HPP_INCLUDED

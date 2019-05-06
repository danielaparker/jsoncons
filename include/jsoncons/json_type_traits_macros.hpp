// Copyright 2019 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TYPE_TRAITS_MACROS_HPP
#define JSONCONS_JSON_TYPE_TRAITS_MACROS_HPP

#include <array>
#include <string>
#include <vector>
#include <valarray>
#include <exception>
#include <cstring>
#include <utility>
#include <algorithm> // std::swap
#include <limits> // std::numeric_limits
#include <type_traits> // std::enable_if
#include <iterator> // std::iterator_traits, std::input_iterator_tag
#include <jsoncons/bignum.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/detail/type_traits_helper.hpp>
#include <string>
#include <tuple>
#include <map>
#include <functional>
#include <memory>

// This follows https://github.com/Loki-Astari/ThorsSerializer/blob/master/src/Serialize/Traits.h

#define JSONCONS_EXPAND(X) X    

#define JSONCONS_NARGS(...) JSONCONS_EXPAND( JSONCONS_NARG_(__VA_ARGS__, JSONCONS_RSEQ_N()) )
#define JSONCONS_NARG_(...) JSONCONS_EXPAND( JSONCONS_ARG_N(__VA_ARGS__) )
#define JSONCONS_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, N, ...) N
#define JSONCONS_RSEQ_N() 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define JSONCONS_QUOTE(A) JSONCONS_EXPAND(#A)

#define JSONCONS_GENERATE(Act, TC, JV, P1, V, P2)         JSONCONS_EXPAND(JSONCONS_EXPAND(Act(TC, JV, P1, V, P2)))

#define JSONCONS_REP_N(Act, TC, JV, P1, V, ...)            JSONCONS_EXPAND(JSONCONS_REP_OF_N(Act, TC, JV, P1, V, JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__))
#define JSONCONS_REP_OF_N(Act, TC, JV, P1, V, Count, ...)  JSONCONS_EXPAND(JSONCONS_REP_OF_N_(Act, TC, JV, P1, V, Count, __VA_ARGS__))
#define JSONCONS_REP_OF_N_(Act, TC, JV, P1, V, Count, ...) JSONCONS_EXPAND(JSONCONS_REP_OF_ ## Count(Act, TC, JV, P1, V, __VA_ARGS__))

#define JSONCONS_REP_OF_50(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_49(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_49(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_48(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_48(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_47(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_47(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_46(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_46(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_45(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_45(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_44(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_44(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_43(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_43(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_42(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_42(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_41(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_41(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_40(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_40(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_39(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_39(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_38(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_38(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_37(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_37(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_36(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_36(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_35(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_35(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_34(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_34(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_33(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_33(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_32(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_32(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_31(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_31(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_30(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_30(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_29(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_29(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_28(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_28(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_27(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_27(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_26(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_26(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_25(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_25(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_24(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_24(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_23(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_23(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_22(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_22(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_21(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_21(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_20(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_20(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_19(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_19(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_18(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_18(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_17(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_17(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_16(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_16(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_15(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_15(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_14(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_14(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_13(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_13(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_12(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_12(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_11(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_11(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_10(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_10(Act, TC, JV, P1, V, P2, ...)    JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_9(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_9(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_8(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_8(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_7(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_7(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_6(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_6(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_5(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_5(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_4(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_4(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_3(Act, TC, JV, P1, V, __VA_ARGS__))
#define JSONCONS_REP_OF_3(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V ,P2) JSONCONS_EXPAND(JSONCONS_REP_OF_2(Act, TC, JV, P1, V, __VA_ARGS__)) 
#define JSONCONS_REP_OF_2(Act, TC, JV, P1, V, P2, ...)     JSONCONS_GENERATE(Act, TC, JV, P1, V, P2) JSONCONS_EXPAND(JSONCONS_REP_OF_1(Act, TC, JV, P1, V, __VA_ARGS__)) 
#define JSONCONS_REP_OF_1(Act, TC, JV, P1, V, P2)          Act ## _LAST(TC, JV, P1, V, P2)

#define JSONCONS_IS(TC, JV, Type, V, Member) if (!(JV).contains(JSONCONS_QUOTE(Member))) return false;
#define JSONCONS_IS_LAST(TC, JV, Type, V, Member) if (!(JV).contains(JSONCONS_QUOTE(Member))) return false;

#define JSONCONS_TO_JSON(TC, JV, Type, V, Member) (JV).try_emplace(JSONCONS_QUOTE(Member), V.Member);
#define JSONCONS_TO_JSON_LAST(TC, JV, Type, V, Member) (JV).try_emplace(JSONCONS_QUOTE(Member), V.Member);

#define JSONCONS_AS(TC, JV, Type, V, Member) if ((JV).contains(JSONCONS_QUOTE(Member))) {val.Member = (JV).at(JSONCONS_QUOTE(Member)).template as<decltype(V.Member)>();}
#define JSONCONS_AS_LAST(TC, JV, Type, V, Member) if ((JV).contains(JSONCONS_QUOTE(Member))) {val.Member = (JV).at(JSONCONS_QUOTE(Member)).template as<decltype(V.Member)>();}

#define JSONCONS_MEMBER_TRAITS_DECL_BASE(Count, ValueType, ...)  \
namespace jsoncons \
{ \
    template<class Json> \
    struct json_type_traits<Json, ValueType> \
    { \
        typedef typename Json::allocator_type allocator_type; \
        static bool is(const Json& j) noexcept \
        { \
            if (!j.is_object()) return false; \
            JSONCONS_REP_N(JSONCONS_IS, Count, j, ValueType, void(), __VA_ARGS__)\
            return true; \
        } \
        static ValueType as(const Json& j) \
        { \
            ValueType val{}; \
            JSONCONS_REP_N(JSONCONS_AS, Count, j, ValueType, val, __VA_ARGS__) \
            return val; \
        } \
        static Json to_json(const ValueType& val, allocator_type allocator=allocator_type()) \
        { \
            Json j(allocator); \
            JSONCONS_REP_N(JSONCONS_TO_JSON, Count, j, ValueType, val, __VA_ARGS__) \
            return j; \
        } \
    }; \
} \
  /**/
 
#define JSONCONS_MEMBER_TRAITS_DECL(ValueType,...) \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(0, ValueType, __VA_ARGS__)

#define JSONCONS_IS2(TC, JV, Type, V, Member) if (!(JV).contains(JSONCONS_QUOTE(Member))) return false;
#define JSONCONS_IS2_LAST(TC, JV, Type, V, Member) if (!(JV).contains(JSONCONS_QUOTE(Member))) return false;

#define JSONCONS_TO_JSON2(TC, JV, Type, V, Member) (JV).try_emplace(JSONCONS_QUOTE(Member), V.Member() );
#define JSONCONS_TO_JSON2_LAST(TC, JV, Type, V, Member) (JV).try_emplace(JSONCONS_QUOTE(Member), V.Member() );

#define JSONCONS_AS2(TC, JV, Type, V, Member) (JV).at(JSONCONS_QUOTE(Member)).template as<typename std::decay<decltype(std::declval<Type>().Member())>::type>(),
#define JSONCONS_AS2_LAST(TC, JV, Type, V, Member) (JV).at(JSONCONS_QUOTE(Member)).template as<typename std::decay<decltype(std::declval<Type>().Member())>::type>()
 
#define JSONCONS_GETTER_CTOR_TRAITS_DECL_BASE(Count, ValueType, ...)  \
namespace jsoncons \
{ \
    template<class Json> \
    struct json_type_traits<Json, ValueType> \
    { \
        typedef ValueType value_type; \
        typedef typename Json::allocator_type allocator_type; \
        static bool is(const Json& j) noexcept \
        { \
            if (!j.is_object()) return false; \
            JSONCONS_REP_N(JSONCONS_IS2, Count, j, ValueType, void(), __VA_ARGS__)\
            return true; \
        } \
        static ValueType as(const Json& j) \
        { \
            ValueType val{ \
            JSONCONS_REP_N(JSONCONS_AS2, Count, j, ValueType, void(), __VA_ARGS__) \
            }; \
            return val; \
        } \
        static Json to_json(const ValueType& val, allocator_type allocator=allocator_type()) \
        { \
            Json j(allocator); \
            JSONCONS_REP_N(JSONCONS_TO_JSON2, Count, j, ValueType, val, __VA_ARGS__) \
            return j; \
        } \
    }; \
} \
  /**/

#define JSONCONS_GETTER_CTOR_TRAITS_DECL(ValueType,...) \
    JSONCONS_GETTER_CTOR_TRAITS_DECL_BASE(0, ValueType, __VA_ARGS__)

#define JSONCONS_TYPE_TRAITS_FRIEND \
    template <class JSON,class T,class Enable> \
    friend struct jsoncons::json_type_traits

#if !defined(JSONCONS_NO_DEPRECATED)
#define JSONCONS_TYPE_TRAITS_DECL JSONCONS_MEMBER_TRAITS_DECL
#endif

#endif

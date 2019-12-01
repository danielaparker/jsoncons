// Copyright 2019 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TYPE_TRAITS_MACROS_HPP
#define JSONCONS_JSON_TYPE_TRAITS_MACROS_HPP

#include <algorithm> // std::swap
#include <array>
#include <cstring>
#include <exception>
#include <functional>
#include <iterator> // std::iterator_traits, std::input_iterator_tag
#include <jsoncons/bignum.hpp>
#include <jsoncons/config/jsoncons_config.hpp> // JSONCONS_EXPAND, JSONCONS_QUOTE
#include <jsoncons/detail/type_traits.hpp>
#include <jsoncons/json_content_handler.hpp>
#include <limits> // std::numeric_limits
#include <memory>
#include <string>
#include <type_traits> // std::enable_if
#include <unordered_map> // std::unordered_map
#include <utility>
#include <valarray>
#include <jsoncons/json_type_traits.hpp>

#if defined(_MSC_VER)
#pragma warning( disable : 4127)
#endif

// Inspired by https://github.com/Loki-Astari/ThorsSerializer/blob/master/src/Serialize/Traits.h

#define JSONCONS_NARGS(...) JSONCONS_NARG_(__VA_ARGS__, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define JSONCONS_NARG_(...) JSONCONS_EXPAND( JSONCONS_ARG_N(__VA_ARGS__) )
#define JSONCONS_ARG_N(e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16, e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31, e32, e33, e34, e35, e36, e37, e38, e39, e40, e41, e42, e43, e44, e45, e46, e47, e48, e49, e50, N, ...) N

#define JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, Count) Call(P1, P2, P3, P4, Count) 
 
#define JSONCONS_VARIADIC_REP_N(Call, P1, P2, P3, ...)            JSONCONS_VARIADIC_REP_OF_N(Call, P1,P2, P3, JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__)
#define JSONCONS_VARIADIC_REP_OF_N(Call, P1, P2, P3, Count, ...)  JSONCONS_VARIADIC_REP_OF_N_(Call, P1, P2, P3, Count, __VA_ARGS__)
#define JSONCONS_VARIADIC_REP_OF_N_(Call, P1, P2, P3, Count, ...) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_ ## Count(Call, P1, P2, P3, __VA_ARGS__))

#define JSONCONS_VARIADIC_REP_OF_50(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 50) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_49(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_49(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 49) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_48(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_48(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 48) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_47(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_47(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 47, ) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_46(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_46(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 46) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_45(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_45(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 45) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_44(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_44(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 44) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_43(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_43(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 43) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_42(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_42(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 42) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_41(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_41(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 41) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_40(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_40(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 40) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_39(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_39(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 39) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_38(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_38(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 38) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_37(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_37(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 37) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_36(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_36(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 36) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_35(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_35(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 35) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_34(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_34(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 34) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_33(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_33(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 33) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_32(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_32(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 32) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_31(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_31(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 31) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_30(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_30(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 30) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_29(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_29(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 29) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_28(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_28(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 28) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_27(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_27(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 27) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_26(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_26(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 26) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_25(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_25(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 25) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_24(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_24(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 24) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_23(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_23(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 23) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_22(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_22(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 22) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_21(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_21(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 21) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_20(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_20(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 20) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_19(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_19(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 19) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_18(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_18(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 18) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_17(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_17(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 17) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_16(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_16(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 16) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_15(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_15(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 15) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_14(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_14(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 14) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_13(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_13(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 13) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_12(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_12(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 12) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_11(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_11(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 11) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_10(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_10(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 10) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_9(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_9(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 9) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_8(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_8(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 8) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_7(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_7(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 7) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_6(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_6(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 6) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_5(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_5(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 5) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_4(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_4(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 4) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_3(Call, P1, P2, P3, __VA_ARGS__))
#define JSONCONS_VARIADIC_REP_OF_3(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 3) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_2(Call, P1, P2, P3, __VA_ARGS__)) 
#define JSONCONS_VARIADIC_REP_OF_2(Call, P1, P2, P3, P4, ...)    JSONCONS_EXPAND_CALL5(Call, P1, P2, P3, P4, 2) JSONCONS_EXPAND(JSONCONS_VARIADIC_REP_OF_1(Call, P1, P2, P3, __VA_ARGS__)) 
#define JSONCONS_VARIADIC_REP_OF_1(Call, P1, P2, P3, P4)         JSONCONS_EXPAND(Call ## _LAST(P1, P2, P3, P4, 1))

#define JSONCONS_TYPE_TRAITS_FRIEND \
    template <class JSON,class T,class Enable> \
    friend struct jsoncons::json_type_traits;

#define JSONCONS_EXPAND_CALL2(Call, Expr, Id) JSONCONS_EXPAND(Call(Expr, Id))

#define JSONCONS_REP_OF_N(Call, Expr, Pre, App, Count)  JSONCONS_REP_OF_ ## Count(Call, Expr, Pre, App)

#define JSONCONS_REP_OF_50(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 50) JSONCONS_REP_OF_49(Call, Expr, , App)
#define JSONCONS_REP_OF_49(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 49) JSONCONS_REP_OF_48(Call, Expr, , App)
#define JSONCONS_REP_OF_48(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 48) JSONCONS_REP_OF_47(Call, Expr, , App)
#define JSONCONS_REP_OF_47(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 47) JSONCONS_REP_OF_46(Call, Expr, , App)
#define JSONCONS_REP_OF_46(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 46) JSONCONS_REP_OF_45(Call, Expr, , App)
#define JSONCONS_REP_OF_45(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 45) JSONCONS_REP_OF_44(Call, Expr, , App)
#define JSONCONS_REP_OF_44(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 44) JSONCONS_REP_OF_43(Call, Expr, , App)
#define JSONCONS_REP_OF_43(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 43) JSONCONS_REP_OF_42(Call, Expr, , App)
#define JSONCONS_REP_OF_42(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 42) JSONCONS_REP_OF_41(Call, Expr, , App)
#define JSONCONS_REP_OF_41(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 41) JSONCONS_REP_OF_40(Call, Expr, , App)
#define JSONCONS_REP_OF_40(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 40) JSONCONS_REP_OF_39(Call, Expr, , App)
#define JSONCONS_REP_OF_39(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 39) JSONCONS_REP_OF_38(Call, Expr, , App)
#define JSONCONS_REP_OF_38(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 38) JSONCONS_REP_OF_37(Call, Expr, , App)
#define JSONCONS_REP_OF_37(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 37) JSONCONS_REP_OF_36(Call, Expr, , App)
#define JSONCONS_REP_OF_36(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 36) JSONCONS_REP_OF_35(Call, Expr, , App)
#define JSONCONS_REP_OF_35(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 35) JSONCONS_REP_OF_34(Call, Expr, , App)
#define JSONCONS_REP_OF_34(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 34) JSONCONS_REP_OF_33(Call, Expr, , App)
#define JSONCONS_REP_OF_33(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 33) JSONCONS_REP_OF_32(Call, Expr, , App)
#define JSONCONS_REP_OF_32(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 32) JSONCONS_REP_OF_31(Call, Expr, , App)
#define JSONCONS_REP_OF_31(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 31) JSONCONS_REP_OF_30(Call, Expr, , App)
#define JSONCONS_REP_OF_30(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 30) JSONCONS_REP_OF_29(Call, Expr, , App)
#define JSONCONS_REP_OF_29(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 29) JSONCONS_REP_OF_28(Call, Expr, , App)
#define JSONCONS_REP_OF_28(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 28) JSONCONS_REP_OF_27(Call, Expr, , App)
#define JSONCONS_REP_OF_27(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 27) JSONCONS_REP_OF_26(Call, Expr, , App)
#define JSONCONS_REP_OF_26(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 26) JSONCONS_REP_OF_25(Call, Expr, , App)
#define JSONCONS_REP_OF_25(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 25) JSONCONS_REP_OF_24(Call, Expr, , App)
#define JSONCONS_REP_OF_24(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 24) JSONCONS_REP_OF_23(Call, Expr, , App)
#define JSONCONS_REP_OF_23(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 23) JSONCONS_REP_OF_22(Call, Expr, , App)
#define JSONCONS_REP_OF_22(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 22) JSONCONS_REP_OF_21(Call, Expr, , App)
#define JSONCONS_REP_OF_21(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 21) JSONCONS_REP_OF_20(Call, Expr, , App)
#define JSONCONS_REP_OF_20(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 20) JSONCONS_REP_OF_19(Call, Expr, , App)
#define JSONCONS_REP_OF_19(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 19) JSONCONS_REP_OF_18(Call, Expr, , App)
#define JSONCONS_REP_OF_18(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 18) JSONCONS_REP_OF_17(Call, Expr, , App)
#define JSONCONS_REP_OF_17(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 17) JSONCONS_REP_OF_16(Call, Expr, , App)
#define JSONCONS_REP_OF_16(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 16) JSONCONS_REP_OF_15(Call, Expr, , App)
#define JSONCONS_REP_OF_15(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 15) JSONCONS_REP_OF_14(Call, Expr, , App)
#define JSONCONS_REP_OF_14(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 14) JSONCONS_REP_OF_13(Call, Expr, , App)
#define JSONCONS_REP_OF_13(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 13) JSONCONS_REP_OF_12(Call, Expr, , App)
#define JSONCONS_REP_OF_12(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 12) JSONCONS_REP_OF_11(Call, Expr, , App)
#define JSONCONS_REP_OF_11(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 11) JSONCONS_REP_OF_10(Call, Expr, , App)
#define JSONCONS_REP_OF_10(Call, Expr, Pre, App)     Pre JSONCONS_EXPAND_CALL2(Call, Expr, 10) JSONCONS_REP_OF_9(Call, Expr, , App)
#define JSONCONS_REP_OF_9(Call, Expr, Pre, App)      Pre JSONCONS_EXPAND_CALL2(Call, Expr, 9) JSONCONS_REP_OF_8(Call, Expr, , App)
#define JSONCONS_REP_OF_8(Call, Expr, Pre, App)      Pre JSONCONS_EXPAND_CALL2(Call, Expr, 8) JSONCONS_REP_OF_7(Call, Expr, , App)
#define JSONCONS_REP_OF_7(Call, Expr, Pre, App)      Pre JSONCONS_EXPAND_CALL2(Call, Expr, 7) JSONCONS_REP_OF_6(Call, Expr, , App)
#define JSONCONS_REP_OF_6(Call, Expr, Pre, App)      Pre JSONCONS_EXPAND_CALL2(Call, Expr, 6) JSONCONS_REP_OF_5(Call, Expr, , App)
#define JSONCONS_REP_OF_5(Call, Expr, Pre, App)      Pre JSONCONS_EXPAND_CALL2(Call, Expr, 5) JSONCONS_REP_OF_4(Call, Expr, , App)
#define JSONCONS_REP_OF_4(Call, Expr, Pre, App)      Pre JSONCONS_EXPAND_CALL2(Call, Expr, 4) JSONCONS_REP_OF_3(Call, Expr, , App)
#define JSONCONS_REP_OF_3(Call, Expr, Pre, App)      Pre JSONCONS_EXPAND_CALL2(Call, Expr, 3) JSONCONS_REP_OF_2(Call, Expr, , App)
#define JSONCONS_REP_OF_2(Call, Expr, Pre, App)      Pre JSONCONS_EXPAND_CALL2(Call, Expr, 2) JSONCONS_REP_OF_1(Call, Expr, , App)
#define JSONCONS_REP_OF_1(Call, Expr, Pre, App)      Pre JSONCONS_EXPAND_CALL2(Call ## _LAST, Expr, 1) App
#define JSONCONS_REP_OF_0(Call, Expr, Pre, App)

#define JSONCONS_GENERATE_TPL_PARAMS(Call, Count) JSONCONS_REP_OF_N(Call, , , ,Count)
#define JSONCONS_GENERATE_TPL_ARGS(Call, Count) JSONCONS_REP_OF_N(Call, ,<,>,Count)
#define JSONCONS_GENERATE_TPL_PARAM(Expr, Id) , typename T ## Id
#define JSONCONS_GENERATE_TPL_PARAM_LAST(Expr, Id) , typename T ## Id
#define JSONCONS_GENERATE_TPL_ARG(Expr, Id) T ## Id,
#define JSONCONS_GENERATE_TPL_ARG_LAST(Ex, Id) T ## Id 

#define JSONCONS_IS(Prefix, P2, P3, Member, Count) JSONCONS_IS_LAST(Prefix, P2, P3, Member, Count)
#define JSONCONS_IS_LAST(Prefix, P2, P3, Member, Count) if ((num_params-Count) < num_mandatory_params1 && !ajson.contains(JSONCONS_QUOTE(Prefix, Member))) return false;

#define JSONCONS_AS(Prefix,P2,P3, Member, Count) JSONCONS_AS_LAST(Prefix,P2,P3, Member, Count)
#define JSONCONS_AS_LAST(Prefix,P2,P3, Member, Count) if ((num_params-Count) < num_mandatory_params2 || ajson.contains(JSONCONS_QUOTE(Prefix, Member))) {set_member(std::is_const<decltype(aval.Member)>(),ajson,JSONCONS_QUOTE(Prefix, Member),aval.Member);}

#define JSONCONS_ALL_AS(Prefix, P2,P3,Member, Count) JSONCONS_ALL_AS_LAST(Prefix,P2,P3, Member, Count)
#define JSONCONS_ALL_AS_LAST(Prefix,P2,P3, Member, Count) set_member(std::is_const<decltype(aval.Member)>(),ajson,JSONCONS_QUOTE(Prefix, Member),aval.Member);

#define JSONCONS_TO_JSON(Prefix, P2, P3, Member, Count) JSONCONS_TO_JSON_LAST(Prefix, P2, P3, Member, Count)
#define JSONCONS_TO_JSON_LAST(Prefix, P2, P3, Member, Count) ajson.try_emplace(JSONCONS_QUOTE(Prefix, Member), aval.Member);

#define JSONCONS_MEMBER_TRAITS_DECL_BASE(As,CharT,Prefix,NumTemplateParams,ValueType,NumMandatoryParams1,NumMandatoryParams2, ...)  \
namespace jsoncons \
{ \
    template<typename Json JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> \
    struct json_type_traits<Json, ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams), typename std::enable_if<std::is_same<typename Json::char_type,CharT>::value>::type> \
    { \
        typedef ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams) value_type; \
        typedef typename Json::allocator_type allocator_type; \
        typedef typename Json::string_view_type string_view_type; \
        constexpr static size_t num_params = JSONCONS_NARGS(__VA_ARGS__); \
        constexpr static size_t num_mandatory_params1 = NumMandatoryParams1; \
        constexpr static size_t num_mandatory_params2 = NumMandatoryParams2; \
        static bool is(const Json& ajson) noexcept \
        { \
            if (!ajson.is_object()) return false; \
            JSONCONS_VARIADIC_REP_N(JSONCONS_IS, Prefix,,, __VA_ARGS__)\
            return true; \
        } \
        static value_type as(const Json& ajson) \
        { \
            value_type aval{}; \
            JSONCONS_VARIADIC_REP_N(As, Prefix,,, __VA_ARGS__) \
            return aval; \
        } \
        static Json to_json(const value_type& aval, allocator_type alloc=allocator_type()) \
        { \
            Json ajson(json_object_arg, semantic_tag::none, alloc); \
            JSONCONS_VARIADIC_REP_N(JSONCONS_TO_JSON, Prefix,,, __VA_ARGS__) \
            return ajson; \
        } \
    private: \
        template <class U> \
        static void set_member(std::true_type, const Json&, const string_view_type&, U&) \
        { \
        } \
        template <class U> \
        static void set_member(std::false_type, const Json& j, const string_view_type& name, U& val) \
        { \
            val = j.at(name).template as<U>(); \
        } \
    }; \
} \
  /**/

#define JSONCONS_N_MEMBER_TRAITS_DECL(ValueType,NumMandatoryParams,...)  \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_AS, char,,0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_AS, wchar_t,L,0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/

#define JSONCONS_TPL_N_MEMBER_TRAITS_DECL(NumTemplateParams, ValueType,NumMandatoryParams, ...)  \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_AS, char,,NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_AS, wchar_t,L,NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/

#define JSONCONS_TPL_N_MEMBER_TRAITS_DECL(NumTemplateParams, ValueType,NumMandatoryParams, ...)  \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_AS, char,,NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_AS, wchar_t,L,NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/

#define JSONCONS_ALL_MEMBER_TRAITS_DECL(ValueType, ...)  \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_ALL_AS,char,,0,ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_ALL_AS,wchar_t,L,0,ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
  /**/

#define JSONCONS_TPL_ALL_MEMBER_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_ALL_AS,char,,NumTemplateParams,ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_ALL_AS,wchar_t,L,NumTemplateParams,ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
  /**/

#if !defined(JSONCONS_NO_DEPRECATED)

#define JSONCONS_MEMBER_TRAITS_DECL(ValueType, ...)  \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_AS, char,,0, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_AS, wchar_t,L,0, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
  /**/

#define JSONCONS_TPL_MEMBER_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_AS, char,,NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_DECL_BASE(JSONCONS_AS, wchar_t,L,NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
  /**/

#endif

#define JSONCONS_NAMED_IS(P1, P2, P3, Seq, Count) JSONCONS_NAMED_IS_LAST(P1, P2, P3, Seq, Count)
#define JSONCONS_NAMED_IS_LAST(P1, P2, P3, Seq, Count) if ((num_params-Count) < num_mandatory_params1 && JSONCONS_EXPAND(JSONCONS_NAMED_IS_ Seq) 
#define JSONCONS_NAMED_IS_(Member, Name) !ajson.contains(Name)) return false;

#define JSONCONS_NAMED_AS(P1, P2, P3, Seq, Count) JSONCONS_NAMED_AS_LAST(P1, P2, P3, Seq, Count)
#define JSONCONS_NAMED_AS_LAST(P1, P2, P3, Seq, Count)  if ((num_params-Count) < num_mandatory_params2 || JSONCONS_EXPAND(JSONCONS_NAMED_AS_ Seq)
#define JSONCONS_NAMED_AS_(Member, Name) ajson.contains(Name)) {set_member(std::is_const<decltype(aval.Member)>(),ajson,Name,aval.Member);}

#define JSONCONS_ALL_NAMED_AS(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(JSONCONS_ALL_NAMED_AS_ Seq)
#define JSONCONS_ALL_NAMED_AS_LAST(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(JSONCONS_ALL_NAMED_AS_ Seq)
#define JSONCONS_ALL_NAMED_AS_(Member, Name) set_member(std::is_const<decltype(aval.Member)>(),ajson,Name,aval.Member);

#define JSONCONS_NAMED_TO_JSON(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(JSONCONS_NAMED_TO_ Seq)
#define JSONCONS_NAMED_TO_JSON_LAST(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(JSONCONS_NAMED_TO_ Seq)
#define JSONCONS_NAMED_TO_(Member, Name) ajson.try_emplace(Name, aval.Member);

#define JSONCONS_MEMBER_NAMED_TRAITS_DECL_BASE(As, NumTemplateParams, ValueType,NumMandatoryParams1,NumMandatoryParams2, ...)  \
namespace jsoncons \
{ \
    template<typename Json JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> \
    struct json_type_traits<Json, ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> \
    { \
        typedef ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams) value_type; \
        typedef typename Json::allocator_type allocator_type; \
        typedef typename Json::string_view_type string_view_type; \
        constexpr static size_t num_params = JSONCONS_NARGS(__VA_ARGS__); \
        constexpr static size_t num_mandatory_params1 = NumMandatoryParams1; \
        constexpr static size_t num_mandatory_params2 = NumMandatoryParams2; \
        static bool is(const Json& ajson) noexcept \
        { \
            if (!ajson.is_object()) return false; \
            JSONCONS_VARIADIC_REP_N(JSONCONS_NAMED_IS,,,, __VA_ARGS__)\
            return true; \
        } \
        static value_type as(const Json& ajson) \
        { \
            value_type aval{}; \
            JSONCONS_VARIADIC_REP_N(As,,,, __VA_ARGS__) \
            return aval; \
        } \
        static Json to_json(const value_type& aval, allocator_type alloc=allocator_type()) \
        { \
            Json ajson(json_object_arg, semantic_tag::none, alloc); \
            JSONCONS_VARIADIC_REP_N(JSONCONS_NAMED_TO_JSON,,,, __VA_ARGS__) \
            return ajson; \
        } \
    private: \
        template <class U> \
        static void set_member(std::true_type, const Json&, const string_view_type&, U&) \
        { \
        } \
        template <class U> \
        static void set_member(std::false_type, const Json& j, const string_view_type& name, U& val) \
        { \
            val = j.at(name).template as<U>(); \
        } \
    }; \
} \
  /**/


#define JSONCONS_N_MEMBER_NAMED_TRAITS_DECL(ValueType,NumMandatoryParams, ...)  \
    JSONCONS_MEMBER_NAMED_TRAITS_DECL_BASE(JSONCONS_NAMED_AS, 0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/

#define JSONCONS_TPL_N_MEMBER_NAMED_TRAITS_DECL(NumTemplateParams, ValueType,NumMandatoryParams, ...)  \
    JSONCONS_MEMBER_NAMED_TRAITS_DECL_BASE(JSONCONS_NAMED_AS, NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/

#define JSONCONS_ALL_MEMBER_NAMED_TRAITS_DECL(ValueType, ...)  \
    JSONCONS_MEMBER_NAMED_TRAITS_DECL_BASE(JSONCONS_ALL_NAMED_AS, 0, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
  /**/

#define JSONCONS_TPL_ALL_MEMBER_NAMED_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
    JSONCONS_MEMBER_NAMED_TRAITS_DECL_BASE(JSONCONS_ALL_NAMED_AS, NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
  /**/

#if !defined(JSONCONS_NO_DEPRECATED)

#define JSONCONS_MEMBER_NAMED_TRAITS_DECL(ValueType, ...)  \
    JSONCONS_MEMBER_NAMED_TRAITS_DECL_BASE(JSONCONS_NAMED_AS, 0, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
  /**/

#define JSONCONS_TPL_MEMBER_NAMED_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
    JSONCONS_MEMBER_NAMED_TRAITS_DECL_BASE(JSONCONS_NAMED_AS, NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
  /**/
 
#endif 
 
#define JSONCONS_GETTER_CTOR_IS(Prefix, P2, P3, Member, Count) JSONCONS_GETTER_CTOR_IS_LAST(Prefix, P2, P3, Member, Count)
#define JSONCONS_GETTER_CTOR_IS_LAST(Prefix, P2, P3, Member, Count) if ((num_params-Count) < num_mandatory_params1 && !ajson.contains(JSONCONS_QUOTE(Prefix, Member))) return false;

#define JSONCONS_GETTER_CTOR_AS(Prefix, P2, P3, Member, Count) JSONCONS_GETTER_CTOR_AS_LAST(Prefix, P2, P3, Member, Count),
#define JSONCONS_GETTER_CTOR_AS_LAST(Prefix, P2, P3, Member, Count) ((num_params-Count) < num_mandatory_params2) ? (ajson.at(JSONCONS_QUOTE(Prefix, Member))).template as<typename std::decay<decltype(((value_type*)nullptr)->Member())>::type>() : (ajson.contains(JSONCONS_QUOTE(Prefix, Member)) ? (ajson.at(JSONCONS_QUOTE(Prefix, Member))).template as<typename std::decay<decltype(((value_type*)nullptr)->Member())>::type>() : typename std::decay<decltype(((value_type*)nullptr)->Member())>::type())

#define JSONCONS_GETTER_CTOR_TO_JSON(Prefix, P2, P3, Member, Count) JSONCONS_GETTER_CTOR_TO_JSON_LAST(Prefix, P2, P3, Member, Count)
#define JSONCONS_GETTER_CTOR_TO_JSON_LAST(Prefix, P2, P3, Member, Count) ajson.try_emplace(JSONCONS_QUOTE(Prefix, Member), aval.Member() );

#define JSONCONS_GETTER_CTOR_TRAITS_DECL_BASE(CharT,Prefix,NumTemplateParams, ValueType,NumMandatoryParams1,NumMandatoryParams2, ...)  \
namespace jsoncons \
{ \
    template<typename Json JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> \
    struct json_type_traits<Json, ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams), typename std::enable_if<std::is_same<typename Json::char_type,CharT>::value>::type> \
    { \
        typedef ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams) value_type; \
        typedef typename Json::allocator_type allocator_type; \
        constexpr static size_t num_params = JSONCONS_NARGS(__VA_ARGS__); \
        constexpr static size_t num_mandatory_params1 = NumMandatoryParams1; \
        constexpr static size_t num_mandatory_params2 = NumMandatoryParams2; \
        static bool is(const Json& ajson) noexcept \
        { \
            if (!ajson.is_object()) return false; \
            JSONCONS_VARIADIC_REP_N(JSONCONS_GETTER_CTOR_IS, Prefix,,, __VA_ARGS__)\
            return true; \
        } \
        static value_type as(const Json& ajson) \
        { \
            return value_type ( JSONCONS_VARIADIC_REP_N(JSONCONS_GETTER_CTOR_AS, Prefix,,, __VA_ARGS__) ); \
        } \
        static Json to_json(const value_type& aval, allocator_type alloc=allocator_type()) \
        { \
            Json ajson(json_object_arg, semantic_tag::none, alloc); \
            JSONCONS_VARIADIC_REP_N(JSONCONS_GETTER_CTOR_TO_JSON, Prefix,,, __VA_ARGS__) \
            return ajson; \
        } \
    }; \
} \
  /**/
 
#define JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ValueType, ...)  \
JSONCONS_GETTER_CTOR_TRAITS_DECL_BASE(char,,0, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
JSONCONS_GETTER_CTOR_TRAITS_DECL_BASE(wchar_t,L,0, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
  /**/
 
#define JSONCONS_TPL_ALL_GETTER_CTOR_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
JSONCONS_GETTER_CTOR_TRAITS_DECL_BASE(char,,NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
JSONCONS_GETTER_CTOR_TRAITS_DECL_BASE(wchar_t,L,NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
  /**/
 
#define JSONCONS_N_GETTER_CTOR_TRAITS_DECL(ValueType,NumMandatoryParams, ...)  \
JSONCONS_GETTER_CTOR_TRAITS_DECL_BASE(char,,0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
JSONCONS_GETTER_CTOR_TRAITS_DECL_BASE(wchar_t,L,0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/
 
#define JSONCONS_N_ALL_GETTER_CTOR_TRAITS_DECL(NumTemplateParams, ValueType,NumMandatoryParams, ...)  \
JSONCONS_GETTER_CTOR_TRAITS_DECL_BASE(char,,NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
JSONCONS_GETTER_CTOR_TRAITS_DECL_BASE(wchar_t,L,NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/
 
#define JSONCONS_GETTER_CTOR_NAMED_IS(P1, P2, P3, Seq, Count) JSONCONS_GETTER_CTOR_NAMED_IS_LAST(P1, P2, P3, Seq, Count)
#define JSONCONS_GETTER_CTOR_NAMED_IS_LAST(P1, P2, P3, Seq, Count) if ((num_params-Count) < num_mandatory_params1 && JSONCONS_EXPAND(JSONCONS_GETTER_CTOR_NAMED_IS_ Seq)
#define JSONCONS_GETTER_CTOR_NAMED_IS_(Member, Name) !ajson.contains(Name)) return false;

#define JSONCONS_GETTER_CTOR_NAMED_AS(P1, P2, P3, Seq, Count) JSONCONS_GETTER_CTOR_NAMED_AS_LAST(P1, P2, P3, Seq, Count),
#define JSONCONS_GETTER_CTOR_NAMED_AS_LAST(P1, P2, P3, Seq, Count) ((num_params-Count) < num_mandatory_params2) ? JSONCONS_EXPAND(JSONCONS_GETTER_CTOR_NAMED_AS_ Seq)
#define JSONCONS_GETTER_CTOR_NAMED_AS_(Member, Name) (ajson.at(Name)).template as<typename std::decay<decltype(((value_type*)nullptr)->Member())>::type>() : (ajson.contains(Name)) ? (ajson.at(Name)).template as<typename std::decay<decltype(((value_type*)nullptr)->Member())>::type>() : typename std::decay<decltype(((value_type*)nullptr)->Member())>::type()

#define JSONCONS_GETTER_CTOR_NAMED_TO_JSON(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(JSONCONS_GETTER_CTOR_NAMED_TO_JSON_ Seq)
#define JSONCONS_GETTER_CTOR_NAMED_TO_JSON_LAST(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(JSONCONS_GETTER_CTOR_NAMED_TO_JSON_ Seq)
#define JSONCONS_GETTER_CTOR_NAMED_TO_JSON_(Member, Name) ajson.try_emplace(Name, aval.Member() );
 
#define JSONCONS_GETTER_CTOR_NAMED_TRAITS_DECL_BASE(NumTemplateParams, ValueType,NumMandatoryParams1,NumMandatoryParams2, ...)  \
namespace jsoncons \
{ \
    template<typename Json JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> \
    struct json_type_traits<Json, ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> \
    { \
        typedef ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams) value_type; \
        typedef typename Json::allocator_type allocator_type; \
        constexpr static size_t num_params = JSONCONS_NARGS(__VA_ARGS__); \
        constexpr static size_t num_mandatory_params1 = NumMandatoryParams1; \
        constexpr static size_t num_mandatory_params2 = NumMandatoryParams2; \
        static bool is(const Json& ajson) noexcept \
        { \
            if (!ajson.is_object()) return false; \
            JSONCONS_VARIADIC_REP_N(JSONCONS_GETTER_CTOR_NAMED_IS,,,, __VA_ARGS__)\
            return true; \
        } \
        static value_type as(const Json& ajson) \
        { \
            return value_type ( JSONCONS_VARIADIC_REP_N(JSONCONS_GETTER_CTOR_NAMED_AS,,,, __VA_ARGS__) ); \
        } \
        static Json to_json(const value_type& aval, allocator_type alloc=allocator_type()) \
        { \
            Json ajson(json_object_arg, semantic_tag::none, alloc); \
            JSONCONS_VARIADIC_REP_N(JSONCONS_GETTER_CTOR_NAMED_TO_JSON,,,, __VA_ARGS__) \
            return ajson; \
        } \
    }; \
} \
  /**/
 
#define JSONCONS_ALL_GETTER_CTOR_NAMED_TRAITS_DECL(ValueType, ...)  \
JSONCONS_GETTER_CTOR_NAMED_TRAITS_DECL_BASE(0, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
  /**/
 
#define JSONCONS_TPL_ALL_GETTER_CTOR_NAMED_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
JSONCONS_GETTER_CTOR_NAMED_TRAITS_DECL_BASE(NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
  /**/
 
#define JSONCONS_N_GETTER_CTOR_NAMED_TRAITS_DECL(ValueType,NumMandatoryParams, ...)  \
JSONCONS_GETTER_CTOR_NAMED_TRAITS_DECL_BASE(0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/
 
#define JSONCONS_TPL_N_GETTER_CTOR_NAMED_TRAITS_DECL(NumTemplateParams, ValueType,NumMandatoryParams, ...)  \
JSONCONS_GETTER_CTOR_NAMED_TRAITS_DECL_BASE(NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/

#define JSONCONS_ENUM_PAIR(Prefix, P2, P3, Member, Count) {value_type::Member, JSONCONS_QUOTE(Prefix,Member)},
#define JSONCONS_ENUM_PAIR_LAST(Prefix, P2, P3, Member, Count) {value_type::Member, JSONCONS_QUOTE(Prefix,Member)}

#define JSONCONS_ENUM_TRAITS_DECL_BASE(CharT,Prefix,EnumType, ...)  \
namespace jsoncons \
{ \
    template<typename Json> \
    struct json_type_traits<Json, EnumType, typename std::enable_if<std::is_same<typename Json::char_type,CharT>::value>::type> \
    { \
        static_assert(std::is_enum<EnumType>::value, # EnumType " must be an enum"); \
        typedef EnumType value_type; \
        typedef std::basic_string<CharT> string_type; \
        typedef basic_string_view<CharT> string_view_type; \
        typedef typename Json::allocator_type allocator_type; \
        typedef std::pair<EnumType,string_type> mapped_type; \
        \
        static std::pair<const mapped_type*,const mapped_type*> get_values() \
        { \
            static const mapped_type v[] = { \
                JSONCONS_VARIADIC_REP_N(JSONCONS_ENUM_PAIR, Prefix,,, __VA_ARGS__)\
            };\
            return std::make_pair(v,v+JSONCONS_NARGS(__VA_ARGS__)); \
        } \
        \
        static bool is(const Json& ajson) noexcept \
        { \
            if (!ajson.is_string()) return false; \
            auto first = get_values().first; \
            auto last = get_values().second; \
            const string_view_type s = ajson.template as<string_view_type>(); \
            if (s.empty() && std::find_if(first, last, \
                                          [](const mapped_type& item) -> bool \
                                          { return item.first == value_type(); }) == last) \
            { \
                return true; \
            } \
            auto it = std::find_if(first, last, \
                                   [&](const mapped_type& item) -> bool \
                                   { return item.second == s; }); \
            return it != last; \
        } \
        static value_type as(const Json& ajson) \
        { \
            if (!ajson.is_string()) \
            { \
                JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not an enum")); \
            } \
            const string_view_type s = ajson.template as<string_view_type>(); \
            auto first = get_values().first; \
            auto last = get_values().second; \
            if (s.empty() && std::find_if(first, last, \
                                          [](const mapped_type& item) -> bool \
                                          { return item.first == value_type(); }) == last) \
            { \
                return value_type(); \
            } \
            auto it = std::find_if(first, last, \
                                   [&](const mapped_type& item) -> bool \
                                   { return item.second == s; }); \
            if (it == last) \
            { \
                if (s.empty()) \
                { \
                    return value_type(); \
                } \
                else \
                { \
                    JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not an enum")); \
                } \
            } \
            return it->first; \
        } \
        static Json to_json(value_type aval, allocator_type alloc=allocator_type()) \
        { \
            static constexpr CharT empty_string[] = {0}; \
            auto first = get_values().first; \
            auto last = get_values().second; \
            auto it = std::find_if(first, last, \
                                   [aval](const mapped_type& item) -> bool \
                                   { return item.first == aval; }); \
            if (it == last) \
            { \
                if (aval == value_type()) \
                { \
                    return Json(empty_string); \
                } \
                else \
                { \
                    JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not an enum")); \
                } \
            } \
            return Json(it->second,alloc); \
        } \
    }; \
} \
    /**/

#define JSONCONS_ENUM_TRAITS_DECL(EnumType, ...)  \
    JSONCONS_ENUM_TRAITS_DECL_BASE(char,,EnumType,__VA_ARGS__) \
    JSONCONS_ENUM_TRAITS_DECL_BASE(wchar_t,L,EnumType,__VA_ARGS__) \
    /**/

#define JSONCONS_NAMED_ENUM_PAIR(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(JSONCONS_NAMED_ENUM_PAIR_ Seq),
#define JSONCONS_NAMED_ENUM_PAIR_LAST(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(JSONCONS_NAMED_ENUM_PAIR_ Seq)
#define JSONCONS_NAMED_ENUM_PAIR_(Member, Name) {value_type::Member, Name}

#define JSONCONS_ENUM_NAMED_TRAITS_DECL(EnumType, ...)  \
namespace jsoncons \
{ \
    template<typename Json> \
    struct json_type_traits<Json, EnumType> \
    { \
        typedef typename Json::char_type char_type; \
        static_assert(std::is_enum<EnumType>::value, # EnumType " must be an enum"); \
        typedef EnumType value_type; \
        typedef std::basic_string<char_type> string_type; \
        typedef basic_string_view<char_type> string_view_type; \
        typedef typename Json::allocator_type allocator_type; \
        typedef std::pair<EnumType,string_type> mapped_type; \
        \
        static std::pair<const mapped_type*,const mapped_type*> get_values() \
        { \
            static const mapped_type v[] = { \
                JSONCONS_VARIADIC_REP_N(JSONCONS_NAMED_ENUM_PAIR,,,, __VA_ARGS__)\
            };\
            return std::make_pair(v,v+JSONCONS_NARGS(__VA_ARGS__)); \
        } \
        \
        static bool is(const Json& ajson) noexcept \
        { \
            if (!ajson.is_string()) return false; \
            auto first = get_values().first; \
            auto last = get_values().second; \
            const string_view_type s = ajson.template as<string_view_type>(); \
            if (s.empty() && std::find_if(first, last, \
                                          [](const mapped_type& item) -> bool \
                                          { return item.first == value_type(); }) == last) \
            { \
                return true; \
            } \
            auto it = std::find_if(first, last, \
                                   [&](const mapped_type& item) -> bool \
                                   { return item.second == s; }); \
            return it != last; \
        } \
        static value_type as(const Json& ajson) \
        { \
            if (!ajson.is_string()) \
            { \
                JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not an enum")); \
            } \
            const string_view_type s = ajson.template as<string_view_type>(); \
            auto first = get_values().first; \
            auto last = get_values().second; \
            if (s.empty() && std::find_if(first, last, \
                                          [](const mapped_type& item) -> bool \
                                          { return item.first == value_type(); }) == last) \
            { \
                return value_type(); \
            } \
            auto it = std::find_if(first, last, \
                                   [&](const mapped_type& item) -> bool \
                                   { return item.second == s; }); \
            if (it == last) \
            { \
                if (s.empty()) \
                { \
                    return value_type(); \
                } \
                else \
                { \
                    JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not an enum")); \
                } \
            } \
            return it->first; \
        } \
        static Json to_json(value_type aval, allocator_type alloc=allocator_type()) \
        { \
            static constexpr char_type empty_string[] = {0}; \
            auto first = get_values().first; \
            auto last = get_values().second; \
            auto it = std::find_if(first, last, \
                                   [aval](const mapped_type& item) -> bool \
                                   { return item.first == aval; }); \
            if (it == last) \
            { \
                if (aval == value_type()) \
                { \
                    return Json(empty_string); \
                } \
                else \
                { \
                    JSONCONS_THROW(json_runtime_error<std::runtime_error>("Not an enum")); \
                } \
            } \
            return Json(it->second,alloc); \
        } \
    }; \
} \
    /**/

#define JSONCONS_PROPERTY_AS(Prefix, GetPrefix, SetPrefix, Property, Count) JSONCONS_PROPERTY_AS_(Prefix, GetPrefix ## Property, SetPrefix ## Property, Property, Count) 
#define JSONCONS_PROPERTY_AS_LAST(Prefix, GetPrefix, SetPrefix, Property, Count) JSONCONS_PROPERTY_AS_(Prefix, GetPrefix ## Property, SetPrefix ## Property, Property, Count)  
#define JSONCONS_PROPERTY_AS_(Prefix, Getter, Setter, Property, Count) if ((num_params-Count) < num_mandatory_params2 || ajson.contains(JSONCONS_QUOTE(Prefix, Property))) {aval.Setter(ajson.at(JSONCONS_QUOTE(Prefix, Property)).template as<typename std::decay<decltype(aval.Getter())>::type>());}

#define JSONCONS_ALL_PROPERTY_AS(Prefix, GetPrefix, SetPrefix, Property, Count) JSONCONS_ALL_PROPERTY_AS_(Prefix, GetPrefix ## Property, SetPrefix ## Property, Property, Count) 
#define JSONCONS_ALL_PROPERTY_AS_LAST(Prefix, GetPrefix, SetPrefix, Property, Count) JSONCONS_ALL_PROPERTY_AS_(Prefix, GetPrefix ## Property, SetPrefix ## Property, Property, Count) 
#define JSONCONS_ALL_PROPERTY_AS_(Prefix, Getter, Setter, Property, Count) aval.Setter(ajson.at(JSONCONS_QUOTE(Prefix, Property)).template as<typename std::decay<decltype(aval.Getter())>::type>());

#define JSONCONS_PROPERTY_TO_JSON(Prefix, GetPrefix, SetPrefix, Property, Count) JSONCONS_PROPERTY_TO_JSON_(Prefix, GetPrefix ## Property, SetPrefix ## Property, Property, Count) 
#define JSONCONS_PROPERTY_TO_JSON_LAST(Prefix, GetPrefix, SetPrefix, Property, Count) JSONCONS_PROPERTY_TO_JSON_(Prefix, GetPrefix ## Property, SetPrefix ## Property, Property, Count) 
#define JSONCONS_PROPERTY_TO_JSON_(Prefix, Getter, Setter, Property, Count) ajson.try_emplace(JSONCONS_QUOTE(Prefix, Property), aval.Getter() );

#define JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(As,CharT,Prefix,NumTemplateParams, ValueType,GetPrefix,SetPrefix,NumMandatoryParams1,NumMandatoryParams2, ...)  \
namespace jsoncons \
{ \
    template<typename Json JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> \
    struct json_type_traits<Json, ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams), typename std::enable_if<std::is_same<typename Json::char_type,CharT>::value>::type> \
    { \
        typedef ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams) value_type; \
        typedef typename Json::allocator_type allocator_type; \
        typedef typename Json::string_view_type string_view_type; \
        constexpr static size_t num_params = JSONCONS_NARGS(__VA_ARGS__); \
        constexpr static size_t num_mandatory_params1 = NumMandatoryParams1; \
        constexpr static size_t num_mandatory_params2 = NumMandatoryParams2; \
        static bool is(const Json& ajson) noexcept \
        { \
            if (!ajson.is_object()) return false; \
            JSONCONS_VARIADIC_REP_N(JSONCONS_IS, Prefix,GetPrefix,SetPrefix, __VA_ARGS__)\
            return true; \
        } \
        static value_type as(const Json& ajson) \
        { \
            value_type aval{}; \
            JSONCONS_VARIADIC_REP_N(As, Prefix,GetPrefix,SetPrefix, __VA_ARGS__) \
            return aval; \
        } \
        static Json to_json(const value_type& aval, allocator_type alloc=allocator_type()) \
        { \
            Json ajson(json_object_arg, semantic_tag::none, alloc); \
            JSONCONS_VARIADIC_REP_N(JSONCONS_PROPERTY_TO_JSON, Prefix,GetPrefix,SetPrefix, __VA_ARGS__) \
            return ajson; \
        } \
    }; \
} \
  /**/

#define JSONCONS_N_GETTER_SETTER_TRAITS_DECL(ValueType,GetPrefix,SetPrefix,NumMandatoryParams, ...)  \
    JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(JSONCONS_PROPERTY_AS, char,,0, ValueType,GetPrefix,SetPrefix,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(JSONCONS_PROPERTY_AS, wchar_t,L,0, ValueType,GetPrefix,SetPrefix,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/

#define JSONCONS_TPL_N_GETTER_SETTER_TRAITS_DECL(NumTemplateParams, ValueType,GetPrefix,SetPrefix,NumMandatoryParams, ...)  \
    JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(JSONCONS_PROPERTY_AS, char,,NumTemplateParams, ValueType,GetPrefix,SetPrefix,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(JSONCONS_PROPERTY_AS, wchar_t,L,NumTemplateParams, ValueType,GetPrefix,SetPrefix,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/

#define JSONCONS_ALL_GETTER_SETTER_TRAITS_DECL(ValueType,GetPrefix,SetPrefix, ...)  \
    JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(JSONCONS_ALL_PROPERTY_AS,char,,0,ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
    JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(JSONCONS_ALL_PROPERTY_AS,wchar_t,L,0,ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
  /**/

#define JSONCONS_TPL_ALL_GETTER_SETTER_TRAITS_DECL(NumTemplateParams, ValueType,GetPrefix,SetPrefix, ...)  \
    JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(JSONCONS_ALL_PROPERTY_AS,char,,NumTemplateParams,ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
    JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(JSONCONS_ALL_PROPERTY_AS,wchar_t,L,NumTemplateParams,ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
  /**/

#if !defined(JSONCONS_NO_DEPRECATED)

#define JSONCONS_GETTER_SETTER_TRAITS_DECL(ValueType,GetPrefix,SetPrefix, ...)  \
    JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(JSONCONS_PROPERTY_AS, char,,0, ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(JSONCONS_PROPERTY_AS, wchar_t,L,0, ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
  /**/

#define JSONCONS_TPL_GETTER_SETTER_TRAITS_DECL(NumTemplateParams, ValueType,GetPrefix,SetPrefix, ...)  \
    JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(JSONCONS_PROPERTY_AS, char,,NumTemplateParams, ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    JSONCONS_GETTER_SETTER_TRAITS_DECL_BASE(JSONCONS_PROPERTY_AS, wchar_t,L,NumTemplateParams, ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
  /**/
 
#endif 
 
#define JSONCONS_GETTER_SETTER_IS(P1, P2, P3, Seq, Count) JSONCONS_GETTER_SETTER_IS_LAST(P1, P2, P3, Seq, Count)
#define JSONCONS_GETTER_SETTER_IS_LAST(P1, P2, P3, Seq, Count) if ((num_params-Count) < num_mandatory_params1 && JSONCONS_EXPAND(JSONCONS_GETTER_SETTER_IS_ Seq)
#define JSONCONS_GETTER_SETTER_IS_(Getter, Setter, Name) !ajson.contains(Name)) return false;

#define JSONCONS_GETTER_SETTER_NAMED_AS(P1, P2, P3, Seq, Count) JSONCONS_GETTER_SETTER_NAMED_AS_LAST(P1, P2, P3, Seq, Count)
#define JSONCONS_GETTER_SETTER_NAMED_AS_LAST(P1, P2, P3, Seq, Count) if ((num_params-Count) < num_mandatory_params2 || JSONCONS_EXPAND(JSONCONS_GETTER_SETTER_NAMED_AS_ Seq)
#define JSONCONS_GETTER_SETTER_NAMED_AS_(Getter, Setter, Name) ajson.contains(Name)) aval.Setter(ajson.at(Name).template as<typename std::decay<decltype(aval.Getter())>::type>());

#define JSONCONS_ALL_GETTER_SETTER_NAMED_AS(P1, P2, P3, Seq, Count) JSONCONS_ALL_GETTER_SETTER_NAMED_AS_LAST(P1, P2, P3, Seq, Count)
#define JSONCONS_ALL_GETTER_SETTER_NAMED_AS_LAST(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(JSONCONS_ALL_GETTER_SETTER_NAMED_AS_ Seq)
#define JSONCONS_ALL_GETTER_SETTER_NAMED_AS_(Getter, Setter, Name) aval.Setter(ajson.at(Name).template as<typename std::decay<decltype(aval.Getter())>::type>());

#define JSONCONS_GETTER_SETTER_TO_JSON(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(JSONCONS_GETTER_SETTER_TO_JSON_ Seq)
#define JSONCONS_GETTER_SETTER_TO_JSON_LAST(P1, P2, P3, Seq, Count) JSONCONS_EXPAND(JSONCONS_GETTER_SETTER_TO_JSON_ Seq)
#define JSONCONS_GETTER_SETTER_TO_JSON_(Getter, Setter, Name) ajson.try_emplace(Name, aval.Getter() );
 
#define JSONCONS_GETTER_SETTER_NAMED_TRAITS_DECL_BASE(As, NumTemplateParams, ValueType,NumMandatoryParams1,NumMandatoryParams2, ...)  \
namespace jsoncons \
{ \
    template<typename Json JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> \
    struct json_type_traits<Json, ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> \
    { \
        typedef ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams) value_type; \
        typedef typename Json::allocator_type allocator_type; \
        constexpr static size_t num_params = JSONCONS_NARGS(__VA_ARGS__); \
        constexpr static size_t num_mandatory_params1 = NumMandatoryParams1; \
        constexpr static size_t num_mandatory_params2 = NumMandatoryParams2; \
        static bool is(const Json& ajson) noexcept \
        { \
            if (!ajson.is_object()) return false; \
            JSONCONS_VARIADIC_REP_N(JSONCONS_GETTER_SETTER_IS,,,, __VA_ARGS__)\
            return true; \
        } \
        static value_type as(const Json& ajson) \
        { \
            value_type aval{}; \
            JSONCONS_VARIADIC_REP_N(As,,,, __VA_ARGS__) \
            return aval; \
        } \
        static Json to_json(const value_type& aval, allocator_type alloc=allocator_type()) \
        { \
            Json ajson(json_object_arg, semantic_tag::none, alloc); \
            JSONCONS_VARIADIC_REP_N(JSONCONS_GETTER_SETTER_TO_JSON,,,, __VA_ARGS__) \
            return ajson; \
        } \
    }; \
} \
  /**/
 
#define JSONCONS_N_GETTER_SETTER_NAMED_TRAITS_DECL(ValueType,NumMandatoryParams, ...)  \
JSONCONS_GETTER_SETTER_NAMED_TRAITS_DECL_BASE(JSONCONS_GETTER_SETTER_NAMED_AS, 0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/
 
#define JSONCONS_TPL_N_GETTER_SETTER_NAMED_TRAITS_DECL(NumTemplateParams, ValueType,NumMandatoryParams, ...)  \
JSONCONS_GETTER_SETTER_NAMED_TRAITS_DECL_BASE(JSONCONS_GETTER_SETTER_NAMED_AS, NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
  /**/
 
#define JSONCONS_ALL_GETTER_SETTER_NAMED_TRAITS_DECL(ValueType, ...)  \
JSONCONS_GETTER_SETTER_NAMED_TRAITS_DECL_BASE(JSONCONS_ALL_GETTER_SETTER_NAMED_AS, 0, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
  /**/
 
#define JSONCONS_TPL_ALL_GETTER_SETTER_NAMED_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
JSONCONS_GETTER_SETTER_NAMED_TRAITS_DECL_BASE(JSONCONS_ALL_GETTER_SETTER_NAMED_AS, NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
  /**/
 
#if !defined(JSONCONS_NO_DEPRECATED)

#define JSONCONS_GETTER_SETTER_NAMED_TRAITS_DECL(ValueType, ...)  \
JSONCONS_GETTER_SETTER_NAMED_TRAITS_DECL_BASE(JSONCONS_GETTER_SETTER_NAMED_AS, 0, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
  /**/
 
#define JSONCONS_TPL_GETTER_SETTER_NAMED_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
JSONCONS_GETTER_SETTER_NAMED_TRAITS_DECL_BASE(JSONCONS_GETTER_SETTER_NAMED_AS, NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
  /**/

#endif

#define JSONCONS_POLYMORPHIC_IS(BaseClass, P2, P3, DerivedClass, Count) if (ajson.template is<DerivedClass>()) return true;
#define JSONCONS_POLYMORPHIC_IS_LAST(BaseClass, P2, P3, DerivedClass, Count)  if (ajson.template is<DerivedClass>()) return true;

#define JSONCONS_POLYMORPHIC_AS(BaseClass, P2, P3, DerivedClass, Count) if (ajson.template is<DerivedClass>()) return std::make_shared<DerivedClass>(ajson.template as<DerivedClass>());
#define JSONCONS_POLYMORPHIC_AS_LAST(BaseClass, P2, P3, DerivedClass, Count)  if (ajson.template is<DerivedClass>()) return std::make_shared<DerivedClass>(ajson.template as<DerivedClass>());

#define JSONCONS_POLYMORPHIC_AS_UNIQUE_PTR(BaseClass, P2, P3, DerivedClass, Count) if (ajson.template is<DerivedClass>()) return std::unique_ptr<BaseClass>(new DerivedClass(ajson.template as<DerivedClass>()));
#define JSONCONS_POLYMORPHIC_AS_UNIQUE_PTR_LAST(BaseClass, P2, P3, DerivedClass, Count)  if (ajson.template is<DerivedClass>()) return std::unique_ptr<BaseClass>(new DerivedClass(ajson.template as<DerivedClass>()));

#define JSONCONS_POLYMORPHIC_AS_SHARED_PTR(BaseClass, P2, P3, DerivedClass, Count) if (ajson.template is<DerivedClass>()) return std::make_shared<DerivedClass>(ajson.template as<DerivedClass>());
#define JSONCONS_POLYMORPHIC_AS_SHARED_PTR_LAST(BaseClass, P2, P3, DerivedClass, Count)  if (ajson.template is<DerivedClass>()) return std::make_shared<DerivedClass>(ajson.template as<DerivedClass>());

#define JSONCONS_POLYMORPHIC_TO_JSON(BaseClass, P2, P3, DerivedClass, Count) if (DerivedClass* p = dynamic_cast<DerivedClass*>(ptr.get())) {return Json(*p);}
#define JSONCONS_POLYMORPHIC_TO_JSON_LAST(BaseClass, P2, P3, DerivedClass, Count) if (DerivedClass* p = dynamic_cast<DerivedClass*>(ptr.get())) {return Json(*p);}

#define JSONCONS_POLYMORPHIC_TRAITS_DECL(BaseClass, ...)  \
namespace jsoncons { \
    template<class Json> \
    struct json_type_traits<Json, std::shared_ptr<BaseClass>> { \
        static bool is(const Json& ajson) noexcept { \
            if (!ajson.is_object()) return false; \
            JSONCONS_VARIADIC_REP_N(JSONCONS_POLYMORPHIC_IS, BaseClass,,, __VA_ARGS__)\
            return false; \
        } \
\
        static std::shared_ptr<BaseClass> as(const Json& ajson) { \
            if (!ajson.is_object()) return std::shared_ptr<BaseClass>(); \
            JSONCONS_VARIADIC_REP_N(JSONCONS_POLYMORPHIC_AS_SHARED_PTR, BaseClass,,, __VA_ARGS__)\
            return std::shared_ptr<BaseClass>(); \
        } \
\
        static Json to_json(const std::shared_ptr<BaseClass>& ptr) { \
            if (ptr.get() == nullptr) {return Json::null();} \
            JSONCONS_VARIADIC_REP_N(JSONCONS_POLYMORPHIC_TO_JSON, BaseClass,,, __VA_ARGS__)\
            return Json::null(); \
        } \
    }; \
    template<class Json> \
    struct json_type_traits<Json, std::unique_ptr<BaseClass>> { \
        static bool is(const Json& ajson) noexcept { \
            if (!ajson.is_object()) return false; \
            JSONCONS_VARIADIC_REP_N(JSONCONS_POLYMORPHIC_IS, BaseClass,,, __VA_ARGS__)\
            return false; \
        } \
        static std::unique_ptr<BaseClass> as(const Json& ajson) { \
            if (!ajson.is_object()) return std::unique_ptr<BaseClass>(); \
            JSONCONS_VARIADIC_REP_N(JSONCONS_POLYMORPHIC_AS_UNIQUE_PTR, BaseClass,,, __VA_ARGS__)\
            return std::unique_ptr<BaseClass>(); \
        } \
        static Json to_json(const std::unique_ptr<BaseClass>& ptr) { \
            if (ptr.get() == nullptr) {return Json::null();} \
            JSONCONS_VARIADIC_REP_N(JSONCONS_POLYMORPHIC_TO_JSON, BaseClass,,, __VA_ARGS__)\
            return Json::null(); \
        } \
    }; \
}  \
  /**/
 
#if !defined(JSONCONS_NO_DEPRECATED)

#define JSONCONS_TYPE_TRAITS_DECL JSONCONS_MEMBER_TRAITS_DECL
#define JSONCONS_NONDEFAULT_MEMBER_TRAITS_DECL JSONCONS_ALL_MEMBER_TRAITS_DECL
#define JSONCONS_TEMPLATE_STRICT_MEMBER_TRAITS_DECL JSONCONS_TPL_ALL_MEMBER_TRAITS_DECL

#define JSONCONS_MEMBER_TRAITS_NAMED_DECL                        JSONCONS_MEMBER_NAMED_TRAITS_DECL                   
#define JSONCONS_STRICT_MEMBER_TRAITS_NAMED_DECL                 JSONCONS_ALL_MEMBER_NAMED_TRAITS_DECL            
#define JSONCONS_TEMPLATE_MEMBER_TRAITS_DECL                     JSONCONS_TPL_MEMBER_TRAITS_DECL                     
#define JSONCONS_STRICT_TEMPLATE_MEMBER_TRAITS_DECL              JSONCONS_TPL_ALL_MEMBER_TRAITS_DECL              
#define JSONCONS_TEMPLATE_MEMBER_TRAITS_NAMED_DECL               JSONCONS_TPL_MEMBER_NAMED_TRAITS_DECL               
#define JSONCONS_STRICT_TEMPLATE_MEMBER_TRAITS_NAMED_DECL        JSONCONS_TPL_ALL_MEMBER_NAMED_TRAITS_DECL        
#define JSONCONS_ENUM_TRAITS_NAMED_DECL                          JSONCONS_ENUM_NAMED_TRAITS_DECL                     
#define JSONCONS_GETTER_CTOR_TRAITS_NAMED_DECL                   JSONCONS_ALL_GETTER_CTOR_NAMED_TRAITS_DECL              
#define JSONCONS_TEMPLATE_GETTER_CTOR_TRAITS_DECL                JSONCONS_TPL_ALL_GETTER_CTOR_TRAITS_DECL                
#define JSONCONS_TEMPLATE_GETTER_CTOR_TRAITS_NAMED_DECL          JSONCONS_TPL_ALL_GETTER_CTOR_NAMED_TRAITS_DECL          
#define JSONCONS_GETTER_SETTER_TRAITS_NAMED_DECL                 JSONCONS_ALL_GETTER_SETTER_NAMED_TRAITS_DECL            
#define JSONCONS_STRICT_GETTER_SETTER_TRAITS_NAMED_DECL          JSONCONS_ALL_GETTER_SETTER_NAMED_TRAITS_DECL     
#define JSONCONS_TEMPLATE_GETTER_SETTER_TRAITS_NAMED_DECL        JSONCONS_TPL_GETTER_SETTER_NAMED_TRAITS_DECL        
#define JSONCONS_STRICT_TEMPLATE_GETTER_SETTER_TRAITS_NAMED_DECL JSONCONS_TPL_ALL_GETTER_SETTER_NAMED_TRAITS_DECL 
#define JSONCONS_STRICT_TPL_MEMBER_TRAITS_DECL                   JSONCONS_TPL_ALL_MEMBER_TRAITS_DECL
#define JSONCONS_STRICT_TPL_MEMBER_NAMED_TRAITS_DECL             JSONCONS_TPL_ALL_MEMBER_NAMED_TRAITS_DECL
#define JSONCONS_STRICT_TPL_GETTER_SETTER_NAMED_TRAITS_DECL      JSONCONS_TPL_ALL_GETTER_SETTER_NAMED_TRAITS_DECL

#define JSONCONS_STRICT_MEMBER_TRAITS_DECL JSONCONS_ALL_MEMBER_TRAITS_DECL 
#define JSONCONS_TPL_STRICT_MEMBER_TRAITS_DECL JSONCONS_TPL_ALL_MEMBER_TRAITS_DECL
#define JSONCONS_STRICT_MEMBER_NAMED_TRAITS_DECL JSONCONS_ALL_MEMBER_NAMED_TRAITS_DECL
#define JSONCONS_TPL_STRICT_MEMBER_NAMED_TRAITS_DECL JSONCONS_ALL_STRICT_MEMBER_NAMED_TRAITS_DECL
#define JSONCONS_STRICT_PROPERTY_TRAITS_DECL JSONCONS_ALL_GETTER_SETTER_TRAITS_DECL
#define JSONCONS_TPL_STRICT_PROPERTY_TRAITS_DECL JSONCONS_TPL_ALL_GETTER_SETTER_TRAITS_DECL
#define JSONCONS_STRICT_GETTER_SETTER_NAMED_TRAITS_DECL JSONCONS_ALL_GETTER_SETTER_NAMED_TRAITS_DECL
#define JSONCONS_TPL_STRICT_GETTER_SETTER_NAMED_TRAITS_DECL JSONCONS_TPL_ALL_GETTER_SETTER_NAMED_TRAITS_DECL
#define JSONCONS_GETTER_CTOR_TRAITS_DECL JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL
#define JSONCONS_TPL_GETTER_CTOR_TRAITS_DECL JSONCONS_TPL_ALL_GETTER_CTOR_TRAITS_DECL  
#define JSONCONS_GETTER_CTOR_NAMED_TRAITS_DECL JSONCONS_ALL_GETTER_CTOR_NAMED_TRAITS_DECL
#define JSONCONS_TPL_GETTER_CTOR_NAMED_TRAITS_DECL JSONCONS_TPL_ALL_GETTER_CTOR_NAMED_TRAITS_DECL
#define JSONCONS_N_PROPERTY_TRAITS_DECL JSONCONS_N_GETTER_SETTER_TRAITS_DECL
#define JSONCONS_ALL_PROPERTY_TRAITS_DECL JSONCONS_ALL_GETTER_SETTER_TRAITS_DECL
#define JSONCONS_TPL_N_PROPERTY_TRAITS_DECL JSONCONS_TPL_N_GETTER_SETTER_TRAITS_DECL
#define JSONCONS_TPL_ALL_PROPERTY_TRAITS_DECL JSONCONS_TPL_ALL_GETTER_SETTER_TRAITS_DECL

#endif

#endif

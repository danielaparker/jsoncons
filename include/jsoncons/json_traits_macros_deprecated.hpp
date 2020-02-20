// Copyright 2019 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TRAITS_MACROS_DEPRECATED_HPP
#define JSONCONS_JSON_TRAITS_MACROS_DEPRECATED_HPP

#include <jsoncons/json_traits_macros.hpp>

#if !defined(JSONCONS_NO_DEPRECATED)

#define JSONCONS_N_MEMBER_TRAITS_DECL(ValueType,NumMandatoryParams,...)  \
    JSONCONS_MEMBER_TRAITS_BASE(JSONCONS_AS, JSONCONS_TO_JSON, char,,0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_BASE(JSONCONS_AS, JSONCONS_TO_JSON, wchar_t,L,0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/

#define JSONCONS_TPL_N_MEMBER_TRAITS_DECL(NumTemplateParams, ValueType,NumMandatoryParams, ...)  \
    JSONCONS_MEMBER_TRAITS_BASE(JSONCONS_AS, JSONCONS_TO_JSON, char,,NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_BASE(JSONCONS_AS, JSONCONS_TO_JSON, wchar_t,L,NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/

#define JSONCONS_ALL_MEMBER_TRAITS_DECL(ValueType, ...)  \
    JSONCONS_MEMBER_TRAITS_BASE(JSONCONS_ALL_AS,JSONCONS_ALL_TO_JSON,char,,0,ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_BASE(JSONCONS_ALL_AS,JSONCONS_ALL_TO_JSON,wchar_t,L,0,ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/

#define JSONCONS_TPL_ALL_MEMBER_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
    JSONCONS_MEMBER_TRAITS_BASE(JSONCONS_ALL_AS,JSONCONS_ALL_TO_JSON,char,,NumTemplateParams,ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_BASE(JSONCONS_ALL_AS,JSONCONS_ALL_TO_JSON,wchar_t,L,NumTemplateParams,ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/

#define JSONCONS_MEMBER_TRAITS_DECL(ValueType, ...)  \
    JSONCONS_MEMBER_TRAITS_BASE(JSONCONS_AS, JSONCONS_TO_JSON, char,,0, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_BASE(JSONCONS_AS, JSONCONS_TO_JSON, wchar_t,L,0, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/

#define JSONCONS_TPL_MEMBER_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
    JSONCONS_MEMBER_TRAITS_BASE(JSONCONS_AS, JSONCONS_TO_JSON, char,,NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    JSONCONS_MEMBER_TRAITS_BASE(JSONCONS_AS, JSONCONS_TO_JSON, wchar_t,L,NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/


#define JSONCONS_N_MEMBER_NAMED_TRAITS_DECL(ValueType,NumMandatoryParams, ...)  \
    JSONCONS_MEMBER_NAMED_TRAITS_BASE(JSONCONS_NAMED_AS, JSONCONS_NAMED_TO_JSON, 0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/

#define JSONCONS_TPL_N_MEMBER_NAMED_TRAITS_DECL(NumTemplateParams, ValueType,NumMandatoryParams, ...)  \
    JSONCONS_MEMBER_NAMED_TRAITS_BASE(JSONCONS_NAMED_AS, JSONCONS_NAMED_TO_JSON, NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/

#define JSONCONS_ALL_MEMBER_NAMED_TRAITS_DECL(ValueType, ...)  \
    JSONCONS_MEMBER_NAMED_TRAITS_BASE(JSONCONS_ALL_NAMED_AS, JSONCONS_ALL_NAMED_TO_JSON, 0, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/

#define JSONCONS_TPL_ALL_MEMBER_NAMED_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
    JSONCONS_MEMBER_NAMED_TRAITS_BASE(JSONCONS_ALL_NAMED_AS, JSONCONS_ALL_NAMED_TO_JSON, NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/

#define JSONCONS_MEMBER_NAMED_TRAITS_DECL(ValueType, ...)  \
    JSONCONS_MEMBER_NAMED_TRAITS_BASE(JSONCONS_NAMED_AS, JSONCONS_NAMED_TO_JSON, 0, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/

#define JSONCONS_TPL_MEMBER_NAMED_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
    JSONCONS_MEMBER_NAMED_TRAITS_BASE(JSONCONS_NAMED_AS, JSONCONS_NAMED_TO_JSON, NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_ALL_GETTER_CTOR_TRAITS_DECL(ValueType, ...)  \
    JSONCONS_GETTER_CTOR_TRAITS_BASE(char,,0, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
    JSONCONS_GETTER_CTOR_TRAITS_BASE(wchar_t,L,0, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_TPL_ALL_GETTER_CTOR_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
    JSONCONS_GETTER_CTOR_TRAITS_BASE(char,,NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
    JSONCONS_GETTER_CTOR_TRAITS_BASE(wchar_t,L,NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_N_GETTER_CTOR_TRAITS_DECL(ValueType,NumMandatoryParams, ...)  \
    JSONCONS_GETTER_CTOR_TRAITS_BASE(char,,0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    JSONCONS_GETTER_CTOR_TRAITS_BASE(wchar_t,L,0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_N_ALL_GETTER_CTOR_TRAITS_DECL(NumTemplateParams, ValueType,NumMandatoryParams, ...)  \
    JSONCONS_GETTER_CTOR_TRAITS_BASE(char,,NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    JSONCONS_GETTER_CTOR_TRAITS_BASE(wchar_t,L,NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_ALL_GETTER_CTOR_NAMED_TRAITS_DECL(ValueType, ...)  \
    JSONCONS_GETTER_CTOR_NAMED_TRAITS_BASE(0, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_TPL_ALL_GETTER_CTOR_NAMED_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
    JSONCONS_GETTER_CTOR_NAMED_TRAITS_BASE(NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_N_GETTER_CTOR_NAMED_TRAITS_DECL(ValueType,NumMandatoryParams, ...)  \
    JSONCONS_GETTER_CTOR_NAMED_TRAITS_BASE(0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_TPL_N_GETTER_CTOR_NAMED_TRAITS_DECL(NumTemplateParams, ValueType,NumMandatoryParams, ...)  \
JSONCONS_GETTER_CTOR_NAMED_TRAITS_BASE(NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/

#define JSONCONS_ENUM_TRAITS_DECL(EnumType, ...)  \
    JSONCONS_ENUM_TRAITS_BASE(char,,EnumType,__VA_ARGS__) \
    JSONCONS_ENUM_TRAITS_BASE(wchar_t,L,EnumType,__VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<EnumType> : public std::true_type {}; } \
  /**/

#define JSONCONS_ENUM_NAMED_TRAITS_DECL(EnumType, ...)  \
    JSONCONS_ENUM_NAMED_TRAITS_DECL(EnumType,__VA_ARGS__)
 

#define JSONCONS_N_GETTER_SETTER_TRAITS_DECL(ValueType,GetPrefix,SetPrefix,NumMandatoryParams, ...)  \
    JSONCONS_GETTER_SETTER_TRAITS_BASE(JSONCONS_GETTER_SETTER_AS, JSONCONS_GETTER_SETTER_TO_JSON, char,,0, ValueType,GetPrefix,SetPrefix,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    JSONCONS_GETTER_SETTER_TRAITS_BASE(JSONCONS_GETTER_SETTER_AS, JSONCONS_GETTER_SETTER_TO_JSON, wchar_t,L,0, ValueType,GetPrefix,SetPrefix,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/

#define JSONCONS_TPL_N_GETTER_SETTER_TRAITS_DECL(NumTemplateParams, ValueType,GetPrefix,SetPrefix,NumMandatoryParams, ...)  \
    JSONCONS_GETTER_SETTER_TRAITS_BASE(JSONCONS_GETTER_SETTER_AS, JSONCONS_GETTER_SETTER_TO_JSON, char,,NumTemplateParams, ValueType,GetPrefix,SetPrefix,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    JSONCONS_GETTER_SETTER_TRAITS_BASE(JSONCONS_GETTER_SETTER_AS, JSONCONS_GETTER_SETTER_TO_JSON, wchar_t,L,NumTemplateParams, ValueType,GetPrefix,SetPrefix,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/

#define JSONCONS_ALL_GETTER_SETTER_TRAITS_DECL(ValueType,GetPrefix,SetPrefix, ...)  \
    JSONCONS_GETTER_SETTER_TRAITS_BASE(JSONCONS_ALL_GETTER_SETTER_AS, JSONCONS_ALL_GETTER_SETTER_TO_JSON,char,,0,ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
    JSONCONS_GETTER_SETTER_TRAITS_BASE(JSONCONS_ALL_GETTER_SETTER_AS, JSONCONS_ALL_GETTER_SETTER_TO_JSON,wchar_t,L,0,ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/

#define JSONCONS_TPL_ALL_GETTER_SETTER_TRAITS_DECL(NumTemplateParams, ValueType,GetPrefix,SetPrefix, ...)  \
    JSONCONS_GETTER_SETTER_TRAITS_BASE(JSONCONS_ALL_GETTER_SETTER_AS, JSONCONS_ALL_GETTER_SETTER_TO_JSON,char,,NumTemplateParams,ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
    JSONCONS_GETTER_SETTER_TRAITS_BASE(JSONCONS_ALL_GETTER_SETTER_AS, JSONCONS_ALL_GETTER_SETTER_TO_JSON,wchar_t,L,NumTemplateParams,ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__),__VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/

#define JSONCONS_GETTER_SETTER_TRAITS_DECL(ValueType,GetPrefix,SetPrefix, ...)  \
    JSONCONS_GETTER_SETTER_TRAITS_BASE(JSONCONS_GETTER_SETTER_AS, JSONCONS_GETTER_SETTER_TO_JSON, char,,0, ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    JSONCONS_GETTER_SETTER_TRAITS_BASE(JSONCONS_GETTER_SETTER_AS, JSONCONS_GETTER_SETTER_TO_JSON, wchar_t,L,0, ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/

#define JSONCONS_TPL_GETTER_SETTER_TRAITS_DECL(NumTemplateParams, ValueType,GetPrefix,SetPrefix, ...)  \
    JSONCONS_GETTER_SETTER_TRAITS_BASE(JSONCONS_GETTER_SETTER_AS, JSONCONS_GETTER_SETTER_TO_JSON, char,,NumTemplateParams, ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    JSONCONS_GETTER_SETTER_TRAITS_BASE(JSONCONS_GETTER_SETTER_AS, JSONCONS_GETTER_SETTER_TO_JSON, wchar_t,L,NumTemplateParams, ValueType,GetPrefix,SetPrefix, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_N_GETTER_SETTER_NAMED_TRAITS_DECL(ValueType,NumMandatoryParams, ...)  \
    JSONCONS_GETTER_SETTER_NAMED_TRAITS_BASE(JSONCONS_GETTER_SETTER_NAMED_AS,JSONCONS_GETTER_SETTER_NAMED_TO_JSON, 0, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_TPL_N_GETTER_SETTER_NAMED_TRAITS_DECL(NumTemplateParams, ValueType,NumMandatoryParams, ...)  \
    JSONCONS_GETTER_SETTER_NAMED_TRAITS_BASE(JSONCONS_GETTER_SETTER_NAMED_AS,JSONCONS_GETTER_SETTER_NAMED_TO_JSON, NumTemplateParams, ValueType,NumMandatoryParams,NumMandatoryParams, __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_ALL_GETTER_SETTER_NAMED_TRAITS_DECL(ValueType, ...)  \
    JSONCONS_GETTER_SETTER_NAMED_TRAITS_BASE(JSONCONS_ALL_GETTER_SETTER_NAMED_AS,JSONCONS_ALL_GETTER_SETTER_NAMED_TO_JSON, 0, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_TPL_ALL_GETTER_SETTER_NAMED_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
    JSONCONS_GETTER_SETTER_NAMED_TRAITS_BASE(JSONCONS_ALL_GETTER_SETTER_NAMED_AS,JSONCONS_ALL_GETTER_SETTER_NAMED_TO_JSON, NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), JSONCONS_NARGS(__VA_ARGS__), __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/

#define JSONCONS_GETTER_SETTER_NAMED_TRAITS_DECL(ValueType, ...)  \
    JSONCONS_GETTER_SETTER_NAMED_TRAITS_BASE(JSONCONS_GETTER_SETTER_NAMED_AS,JSONCONS_GETTER_SETTER_NAMED_TO_JSON, 0, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    namespace jsoncons { template <> struct is_json_type_traits_declared<ValueType> : public std::true_type {}; } \
  /**/
 
#define JSONCONS_TPL_GETTER_SETTER_NAMED_TRAITS_DECL(NumTemplateParams, ValueType, ...)  \
    JSONCONS_GETTER_SETTER_NAMED_TRAITS_BASE(JSONCONS_GETTER_SETTER_NAMED_AS,JSONCONS_GETTER_SETTER_NAMED_TO_JSON, NumTemplateParams, ValueType, JSONCONS_NARGS(__VA_ARGS__), 0, __VA_ARGS__) \
    namespace jsoncons { template <JSONCONS_GENERATE_TPL_PARAMS(JSONCONS_GENERATE_TPL_PARAM, NumTemplateParams)> struct is_json_type_traits_declared<ValueType JSONCONS_GENERATE_TPL_ARGS(JSONCONS_GENERATE_TPL_ARG, NumTemplateParams)> : public std::true_type {}; } \
  /**/

#define JSONCONS_POLYMORPHIC_TRAITS_DECL(BaseClass, ...)  \
    JSONCONS_POLYMORPHIC_TRAITS_DECL(BaseClass, __VA_ARGS__) \
    /**/

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

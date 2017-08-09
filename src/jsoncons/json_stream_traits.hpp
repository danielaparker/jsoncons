// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_STREAM_TRAITS_HPP
#define JSONCONS_JSON_STREAM_TRAITS_HPP

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

#include <iostream>
#include <string>
#include <type_traits>
#include <jsoncons/json_output_handler.hpp>
#include <jsoncons/serialization_options.hpp>
#include <jsoncons/json_serializer.hpp>

namespace jsoncons {

template <class CharT, class T, class Enable = void>
struct json_stream_traits
{
    static const bool is_compatible = false;

    static void encode(const T&, basic_json_output_handler<CharT>&)
    {
    }
};

namespace detail {

// string_requirement_traits

template <class T, class Enable=void>
struct string_requirement_traits
{
    typedef void value_type;
};

template <class T>
struct string_requirement_traits<T, typename std::enable_if<!std::is_void<typename T::traits_type>::value>::type>
{
    typedef typename T::traits_type value_type;
};

// is_json_integer

template <class T, class Enable=void>
struct is_json_integer : std::false_type {};

template <class T>
struct is_json_integer<T, 
                       typename std::enable_if<std::is_integral<T>::value && 
                       std::is_signed<T>::value && 
                       !std::is_same<T,bool>::value>::type> : std::true_type {};

// is_json_uinteger

template <class T, class Enable=void>
struct is_json_uinteger : std::false_type {};

template <class T>
struct is_json_uinteger<T, 
                        typename std::enable_if<std::is_integral<T>::value && 
                        !std::is_signed<T>::value && 
                        !std::is_same<T,bool>::value>::type> : std::true_type {};

// is_json_floating_point

template <class T, class Enable=void>
struct is_json_floating_point : std::false_type {};

template <class T>
struct is_json_floating_point<T, 
                              typename std::enable_if<std::is_floating_point<T>::value>::type> : std::true_type {};

// map_requirement_traits

template <class T, class Enable=void>
struct map_requirement_traits
{
    typedef void value_type;
};

template <class T>
struct map_requirement_traits<T, typename std::enable_if<!std::is_void<typename T::mapped_type>::value>::type>
{
    typedef typename T::mapped_type value_type;
};

// is_incompatible
template<class CharT, class T, class Enable = void>
struct is_incompatible : std::false_type {};

// is_incompatible
template<class CharT, class T>
struct is_incompatible<CharT,T,
    typename std::enable_if<!std::integral_constant<bool, json_stream_traits<CharT, T>::is_compatible>::value>::type
> : std::true_type {};

// is_compatible_string_type
template<class CharT, class T, class Enable=void>
struct is_compatible_string_type : std::false_type {};

template<class CharT, class T>
struct is_compatible_string_type<CharT,T, 
    typename std::enable_if<!std::is_void<typename string_requirement_traits<T>::value_type>::value && 
    !is_incompatible<CharT,typename std::iterator_traits<typename T::iterator>::value_type>::value
>::type> : std::true_type {};

// is_compatible_array_type
template<class CharT, class T, class Enable=void>
struct is_compatible_array_type : std::false_type {};

template<class CharT, class T>
struct is_compatible_array_type<CharT,T, 
    typename std::enable_if<std::is_void<typename map_requirement_traits<T>::value_type>::value && 
    std::is_void<typename string_requirement_traits<T>::value_type>::value && 
    !is_incompatible<CharT,typename std::iterator_traits<typename T::iterator>::value_type>::value
>::type> : std::true_type {};

// is_compatible_object_type
template<class CharT, class T, class Enable=void>
struct is_compatible_object_type : std::false_type {};

template<class CharT, class T>
struct is_compatible_object_type<CharT,T, 
                       typename std::enable_if<
    !is_incompatible<CharT,typename T::mapped_type>::value
>::type> : std::true_type {};

// is_std_array
template<class T>
struct is_std_array : std::false_type {};

template<class E, size_t N>
struct is_std_array<std::array<E, N>> : std::true_type {};

}

template <class CharT, class T>
void dump(const T& val, basic_json_output_handler<CharT>& handler)
{
    handler.begin_json();
    json_stream_traits<CharT,T>::encode(val,handler);
    handler.end_json();
}

template <class CharT, class T>
void dump(const T& val, std::basic_ostream<CharT>& os)
{
    basic_json_serializer<CharT> serializer(os);
    dump(val, serializer);
}

// integer

template<class CharT, class T>
struct json_stream_traits<CharT, T,
                          typename std::enable_if<detail::is_json_integer<T>::value
>::type>
{
    static void encode(T val, basic_json_output_handler<CharT>& handler)
    {
        handler.integer_value(val);
    }
};

// uinteger

template<class CharT, class T>
struct json_stream_traits<CharT, T,
                          typename std::enable_if<detail::is_json_uinteger<T>::value
>::type>
{
    static void encode(T val, basic_json_output_handler<CharT>& handler)
    {
        handler.uinteger_value(val);
    }
};

// double

template<class CharT, class T>
struct json_stream_traits<CharT, T,
                          typename std::enable_if<detail::is_json_floating_point<T>::value
>::type>
{
    static void encode(T val, basic_json_output_handler<CharT>& handler)
    {
        handler.double_value(val);
    }
};

// bool

template<class CharT>
struct json_stream_traits<CharT, bool>
{
    static void encode(bool val, basic_json_output_handler<CharT>& handler)
    {
        handler.bool_value(val);
    }
};

// string

template<class CharT, class T>
struct json_stream_traits<CharT, T,
    typename std::enable_if<detail::is_compatible_string_type<CharT,T>::value>::type>
{
    static void encode(const T& val, basic_json_output_handler<CharT>& handler)
    {
        handler.string_value(val);
    }
};

// sequence container (except string and array)

template<class CharT, class T>
struct json_stream_traits<CharT, T,
    typename std::enable_if<detail::is_compatible_array_type<CharT,T>::value && !detail::is_std_array<T>::value>::type>
{
    typedef typename std::iterator_traits<typename T::iterator>::value_type value_type;

    static void encode(const T& val, basic_json_output_handler<CharT>& handler)
    {
        handler.begin_array();
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            json_stream_traits<CharT,value_type>::encode(*it,handler);
        }
        handler.end_array();
    }
};

// associative container

template<class CharT, class T>
struct json_stream_traits<CharT, T,
    typename std::enable_if<detail::is_compatible_object_type<CharT,T>::value>::type>
{
    typedef typename T::mapped_type mapped_type;
    typedef typename T::value_type value_type;

    static void encode(const T& val, basic_json_output_handler<CharT>& handler)
    {
        handler.begin_object();
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            handler.name(it->first);
            json_stream_traits<CharT, mapped_type>::encode(it->second,handler);
        }
        handler.end_object();
    }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif



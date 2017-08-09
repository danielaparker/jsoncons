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
#include <jsoncons/jsoncons_util.hpp>

namespace jsoncons {

template <class CharT, class T, class Enable = void>
struct json_stream_traits
{
    static void encode(const T&, basic_json_output_handler<CharT>&)
    {
    }
};

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

template <class CharT, class T>
void dump(const T& val, const basic_serialization_options<CharT>& options,
          std::basic_ostream<CharT>& os)
{
    basic_json_serializer<CharT> serializer(os, options);
    dump(val, serializer);
}

template <class CharT, class T>
void dump(const T& val, bool pprint,
          std::basic_ostream<CharT>& os)
{
    basic_json_serializer<CharT> serializer(os, pprint);
    dump(val, serializer);
}

template <class CharT, class T>
void dump(const T& val, const basic_serialization_options<CharT>& options, bool pprint,
          std::basic_ostream<CharT>& os)
{
    basic_json_serializer<CharT> serializer(os, options, pprint);
    dump(val, serializer);
}

// integer

template<class CharT, class T>
struct json_stream_traits<CharT, T,
                          typename std::enable_if<detail::is_integer_like<T>::value
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
                          typename std::enable_if<detail::is_uinteger_like<T>::value
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
                          typename std::enable_if<detail::is_floating_point_like<T>::value
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
    typename std::enable_if<detail::is_string_like<T>::value
>::type>
{
    static void encode(const T& val, basic_json_output_handler<CharT>& handler)
    {
        handler.string_value(val);
    }
};

// sequence container (except string and array)

template<class CharT, class T>
struct json_stream_traits<CharT, T,
    typename std::enable_if<detail::is_vector_like<T>::value
>::type>
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
    typename std::enable_if<detail::is_map_like<T>::value
>::type>
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



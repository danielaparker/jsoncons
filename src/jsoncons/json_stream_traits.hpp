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
#include <tuple>
#include <array>
#include <type_traits>
#include <memory>
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
void dump_body(const T& val, basic_json_output_handler<CharT>& handler)
{
    json_stream_traits<CharT,T>::encode(val,handler);
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
void dump(const T& val, std::basic_ostream<CharT>& os, bool pprint)
{
    basic_json_serializer<CharT> serializer(os, pprint);
    dump(val, serializer);
}

template <class CharT, class T>
void dump(const T& val, const basic_serialization_options<CharT>& options,
          std::basic_ostream<CharT>& os, bool pprint)
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

template<class CharT>
struct json_stream_traits<CharT, typename type_wrapper<CharT>::const_pointer_type>
{
    static void encode(typename type_wrapper<CharT>::const_pointer_type val, basic_json_output_handler<CharT>& handler)
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

// std::array

template<class CharT, class T, size_t N>
struct json_stream_traits<CharT, std::array<T,N>>
{
    typedef typename std::array<T,N>::value_type value_type;
public:
   
    static void encode(const std::array<T, N>& val, basic_json_output_handler<CharT>& handler)
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

namespace detail { namespace streaming {

template<size_t Pos, class CharT, class Tuple>
struct tuple_helper
{
    using element_type = typename std::tuple_element<std::tuple_size<Tuple>::value - Pos, Tuple>::type;
    using next = tuple_helper<Pos - 1, CharT, Tuple>;
    
    static void encode(const Tuple& tuple, basic_json_output_handler<CharT>& handler)
    {
        json_stream_traits<CharT, element_type>::encode(std::get<std::tuple_size<Tuple>::value - Pos>(tuple),handler);
        next::encode(tuple, handler);
    }
};

template<class CharT, class Tuple>
struct tuple_helper<0, CharT, Tuple>
{
    static void encode(const Tuple&, basic_json_output_handler<CharT>&)
    {
    }
};

}}

template<class CharT, typename... E>
struct json_stream_traits<CharT, std::tuple<E...>>
{
private:
    using helper = detail::streaming::tuple_helper<sizeof...(E), CharT, std::tuple<E...>>;

public:
    static void encode(const std::tuple<E...>& value, basic_json_output_handler<CharT>& handler)
    {
        handler.begin_array();
        helper::encode(value, handler);
        handler.end_array();
    }
};

template<class CharT, class T1, class T2>
struct json_stream_traits<CharT, std::pair<T1,T2>>
{
public:
   
    static void encode(const std::pair<T1,T2>& value, basic_json_output_handler<CharT>& handler)
    {
        handler.begin_array();
        json_stream_traits<CharT,T1>::encode(value.first, handler);
        json_stream_traits<CharT,T2>::encode(value.second, handler);
        handler.end_array();
    }
};

#if !defined(JSONCONS_NO_DEPRECATED)
template<class CharT, class T>
struct json_stream_traits<CharT, std::shared_ptr<T>>
{
public:
   
    static void encode(std::shared_ptr<T> p, basic_json_output_handler<CharT>& handler)
    {
        json_stream_traits<CharT,T>::encode(*p, handler);
    }
};
#endif

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif



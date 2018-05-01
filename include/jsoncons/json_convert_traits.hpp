// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONCONVERT_HPP
#define JSONCONS_JSONCONVERT_HPP

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
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/json_type_traits.hpp>

namespace jsoncons {

struct sorted_policy;

template <class CharT, 
          class ImplementationPolicy = sorted_policy, 
          class Allocator = std::allocator<CharT>>
class basic_json;

// json_convert_traits

template <class T, class Enable = void>
struct json_convert_traits
{
    template <class CharT>
    static T decode(const std::basic_string<CharT>& s)
    {
        basic_json<CharT> j = basic_json<CharT>::parse(s);
        return j. template as<T>();
    }

    template <class CharT>
    static void encode(const T& val, basic_json_output_handler<CharT>& serializer)
    {
        auto j = json_type_traits<basic_json<CharT>, T>::to_json(val);
        j.dump(serializer);
    }
};

// json_convert_traits specializations

// vector like

template <class T>
struct json_convert_traits<T,
    typename std::enable_if<detail::is_vector_like<T>::value
>::type>
{
    typedef typename std::iterator_traits<typename T::iterator>::value_type value_type;

    template <class CharT>
    static T decode(const std::basic_string<CharT>& s)
    {
        basic_json<CharT> j = basic_json<CharT>::parse(s);
        return j. template as<T>();
    }

    template <class CharT>
    static void encode(const T& val, basic_json_output_handler<CharT>& serializer)
    {
        serializer.begin_json();
        serializer.begin_array();
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            json_convert_traits<value_type>::encode(*it,serializer);
        }
        serializer.end_array();
        serializer.end_json();
    }
};

// std::array

template <class T, size_t N>
struct json_convert_traits<std::array<T,N>>
{
    typedef typename std::array<T,N>::value_type value_type;

    template <class CharT>
    static std::array<T, N> decode(const std::basic_string<CharT>& s)
    {
        basic_json<CharT> j = basic_json<CharT>::parse(s);
        return j. template as<std::array<T, N>>();
    }

    template <class CharT>
    static void encode(const std::array<T, N>& val, std::basic_string<CharT>& s)
    {
        basic_json_serializer<CharT,detail::string_writer<CharT>> serializer(s);
        encode(val,serializer);
    }

    template <class CharT>
    static void encode(const std::array<T, N>& val, basic_json_output_handler<CharT>& serializer)
    {
        serializer.begin_json();
        serializer.begin_array();
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            json_convert_traits<value_type>::encode(*it,serializer);
        }
        serializer.end_array();
        serializer.end_json();
    }
};

// map like

template <class T>
struct json_convert_traits<T,
    typename std::enable_if<detail::is_map_like<T>::value
>::type>
{
    typedef typename T::mapped_type mapped_type;
    typedef typename T::value_type value_type;

    template <class CharT>
    static T decode(const std::basic_string<CharT>& s)
    {
        basic_json<CharT> j = basic_json<CharT>::parse(s);
        return j. template as<T>();
    }

    template <class CharT>
    static void encode(const T& val, basic_json_output_handler<CharT>& serializer)
    {
        serializer.begin_json();
        serializer.begin_object();
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            serializer.name(it->first);
            json_convert_traits<mapped_type>::encode(it->second,serializer);
        }
        serializer.end_object();
        serializer.end_json();
    }
};

// decode_json

template <class T, class CharT>
T decode_json(const std::basic_string<CharT>& s)
{
    return json_convert_traits<T>::decode(s);
}

// encode_json

template <class T, class CharT>
void encode_json(const T& val, basic_json_output_handler<CharT>& handler)
{
    handler.begin_json();
    json_convert_traits<T>::encode(val,handler);
    handler.end_json();
}

template <class T, class CharT>
void dump_fragment(const T& val, basic_json_output_handler<CharT>& handler)
{
    json_convert_traits<T>::encode(val,handler);
}

template <class T, class CharT>
void encode_json(const T& val, std::basic_ostream<CharT>& os)
{
    basic_json_serializer<CharT> serializer(os);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, const basic_serialization_options<CharT>& options,
          std::basic_ostream<CharT>& os)
{
    basic_json_serializer<CharT> serializer(os, options);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, std::basic_ostream<CharT>& os, bool pprint)
{
    basic_json_serializer<CharT> serializer(os, pprint);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, const basic_serialization_options<CharT>& options,
          std::basic_ostream<CharT>& os, bool pprint)
{
    basic_json_serializer<CharT> serializer(os, options, pprint);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, std::basic_string<CharT>& s)
{
    basic_json_serializer<CharT,detail::string_writer<CharT>> serializer(s);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, const basic_serialization_options<CharT>& options,
          std::basic_string<CharT>& s)
{
    basic_json_serializer<CharT,detail::string_writer<CharT>> serializer(s, options);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, std::basic_string<CharT>& s, bool pprint)
{
    basic_json_serializer<CharT,detail::string_writer<CharT>> serializer(s, pprint);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, const basic_serialization_options<CharT>& options,
          std::basic_string<CharT,detail::string_writer<CharT>>& s, bool pprint)
{
    basic_json_serializer<CharT> serializer(s, options, pprint);
    encode_json(val, serializer);
}

}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif



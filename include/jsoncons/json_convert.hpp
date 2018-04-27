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
#include <jsoncons/json.hpp>

namespace jsoncons {

// json_convert

template <class T, class Enable = void>
struct json_convert
{
    template <class CharT>
    static T decode(const std::basic_string<CharT>& s)
    {
        basic_json<CharT> j = basic_json<CharT>::parse(s);
        return j.as<T>();
    }

    template <class CharT>
    static void encode(const T& val, std::basic_string<CharT>& s)
    {
        auto j = json_type_traits<basic_json<CharT>, T>::to_json(val);
        j.dump(s);
    }
};

// vector like

template <class T>
struct json_convert<T,
    typename std::enable_if<detail::is_vector_like<T>::value
>::type>
{
    typedef typename std::iterator_traits<typename T::iterator>::value_type value_type;

    template <class CharT>
    static T decode(const std::basic_string<CharT>& s)
    {
        basic_json<CharT> j = basic_json<CharT>::parse(s);
        return j.as<T>();
    }

    template <class CharT>
    static void encode(const T& val, std::basic_string<CharT>& s)
    {
        basic_json_serializer<CharT,detail::string_writer<CharT>> serializer(s);
        serializer.begin_json();
        serializer.begin_array();
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            if (it != std::begin(val))
            {
                s.push_back(',');
            }
            json_convert<value_type>::encode(*it,s);
        }
        serializer.end_array();
        serializer.end_json();
    }
};

// map like

template <class T>
struct json_convert<T,
    typename std::enable_if<detail::is_map_like<T>::value
>::type>
{
    typedef typename T::mapped_type mapped_type;
    typedef typename T::value_type value_type;

    template <class CharT>
    static T decode(const std::basic_string<CharT>& s)
    {
        basic_json<CharT> j = basic_json<CharT>::parse(s);
        return j.as<T>();
    }

    template <class CharT>
    static void encode(const T& val, std::basic_string<CharT>& s)
    {
        basic_json_serializer<CharT,detail::string_writer<CharT>> serializer(s);
        serializer.begin_json();
        serializer.begin_object();
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            if (it != std::begin(val))
            {
                s.push_back(',');
            }
            s.push_back('\"');
            s.append(it->first);
            s.append("\":");
            json_convert<mapped_type>::encode(it->second,s);
        }
        serializer.end_object();
        serializer.end_json();
    }
};

}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif



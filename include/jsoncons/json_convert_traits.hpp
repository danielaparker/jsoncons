// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONCONVERTTRAITS_HPP
#define JSONCONS_JSONCONVERTTRAITS_HPP

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
#include <jsoncons/json_content_handler.hpp>
#include <jsoncons/json_serializing_options.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/json_type_traits.hpp>

namespace jsoncons {

// json_convert_traits

template <class T, class Enable = void>
struct json_convert_traits
{
    template <class CharT>
    static T decode(std::basic_istringstream<CharT>& is,
                    const basic_json_serializing_options<CharT>& options)
    {
        basic_json<CharT> j = basic_json<CharT>::parse(is, options);
        return j. template as<T>();
    }

    template <class CharT>
    static void encode(const T& val, basic_json_content_handler<CharT>& serializer)
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
    static T decode(std::basic_istringstream<CharT>& is,
                    const basic_json_serializing_options<CharT>& options)
    {
        basic_json<CharT> j = basic_json<CharT>::parse(is, options);
        return j. template as<T>();
    }

    template <class CharT>
    static void encode(const T& val, basic_json_content_handler<CharT>& serializer)
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
    static std::array<T, N> decode(std::basic_istringstream<CharT>& is,
                                   const basic_json_serializing_options<CharT>& options)
    {
        basic_json<CharT> j = basic_json<CharT>::parse(is,options);
        return j. template as<std::array<T, N>>();
    }

    template <class CharT>
    static void encode(const std::array<T, N>& val, std::basic_string<CharT>& s)
    {
        basic_json_serializer<CharT,detail::string_writer<CharT>> serializer(s);
        encode(val,serializer);
    }

    template <class CharT>
    static void encode(const std::array<T, N>& val, basic_json_content_handler<CharT>& serializer)
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
    static T decode(std::basic_istringstream<CharT>& is,
                    const basic_json_serializing_options<CharT>& options)
    {
        basic_json<CharT> j = basic_json<CharT>::parse(is, options);
        return j. template as<T>();
    }

    template <class CharT>
    static void encode(const T& val, basic_json_content_handler<CharT>& serializer)
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

// std::tuple

namespace detail { namespace streaming {

template<size_t Pos, class Tuple>
struct tuple_helper
{
    using element_type = typename std::tuple_element<std::tuple_size<Tuple>::value - Pos, Tuple>::type;
    using next = tuple_helper<Pos - 1, Tuple>;
    
    template <class CharT>
    static void encode(const Tuple& tuple, basic_json_content_handler<CharT>& handler)
    {
        json_convert_traits<element_type>::encode(std::get<std::tuple_size<Tuple>::value - Pos>(tuple),handler);
        next::encode(tuple, handler);
    }
};

template<class Tuple>
struct tuple_helper<0, Tuple>
{
    template <class CharT>
    static void encode(const Tuple&, basic_json_content_handler<CharT>&)
    {
    }
};

}}

template<typename... E>
struct json_convert_traits<std::tuple<E...>>
{
private:
    using helper = detail::streaming::tuple_helper<sizeof...(E), std::tuple<E...>>;
public:

    template <class CharT>
    static std::tuple<E...> decode(std::basic_istringstream<CharT>& is,
                                   const basic_json_serializing_options<CharT>& options)
    {
        basic_json<CharT> j = basic_json<CharT>::parse(is, options);
        return j. template as<std::tuple<E...>>();
    }

    template <class CharT>
    static void encode(const std::tuple<E...>& val, basic_json_content_handler<CharT>& serializer)
    {
        serializer.begin_array();
        helper::encode(val, serializer);
        serializer.end_array();
    }
};

// decode_json

template <class T, class CharT>
T decode_json(const std::basic_string<CharT>& s)
{
    std::basic_istringstream<CharT> is(s);
    return json_convert_traits<T>::decode(is, basic_json_serializing_options<CharT>());
}

template <class T, class CharT>
T decode_json(const std::basic_string<CharT>& s,
              const basic_json_serializing_options<CharT>& options)
{
    std::basic_istringstream<CharT> is(s);
    return json_convert_traits<T>::decode(is, options);
}

template <class T, class CharT>
T decode_json(std::basic_istringstream<CharT>& is)
{
    return json_convert_traits<T>::decode(is, basic_json_serializing_options<CharT>());
}

template <class T, class CharT>
T decode_json(std::basic_istringstream<CharT>& is,
              const basic_json_serializing_options<CharT>& options)
{
    return json_convert_traits<T>::decode(is, options);
}

// encode_json

template <class T, class CharT>
void encode_json(const T& val, basic_json_content_handler<CharT>& handler)
{
    handler.begin_json();
    json_convert_traits<T>::encode(val,handler);
    handler.end_json();
}

template <class T, class CharT>
void encode_fragment(const T& val, basic_json_content_handler<CharT>& handler)
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
void encode_json(const T& val, const basic_json_serializing_options<CharT>& options,
          std::basic_ostream<CharT>& os)
{
    basic_json_serializer<CharT> serializer(os, options);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, std::basic_ostream<CharT>& os, indenting line_indent)
{
    basic_json_serializer<CharT> serializer(os, line_indent);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, const basic_json_serializing_options<CharT>& options,
          std::basic_ostream<CharT>& os, indenting line_indent)
{
    basic_json_serializer<CharT> serializer(os, options, line_indent);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, std::basic_string<CharT>& s)
{
    basic_json_serializer<CharT,detail::string_writer<CharT>> serializer(s);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, const basic_json_serializing_options<CharT>& options,
          std::basic_string<CharT>& s)
{
    basic_json_serializer<CharT,detail::string_writer<CharT>> serializer(s, options);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, std::basic_string<CharT>& s, indenting line_indent)
{
    basic_json_serializer<CharT,detail::string_writer<CharT>> serializer(s, line_indent);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, const basic_json_serializing_options<CharT>& options,
          std::basic_string<CharT,detail::string_writer<CharT>>& s, indenting line_indent)
{
    basic_json_serializer<CharT> serializer(s, options, line_indent);
    encode_json(val, serializer);
}

}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif


// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_SERIALIZATION_TRAITS_HPP
#define JSONCONS_SERIALIZATION_TRAITS_HPP

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

namespace jsoncons {

// serialization_traits

template <class T, class Enable = void>
struct serialization_traits
{
    template <class CharT>
    static void encode(const T&, basic_json_content_handler<CharT>&)
    {
    }
};

// dump

template <class CharT, class T>
void dump(const T& val, basic_json_content_handler<CharT>& handler)
{
    handler.begin_document();
    serialization_traits<T>::encode(val,handler);
    handler.end_document();
}

#if !defined(JSONCONS_NO_DEPRECATED)
template <class CharT, class T>
void dump_body(const T& val, basic_json_content_handler<CharT>& handler)
{
    dump_fragment(val,handler);
}
#endif

template <class CharT, class T>
void dump_fragment(const T& val, basic_json_content_handler<CharT>& handler)
{
    serialization_traits<T>::encode(val,handler);
}

template <class CharT, class T>
void dump(const T& val, std::basic_ostream<CharT>& os)
{
    basic_json_serializer<CharT> serializer(os);
    dump(val, serializer);
}

template <class CharT, class T>
void dump(const T& val, const basic_json_serializing_options<CharT>& options,
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
void dump(const T& val, const basic_json_serializing_options<CharT>& options,
          std::basic_ostream<CharT>& os, bool pprint)
{
    basic_json_serializer<CharT> serializer(os, options, pprint);
    dump(val, serializer);
}

// integer

template<class T>
struct serialization_traits<T,
                          typename std::enable_if<detail::is_integer_like<T>::value
>::type>
{
    template <class CharT>
    static void encode(T val, basic_json_content_handler<CharT>& handler)
    {
        handler.integer_value(val);
    }
};

// uinteger

template<class T>
struct serialization_traits<T,
                          typename std::enable_if<detail::is_uinteger_like<T>::value
>::type>
{
    template <class CharT>
    static void encode(T val, basic_json_content_handler<CharT>& handler)
    {
        handler.uinteger_value(val);
    }
};

// double

template<class T>
struct serialization_traits<T,
                          typename std::enable_if<detail::is_floating_point_like<T>::value
>::type>
{
    template <class CharT>
    static void encode(T val, basic_json_content_handler<CharT>& handler)
    {
        handler.double_value(val);
    }
};

// bool

template<>
struct serialization_traits<bool>
{
    template <class CharT>
    static void encode(bool val, basic_json_content_handler<CharT>& handler)
    {
        handler.bool_value(val);
    }
};

// string

template<class T>
struct serialization_traits<T,
    typename std::enable_if<detail::is_string_like<T>::value
>::type>
{
    template <class CharT>
    static void encode(const T& val, basic_json_content_handler<CharT>& handler)
    {
        handler.string_value(val);
    }
};

/*template<>
struct serialization_traits<typename type_wrapper<CharT>::const_pointer_type>
{
    template <class CharT>
    static void encode(typename type_wrapper<CharT>::const_pointer_type val, basic_json_content_handler<CharT>& handler)
    {
        handler.string_value(val);
    }
};*/

// sequence container (except string and array)

template<class T>
struct serialization_traits<T,
    typename std::enable_if<detail::is_vector_like<T>::value
>::type>
{
    typedef typename std::iterator_traits<typename T::iterator>::value_type value_type;

    template <class CharT>
    static void encode(const T& val, basic_json_content_handler<CharT>& handler)
    {
        handler.begin_array();
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            serialization_traits<value_type>::encode(*it,handler);
        }
        handler.end_array();
    }
};

// std::array

template<class T, size_t N>
struct serialization_traits<std::array<T,N>>
{
    typedef typename std::array<T,N>::value_type value_type;
public:
   
    template <class CharT>
    static void encode(const std::array<T, N>& val, basic_json_content_handler<CharT>& handler)
    {
        handler.begin_array();
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            serialization_traits<value_type>::encode(*it,handler);
        }
        handler.end_array();
    }
};

// associative container

template<class T>
struct serialization_traits<T,
    typename std::enable_if<detail::is_map_like<T>::value
>::type>
{
    typedef typename T::mapped_type mapped_type;
    typedef typename T::value_type value_type;

    template <class CharT>
    static void encode(const T& val, basic_json_content_handler<CharT>& handler)
    {
        handler.begin_object();
        for (auto it = std::begin(val); it != std::end(val); ++it)
        {
            handler.name(it->first);
            serialization_traits<mapped_type>::encode(it->second,handler);
        }
        handler.end_object();
    }
};

namespace detail { namespace streaming {

template<size_t Pos, class Tuple>
struct tuple_helper
{
    using element_type = typename std::tuple_element<std::tuple_size<Tuple>::value - Pos, Tuple>::type;
    using next = tuple_helper<Pos - 1, Tuple>;
    
    template <class CharT>
    static void encode(const Tuple& tuple, basic_json_content_handler<CharT>& handler)
    {
        serialization_traits<element_type>::encode(std::get<std::tuple_size<Tuple>::value - Pos>(tuple),handler);
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
struct serialization_traits<std::tuple<E...>>
{
private:
    using helper = detail::streaming::tuple_helper<sizeof...(E), std::tuple<E...>>;

public:
    template <class CharT>
    static void encode(const std::tuple<E...>& value, basic_json_content_handler<CharT>& handler)
    {
        handler.begin_array();
        helper::encode(value, handler);
        handler.end_array();
    }
};

template<class T1, class T2>
struct serialization_traits<std::pair<T1,T2>>
{
public:
   
    template <class CharT>
    static void encode(const std::pair<T1,T2>& value, basic_json_content_handler<CharT>& handler)
    {
        handler.begin_array();
        serialization_traits<T1>::encode(value.first, handler);
        serialization_traits<T2>::encode(value.second, handler);
        handler.end_array();
    }
};

#if !defined(JSONCONS_NO_DEPRECATED)
template<class T>
struct serialization_traits<std::shared_ptr<T>>
{
public:
   
    template <class CharT>
    static void encode(std::shared_ptr<T> p, basic_json_content_handler<CharT>& handler)
    {
        serialization_traits<T>::encode(*p, handler);
    }
};
#endif

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif



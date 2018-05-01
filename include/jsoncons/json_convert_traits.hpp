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
#include <jsoncons/json_output_handler.hpp>
#include <jsoncons/serialization_options.hpp>
#include <jsoncons/json_serializer.hpp>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/json_type_traits.hpp>
#include <jsoncons/detail/json_convert_traits.hpp>

namespace jsoncons {

// json_convert_traits

template <class T, class Enable = void>
struct json_convert_traits
{
    template <class CharT>
    static T decode(const std::basic_string<CharT>& s)
    {
        return detail::json_convert_traits<T>::decode(s);
    }

    template <class CharT>
    static void encode(const T& val, basic_json_output_handler<CharT>& serializer)
    {
        detail::json_convert_traits<T>::encode(val, serializer);
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
void encode_fragment(const T& val, basic_json_output_handler<CharT>& handler)
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
void encode_json(const T& val, std::basic_ostream<CharT>& os, pretty_printer pprinter)
{
    basic_json_serializer<CharT> serializer(os, true);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, const basic_serialization_options<CharT>& options,
          std::basic_ostream<CharT>& os, pretty_printer pprinter)
{
    basic_json_serializer<CharT> serializer(os, options, true);
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
void encode_json(const T& val, std::basic_string<CharT>& s, pretty_printer pprinter)
{
    basic_json_serializer<CharT,detail::string_writer<CharT>> serializer(s, true);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, const basic_serialization_options<CharT>& options,
          std::basic_string<CharT,detail::string_writer<CharT>>& s, pretty_printer pprinter)
{
    basic_json_serializer<CharT> serializer(s, options, true);
    encode_json(val, serializer);
}

}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif



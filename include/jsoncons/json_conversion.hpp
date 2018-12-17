// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_CONVERSION_HPP
#define JSONCONS_JSON_CONVERSION_HPP

#include <iostream>
#include <string>
#include <tuple>
#include <memory>
#include <istream> // std::basic_istream
#include <jsoncons/conversion_traits.hpp>
#include <jsoncons/json_staj_reader.hpp>

namespace jsoncons {

// decode_json

template <class T, class CharT>
T decode_json(const std::basic_string<CharT>& s)
{
    basic_json_staj_reader<CharT> reader(s);
    T val;
    decode_stream(reader, val);
    return val;
}

template <class T, class CharT>
T decode_json(const std::basic_string<CharT>& s,
              const basic_json_options<CharT>& options)
{
    basic_json_staj_reader<CharT> reader(s, options);
    T val;
    decode_stream(reader, val);
    return val;
}

template <class T, class CharT>
T decode_json(std::basic_istream<CharT>& is)
{
    basic_json_staj_reader<CharT> reader(is);
    T val;
    decode_stream(reader, val);
    return val;
}

template <class T, class CharT>
T decode_json(std::basic_istream<CharT>& is,
              const basic_json_options<CharT>& options)
{
    basic_json_staj_reader<CharT> reader(is, options);
    T val;
    decode_stream(reader, val);
    return val;
}

// encode_json

template <class T, class CharT>
void encode_json(const T& val, basic_json_content_handler<CharT>& writer)
{
    encode_stream(val, writer);
    writer.flush();
}

#if !defined(JSONCONS_NO_DEPRECATED)
template <class T, class CharT>
void encode_fragment(const T& val, basic_json_content_handler<CharT>& writer)
{
    encode_stream(val, writer);
}
#endif

template <class T, class CharT>
void encode_json(const T& val, std::basic_ostream<CharT>& os)
{
    basic_json_compressed_serializer<CharT> serializer(os);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, const basic_json_options<CharT>& options,
          std::basic_ostream<CharT>& os)
{
    basic_json_compressed_serializer<CharT> serializer(os, options);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, std::basic_ostream<CharT>& os, indenting line_indent)
{
    if (line_indent == indenting::indent)
    {
        basic_json_serializer<CharT> serializer(os);
        encode_json(val, serializer);
    }
    else
    {
        basic_json_compressed_serializer<CharT> serializer(os);
        encode_json(val, serializer);
    }
}

template <class T, class CharT>
void encode_json(const T& val, const basic_json_options<CharT>& options,
          std::basic_ostream<CharT>& os, indenting line_indent)
{
    if (line_indent == indenting::indent)
    {
        basic_json_serializer<CharT> serializer(os, options);
        encode_json(val, serializer);
    }
    else
    {
        basic_json_compressed_serializer<CharT> serializer(os, options);
        encode_json(val, serializer);
    }
}

template <class T, class CharT>
void encode_json(const T& val, std::basic_string<CharT>& s)
{
    basic_json_compressed_serializer<CharT,jsoncons::string_result<std::basic_string<CharT>>> serializer(s);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, const basic_json_options<CharT>& options,
          std::basic_string<CharT>& s)
{
    basic_json_compressed_serializer<CharT,jsoncons::string_result<std::basic_string<CharT>>> serializer(s, options);
    encode_json(val, serializer);
}

template <class T, class CharT>
void encode_json(const T& val, std::basic_string<CharT>& s, indenting line_indent)
{
    if (line_indent == indenting::indent)
    {
        basic_json_serializer<CharT,jsoncons::string_result<std::basic_string<CharT>>> serializer(s);
        encode_json(val, serializer);
    }
    else
    {
        basic_json_compressed_serializer<CharT,jsoncons::string_result<std::basic_string<CharT>>> serializer(s);
        encode_json(val, serializer);
    }
}

template <class T, class CharT>
void encode_json(const T& val, const basic_json_options<CharT>& options,
                 std::basic_string<CharT,jsoncons::string_result<std::basic_string<CharT>>>& s, indenting line_indent)
{
    if (line_indent == indenting::indent)
    {
        basic_json_serializer<CharT> serializer(s, options);
        encode_json(val, serializer);
    }
    else
    {
        basic_json_compressed_serializer<CharT> serializer(s, options);
        encode_json(val, serializer);
    }
}

}

#endif


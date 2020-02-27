// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DECODE_JSON_HPP
#define JSONCONS_DECODE_JSON_HPP

#include <iostream>
#include <string>
#include <tuple>
#include <memory>
#include <istream> // std::basic_istream
#include <jsoncons/deser_traits.hpp>
#include <jsoncons/json_cursor.hpp>

namespace jsoncons {

    // decode_json

    template <class T, class CharT>
    typename std::enable_if<is_basic_json_class<T>::value,T>::type
    decode_json(const std::basic_string<CharT>& s,
                const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
    {
        jsoncons::json_decoder<T> decoder;
        basic_json_reader<CharT, string_source<CharT>> reader(s, decoder, options);
        reader.read();
        return decoder.get_result();
    }

    template <class T, class CharT>
    typename std::enable_if<!is_basic_json_class<T>::value,T>::type
    decode_json(const std::basic_string<CharT>& s,
                const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
    {
        basic_json_cursor<CharT> cursor(s, options);
        jsoncons::json_decoder<basic_json<CharT>> decoder;
        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    template <class T, class CharT>
    typename std::enable_if<is_basic_json_class<T>::value,T>::type
    decode_json(std::basic_istream<CharT>& is,
                const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
    {
        jsoncons::json_decoder<T> decoder;
        basic_json_reader<CharT, stream_source<CharT>> reader(is, decoder, options);
        reader.read();
        return decoder.get_result();
    }

    template <class T, class CharT>
    typename std::enable_if<!is_basic_json_class<T>::value,T>::type
    decode_json(std::basic_istream<CharT>& is,
                const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
    {
        basic_json_cursor<CharT> cursor(is, options);
        json_decoder<basic_json<CharT>> decoder{};

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }
    template <class T, class CharT, class ImplementationPolicy, class Allocator>
    T decode_json(const std::basic_string<CharT>& s,
                  const basic_json_decode_options<CharT>& options,
                  const basic_json<CharT,ImplementationPolicy,Allocator>& context_j)
    {
        basic_json_cursor<CharT> cursor(s, options);
        json_decoder<basic_json<CharT,ImplementationPolicy,Allocator>> decoder(context_j.get_allocator());

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    template <class T, class CharT, class ImplementationPolicy, class Allocator>
    T decode_json(std::basic_istream<CharT>& is,
                  const basic_json_decode_options<CharT>& options,
                  const basic_json<CharT,ImplementationPolicy,Allocator>& context_j)
    {
        basic_json_cursor<CharT> cursor(is, options);
        json_decoder<basic_json<CharT,ImplementationPolicy,Allocator>> decoder(context_j.get_allocator());

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    #if !defined(JSONCONS_NO_DEPRECATED)
    template <class T, class CharT, class ImplementationPolicy, class Allocator>
    JSONCONS_DEPRECATED_MSG("Instead, use decode_json(const std::basic_string<CharT>&, const basic_json_decode_options<CharT>&, const basic_json<CharT,ImplementationPolicy,Allocator>&)")
    T decode_json(const basic_json<CharT,ImplementationPolicy,Allocator>& context_j,
                  const std::basic_string<CharT>& s,
                  const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
    {
        basic_json_cursor<CharT> cursor(s, options);
        json_decoder<basic_json<CharT,ImplementationPolicy,Allocator>> decoder(context_j.get_allocator());

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }

    template <class T, class CharT, class ImplementationPolicy, class Allocator>
    JSONCONS_DEPRECATED_MSG("Instead, use decode_json(const std::basic_istream<CharT>&, const basic_json_decode_options<CharT>&, const basic_json<CharT,ImplementationPolicy,Allocator>&)")
    T decode_json(const basic_json<CharT,ImplementationPolicy,Allocator>& context_j,
                  std::basic_istream<CharT>& is,
                  const basic_json_decode_options<CharT>& options = basic_json_decode_options<CharT>())
    {
        basic_json_cursor<CharT> cursor(is, options);
        json_decoder<basic_json<CharT,ImplementationPolicy,Allocator>> decoder(context_j.get_allocator());

        std::error_code ec;
        T val = deser_traits<T>::deserialize(cursor, decoder, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec, cursor.context().line(), cursor.context().column()));
        }
        return val;
    }
    #endif

} // jsoncons

#endif


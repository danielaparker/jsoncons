// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_ENCODE_JSON_HPP
#define JSONCONS_ENCODE_JSON_HPP

#include <iostream>
#include <string>
#include <tuple>
#include <memory>
#include <istream> // std::basic_istream
#include <jsoncons/ser_traits.hpp>
#include <jsoncons/json_cursor.hpp>

namespace jsoncons {

    // to string

    template <class T, class CharT>
    void encode_json(const T& val, 
                     std::basic_string<CharT>& s, 
                     indenting line_indent = indenting::no_indent)
    {
        encode_json(val, s, basic_json_encode_options<CharT>(), line_indent);
    }

    template <class T, class CharT>
    typename std::enable_if<is_basic_json_class<T>::value>::type
    encode_json(const T& val,
                std::basic_string<CharT>& s, 
                const basic_json_encode_options<CharT>& options, 
                indenting line_indent = indenting::no_indent)
    {
        if (line_indent == indenting::indent)
        {
            basic_json_encoder<CharT,jsoncons::string_sink<std::basic_string<CharT>>> encoder(s, options);
            val.dump(encoder);
        }
        else
        {
            basic_json_compressed_encoder<CharT, jsoncons::string_sink<std::basic_string<CharT>>> encoder(s, options);
            val.dump(encoder);
        }
    }

    template <class T, class CharT>
    typename std::enable_if<!is_basic_json_class<T>::value>::type
    encode_json(const T& val,
                std::basic_string<CharT>& s, 
                const basic_json_encode_options<CharT>& options, 
                indenting line_indent = indenting::no_indent)
    {
        if (line_indent == indenting::indent)
        {
            basic_json_encoder<CharT,jsoncons::string_sink<std::basic_string<CharT>>> encoder(s, options);
            encode_json(val, encoder);
        }
        else
        {
            basic_json_compressed_encoder<CharT, jsoncons::string_sink<std::basic_string<CharT>>> encoder(s, options);
            encode_json(val, encoder);
        }
    }

    // to stream

    template <class T, class CharT>
    void encode_json(const T& val, 
                     std::basic_ostream<CharT>& os, 
                     indenting line_indent = indenting::no_indent)
    {
        encode_json(val, os, basic_json_encode_options<CharT>(), line_indent);
    }

    template <class T, class CharT>
    typename std::enable_if<is_basic_json_class<T>::value>::type
    encode_json(const T& val,
                std::basic_ostream<CharT>& os, 
                const basic_json_encode_options<CharT>& options, 
                indenting line_indent = indenting::no_indent)
    {
        if (line_indent == indenting::indent)
        {
            basic_json_encoder<CharT> encoder(os, options);
            val.dump(encoder);
        }
        else
        {
            basic_json_compressed_encoder<CharT> encoder(os, options);
            val.dump(encoder);
        }
    }

    template <class T, class CharT>
    typename std::enable_if<!is_basic_json_class<T>::value>::type
    encode_json(const T& val,
                std::basic_ostream<CharT>& os, 
                const basic_json_encode_options<CharT>& options, 
                indenting line_indent = indenting::no_indent)
    {
        if (line_indent == indenting::indent)
        {
            basic_json_encoder<CharT> encoder(os, options);
            encode_json(val, encoder);
        }
        else
        {
            basic_json_compressed_encoder<CharT> encoder(os, options);
            encode_json(val, encoder);
        }
    }

    template <class T, class CharT>
    void encode_json(const T& val, 
                     basic_json_content_handler<CharT>& encoder)
    {
        std::error_code ec;
        ser_traits<T>::serialize(val, encoder, basic_json<CharT>(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
        encoder.flush();
    }

    template <class T, class CharT, class ImplementationPolicy, class Allocator>
    typename std::enable_if<!is_basic_json_class<T>::value>::type
    encode_json(const T& val,
                std::basic_string<CharT>& s,
                const basic_json_encode_options<CharT>& options, 
                indenting line_indent, 
                const basic_json<CharT,ImplementationPolicy,Allocator>& context_j)
    { 
        if (line_indent == indenting::indent)
        {
            basic_json_encoder<CharT,jsoncons::string_sink<std::basic_string<CharT>>> encoder(s, options);
            encode_json(val, encoder, context_j);
        }
        else
        {
            basic_json_compressed_encoder<CharT,jsoncons::string_sink<std::basic_string<CharT>>> encoder(s, options);
            encode_json(val, encoder, context_j);
        }
    }


    template <class T, class CharT, class ImplementationPolicy, class Allocator>
    typename std::enable_if<!is_basic_json_class<T>::value>::type
    encode_json(const T& val,
                std::basic_ostream<CharT>& os, 
                const basic_json_encode_options<CharT>& options, 
                indenting line_indent,
                const basic_json<CharT,ImplementationPolicy,Allocator>& context_j)
    {
        if (line_indent == indenting::indent)
        {
            basic_json_encoder<CharT> encoder(os, options);
            encode_json(val, encoder, context_j);
        }
        else
        {
            basic_json_compressed_encoder<CharT> encoder(os, options);
            encode_json(val, encoder, context_j);
        }
    }

    template <class T, class CharT, class ImplementationPolicy, class Allocator>
    typename std::enable_if<!is_basic_json_class<T>::value>::type
    encode_json(const T& val,
                basic_json_content_handler<CharT>& encoder,
                const basic_json<CharT, ImplementationPolicy, Allocator>& context_j)
    {
        std::error_code ec;
        ser_traits<T>::serialize(val, encoder, context_j, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
        encoder.flush();
    }

} // jsoncons

#endif


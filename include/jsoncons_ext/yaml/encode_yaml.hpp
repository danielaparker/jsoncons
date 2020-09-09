// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_YAML_ENCODE_YAML_HPP
#define JSONCONS_YAML_ENCODE_YAML_HPP

#include <iostream>
#include <string>
#include <tuple>
#include <memory>
#include <istream> // std::basic_istream
#include <jsoncons/encode_traits.hpp>
#include <jsoncons/yaml_cursor.hpp>

namespace jsoncons {
namespace yaml {

    // to string

    template <class T, class Container>
    typename std::enable_if<is_basic_json<T>::value &&
                            jsoncons::detail::is_back_insertable_char_container<Container>::value>::type
    encode_yaml(const T& val,
                Container& s, 
                const basic_yaml_encode_options<typename Container::value_type>& options = 
                    basic_yaml_encode_options<typename Container::value_type>())
    {
        using char_type = typename Container::value_type;

        basic_compact_yaml_encoder<char_type, jsoncons::string_sink<Container>> encoder(s, options);
        val.dump(encoder);
    }

    template <class T, class Container>
    typename std::enable_if<!is_basic_json<T>::value &&
                            jsoncons::detail::is_back_insertable_char_container<Container>::value>::type
    encode_yaml(const T& val,
                Container& s, 
                const basic_yaml_encode_options<typename Container::value_type>& options = basic_yaml_encode_options<typename Container::value_type>())
    {
        using char_type = typename Container::value_type;

        basic_compact_yaml_encoder<char_type, jsoncons::string_sink<Container>> encoder(s, options);
        encode_yaml(val, encoder);
    }

    // to stream

    template <class T, class CharT>
    typename std::enable_if<is_basic_json<T>::value>::type
    encode_yaml(const T& val,
                std::basic_ostream<CharT>& os, 
                const basic_yaml_encode_options<CharT>& options = basic_yaml_encode_options<CharT>())
    {
        basic_compact_yaml_encoder<CharT> encoder(os, options);
        val.dump(encoder);
    }

    template <class T, class CharT>
    typename std::enable_if<!is_basic_json<T>::value>::type
    encode_yaml(const T& val,
                std::basic_ostream<CharT>& os, 
                const basic_yaml_encode_options<CharT>& options = basic_yaml_encode_options<CharT>())
    {
        basic_compact_yaml_encoder<CharT> encoder(os, options);
        encode_yaml(val, encoder);
    }

    // encode_yaml_pretty

    template <class T, class Container>
    typename std::enable_if<is_basic_json<T>::value &&
                            jsoncons::detail::is_back_insertable_char_container<Container>::value>::type
    encode_yaml_pretty(const T& val,
                       Container& s, 
                       const basic_yaml_encode_options<typename Container::value_type>& options = basic_yaml_encode_options<typename Container::value_type>())
    {
        using char_type = typename Container::value_type;

        basic_yaml_encoder<char_type,jsoncons::string_sink<Container>> encoder(s, options);
        val.dump(encoder);
    }

    template <class T, class Container>
    typename std::enable_if<!is_basic_json<T>::value &&
                            jsoncons::detail::is_back_insertable_char_container<Container>::value>::type
    encode_yaml_pretty(const T& val,
                       Container& s, 
                       const basic_yaml_encode_options<typename Container::value_type>& options = basic_yaml_encode_options<typename Container::value_type>())
    {
        using char_type = typename Container::value_type;
        basic_yaml_encoder<char_type,jsoncons::string_sink<Container>> encoder(s, options);
        encode_yaml(val, encoder);
    }

    template <class T, class CharT>
    typename std::enable_if<is_basic_json<T>::value>::type
    encode_yaml_pretty(const T& val,
                       std::basic_ostream<CharT>& os, 
                       const basic_yaml_encode_options<CharT>& options = basic_yaml_encode_options<CharT>())
    {
        basic_yaml_encoder<CharT> encoder(os, options);
        val.dump(encoder);
    }

    template <class T, class CharT>
    typename std::enable_if<!is_basic_json<T>::value>::type
    encode_yaml_pretty(const T& val,
                       std::basic_ostream<CharT>& os, 
                       const basic_yaml_encode_options<CharT>& options = basic_yaml_encode_options<CharT>())
    {
        basic_yaml_encoder<CharT> encoder(os, options);
        encode_yaml(val, encoder);
    }

    template <class T, class CharT>
    void encode_yaml(const T& val, basic_json_visitor<CharT>& encoder)
    {
        std::error_code ec;
        encode_traits<T,CharT>::encode(val, encoder, basic_json<CharT>(), ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
        encoder.flush();
    }

    template <class T, class Container, class TempAllocator>
    void encode_yaml(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                     const T& val, 
                     Container& s, 
                     indenting line_indent = indenting::no_indent)
    {
        encode_yaml(temp_allocator_arg, temp_alloc, val, s, basic_yaml_encode_options<typename Container::value_type>(), line_indent);
    }

// legacy

    template <class T, class Container>
    void encode_yaml(const T& val, Container& s, indenting line_indent)
    {
        if (line_indent == indenting::indent)
        {
            encode_yaml_pretty(val,s);
        }
        else
        {
            encode_yaml(val,s);
        }
    }

    template <class T, class Container>
    void encode_yaml(const T& val,
                     Container& s, 
                     const basic_yaml_encode_options<typename Container::value_type>& options, 
                     indenting line_indent)
    {
        if (line_indent == indenting::indent)
        {
            encode_yaml_pretty(val,s,options);
        }
        else
        {
            encode_yaml(val,s,options);
        }
    }

    template <class T, class CharT>
    void encode_yaml(const T& val,
                     std::basic_ostream<CharT>& os, 
                     indenting line_indent)
    {
        if (line_indent == indenting::indent)
        {
            encode_yaml_pretty(val, os);
        }
        else
        {
            encode_yaml(val, os);
        }
    }

    template <class T, class CharT>
    void encode_yaml(const T& val,
                     std::basic_ostream<CharT>& os, 
                     const basic_yaml_encode_options<CharT>& options, 
                     indenting line_indent)
    {
        if (line_indent == indenting::indent)
        {
            encode_yaml_pretty(val, os, options);
        }
        else
        {
            encode_yaml(val, os, options);
        }
    }

//end legacy

    template <class T, class Container, class TempAllocator>
    typename std::enable_if<is_basic_json<T>::value &&
                            jsoncons::detail::is_back_insertable_char_container<Container>::value>::type
    encode_yaml(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& val,
                Container& s, 
                const basic_yaml_encode_options<typename Container::value_type>& options, 
                indenting line_indent = indenting::no_indent)
    {
        using char_type = typename Container::value_type;
        if (line_indent == indenting::indent)
        {
            basic_yaml_encoder<char_type,jsoncons::string_sink<Container>,TempAllocator> encoder(s, options, temp_alloc);
            val.dump(encoder);
        }
        else
        {
            basic_compact_yaml_encoder<char_type, jsoncons::string_sink<Container>,TempAllocator> encoder(s, options, temp_alloc);
            val.dump(encoder);
        }
    }

    template <class T, class Container, class TempAllocator>
    typename std::enable_if<!is_basic_json<T>::value &&
                            jsoncons::detail::is_back_insertable_char_container<Container>::value>::type
    encode_yaml(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& val,
                Container& s,
                const basic_yaml_encode_options<typename Container::value_type>& options, 
                indenting line_indent)
    { 
        using char_type = typename Container::value_type;
        if (line_indent == indenting::indent)
        {
            basic_yaml_encoder<char_type,jsoncons::string_sink<Container>,TempAllocator> encoder(s, options, temp_alloc);
            encode_yaml(temp_allocator_arg, temp_alloc, val, encoder);
        }
        else
        {
            basic_compact_yaml_encoder<char_type,jsoncons::string_sink<Container>,TempAllocator> encoder(s, options, temp_alloc);
            encode_yaml(temp_allocator_arg, temp_alloc, val, encoder);
        }
    }

    template <class T, class CharT, class TempAllocator>
    void encode_yaml(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                     const T& val, 
                     std::basic_ostream<CharT>& os, 
                     indenting line_indent = indenting::no_indent)
    {
        encode_yaml(temp_allocator_arg, temp_alloc, val, os, basic_yaml_encode_options<CharT>(), line_indent);
    }

    template <class T, class CharT, class TempAllocator>
    typename std::enable_if<is_basic_json<T>::value>::type
    encode_yaml(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& val,
                std::basic_ostream<CharT>& os, 
                const basic_yaml_encode_options<CharT>& options, 
                indenting line_indent = indenting::no_indent)
    {
        if (line_indent == indenting::indent)
        {
            basic_yaml_encoder<CharT,jsoncons::stream_sink<CharT>,TempAllocator> encoder(os, options, temp_alloc);
            val.dump(encoder);
        }
        else
        {
            basic_compact_yaml_encoder<CharT,jsoncons::stream_sink<CharT>,TempAllocator> encoder(os, options, temp_alloc);
            val.dump(encoder);
        }
    }

    template <class T, class CharT, class TempAllocator>
    typename std::enable_if<!is_basic_json<T>::value>::type
    encode_yaml(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& val,
                std::basic_ostream<CharT>& os, 
                const basic_yaml_encode_options<CharT>& options, 
                indenting line_indent)
    {
        if (line_indent == indenting::indent)
        {
            basic_yaml_encoder<CharT> encoder(os, options);
            encode_yaml(temp_allocator_arg, temp_alloc, val, encoder);
        }
        else
        {
            basic_compact_yaml_encoder<CharT> encoder(os, options);
            encode_yaml(temp_allocator_arg, temp_alloc, val, encoder);
        }
    }

    template <class T, class CharT, class TempAllocator>
    typename std::enable_if<!is_basic_json<T>::value>::type
    encode_yaml(temp_allocator_arg_t, const TempAllocator& temp_alloc,
                const T& val,
                basic_json_visitor<CharT>& encoder)
    {
        std::error_code ec;
        basic_json<CharT,sorted_policy,TempAllocator> proto(temp_alloc);
        encode_traits<T,CharT>::encode(val, encoder, proto, ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
        encoder.flush();
    }

} // namespace yaml
} // namespace jsoncons

#endif


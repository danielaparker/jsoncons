// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_ENCODE_JSON_HPP
#define JSONCONS_ENCODE_JSON_HPP

#include <iostream>
#include <system_error>
#include <tuple>

#include <jsoncons/reflect/encode_traits.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_cursor.hpp>
#include <jsoncons/basic_json.hpp>
#include <jsoncons/write_result.hpp>

namespace jsoncons {

// to string

template <typename T,typename CharContainer>
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
    ext_traits::is_back_insertable_char_container<CharContainer>::value>::type
try_encode_json(const T& val, CharContainer& cont, 
    const basic_json_encode_options<typename CharContainer::value_type>& options = 
    basic_json_encode_options<typename CharContainer::value_type>(),
    indenting indent = indenting::no_indent)
{
    using char_type = typename CharContainer::value_type;

    if (indent == indenting::no_indent)
    {
        basic_compact_json_encoder<char_type, jsoncons::string_sink<CharContainer>> encoder(cont, options);
        val.dump(encoder);
    }
    else
    {
        basic_json_encoder<char_type,jsoncons::string_sink<CharContainer>> encoder(cont, options);
        val.dump(encoder);
    }
}

template <typename T,typename CharContainer>
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_back_insertable_char_container<CharContainer>::value>::type
try_encode_json(const T& val, CharContainer& cont, 
    const basic_json_encode_options<typename CharContainer::value_type>& options = 
    basic_json_encode_options<typename CharContainer::value_type>(),
    indenting indent = indenting::no_indent)
{
    using char_type = typename CharContainer::value_type;

    if (indent == indenting::no_indent)
    {
        basic_compact_json_encoder<char_type, jsoncons::string_sink<CharContainer>> encoder(cont, options);
        try_encode_json(val, encoder);
    }
    else
    {
        basic_json_encoder<char_type, jsoncons::string_sink<CharContainer>> encoder(cont, options);
        try_encode_json(val, encoder);
    }
}

// to stream

template <typename T,typename CharT>
typename std::enable_if<ext_traits::is_basic_json<T>::value>::type
try_encode_json(const T& val, std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options = basic_json_encode_options<CharT>(),
    indenting indent = indenting::no_indent)
{
    if (indent == indenting::no_indent)
    {
        basic_compact_json_encoder<CharT> encoder(os, options);
        val.dump(encoder);
    }
    else
    {
        basic_json_encoder<CharT> encoder(os, options);
        val.dump(encoder);
    }
}

template <typename T,typename CharT>
typename std::enable_if<!ext_traits::is_basic_json<T>::value>::type
try_encode_json(const T& val, std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options = basic_json_encode_options<CharT>(),
    indenting indent = indenting::no_indent)
{
    if (indent == indenting::no_indent)
    {
        basic_compact_json_encoder<CharT> encoder(os, options);
        try_encode_json(val, encoder);
    }
    else
    {
        basic_json_encoder<CharT> encoder(os, options);
        try_encode_json(val, encoder);
    }
}

// to string with allocator_set

template <typename T,typename CharContainer,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
    ext_traits::is_back_insertable_char_container<CharContainer>::value>::type
try_encode_json(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, CharContainer& cont, 
    const basic_json_encode_options<typename CharContainer::value_type>& options = 
    basic_json_encode_options<typename CharContainer::value_type>(),
    indenting indent = indenting::no_indent)
{
    using char_type = typename CharContainer::value_type;

    if (indent == indenting::no_indent)
    {
        basic_compact_json_encoder<char_type, jsoncons::string_sink<CharContainer>,TempAlloc> encoder(cont, options, aset.get_temp_allocator());
        val.dump(encoder);
    }
    else
    {
        basic_json_encoder<char_type,jsoncons::string_sink<CharContainer>,TempAlloc> encoder(cont, options, aset.get_temp_allocator());
        val.dump(encoder);
    }
}

template <typename T,typename CharContainer,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_back_insertable_char_container<CharContainer>::value>::type
try_encode_json(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, CharContainer& cont, 
    const basic_json_encode_options<typename CharContainer::value_type>& options = 
    basic_json_encode_options<typename CharContainer::value_type>(),
    indenting indent = indenting::no_indent)
{
    using char_type = typename CharContainer::value_type;

    if (indent == indenting::no_indent)
    {
        basic_compact_json_encoder<char_type, jsoncons::string_sink<CharContainer>,TempAlloc> encoder(cont, options,
            aset.get_temp_allocator());
        try_encode_json(val, encoder);
    }
    else
    {
        basic_json_encoder<char_type, jsoncons::string_sink<CharContainer>, TempAlloc> encoder(cont, options, 
            aset.get_temp_allocator());
        try_encode_json(val, encoder);
    }
}

// to stream with allocator_set

template <typename T,typename CharT,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_basic_json<T>::value>::type
try_encode_json(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options = basic_json_encode_options<CharT>(),
    indenting indent = indenting::no_indent)
{
    if (indent == indenting::no_indent)
    {
        basic_compact_json_encoder<CharT,TempAlloc> encoder(os, options, aset.get_temp_allocator());
        val.dump(encoder);
    }
    else
    {
        basic_json_encoder<CharT,TempAlloc> encoder(os, options, aset.get_temp_allocator());
        val.dump(encoder);
    }
}

template <typename T,typename CharT,typename Alloc,typename TempAlloc >
typename std::enable_if<!ext_traits::is_basic_json<T>::value>::type
try_encode_json(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options = basic_json_encode_options<CharT>(),
    indenting indent = indenting::no_indent)
{
    if (indent == indenting::no_indent)
    {
        basic_compact_json_encoder<CharT,TempAlloc> encoder(os, options, aset.get_temp_allocator());
        try_encode_json(val, encoder);
    }
    else
    {
        basic_json_encoder<CharT,TempAlloc> encoder(os, options, aset.get_temp_allocator());
        try_encode_json(val, encoder);
    }
}

// to encoder

template <typename T,typename CharT>
void try_encode_json(const T& val, basic_json_visitor<CharT>& encoder)
{
    std::error_code ec;
    reflect::encode_traits<T>::try_encode(make_alloc_set(), val, encoder, ec);
    if (JSONCONS_UNLIKELY(ec))
    {
        JSONCONS_THROW(ser_error(ec));
    }
    encoder.flush();
}

// encode_json_pretty

template <typename T,typename CharContainer>
typename std::enable_if<ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_back_insertable_char_container<CharContainer>::value>::type
encode_json_pretty(const T& val,
    CharContainer& cont, 
    const basic_json_encode_options<typename CharContainer::value_type>& options = basic_json_encode_options<typename CharContainer::value_type>())
{
    using char_type = typename CharContainer::value_type;

    basic_json_encoder<char_type,jsoncons::string_sink<CharContainer>> encoder(cont, options);
    val.dump(encoder);
}

template <typename T,typename CharContainer>
typename std::enable_if<!ext_traits::is_basic_json<T>::value &&
                        ext_traits::is_back_insertable_char_container<CharContainer>::value>::type
encode_json_pretty(const T& val,
    CharContainer& cont, 
    const basic_json_encode_options<typename CharContainer::value_type>& options = basic_json_encode_options<typename CharContainer::value_type>())
{
    using char_type = typename CharContainer::value_type;
    basic_json_encoder<char_type,jsoncons::string_sink<CharContainer>> encoder(cont, options);
    try_encode_json(val, encoder);
}

template <typename T,typename CharT>
typename std::enable_if<ext_traits::is_basic_json<T>::value>::type
encode_json_pretty(const T& val,
    std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options = basic_json_encode_options<CharT>())
{
    basic_json_encoder<CharT> encoder(os, options);
    val.dump(encoder);
}

template <typename T,typename CharT>
typename std::enable_if<!ext_traits::is_basic_json<T>::value>::type
encode_json_pretty(const T& val,
    std::basic_ostream<CharT>& os, 
    const basic_json_encode_options<CharT>& options = basic_json_encode_options<CharT>())
{
    basic_json_encoder<CharT> encoder(os, options);
    try_encode_json(val, encoder);
}

// legacy

template <typename T,typename CharContainer>
void try_encode_json(const T& val, CharContainer& cont, indenting indent)
{
    if (indent == indenting::indent)
    {
        encode_json_pretty(val,cont);
    }
    else
    {
        try_encode_json(val,cont);
    }
}

template <typename T,typename CharT>
void try_encode_json(const T& val,
    std::basic_ostream<CharT>& os, 
    indenting indent)
{
    if (indent == indenting::indent)
    {
        encode_json_pretty(val, os);
    }
    else
    {
        try_encode_json(val, os);
    }
}

//end legacy

template <typename... Args>
void encode_json(Args&& ... args)
{
    /*auto result = */try_encode_json(std::forward<Args>(args)...); 
    /*if (!result)
    {
        JSONCONS_THROW(ser_error(result.error().code(), result.error().line(), result.error().column()));
    }*/
    //return std::move(*result);
}

} // namespace jsoncons

#endif // JSONCONS_ENCODE_JSON_HPP


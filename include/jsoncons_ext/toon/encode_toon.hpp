// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_TOON_ENCODE_TOON_HPP 
#define JSONCONS_TOON_ENCODE_TOON_HPP 

#include <ostream>

#include <jsoncons/basic_json.hpp>
#include <jsoncons_ext/toon/toon_options.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/reflect/encode_traits.hpp>
#include <jsoncons/ser_util.hpp>
#include <jsoncons/allocator_set.hpp>

namespace jsoncons {
namespace toon {

namespace detail {

    inline
    bool is_control_character(uint32_t c)
    {
        return c <= 0x1F || c == 0x7f;
    }

    inline
    bool is_non_ascii_codepoint(uint32_t cp)
    {
        return cp >= 0x80;
    }

    template <typename CharT,typename Sink>
    void escape_string(const CharT* s, std::size_t length,
                         bool escape_all_non_ascii, bool escape_solidus,
                         Sink& sink)
    {
        const CharT* begin = s;
        const CharT* end = s + length;
        for (const CharT* it = begin; it != end; ++it)
        {
            CharT c = *it;
            switch (c)
            {
                case '\\':
                    sink.push_back('\\');
                    sink.push_back('\\');
                    break;
                case '"':
                    sink.push_back('\\');
                    sink.push_back('\"');
                    break;
                case '\b':
                    sink.push_back('\\');
                    sink.push_back('b');
                    break;
                case '\f':
                    sink.push_back('\\');
                    sink.push_back('f');
                    break;
                case '\n':
                    sink.push_back('\\');
                    sink.push_back('n');
                    break;
                case '\r':
                    sink.push_back('\\');
                    sink.push_back('r');
                    break;
                case '\t':
                    sink.push_back('\\');
                    sink.push_back('t');
                    break;
                default:
                    if (escape_solidus && c == '/')
                    {
                        sink.push_back('\\');
                        sink.push_back('/');
                    }
                    else if (is_control_character(c) || escape_all_non_ascii)
                    {
                        // convert to codepoint
                        uint32_t cp;
                        auto r = unicode_traits::to_codepoint(it, end, cp, unicode_traits::conv_flags::strict);
                        if (r.ec != unicode_traits::conv_errc())
                        {
                            JSONCONS_THROW(ser_error(json_errc::illegal_codepoint));
                        }
                        it = r.ptr - 1;
                        if (is_non_ascii_codepoint(cp) || is_control_character(c))
                        {
                            if (cp > 0xFFFF)
                            {
                                cp -= 0x10000;
                                uint32_t first = (cp >> 10) + 0xD800;
                                uint32_t second = ((cp & 0x03FF) + 0xDC00);

                                sink.push_back('\\');
                                sink.push_back('u');
                                sink.push_back(jsoncons::to_hex_character(first >> 12 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(first >> 8 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(first >> 4 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(first & 0x000F));
                                sink.push_back('\\');
                                sink.push_back('u');
                                sink.push_back(jsoncons::to_hex_character(second >> 12 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(second >> 8 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(second >> 4 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(second & 0x000F));
                            }
                            else
                            {
                                sink.push_back('\\');
                                sink.push_back('u');
                                sink.push_back(jsoncons::to_hex_character(cp >> 12 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(cp >> 8 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(cp >> 4 & 0x000F));
                                sink.push_back(jsoncons::to_hex_character(cp & 0x000F));
                            }
                        }
                        else
                        {
                            sink.push_back(c);
                        }
                    }
                    else
                    {
                        sink.push_back(c);
                    }
                    break;
            }
        }
    }

    inline
    byte_string_chars_format resolve_byte_string_chars_format(byte_string_chars_format format1,
        byte_string_chars_format format2,
        byte_string_chars_format default_format = byte_string_chars_format::base64url)
    {
        byte_string_chars_format sink;
        switch (format1)
        {
            case byte_string_chars_format::base16:
            case byte_string_chars_format::base64:
            case byte_string_chars_format::base64url:
                sink = format1;
                break;
            default:
                switch (format2)
                {
                    case byte_string_chars_format::base64url:
                    case byte_string_chars_format::base64:
                    case byte_string_chars_format::base16:
                        sink = format2;
                        break;
                    default: // base64url
                    {
                        sink = default_format;
                        break;
                    }
                }
                break;
        }
        return sink;
    }

    template <typename CharT>
    bool is_number(jsoncons::basic_string_view<CharT> str) 
    {
        int state = 0;

        for (auto c : str)
        {
            switch (state)
            {
                case 0:
                    if (c == '-')
                    {
                        state = 1;
                    }
                    else if (c == '0')
                    {
                        state = 2;
                    }
                    else if (c >= '1' && c <= '9')
                    {
                        state = 3;
                    }
                    else
                    {
                        state = 9;
                    }
                    break;
                case 1: // leading minus
                    if (c == '0')
                    {
                        state = 2;
                    }
                    else if (c >= '1' && c <= '9')
                    {
                        state = 3;
                    }
                    else
                    {
                        state = 9;
                    }
                    break;
                case 2: // after 0
                    if (c == '0')
                    {
                        state = 9;
                    }
                    else if (c == '.')
                    {
                        state = 4;
                    }
                    else if (c >= '1' && c <= '9')
                    {
                        state = 3;
                    }
                    else
                    {
                        state = 9;
                    }
                    break;
                case 3: // expect digits or dot
                    if (c == '.')
                    {
                        state = 4;
                    }
                    else if (!(c >= '0' && c <= '9'))
                    {
                        state = 9;
                    }
                    break;
                case 4: // expect digits
                    if (c >= '0' && c <= '9')
                    {
                        state = 5;
                    }
                    else
                    {
                        state = 9;
                    }
                    break;
                case 5: // expect digits
                    if (!(c >= '0' && c <= '9'))
                    {
                        state = 9;
                    }
                    break;
                default:
                    break;
            }
        }
        if (state == 2 || state == 3 || state == 5)
        {
            return true;
        }
        return false;
    }

} // namespace detail

// encode_toon

#if 0

template <typename T,typename CharT>
typename std::enable_if<ext_traits::is_basic_json<T>::value,write_result>::type
    try_encode_toon(const T& val, basic_json_visitor<CharT>& encoder)
{
    return try_encode_toon(make_alloc_set(), val, encoder);
}

template <typename T,typename Alloc,typename TempAlloc,typename CharT>
typename std::enable_if<ext_traits::is_basic_json<T>::value, write_result>::type
try_encode_toon(const allocator_set<Alloc,TempAlloc>&,
    const T& val, basic_json_visitor<CharT>& encoder)
{
    return val.try_dump(encoder);
}

template <typename T,typename CharT>
typename std::enable_if<!ext_traits::is_basic_json<T>::value,write_result>::type
try_encode_toon(const T& val, basic_json_visitor<CharT>& encoder)
{
    auto r = reflect::encode_traits<T>::try_encode(make_alloc_set(), val, encoder);
    encoder.flush();
    return r;
}

template <typename T, typename Alloc, typename TempAlloc, typename CharT>
typename std::enable_if<!ext_traits::is_basic_json<T>::value, write_result>::type
    try_encode_toon(const allocator_set<Alloc, TempAlloc>& aset,
    const T& val, basic_json_visitor<CharT>& encoder)
{
    auto r = reflect::encode_traits<T>::try_encode(aset, val, encoder);
    encoder.flush();
    return r;
}

// to string

template <typename T,typename CharContainer>
typename std::enable_if<ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type
try_encode_toon(const T& val, CharContainer& cont, 
    const basic_toon_encode_options<typename CharContainer::value_type>& options 
        = basic_toon_encode_options<typename CharContainer::value_type>())
{
    using char_type = typename CharContainer::value_type;

    basic_compact_json_encoder<char_type, jsoncons::string_sink<CharContainer>> encoder(cont, options);
    return try_encode_toon(val, encoder);
}

// to stream

template <typename T,typename CharT>
write_result try_encode_toon(const T& val, std::basic_ostream<CharT>& os, 
    const basic_toon_encode_options<CharT>& options 
        = basic_toon_encode_options<CharT>())
{
    basic_compact_json_encoder<CharT> encoder(os, options);
    return try_encode_toon(val, encoder);
}

// to string with allocator_set

template <typename T,typename CharContainer,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type
try_encode_toon(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, CharContainer& cont, 
    const basic_toon_encode_options<typename CharContainer::value_type>& options 
        = basic_toon_encode_options<typename CharContainer::value_type>())
{
    using char_type = typename CharContainer::value_type;

    basic_compact_json_encoder<char_type, jsoncons::string_sink<CharContainer>,TempAlloc> encoder(cont, options,
        aset.get_temp_allocator());
    return try_encode_toon(aset, val, encoder);
}

// to stream with allocator_set

template <typename T,typename CharT,typename Alloc,typename TempAlloc >
write_result try_encode_toon(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, std::basic_ostream<CharT>& os, 
    const basic_toon_encode_options<CharT>& options 
        = basic_toon_encode_options<CharT>())
{
    basic_compact_json_encoder<CharT,TempAlloc> encoder(os, options, aset.get_temp_allocator());
    return try_encode_toon(aset, val, encoder);
}

// try_encode_toon_pretty

template <typename T,typename CharContainer>
typename std::enable_if<ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type
try_encode_toon_pretty(const T& val,
    CharContainer& cont, 
    const basic_toon_encode_options<typename CharContainer::value_type>& options 
        = basic_toon_encode_options<typename CharContainer::value_type>())
{
    using char_type = typename CharContainer::value_type;
    basic_json_encoder<char_type,jsoncons::string_sink<CharContainer>> encoder(cont, options);
    return try_encode_toon(val, encoder);
}

template <typename T,typename CharT>
write_result try_encode_toon_pretty(const T& val,
    std::basic_ostream<CharT>& os, 
    const basic_toon_encode_options<CharT>& options 
        = basic_toon_encode_options<CharT>())
{
    basic_json_encoder<CharT> encoder(os, options);
    return try_encode_toon(val, encoder);
}

template <typename T,typename CharContainer,typename Alloc,typename TempAlloc>
typename std::enable_if<ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type
try_encode_toon_pretty(const allocator_set<Alloc,TempAlloc>& aset, const T& val,
    CharContainer& cont, 
    const basic_toon_encode_options<typename CharContainer::value_type>& options 
        = basic_toon_encode_options<typename CharContainer::value_type>())
{
    using char_type = typename CharContainer::value_type;
    basic_json_encoder<char_type,jsoncons::string_sink<CharContainer>> encoder(cont, options,
        aset.get_temp_allocator());
    return try_encode_toon(aset, val, encoder);
}

template <typename T,typename CharT,typename Alloc,typename TempAlloc>
write_result try_encode_toon_pretty(const allocator_set<Alloc,TempAlloc>& aset,const T& val,
    std::basic_ostream<CharT>& os, 
    const basic_toon_encode_options<CharT>& options 
        = basic_toon_encode_options<CharT>())
{
    basic_json_encoder<CharT> encoder(os, options, aset.get_temp_allocator());
    return try_encode_toon(aset, val, encoder);
}

// legacy

template <typename T,typename CharContainer>
write_result try_encode_toon(const T& val, CharContainer& cont, indenting indent)
{
    if (indent == indenting::indent)
    {
        return try_encode_toon_pretty(val,cont);
    }
    else
    {
        return try_encode_toon(val,cont);
    }
}

template <typename T,typename CharT>
write_result try_encode_toon(const T& val,
    std::basic_ostream<CharT>& os, 
    indenting indent)
{
    if (indent == indenting::indent)
    {
        return try_encode_toon_pretty(val, os);
    }
    else
    {
        return try_encode_toon(val, os);
    }
}


// to string

template <typename T,typename CharContainer>
typename std::enable_if<ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type
try_encode_toon(const T& val, CharContainer& cont, 
    const basic_toon_encode_options<typename CharContainer::value_type>& options,
    indenting indent)
{
    using char_type = typename CharContainer::value_type;

    if (indent == indenting::no_indent)
    {
        basic_compact_json_encoder<char_type, jsoncons::string_sink<CharContainer>> encoder(cont, options);
        return try_encode_toon(val, encoder);
    }
    else
    {
        basic_json_encoder<char_type, jsoncons::string_sink<CharContainer>> encoder(cont, options);
        return try_encode_toon(val, encoder);
    }
}

// to stream

template <typename T,typename CharT>
write_result try_encode_toon(const T& val, std::basic_ostream<CharT>& os, 
    const basic_toon_encode_options<CharT>& options,
    indenting indent)
{
    if (indent == indenting::no_indent)
    {
        basic_compact_json_encoder<CharT> encoder(os, options);
        return try_encode_toon(val, encoder);
    }
    else
    {
        basic_json_encoder<CharT> encoder(os, options);
        return try_encode_toon(val, encoder);
    }
}

// to string with allocator_set

template <typename T,typename CharContainer,typename Alloc,typename TempAlloc >
typename std::enable_if<ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type
try_encode_toon(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, CharContainer& cont, 
    const basic_toon_encode_options<typename CharContainer::value_type>& options,
    indenting indent)
{
    using char_type = typename CharContainer::value_type;

    if (indent == indenting::no_indent)
    {
        basic_compact_json_encoder<char_type, jsoncons::string_sink<CharContainer>,TempAlloc> encoder(cont, options,
            aset.get_temp_allocator());
        return try_encode_toon(aset, val, encoder);
    }
    else
    {
        basic_json_encoder<char_type, jsoncons::string_sink<CharContainer>, TempAlloc> encoder(cont, options, 
            aset.get_temp_allocator());
        return try_encode_toon(aset, val, encoder);
    }
}

// to stream with allocator_set

template <typename T,typename CharT,typename Alloc,typename TempAlloc >
write_result try_encode_toon(const allocator_set<Alloc,TempAlloc>& aset,
    const T& val, std::basic_ostream<CharT>& os, 
    const basic_toon_encode_options<CharT>& options,
    indenting indent)
{
    if (indent == indenting::no_indent)
    {
        basic_compact_json_encoder<CharT,TempAlloc> encoder(os, options, aset.get_temp_allocator());
        return try_encode_toon(aset, val, encoder);
    }
    else
    {
        basic_json_encoder<CharT,TempAlloc> encoder(os, options, aset.get_temp_allocator());
        return try_encode_toon(aset, val, encoder);
    }
}

//end legacy
#endif

template <typename T, typename Alloc, typename TempAlloc, typename Sink>
typename std::enable_if<ext_traits::is_basic_json<T>::value, write_result>::type
try_encode_toon(const allocator_set<Alloc, TempAlloc>&, const T& j, Sink&& sink, 
    const toon_encode_options& options)
{
    if (j.is_array())
    {
    }
    else if (j.is_object())
    {
    }
    else
    {
        //encode_primitive(j, options.delimiter());
    }
    return write_result{};
}

template <typename T,typename Sink>
typename std::enable_if<ext_traits::is_basic_json<T>::value,write_result>::type
try_encode_toon(const T& val, Sink&& sink, const toon_encode_options& options)
{
    return try_encode_toon(make_alloc_set(), val, std::forward<Sink>(sink), options);
}

template <typename T,typename CharContainer>
typename std::enable_if<ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type
try_encode_toon(const T& val, CharContainer& cont, 
    const toon_encode_options& options = toon_encode_options())
{
    using char_type = typename CharContainer::value_type;

    return try_encode_toon(val, string_sink<CharContainer>{cont}, options);
}

template <typename... Args>
void encode_toon(Args&& ... args)
{
    auto result = try_encode_toon(std::forward<Args>(args)...); 
    if (!result)
    {
        JSONCONS_THROW(ser_error(result.error()));
    }
}

} // namespace toon
} // namespace jsoncons

#endif // JSONCONS_TOON_ENCODE_TOON_HPP 


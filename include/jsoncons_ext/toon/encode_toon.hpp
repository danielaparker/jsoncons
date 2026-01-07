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

JSONCONS_INLINE_CONSTEXPR jsoncons::string_view null_literal{"null", 4};
JSONCONS_INLINE_CONSTEXPR jsoncons::string_view true_literal{"true", 4};
JSONCONS_INLINE_CONSTEXPR jsoncons::string_view false_literal{"false", 5};

namespace detail {

inline
bool is_number(jsoncons::string_view str) 
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

inline
bool is_unquoted_safe(jsoncons::string_view str, char delimiter = ',')
{
    if (str.empty())
    {
        return false;
    }
    if (is_number(str))
    {
        return false;
    }
    if (str == null_literal || str == true_literal || str == false_literal)
    {
        return false;
    }
    if (str.front() == '-')
    {
        return false;
    }
    for (auto c : str)
    {
        switch (c)
        {
            case ':':
            case '[':
            case ']':
            case '{':
            case '}':
            case '\"':
            case '\\':
            case '\n':
            case '\r':
            case '\t':
                return false;
        }
        if (c == delimiter)
        {
            return false;
        }
    }
    return true;
}

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

template <typename Sink>
void escape_string(const char* s, std::size_t length,
                     bool escape_all_non_ascii, bool escape_solidus,
                     Sink& sink)
{
    const char* begin = s;
    const char* end = s + length;
    for (const char* it = begin; it != end; ++it)
    {
        char c = *it;
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

template <typename Sink>
write_result encode_string(jsoncons::string_view str, char delimiter, Sink&& sink)
{
    if (is_unquoted_safe(str, delimiter))
    {
        sink.append(str.data(), str.size());
    }
    else
    {
        sink.push_back('\"');
        sink.append(str.data(), str.size());
        sink.push_back('\"');
    }

    return write_result{};
}

} // namespace detail

// encode_toon

template <typename Json, typename Sink>
write_result encode_primitive(const Json& val, char delimiter, Sink&& sink)
{
    if (val.is_null())
    {
        sink.append(null_literal.data(), null_literal.size());
    }
    else if (val.is_bool())
    {
        if (val.as_bool())
        {
            sink.append(true_literal.data(), true_literal.size());
        }
        else
        {
            sink.append(false_literal.data(), false_literal.size());
        }
    }
    else if (val.is_number())
    {
        auto s = val.as_string();
        sink.append(s.data(), s.size());
    }
    else if (val.is_string())
    {
        detail::encode_string(val.as_string_view(), delimiter, std::forward<Sink>(sink));
    }
    return write_result{};
}

template <typename T, typename Alloc, typename TempAlloc, typename Sink>
typename std::enable_if<ext_traits::is_basic_json<T>::value, write_result>::type
try_encode_toon(const allocator_set<Alloc, TempAlloc>&, const T& val, Sink&& sink, 
    const toon_encode_options& options)
{
    if (val.is_array())
    {
    }
    else if (val.is_object())
    {
    }
    else
    {
        encode_primitive(val, options.delimiter(), std::forward<Sink>(sink));
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
    return try_encode_toon(val, string_sink<CharContainer>{cont}, options);
}

template <typename T>
write_result try_encode_toon(const T& val, std::basic_ostream<char>& os, 
    const toon_encode_options& options = toon_encode_options())
{
    return try_encode_toon(val, stream_sink<char>{os}, options);
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


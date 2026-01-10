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
#include <jsoncons/json_encoders.hpp>

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
        jsoncons::detail::escape_string(str.data(), str.size(), false, false, sink);
        sink.push_back('\"');
    }

    return write_result{};
}

} // namespace detail

// encode_toon

template <typename Sink>
void write_header(string_view key, 
    std::size_t length,
    const std::vector<std::string>& fields,
    char delimiter,
    jsoncons::optional<char> length_marker, 
    Sink&& sink)
{
    if (!key.empty())
    {
        detail::encode_string(key, delimiter, std::forward<Sink>(sink));
    }
    if (!fields.empty())
    {
        sink.push_back('[');
        if (length_marker)
        {
            sink.push_back(*length_marker);
        }
        auto s = std::to_string(length);
        sink.append(s.data(), s.size());
        sink.push_back(']');
    }
    else if (delimiter != ',')
    {
        sink.push_back('[');
        if (length_marker)
        {
            sink.push_back(*length_marker);
        }
        auto s = std::to_string(length);
        sink.append(s.data(), s.size());
        sink.push_back(delimiter);
        sink.push_back(']');
    }
    else
    {
        sink.push_back('[');
        if (length_marker)
        {
            sink.push_back(*length_marker);
        }
        auto s = std::to_string(length);
        sink.append(s.data(), s.size());
        sink.push_back(']');
    }

    if (!fields.empty())
    {
        sink.push_back('{');
        bool first = true;
        for (const auto& field : fields)
        {
            if (!first)
            {
                sink.push_back(delimiter);
            }
            else
            {
                first = false;
            }
            sink.append(field.data(), field.size());
        }
        sink.push_back('}');
    }
    sink.push_back(':');
}

template <typename Json>
bool is_array_of_arrays(const Json& val)
{
    if (!val.is_array())
    {
        return false;
    }
    for (const auto& item : val.array_range())
    {
        if (!item.is_array())
        {
            return false;
        }
    }
    return true;
}

template <typename Json>
bool is_array_of_objects(const Json& val)
{
    if (!val.is_array())
    {
        return false;
    }
    for (const auto& item : val.array_range())
    {
        if (!item.is_object())
        {
            return false;
        }
    }
    return true;
}

template <typename Json>
bool is_array_of_primitives(const Json& val)
{
    if (!val.is_array())
    {
        return false;
    }
    for (const auto& item : val.array_range())
    {
        if (item.is_array() || item.is_object())
        {
            return false;
        }
    }
    return true;
}

template <typename Json>
std::vector<std::string> try_get_tabular_header(const Json& val)
{
    if (val.empty())
    {
        return std::vector<std::string>{};
    }

    std::vector<std::string> first_keys;

    for (const auto& item : val[0].object_range())
    {
        first_keys.push_back(item.key());
    }

    for (const auto& row : val.array_range())
    {
        std::vector<std::string> keys;
        for (const auto& item : row.object_range())
        {
            if (item.value().is_array() || item.value().is_object())
            {
                return std::vector<std::string>{};
            }
            keys.push_back(item.key());
        }
        if (keys != first_keys)
        {
            return std::vector<std::string>{};
        }
    }

    return first_keys;
}

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

template <typename Json, typename Sink>
void encode_array_of_arrays(const Json& val, const toon_encode_options& options, 
    Sink&& sink, int depth, jsoncons::string_view key)
{
    write_header(key, val.size(), std::vector<std::string>{}, 
        options.delimiter(), options.length_marker(), std::forward<Sink>(sink));
    sink.push_back('\n');
    bool first = true;
    for (const auto& item : val.array_range())
    {
        if (!first)
        {
            sink.push_back('\n');
        }
        else
        {
            first = false;
        }
        if (is_array_of_primitives(item))
        {
            sink.append((depth+1)*options.indent(), ' ');
            sink.push_back('-');
            sink.push_back(' ');
            sink.push_back('[');
            if (options.length_marker())
            {
                sink.push_back(*options.length_marker());
            }
            auto s = std::to_string(item.size());
            sink.append(s.data(), s.size());
            sink.push_back(']');
            sink.push_back(':');
            if (!item.empty())
            {
                sink.push_back(' ');
            }
            bool first2 = true;
            for (const auto& item2 : item.array_range())
            {
                if (!first2)
                {
                    sink.push_back(options.delimiter());
                }
                else
                {
                    first2 = false;
                }
                encode_primitive(item2, options.delimiter(), std::forward<Sink>(sink));
            }
        }
        else
        {
            encode_array(item, options, sink, depth+1, jsoncons::string_view{});
        }
    }
}

template <typename Json, typename Sink>
void encode_array_of_objects_as_tabular(const Json& val, 
    const std::vector<std::string>& fields,
    const toon_encode_options& options, 
    Sink&& sink, int depth, jsoncons::string_view key)
{
    sink.append(depth*options.indent(), ' ');
    write_header(key, val.size(), fields, 
        options.delimiter(), options.length_marker(), std::forward<Sink>(sink));
    sink.push_back('\n');
}

template <typename Json, typename Sink>
void encode_array(const Json& val, const toon_encode_options& options, 
    Sink&& sink, int depth, jsoncons::string_view key)
{
    if (val.empty())
    {
        write_header(key, 0, std::vector<std::string>{}, options.delimiter(), options.length_marker(), std::forward<Sink>(sink));
    }
    else if (is_array_of_arrays(val))
    {
        encode_array_of_arrays(val, options, std::forward<Sink>(sink), depth, key);
    }
    else if (is_array_of_objects(val))
    {
        auto fields = try_get_tabular_header(val);
        if (!fields.empty())
        {
            encode_array_of_objects_as_tabular(val, fields, options, std::forward<Sink>(sink), depth, key);
        }
    }
}

template <typename Json, typename Sink>
void encode_key_value_pair(jsoncons::string_view key, const Json& val, 
    const toon_encode_options& options, 
    Sink&& sink, int depth)
{
    if (!key.empty())
    {
        sink.append((depth+1)*options.indent(), ' ');
        detail::encode_string(key, options.delimiter(), std::forward<Sink>(sink));
    }
    for (const auto& item : val.object_range())
    {
    }
}

template <typename Json, typename Sink>
void encode_object(const Json& val, const toon_encode_options& options, 
    Sink&& sink, int depth, jsoncons::string_view key)
{
    if (!key.empty())
    {
        sink.append((depth+1)*options.indent(), ' ');
        detail::encode_string(key, options.delimiter(), std::forward<Sink>(sink));
    }
    for (const auto& item : val.object_range())
    {
        (item);
    }
}

template <typename Json, typename Sink>
void encode_value(const Json& val, const toon_encode_options& options, Sink&& sink, int depth = 0)
{
    if (val.is_array())
    {
        encode_array(val, options, std::forward<Sink>(sink), depth, jsoncons::string_view{});
    }
    else if (val.is_object())
    {
        encode_object(val, options, std::forward<Sink>(sink), depth, jsoncons::string_view{});
    }
    else
    {
        encode_primitive(val, options.delimiter(), std::forward<Sink>(sink));
    }
}

template <typename T, typename Alloc, typename TempAlloc, typename Sink>
typename std::enable_if<ext_traits::is_basic_json<T>::value, write_result>::type
try_encode_toon(const allocator_set<Alloc, TempAlloc>&, const T& val, Sink&& sink, 
    const toon_encode_options& options)
{
    encode_value(val, options, std::forward<Sink>(sink));
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


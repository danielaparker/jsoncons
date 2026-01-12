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
write_result encode_string(jsoncons::string_view str, char delimiter, Sink& sink)
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
    const std::vector<jsoncons::string_view>& fields,
    char delimiter,
    jsoncons::optional<char> length_marker, 
    Sink& sink)
{
    if (!key.empty())
    {
        detail::encode_string(key, delimiter, sink);
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
std::vector<jsoncons::string_view> try_get_tabular_header(const Json& val)
{
    if (val.empty())
    {
        return std::vector<jsoncons::string_view>{};
    }

    std::vector<jsoncons::string_view> first_keys;

    for (const auto& item : val[0].object_range())
    {
        first_keys.push_back(item.key());
    }

    for (const auto& row : val.array_range())
    {
        std::vector<jsoncons::string_view> keys;
        for (const auto& item : row.object_range())
        {
            if (item.value().is_array() || item.value().is_object())
            {
                return std::vector<jsoncons::string_view>{};
            }
            keys.push_back(item.key());
        }
        if (keys != first_keys)
        {
            return std::vector<jsoncons::string_view>{};
        }
    }

    return first_keys;
}

template <typename Json, typename Sink>
write_result encode_primitive(const Json& val, char delimiter, Sink& sink)
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
        detail::encode_string(val.as_string_view(), delimiter, sink);
    }
    return write_result{};
}

template <typename Json, typename Sink>
void encode_array_of_arrays(const Json& val, const toon_encode_options& options, 
    Sink& sink, int depth, int line, jsoncons::string_view key)
{
    if (line != 0)
    {
        sink.push_back('\n');
    }
    sink.append(depth*options.indent(), ' ');
    write_header(key, val.size(), std::vector<jsoncons::string_view>{}, 
        options.delimiter(), options.length_marker(), sink);
    ++line;
    for (const auto& item : val.array_range())
    {
        sink.push_back('\n');
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
                encode_primitive(item2, options.delimiter(), sink);
            }
        }
        else
        {
            encode_array(item, options, sink, depth+1, line, jsoncons::string_view{});
        }
        ++line;
    }
}

template <typename Json, typename Sink>
void encode_array_of_objects_as_tabular(const Json& val, 
    const std::vector<jsoncons::string_view>& fields,
    const toon_encode_options& options, 
    Sink& sink, int depth, int line, jsoncons::string_view key)
{
    if (line != 0)
    {
        sink.push_back('\n');
    }
    sink.append(depth*options.indent(), ' ');
    write_header(key, val.size(), fields, 
        options.delimiter(), options.length_marker(), sink);
    ++line;

    for (const auto& row : val.array_range())
    {
        sink.push_back('\n');
        sink.append((depth+1)*options.indent(), ' ');
        bool first_item = true;
        for (const auto& item : row.object_range())
        {
            if (!first_item)
            {
                sink.push_back(options.delimiter());
            }
            else
            {
                first_item = false;
            }
            encode_primitive(item.value(), options.delimiter(), sink);
        }
        ++line;
    }
}

template <typename Json, typename Sink>
void encode_object_as_list_item(const Json& val, const toon_encode_options& options, 
    Sink& sink, int depth, int line)
{
    if (val.empty())
    {
        if (line != 0)
        {
            sink.push_back('\n');
        }
        sink.append(depth*options.indent(), ' ');
        sink.push_back('-');
        return;
    }
    auto first = val.object_range().begin();
    auto last = val.object_range().end();

    if (first->value().is_array() || first->value().is_object())
    {
        if (line != 0)
        {
            sink.push_back('\n');
        }
        sink.append(depth*options.indent(), ' ');
        sink.push_back('-');
        encode_key_value_pair(first->key(), first->value(), options, sink, depth+1, line);
        ++line;
    }
    else
    {
        if (line != 0)
        {
            sink.push_back('\n');
        }
        sink.append(depth*options.indent(), ' ');
        sink.push_back('-');
        sink.push_back(' ');
        detail::encode_string(first->key(), options.delimiter(), sink);
        sink.push_back(':');
        sink.push_back(' ');
        encode_primitive(first->value(), options.delimiter(), sink);
    }
    ++line;
    for (auto it = first+1; it != last; ++it)
    {
        encode_key_value_pair(it->key(), it->value(), options, sink, depth + 1, line);
        ++line;
    }
}

template <typename Json, typename Sink>
void encode_mixed_array_as_list_items(const Json& val, const toon_encode_options& options, 
    Sink& sink, int depth, int line, jsoncons::string_view key)
{
    if (line != 0)
    {
        sink.push_back('\n');
    }
    sink.append(depth*options.indent(), ' ');
    write_header(key, val.size(), std::vector<jsoncons::string_view>{}, 
        options.delimiter(), options.length_marker(), sink);
    ++line;

    for (const auto& item : val.array_range())
    {
        if (item.is_object())
        {
            encode_object_as_list_item(item, options, sink, depth+1, line);
        }
        else if (item.is_array())
        {
            encode_array(item, options, sink, depth + 1, line, jsoncons::string_view{});
        }
        else
        {
            sink.push_back('\n');
            sink.append((depth+1)*options.indent(), ' ');
            sink.push_back('-');
            sink.push_back(' ');
            encode_primitive(item, options.delimiter(), sink);
        }
        ++line;
    }
}

template <typename Json, typename Sink>
void encode_inline_primitive_array(const Json& val, const toon_encode_options& options, 
    Sink& sink, int depth, int line, jsoncons::string_view key)
{
    if (line != 0)
    {
        sink.push_back('\n');
    }
    sink.append(depth*options.indent(), ' ');
    write_header(key, val.size(), std::vector<jsoncons::string_view>{}, options.delimiter(), options.length_marker(), sink);
    sink.push_back(' ');
    ++line;

    bool first_field = true;
    for (const auto& item : val.array_range())
    {
        if (!first_field)
        {
            sink.push_back(options.delimiter());
        }
        else
        {
            first_field = false;
        }
        encode_primitive(item, options.delimiter(), sink);
    }
}

template <typename Json, typename Sink>
void encode_array(const Json& val, const toon_encode_options& options, 
    Sink& sink, int depth, int line, jsoncons::string_view key)
{
    if (val.empty())
    {
        if (line != 0)
        {
            sink.push_back('\n');
        }
        sink.append(depth*options.indent(), ' ');
        write_header(key, 0, std::vector<jsoncons::string_view>{}, options.delimiter(), options.length_marker(), sink);
        ++line;
    }
    else if (is_array_of_primitives(val))
    {
        encode_inline_primitive_array(val, options, sink, depth, line, key);
    }
    else if (is_array_of_arrays(val))
    {
        encode_array_of_arrays(val, options, sink, depth, line, key);
    }
    else if (is_array_of_objects(val))
    {
        auto fields = try_get_tabular_header(val);
        if (!fields.empty())
        {
            encode_array_of_objects_as_tabular(val, fields, options, sink, depth, line, key);
        }
        else
        {
            encode_mixed_array_as_list_items(val, options, sink, depth, line, key);
        }
    }
    else
    {
        encode_mixed_array_as_list_items(val, options, sink, depth, line, key);
    }
}

template <typename Json, typename Sink>
void encode_key_value_pair(jsoncons::string_view key, const Json& val, 
    const toon_encode_options& options, 
    Sink& sink, int depth, int line)
{
    if (val.is_array())
    {
        encode_array(val, options, sink, depth, line, key);
    }
    else if (val.is_object())
    {
        encode_object(val, options, sink, depth, line, key);
    }
    else
    {
        if (line != 0)
        {
            sink.push_back('\n');
        }
        sink.append(depth*options.indent(), ' ');
        detail::encode_string(key, options.delimiter(), sink);
        sink.push_back(':');
        sink.push_back(' ');
        encode_primitive(val, options.delimiter(), sink);
    }
}

template <typename Json, typename Sink>
void encode_object(const Json& val, const toon_encode_options& options, 
    Sink& sink, int depth, int line, jsoncons::string_view key)
{
    if (!key.empty())
    {
        if (line != 0)
        {
            sink.push_back('\n');
        }
        sink.append(depth*options.indent(), ' ');
        detail::encode_string(key, options.delimiter(), sink);
        sink.push_back(':');
        ++line;
        for (const auto& item : val.object_range())
        {
            encode_key_value_pair(item.key(), item.value(), options, sink, depth + 1, line);
            ++line;
        }
    }
    else
    {
        for (const auto& item : val.object_range())
        {
            encode_key_value_pair(item.key(), item.value(), options, sink, depth, line);
            ++line;
        }
    }
}

template <typename Json, typename Sink>
void encode_value(const Json& val, const toon_encode_options& options, Sink& sink, int depth = 0)
{
    if (val.is_array())
    {
        encode_array(val, options, sink, depth, 0, jsoncons::string_view{});
    }
    else if (val.is_object())
    {
        encode_object(val, options, sink, depth, 0, jsoncons::string_view{});
    }
    else
    {
        encode_primitive(val, options.delimiter(), sink);
    }
}

template <typename T, typename Alloc, typename TempAlloc, typename Sink>
typename std::enable_if<ext_traits::is_basic_json<T>::value, write_result>::type
try_encode_toon(const allocator_set<Alloc, TempAlloc>&, const T& val, Sink& sink, 
    const toon_encode_options& options)
{
    encode_value(val, options, sink);
    return write_result{};
}

template <typename T,typename CharContainer>
typename std::enable_if<ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type
try_encode_toon(const T& val, CharContainer& cont, 
    const toon_encode_options& options = toon_encode_options())
{
    string_sink<CharContainer> sink{cont};
    encode_value(val, options, sink);
    return write_result{};
}

template <typename T>
write_result try_encode_toon(const T& val, std::basic_ostream<char>& os, 
    const toon_encode_options& options = toon_encode_options())
{
    stream_sink<char> sink{os};
    encode_value(val, options, sink);
    return write_result{};
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


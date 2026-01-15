// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_TOON_ENCODE_TOON_HPP 
#define JSONCONS_TOON_ENCODE_TOON_HPP 

#include <ostream>
#include <cctype>

#include <jsoncons/basic_json.hpp>
#include <jsoncons_ext/toon/toon_options.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/reflect/encode_traits.hpp>
#include <jsoncons/ser_util.hpp>
#include <jsoncons/allocator_set.hpp>
#include <jsoncons/json_encoders.hpp>
#include <set>

namespace jsoncons {
namespace toon {

JSONCONS_INLINE_CONSTEXPR jsoncons::string_view null_literal{"null", 4};
JSONCONS_INLINE_CONSTEXPR jsoncons::string_view true_literal{"true", 4};
JSONCONS_INLINE_CONSTEXPR jsoncons::string_view false_literal{"false", 5};

namespace detail {

enum class format_number_state{value_sign,coefficient,fraction,exponent_sign,exponent_value,err};

inline
std::string exponential_to_decimal_notation(jsoncons::string_view str)
{
    std::string result;

    std::string num_str;
    std::string exponent_str;

    bool neg_value = false;
    bool neg_exp = false;

    std::size_t decimal_places = 0;

    format_number_state state = format_number_state::value_sign;
    for (std::size_t i = 0; i < str.size();)
    {
        char c = str[i];
        switch (state)
        {
            case format_number_state::value_sign:
                if (c == '-')
                {
                    neg_value = true;
                    ++i;
                }
                state = format_number_state::coefficient;
                break;
            case format_number_state::coefficient:
                if ((c >= '0' && c <= '9') || c == '-')
                {
                    num_str.push_back(c);
                    ++i;
                }
                else if (c == 'e' || c == 'E')
                {
                    state = format_number_state::exponent_sign;
                    ++i;
                }
                else if (c == '.')
                { 
                    state = format_number_state::fraction;
                    ++i;
                }
                break;
            case format_number_state::fraction:
                if ((c >= '0' && c <= '9'))
                {
                    ++decimal_places;
                    num_str.push_back(c);
                    ++i;
                }
                else if (c == 'e' || c == 'E')
                {
                    state = format_number_state::exponent_sign;
                    ++i;
                }
                break;
            case format_number_state::exponent_sign:
                if (c == '-')
                {
                    neg_exp = true;
                    state = format_number_state::exponent_value;
                    ++i;
                }
                else if (c == '+')
                {
                    state = format_number_state::exponent_value;
                    ++i;
                }
                else
                {
                    state = format_number_state::exponent_value;
                }
                break;
            case format_number_state::exponent_value:
                if ((c >= '0' && c <= '9'))
                {
                    exponent_str.push_back(c);
                    ++i;
                }
                break;
            case format_number_state::err:
                i = str.size();
                break;
        }
    }

    std::size_t exponent;
    dec_to_integer(exponent_str.data(), exponent_str.size(), exponent);

    std::size_t n = num_str.size();

    if (neg_exp) // shift decimal point left
    {
        for (std::size_t i = n-decimal_places; i <= exponent; ++i)
        {
            num_str.insert(num_str.begin(), '0');
        }
        std::size_t pos = num_str.size()-(decimal_places+exponent);
        auto first_non_zero = num_str.find_first_not_of('0', pos);
        if (first_non_zero ==  std::string::npos)
        {
            num_str.erase(num_str.begin()+pos, num_str.end());
        }
        else
        {
            num_str.insert(num_str.begin()+(num_str.size()-decimal_places-exponent), '.');
        }
    }
    else // shift decimal point right
    {
        for (std::size_t i = decimal_places; i < exponent; ++i)
        {
            num_str.insert(num_str.begin()+(n-decimal_places), '0');
        }
        if (decimal_places > exponent)
        {
            num_str.insert(num_str.begin() + (num_str.size() - exponent), '.');
        }
    }
    if (neg_value)
    {
        num_str.insert(num_str.begin(), '-');
    }
    return num_str;
}

inline
bool is_unquoted_key_valid(string_view key)
{
    if (key.empty())
    {
        return false;
    }
    char c = key.front();
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'))
    {
        return false;
    }
    for (auto it = key.begin()+1; it != key.end(); ++it)
    {
        c = *it;
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (c >= '0' && c <= '9') || c == '.'))
        {
            return false;
        }
    }
    return true;
}

enum class is_number_state{initial,negative,digits_or_dot_or_exp,octal,leading_zero, 
    leading_decimal_zero, decimal_zero, decimal_digit, exponent, digits_or_exp, digits, not_number};

inline
bool is_number(jsoncons::string_view str) 
{
    is_number_state state = is_number_state::initial;

    for (std::size_t i = 0; i < str.size();)
    {
        char c = str[i];
        switch (state)
        {
            case is_number_state::initial:
                if (c == '-')
                {
                    state = is_number_state::negative;
                    ++i;
                }
                else if (c == '0')
                {
                    state = is_number_state::leading_zero;
                    ++i;
                }
                else 
                {
                    state = is_number_state::digits_or_dot_or_exp;
                }
                break;
            case is_number_state::leading_zero:
                if (c == '.')
                {
                    state = is_number_state::decimal_digit;
                    ++i;
                }
                else 
                {
                    state = is_number_state::octal;
                }
                break;
            case is_number_state::leading_decimal_zero:
                if (c == '.')
                {
                    state = is_number_state::decimal_digit;
                    ++i;
                }
                else
                {
                    state = is_number_state::not_number;
                }
                break;
            case is_number_state::octal:
                if (!(c >= '0' && c <= '7'))
                {
                    state = is_number_state::not_number;
                }
                else
                {
                    ++i;
                }
                break;
            case is_number_state::negative: 
                if (c == '0')
                {
                    state = is_number_state::leading_decimal_zero;
                    ++i;
                }
                else 
                {
                    state = is_number_state::digits_or_dot_or_exp;
                }
                break;
            case is_number_state::decimal_zero: 
                if (c == '0')
                {
                    state = is_number_state::not_number;
                }
                else if (c == '.')
                {
                    state = is_number_state::decimal_digit;
                    ++i;
                }
                else if (c == 'e' || c == 'E')
                {
                    state = is_number_state::digits_or_dot_or_exp;
                    ++i;
                }
                else if (c >= '1' && c <= '9')
                {
                    state = is_number_state::digits_or_dot_or_exp;
                    ++i;
                }
                else
                {
                    state = is_number_state::not_number;
                }
                break;
            case is_number_state::decimal_digit:
                if (c >= '0' && c <= '9')
                {
                    state = is_number_state::digits_or_exp;
                    ++i;
                }
                else
                {
                    state = is_number_state::not_number;
                }
                break;
            case is_number_state::digits_or_dot_or_exp: 
                if (c == '.')
                {
                    state = is_number_state::decimal_digit;
                    ++i;
                }
                else if (c == 'e' || c == 'E')
                {
                    state = is_number_state::exponent;
                    ++i;
                }
                else if (!(c >= '0' && c <= '9'))
                {
                    state = is_number_state::not_number;
                }
                else
                {
                    ++i;
                }
                break;
            case is_number_state::digits_or_exp: 
                if (c == 'e' || c == 'E')
                {
                    state = is_number_state::exponent;
                    ++i;
                }
                else if (!(c >= '0' && c <= '9'))
                {
                    state = is_number_state::not_number;
                }
                else
                {
                    ++i;
                }
                break;
            case is_number_state::exponent: 
                if ((c >= '0' && c <= '9') || c == '-')
                {
                    state = is_number_state::digits;
                    ++i;
                }
                else
                {
                    state = is_number_state::not_number;
                }
                break;
            case is_number_state::digits: 
                if (!(c >= '0' && c <= '9'))
                {
                    state = is_number_state::not_number;
                }
                else
                {
                    ++i;
                }
                break;
            default:
                i = str.size();
                break;
        }
    }
    if (state == is_number_state::digits_or_dot_or_exp || state == is_number_state::octal 
        || state == is_number_state::decimal_zero 
        || state == is_number_state::digits_or_exp || state == is_number_state::digits
        || state == is_number_state::leading_zero || state == is_number_state::leading_decimal_zero)
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
    if (std::isspace(static_cast<unsigned char>(str.front())) || std::isspace(static_cast<unsigned char>(str.back())))
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
void encode_string(jsoncons::string_view str, char delimiter, Sink& sink)
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
}

template <typename Sink>
void encode_key(jsoncons::string_view key, Sink& sink)
{
    if (is_unquoted_key_valid(key))
    {
        sink.append(key.data(), key.size());
    }
    else
    {
        sink.push_back('\"');
        jsoncons::detail::escape_string(key.data(), key.size(), false, false, sink);
        sink.push_back('\"');
    }
}

} // namespace detail

// encode_toon

template <typename Json>
bool is_json_primitive(const Json& val)
{
    if (val.is_array())
    {
        return false;
    }
    if (val.is_object())
    {
        return false;
    }
    return true;
}

template <typename Json>
bool is_json_array(const Json& val)
{
    if (val.is_array())
    {
        return true;
    }
    return false;
}

template <typename Json>
bool is_json_object(const Json& val)
{
    if (val.is_object())
    {
        return true;
    }
    return false;
}

template <typename Sink>
void write_header(jsoncons::optional<string_view> key, 
    std::size_t length,
    const jsoncons::span<const jsoncons::string_view>& fields,
    char delimiter,
    jsoncons::optional<char> length_marker, 
    Sink& sink)
{
    if (key)
    {
        detail::encode_key(*key, sink);
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
        if (delimiter != ',')
        {
            sink.push_back(delimiter);
        }
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
        if (delimiter != ',')
        {
            sink.push_back(delimiter);
        }
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
            detail::encode_key(field, sink);  // key
        }
        sink.push_back('}');
    }
    sink.push_back(':');
}

template <typename Json>
bool is_array_of_arrays(const Json& val)
{
    if (!is_json_array(val))
    {
        return false;
    }
    for (const auto& item : val.array_range())
    {
        if (!is_json_array(item))
        {
            return false;
        }
    }
    return true;
}

template <typename Json>
bool is_array_of_objects(const Json& val)
{
    if (!is_json_array(val))
    {
        return false;
    }
    for (const auto& item : val.array_range())
    {
        if (!is_json_object(item))
        {
            return false;
        }
    }
    return true;
}

template <typename Json>
bool is_array_of_primitives(const Json& val)
{
    //if (val.empty())
    //{
    //    return false;
    //}
    if (!is_json_array(val))
    {
        return false;
    }
    for (const auto& item : val.array_range())
    {
        if (is_json_array(item) || is_json_object(item))
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
    std::set<jsoncons::string_view> first_keys_set;

    for (const auto& item : val[0].object_range())
    {
        first_keys.push_back(item.key());
        first_keys_set.insert(item.key());
    }

    for (const auto& row : val.array_range())
    {
        std::set<jsoncons::string_view> keys_set;
        for (const auto& item : row.object_range())
        {
            if (is_json_array(item.value()) || is_json_object(item.value()))
            {
                return std::vector<jsoncons::string_view>{};
            }
            keys_set.insert(item.key());
        }
        if (keys_set != first_keys_set)
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
        bool exponential_notation = false;
        for (auto c : s)
        {
            if (c == 'e' || c == 'E')
            {
                exponential_notation = true;
                break;
            }
        }
        if (exponential_notation)
        {
            auto dec_str = detail::exponential_to_decimal_notation(s);
            sink.append(dec_str.data(), dec_str.size());
        }
        else
        {
            sink.append(s.data(), s.size());
        }
    }
    else if (val.is_string())
    {
        detail::encode_string(val.as_string_view(), delimiter, sink);
    }
    return write_result{};
}

template <typename Json, typename Sink>
void encode_array_of_arrays(const Json& val, const toon_encode_options& options, 
    Sink& sink, int depth, int& line , jsoncons::optional<string_view> key)
{
    if (line != 0)
    {
        sink.push_back('\n');
    }
    sink.append(depth*options.indent(), ' ');
    write_header(key, val.size(), jsoncons::span<const jsoncons::string_view>{}, 
        options.delimiter(), options.length_marker(), sink);
    ++line;
    for (const auto& item : val.array_range())
    {
        if (is_array_of_primitives(item))
        {
            sink.push_back('\n');
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
            if (options.delimiter() != ',')
            {
                sink.push_back(options.delimiter());
            }
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
            encode_array(item, options, sink, depth+1, line, jsoncons::optional<jsoncons::string_view>{});
        }
        ++line;
    }
}

template <typename Json, typename Sink>
void encode_array_content(const Json& val, const toon_encode_options& options, 
    Sink& sink, int depth, int& line )
{
    if (is_array_of_primitives(val))
    {
        bool first_item = true;
        for (const auto& item : val.array_range())
        {
            if (!first_item)
            {
                sink.push_back(options.delimiter());
            }
            else
            {
                sink.push_back(' ');
                first_item = false;
            }
            encode_primitive(item, options.delimiter(), sink);
        }
    }
    else if (is_array_of_arrays(val))
    {
        for (const auto& item : val.array_range())
        {
            if (is_array_of_primitives(item))
            {
                sink.push_back('\n');
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
                if (options.delimiter() != ',')
                {
                    sink.push_back(options.delimiter());
                }
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
                encode_array(item, options, sink, depth+1, line, jsoncons::optional<jsoncons::string_view>{});
            }
            ++line;
        }
    }
    else if (is_array_of_objects(val))
    {
        auto fields = try_get_tabular_header(val);
        if (!fields.empty())
        {
            for (const auto& row : val.array_range())
            {
                sink.push_back('\n');
                sink.append((depth+1)*options.indent(), ' ');
                bool first_item = true;
                for (auto field : fields)
                {
                    if (!first_item)
                    {
                        sink.push_back(options.delimiter());
                    }
                    else
                    {
                        first_item = false;
                    }
                    encode_primitive(row.at(field), options.delimiter(), sink);
                }
                ++line;
            }
        }
        else
        {
            for (const auto& item : val.array_range())
            {
                encode_object_as_list_item(item, options, sink, depth+1, line);
                ++line;
            }
        }
    }
    else
    {
        for (const auto& item : val.array_range())
        {
            if (is_json_object(item))
            {
                encode_object_as_list_item(item, options, sink, depth+1, line);
            }
            else if (is_json_array(item))
            {
                encode_array(item, options, sink, depth + 1, line, jsoncons::optional<jsoncons::string_view>{});
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
}

template <typename Json, typename Sink>
void encode_array_of_objects_as_tabular(const Json& val, 
    const jsoncons::span<const jsoncons::string_view>& fields,
    const toon_encode_options& options, 
    Sink& sink, int depth, int& line , jsoncons::optional<string_view> key)
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
        for (auto field : fields)
        {
            if (!first_item)
            {
                sink.push_back(options.delimiter());
            }
            else
            {
                first_item = false;
            }
            encode_primitive(row.at(field), options.delimiter(), sink);
        }
        ++line;
    }
}

template <typename Json, typename Sink>
void encode_object_as_list_item(const Json& val, const toon_encode_options& options, 
    Sink& sink, int depth, int& line )
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

    if (is_json_primitive(first->value()))
    {
        if (line != 0)
        {
            sink.push_back('\n');
        }
        sink.append(depth*options.indent(), ' ');
        sink.push_back('-');
        sink.push_back(' ');
        detail::encode_key(first->key(), sink);
        sink.push_back(':');
        sink.push_back(' ');
        encode_primitive(first->value(), options.delimiter(), sink);
    }
    else if (is_json_array(first->value()))
    {
        if (is_array_of_primitives(first->value()))
        {
            if (line != 0)
            {
                sink.push_back('\n');
            }
            sink.append(depth*options.indent(), ' ');
            sink.push_back('-');
            sink.push_back(' ');
            write_header(first->key(), first->value().size(), 
                jsoncons::span<const jsoncons::string_view>{},
                options.delimiter(), options.length_marker(), sink);
            encode_array_content(first->value(), options, sink, depth, line);
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

            std::vector<jsoncons::string_view> fields;
            if (is_array_of_objects(first->value()))
            {
                fields = try_get_tabular_header(first->value());
            }
            write_header(first->key(), first->value().size(), 
                fields,
                options.delimiter(), options.length_marker(), sink);
            encode_array_content(first->value(), options, sink, depth+1, line);
        }
        ++line;
    }
    else // object, this is where problem is
    {
        if (line != 0)
        {
            sink.push_back('\n');
        }
        sink.append(depth*options.indent(), ' ');
        sink.push_back('-');
        encode_key_value_pair(first->key(), first->value(), options, sink, depth + 1, line);
        ++line;
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
    Sink& sink, int depth, int& line , jsoncons::optional<string_view> key)
{
    if (line != 0)
    {
        sink.push_back('\n');
    }
    sink.append(depth*options.indent(), ' ');
    write_header(key, val.size(), jsoncons::span<const jsoncons::string_view>{}, 
        options.delimiter(), options.length_marker(), sink);
    ++line;

    for (const auto& item : val.array_range())
    {
        if (is_json_primitive(item))
        {
            sink.push_back('\n');
            sink.append((depth+1)*options.indent(), ' ');
            sink.push_back('-');
            sink.push_back(' ');
            encode_primitive(item, options.delimiter(), sink);
        }
        else if (is_json_object(item))
        {
            encode_object_as_list_item(item, options, sink, depth+1, line);
        }
        else if (is_json_array(item))
        {
            if (is_array_of_primitives(item))
            {
                if (line != 0)
                {
                    sink.push_back('\n');
                }
                sink.append((depth+1)*options.indent(), ' ');
                sink.push_back('-');
                sink.push_back(' ');
                write_header(jsoncons::optional<jsoncons::string_view>{}, item.size(), 
                    jsoncons::span<const jsoncons::string_view>{},
                    options.delimiter(), options.length_marker(), sink);
                encode_array_content(item, options, sink, depth, line); // +2 in toon-pthon
            }
            else
            {
                if (line > 0)
                {
                    sink.push_back('\n');
                }
                sink.append((depth+1)*options.indent(), ' ');
                sink.push_back('-');
                sink.push_back(' ');

                std::vector<jsoncons::string_view> fields;
                if (is_array_of_objects(item))
                {
                    fields = try_get_tabular_header(item);
                }
                write_header(jsoncons::optional<jsoncons::string_view>{}, item.size(),
                    fields,
                    options.delimiter(), options.length_marker(), sink);
                encode_array_content(item, options, sink, depth+1, line); // +2 in toon-pthon
            }
        }
        ++line;
    }
}

template <typename Json, typename Sink>
void encode_inline_primitive_array(const Json& val, const toon_encode_options& options, 
    Sink& sink, int depth, int& line , jsoncons::optional<string_view> key)
{
    if (line != 0)
    {
        sink.push_back('\n');
    }
    sink.append(depth*options.indent(), ' ');
    write_header(key, val.size(), jsoncons::span<const jsoncons::string_view>{}, options.delimiter(), options.length_marker(), sink);
    ++line;

    bool first_item = true;
    for (const auto& item : val.array_range())
    {
        if (!first_item)
        {
            sink.push_back(options.delimiter());
        }
        else
        {
            sink.push_back(' ');
            first_item = false;
        }
        encode_primitive(item, options.delimiter(), sink);
    }
}

template <typename Json, typename Sink>
void encode_array(const Json& val, const toon_encode_options& options, 
    Sink& sink, int depth, int& line , jsoncons::optional<string_view> key)
{
    if (val.empty())
    {
        if (line != 0)
        {
            sink.push_back('\n');
        }
        sink.append(depth*options.indent(), ' ');
        write_header(key, 0, jsoncons::span<const jsoncons::string_view>{}, options.delimiter(), options.length_marker(), sink);
        ++line;
        return;
    }

    if (is_array_of_primitives(val))
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
            encode_array_of_objects_as_tabular(val, jsoncons::span<const jsoncons::string_view>(fields.data(), fields.size()), 
                options, sink, depth, line, key);
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
void encode_key_value_pair(string_view key, const Json& val, 
    const toon_encode_options& options, 
    Sink& sink, int depth, int& line )
{
    if (is_json_array(val))
    {
        encode_array(val, options, sink, depth, line, key);
    }
    else if (is_json_object(val))
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
        detail::encode_key(key, sink);
        sink.push_back(':');
        sink.push_back(' ');
        encode_primitive(val, options.delimiter(), sink);
    }
}

template <typename Json, typename Sink>
void encode_object(const Json& val, const toon_encode_options& options, 
    Sink& sink, int depth, int& line , jsoncons::optional<string_view> key)
{
    if (key)
    {
        if (line != 0)
        {
            sink.push_back('\n');
        }
        sink.append(depth*options.indent(), ' ');
        detail::encode_key(*key, sink);
        sink.push_back(':');
        ++line;
        for (const auto& item : val.object_range())
        {
            encode_key_value_pair(item.key(), item.value(), options, sink, depth+1, line);
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
void encode_value(const Json& val, const toon_encode_options& options, Sink& sink, int depth)
{
    int line{0};
    if (is_json_array(val))
    {
        encode_array(val, options, sink, depth, line, jsoncons::optional<jsoncons::string_view>{});
    }
    else if (is_json_object(val))
    {
        encode_object(val, options, sink, depth, line, jsoncons::optional<jsoncons::string_view>{});
    }
    else
    {
        sink.append(depth*options.indent(), ' ');
        encode_primitive(val, options.delimiter(), sink);
    }
}

template <typename T, typename Alloc, typename TempAlloc, typename Sink>
typename std::enable_if<ext_traits::is_basic_json<T>::value, write_result>::type
try_encode_toon(const allocator_set<Alloc, TempAlloc>&, const T& val, Sink& sink, 
    const toon_encode_options& options)
{
    encode_value(val, options, sink, 0);
    return write_result{};
}

template <typename T,typename CharContainer>
typename std::enable_if<ext_traits::is_back_insertable_char_container<CharContainer>::value,write_result>::type
try_encode_toon(const T& val, CharContainer& cont, 
    const toon_encode_options& options = toon_encode_options())
{
    string_sink<CharContainer> sink{cont};
    encode_value(val, options, sink, 0);
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


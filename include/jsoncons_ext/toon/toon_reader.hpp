// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

// 6CAMJWK15-75um4m

#ifndef JSONCONS_TOON_TOON_READER_HPP
#define JSONCONS_TOON_TOON_READER_HPP

#include <cstddef>
#include <functional>
#include <ios>
#include <memory> // std::allocator
#include <string>
#include <system_error>
#include <utility> // std::move

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/ser_utils.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/utility/read_number.hpp>
#include <jsoncons/utility/string_utils.hpp>
#include <jsoncons_ext/toon/toon_error.hpp>
#include <jsoncons_ext/toon/toon_options.hpp>

namespace jsoncons {
namespace toon {

inline
bool starts_with(jsoncons::span<char> sv, jsoncons::span<char> prefix)
{
    if (JSONCONS_UNLIKELY(sv.size() < prefix.size()))
    {
        return false;
    }
    const char* p = sv.data();
    const char* q = prefix.data();
    const char* last = prefix.data() + prefix.size();

    while (q < last)
    {
        if (*q++ != *p++)
        {
            return false;
        }
    }
    return true;
}

inline
bool starts_with(jsoncons::span<char> s, char* prefix)
{
    return starts_with(s, jsoncons::span<char>(prefix,strlen(prefix)));
}

inline
bool starts_with(jsoncons::span<char> s, char prefix)
{
    return starts_with(s, jsoncons::span<char>(&prefix, 1));
}

inline
bool ends_with(jsoncons::span<char> sv, jsoncons::span<char> suffix)
{
    if (JSONCONS_UNLIKELY(sv.size() < suffix.size()))
    {
        return false;
    }
    const char* p = sv.data() + (sv.size() - suffix.size());
    const char* q = suffix.data();
    const char* last = suffix.data() + suffix.size();

    while (q < last)
    {
        if (*q++ != *p++)
        {
            return false;
        }
    }
    return true;
}

inline
bool ends_with(jsoncons::span<char> s, char* suffix)
{
    return ends_with(s, jsoncons::span<char>(suffix, strlen(suffix)));
}

inline
bool ends_with(jsoncons::span<char> s, char suffix)
{
    return ends_with(s, jsoncons::span<char>(&suffix, 1));
}

inline
jsoncons::span<char> strip(jsoncons::span<char> sv)
{
    char* first = sv.data();
    char* last = first + sv.size();
    char* p = first;

    while (p < last)
    {
        char c = *p;
        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r'))
        {
            break;
        }
        ++p;
    }
    if (p == last)
    {
        return jsoncons::span<char>{};
    }

    char* q = last;
    do
    {
        --q;
        char c = *q;
        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r'))
        {
            break;
        }
    } while (q > p);

    std::size_t size = (p - first) + ((last-q)-1);
    return jsoncons::span<char>{p, sv.size() - size};
}

inline
jsoncons::expected<jsoncons::string_view,toon_errc> unescape_string(jsoncons::span<char> value)
{
    using result_type = jsoncons::expected<jsoncons::string_view,toon_errc>;

    char* cur = value.data(); 
    char* end = cur + value.size();

    while (cur < end)
    {
        if (*cur == '\\')
        {
            goto copy_escape;
        }
        ++cur;
    }
    return result_type{jsoncons::string_view{value.data(), value.size()}};

copy_escape:

    char* dst = cur;

    while (cur < end)
    {
        if (*cur == '\\')
        {
            if (cur + 1 == end)
            {
                return result_type{jsoncons::unexpect, toon_errc::invalid_escape_sequence};
            }
            char next_char = *(cur+1);
            if (next_char == 'n')
            {
                *dst++ = '\n';
                cur += 2;
                continue;
            }
            if (next_char == 't')
            {
                *dst++ = '\t';
                cur += 2;
                continue;
            }
            if (next_char == 'r')
            {
                *dst++ = '\r';
                cur += 2;
                continue;
            }
            if (next_char == '\\')
            {
                *dst++ = '\\';
                cur += 2;
                continue;
            }
            if (next_char == '\"')
            {
                *dst++ = '\"';
                cur += 2;
                continue;
            }
            return result_type{jsoncons::unexpect, toon_errc::invalid_escape_sequence};
        }
        *dst++ = *cur++;
    }

    return result_type{jsoncons::string_view{value.data(), std::size_t(dst - value.data())}
};
}

enum class parse_number_state{sign,zero,digits,fraction,exponent_sign,exponent_value,err};

inline
jsoncons::expected<void,std::error_code> parse_primitive(jsoncons::span<char> token, jsoncons::json_visitor& visitor)
{
    using result_type = jsoncons::expected<void,std::error_code>;

    token = strip(token);

    if (token.empty())
    {
        visitor.string_value(jsoncons::string_view{});
        return result_type{};
    }

    if (starts_with(token, '\"'))
    {
        if (!ends_with(token, '\"') || token.size() < 2)
        {
            return result_type{jsoncons::unexpect, toon_errc::missing_closing_quote};
        }
        auto result = unescape_string(jsoncons::span<char>(token.data()+1, token.size()-2));
        if (!result)
        {
            return result_type{jsoncons::unexpect, result.error()};
        }
        visitor.string_value(*result);
        return result_type{};
    }
    if (token.size() == 4 && token[0] == 't' &&  token[1] == 'r' && token[2] == 'u' &&  token[3] == 'e')
    {
        visitor.bool_value(true);
        return result_type{};
    }
    if (token.size() == 5 && token[0] == 'f' &&  token[1] == 'a' && token[2] == 'l' &&  token[3] == 's' &&  token[4] == 'e')
    {
        visitor.bool_value(false);
        return result_type{};
    }
    if (token.size() == 4 && token[0] == 'n' &&  token[1] == 'u' && token[2] == 'l' &&  token[3] == 'l')
    {
        visitor.null_value();
        return result_type{};
    }

    std::string num_str;
    std::string exponent_str;

    bool neg_value = false;
    bool neg_exp = false;
    bool not_a_number = false;

    std::size_t decimal_places = 0;

    parse_number_state state = parse_number_state::sign;
    for (std::size_t i = 0; i < token.size() && !not_a_number;)
    {
        char c = token[i];
        switch (state)
        {
            case parse_number_state::sign:
                if (c == '-')
                {
                    neg_value = true;
                    ++i;
                }
                if (i < token.size() && token[i] == '0')
                {
                    num_str.push_back('0');
                    state = parse_number_state::zero;
                    if (++i == token.size())
                    {
                        neg_value = false;
                    }
                }
                else
                {
                    state = parse_number_state::digits;
                }
                break;
            case parse_number_state::zero:
                if (c == '.')
                {
                    state = parse_number_state::digits;
                    ++i;
                }
                else
                {
                    if ((token.size() - i) == 2 && (token[i] == 'e' || token[i] == 'E') && token[i + 1] == '1')
                    {
                        num_str.push_back('0');
                        i += 2;
                    }
                    else
                    {
                        not_a_number = true;
                    }
                }
                break;
            case parse_number_state::digits:
                if ((c >= '0' && c <= '9') || c == '-')
                {
                    num_str.push_back(c);
                    ++i;
                }
                else if (c == 'e' || c == 'E')
                {
                    state = parse_number_state::exponent_sign;
                    ++i;
                }
                else if (c == '.')
                { 
                    state = parse_number_state::fraction;
                    ++i;
                }
                else
                {
                    not_a_number = true;
                }
                break;
            case parse_number_state::fraction:
                if ((c >= '0' && c <= '9'))
                {
                    ++decimal_places;
                    num_str.push_back(c);
                    ++i;
                }
                else if (c == 'e' || c == 'E')
                {
                    state = parse_number_state::exponent_sign;
                    ++i;
                }
                else
                {
                    not_a_number = true;
                }
                break;
            case parse_number_state::exponent_sign:
                if (c == '-')
                {
                    neg_exp = true;
                    state = parse_number_state::exponent_value;
                    ++i;
                }
                else if (c == '+')
                {
                    state = parse_number_state::exponent_value;
                    ++i;
                }
                else
                {
                    state = parse_number_state::exponent_value;
                }
                break;
            case parse_number_state::exponent_value:
                if ((c >= '0' && c <= '9'))
                {
                    exponent_str.push_back(c);
                    ++i;
                }
                else
                {
                    not_a_number = true;
                }
                break;
            case parse_number_state::err:
                i = token.size();
                break;
            default:
                not_a_number = true;
                break;
        }
    }

    if (not_a_number)
    {
        visitor.string_value(jsoncons::string_view(token.data(), token.size()));
        return result_type{};
    }

    if (!exponent_str.empty())
    {
        std::size_t exponent;
        auto r = dec_to_integer(exponent_str.data(), exponent_str.size(), exponent);
        JSONCONS_ASSERT(r);

        std::size_t n = num_str.size();

        if (neg_exp) // shift decimal point left
        {
            if ((exponent+decimal_places+1) > n)
            {
                num_str.insert(num_str.begin(), ((exponent+decimal_places+1) - n), '0');
            }
            std::size_t pos = num_str.size() - (decimal_places + exponent);
            auto first_non_zero = num_str.find_first_not_of('0', pos);
            if (first_non_zero == std::string::npos)
            {
                num_str.erase(num_str.begin() + pos, num_str.end());
            }
            else
            {
                num_str.insert(num_str.begin() + (num_str.size() - decimal_places - exponent), '.');
            }
        }
        else // shift decimal point right
        {
            if (exponent > decimal_places)
            {
                num_str.append(exponent - decimal_places, '0');
            }
            if (decimal_places > exponent)
            {
                num_str.insert(num_str.begin() + (num_str.size() - (decimal_places- exponent)), '.');
            }
        }
    }
    else
    {
        if (decimal_places > 0)
        {
            num_str.insert(num_str.begin() + (num_str.size()-decimal_places), '.');
        }

    }
    if (neg_value)
    {
        num_str.insert(num_str.begin(), '-');
    }

    if (not_a_number)
    {
        visitor.string_value(jsoncons::string_view(token.data(), token.size()));
        return result_type{};
    }
    else
    {

        std::uint64_t u64;
        auto ru64 = jsoncons::to_integer(num_str.data(), num_str.size(), u64);
        if (ru64)
        {
            visitor.uint64_value(u64);
            return result_type{};
        }
        std::int64_t i64;
        auto ri64 = jsoncons::to_integer(num_str.data(), num_str.size(), i64);
        if (ri64)
        {
            visitor.int64_value(i64);
            return result_type{};
        }

        double d;
        auto rd = jsoncons::decstr_to_double(num_str.data(), num_str.size(), d);
        if (rd)
        {
            visitor.double_value(d);
            return result_type{};
        }

        visitor.string_value(jsoncons::string_view(token.data(), token.size()));
    }

    return result_type{};
}

struct header_info
{
    jsoncons::optional<jsoncons::string_view> key;
    std::size_t length{0};
    char delimiter{','};
    std::vector<jsoncons::string_view> fields;
};

using header_result = jsoncons::expected<jsoncons::optional<header_info>, std::error_code>;

using line_result = read_result<std::size_t>;

struct parsed_line
{
    std::size_t depth{0};
    std::size_t indent{0};
    jsoncons::span<char> content;
    std::size_t line_num{0};

    bool is_blank() const
    {
        for (auto c : content)
        {
            if (!(c == ' ' || c == '\t'))
            {
                return false;
            }
        }
        return true;
    }
};

struct c
{
    std::size_t line_num{0};
    std::size_t indent{0};
    std::size_t depth{0};
};

inline
std::size_t find_unquoted_char(jsoncons::span<char> line, 
    char target_char,
    std::size_t start=0)
{
    bool in_quotes = false;
    std::size_t index = jsoncons::string_view::npos;
    bool done = false;
    for (std::size_t i = start; !done && i < line.size(); ++i)
    {
        char c = line[i];
        if (!in_quotes && c == '\"')
        {
            in_quotes = true;
        }
        else if (in_quotes && c == '\\' && i+1 < line.size())
        {
            ++i;
        }
        else if (in_quotes && c == '\"')
        {
            in_quotes = false;
        }
        if (!in_quotes && line[i] == target_char)
        {
            index = i;
            done = true;
        }
    }
    return index;
}

inline 
jsoncons::expected<std::pair<jsoncons::span<char>,jsoncons::span<char>>,std::error_code> split_key_value(jsoncons::span<char> line)
{
    using result_type = jsoncons::expected<std::pair<jsoncons::span<char>,jsoncons::span<char>>,std::error_code>;

    auto colon_idx = find_unquoted_char(line, ':');
    if (colon_idx == jsoncons::string_view::npos)
    {
        return result_type{jsoncons::unexpect, toon_errc::missing_colon};
    }
    return result_type{std::make_pair(
        strip(jsoncons::span<char>{line.data(),colon_idx}),
        strip(jsoncons::span<char>{line.data()+(colon_idx+1), line.size()-(colon_idx+1)}))
    };
}

line_result decode_array_from_header(const std::vector<parsed_line>& lines,
    bool list_item,
    std::size_t header_idx,
    std::size_t base_depth,
    const header_info& header_info,
    const toon_decode_options& options,
    json_visitor& visitor);

inline
jsoncons::expected<jsoncons::string_view, toon_errc> parse_key(jsoncons::span<char> key_str)
{
    using result_type = jsoncons::expected<jsoncons::string_view, toon_errc>;

    bool in_quotes{false};
    std::size_t start{0};
    std::size_t end{0};

    std::size_t i = 0;
    while (i < key_str.size() && key_str[i] == ' ')
    {
        ++start;
    }

    for (; i < key_str.size(); ++i)
    {
        char c = key_str[i];
        if (!in_quotes && c == '\"')
        {
            start = i + 1;
            in_quotes = true;
        }
        else if (in_quotes && c == '\\')
        {
            ++i;
        }
        else if (in_quotes && c == '\"')
        {
            end = i;
            return unescape_string(jsoncons::span<char>(key_str.data() + start, (end - start)));
        }
        else if (c != ' ')
        {
            end = i + 1;
        }
    }
    if (in_quotes) // unterminated_quoted_key
    {
        return result_type{jsoncons::unexpect, toon_errc::unterminated_quoted_key};
    }

    return result_type{jsoncons::string_view(key_str.data() + start, (end - start))};
}

inline
toon_errc parse_number(const char* data, std::size_t length, 
    json_visitor& visitor)
{
    if (length == 0)
    {
        visitor.string_value(jsoncons::string_view{});
        return toon_errc{};
    }
    const char* cur = data;
    bool sign = (*cur == '-');
    cur += sign;
    std::size_t len = length - sign;
    if (len == 0)
    {
        visitor.string_value(jsoncons::string_view{data, length});
        return toon_errc{};
    }

    if (len >= 2 && *cur == '0' && *(cur+1) != '.')
    {
        visitor.string_value(jsoncons::string_view(data, length));
        return toon_errc{};
    }

    const char* end = data + length;
    bool dot = false;
    while (cur < end)
    {
        if (*cur == '.')
        {
            dot = true;
        }
        else if (!((*cur >= '0' && *cur <= '9') || (!dot && (*cur == 'e' || *cur == 'E' || *cur == '-' || *cur == '+'))))
        {
            visitor.string_value(jsoncons::string_view(data, length));
            return toon_errc{};
        }
        ++cur;
    }

    std::uint64_t u64;
    auto ru64 = jsoncons::to_integer(data, length, u64);
    if (ru64)
    {
        visitor.uint64_value(u64);
        return toon_errc{};
    }
    std::int64_t i64;
    auto ri64 = jsoncons::to_integer(data, length, i64);
    if (ri64)
    {
        visitor.int64_value(i64);
        return toon_errc{};
    }

    double d;
    auto result = jsoncons::decstr_to_double(data, length, d);
    if (result)
    {
        visitor.double_value(d);
        return toon_errc{};
    }

    visitor.string_value(jsoncons::string_view(data, length));

    return toon_errc{};
}

inline 
jsoncons::expected<void,std::error_code> parse_delimited_values(jsoncons::span<char> line, 
    char delimiter,
    std::size_t expected_length,
    bool strict,
    json_visitor& visitor)
{
    using result_type = jsoncons::expected<void,std::error_code>;

    bool is_quoted = false;
    std::size_t offset = 0;
    std::size_t length = 0;
    bool is_empty = true;
    std::size_t num_items = 0;
    std::size_t num_delimiters = 0;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];

        if (c == delimiter && !is_quoted)
        {
            auto r = parse_primitive(strip(jsoncons::span<char>(line.data()+offset, length)), visitor);
            if (!r)
            {
                return result_type{jsoncons::unexpect, r.error()};
            }
            ++num_items;
            offset = i+1;
            length = 0;
            is_empty = false;
            ++num_delimiters;
        }
        else if (!is_quoted && c == '\"')
        {
            offset = i;
            length = 0;
            is_quoted = true;
        }
        else if (is_quoted && c == '\\' && i+1 < line.size())
        {
            length += 2;
            ++i;
        }
        else if (is_quoted && c == '\"')
        {
            auto r = parse_primitive(jsoncons::span<char>(line.data()+offset, length+2), visitor);
            if (!r)
            {
                return result_type{jsoncons::unexpect, r.error()};
            }
            ++num_items;
            while (++i < line.size())
            {
                if (line[i] == delimiter)
                {
                    ++num_delimiters;
                    break;
                }
            }
            is_empty = false;
            is_quoted = false;
            offset = i+1;
            length = 0;
        }
        else
        {
            ++length;
        }
    }
    if ((length > 0 || !is_empty) && (num_delimiters == num_items))
    {
        auto r = parse_primitive(strip(jsoncons::span<char>(line.data()+offset, length)), visitor);
        if (!r)
        {
            return result_type{jsoncons::unexpect, r.error()};
        }
        ++num_items;
    }

    return strict && expected_length != num_items ? result_type{jsoncons::unexpect, toon_errc::inline_array_length_mismatch} : result_type{};
}

inline 
jsoncons::expected<void,std::error_code> parse_delimited_values(jsoncons::span<char> line, 
    char delimiter,
    const std::vector<jsoncons::string_view>& fields,
    json_visitor& visitor)
{
    using result_type = jsoncons::expected<void,std::error_code>;

    bool is_quoted = false;
    std::size_t offset = 0;
    std::size_t length = 0;
    bool is_empty = true;

    std::size_t field_index = 0;
    std::size_t num_items = 0;
    std::size_t num_delimiters = 0;

    visitor.begin_object();
    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];

        if (c == delimiter && !is_quoted)
        {
            if (field_index >= fields.size())
            {
                return result_type{jsoncons::unexpect, toon_errc::too_many_values_in_row};
            }
            visitor.key(fields[field_index]);
            auto r = parse_primitive(strip(jsoncons::span<char>(line.data()+offset, length)), visitor);
            if (!r)
            {
                return result_type{jsoncons::unexpect, r.error()};
            }
            ++num_items;
            ++num_delimiters;
            offset = i+1;
            length = 0;
            is_empty = false;
            ++field_index;
        }
        else if (!is_quoted && c == '\"')
        {
            offset = i;
            length = 0;
            is_quoted = true;
        }
        else if (is_quoted && c == '\\' && i+1 < line.size())
        {
            length += 2;
            ++i;
        }
        else if (is_quoted && c == '\"')
        {
            if (field_index >= fields.size())
            {
                return result_type{jsoncons::unexpect, toon_errc::too_many_values_in_row};
            }
            visitor.key(fields[field_index]);
            auto r = parse_primitive(jsoncons::span<char>(line.data()+offset, length+2), visitor);
            if (!r)
            {
                return result_type{jsoncons::unexpect, r.error()};
            }
            ++num_items;
            while (++i < line.size())
            {
                if (line[i] == delimiter)
                {
                    ++num_delimiters;
                    break;
                }
            }
            is_quoted = false;
            offset = i+1;
            length = 0;
            ++field_index;
        }
        else
        {
            ++length;
        }
    }
    if ((length > 0 || !is_empty) && (num_delimiters == num_items))
    {
        if (field_index >= fields.size())
        {
            return result_type{jsoncons::unexpect, toon_errc::too_many_values_in_row};
        }
        visitor.key(fields[field_index]);
        auto r = parse_primitive(strip(jsoncons::span<char>(line.data()+offset, length)), visitor);
        if (!r)
        {
            return result_type{jsoncons::unexpect, r.error()};
        }
        ++field_index;
    }
    if (field_index != fields.size())
    {
        return result_type{jsoncons::unexpect, toon_errc::too_few_values_in_row};
    }
    visitor.end_object();
    return result_type{};
}

inline 
void parse_delimited_values(jsoncons::string_view line, 
    char delimiter,
    std::vector<jsoncons::string_view>& tokens)
{
    bool is_quoted = false;
    std::size_t offset = 0;
    std::size_t length = 0;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];

        if (c == delimiter && !is_quoted)
        {
            tokens.push_back(jsoncons::strip(jsoncons::string_view(line.data()+offset, length)));
            offset = i+1;
            length = 0;
        }
        else if (!is_quoted && c == '\"')
        {
            ++offset;
            length = 0;
            is_quoted = true;
        }
        else if (is_quoted && c == '\\' && i+1 < line.size())
        {
            length += 2;
            ++i;
        }
        else if (is_quoted && c == '\"')
        {
            is_quoted = false;
        }
        else
        {
            ++length;
        }
    }
    if (length > 0 || !tokens.empty())
    {
        tokens.push_back(jsoncons::strip(jsoncons::string_view(line.data()+offset, length)));
    }
}

inline
header_result parse_header(jsoncons::span<char> line)
{
    auto bracket_start = find_unquoted_char(line, '[');
    if (bracket_start == jsoncons::string_view::npos)
    {
        return header_result{};
    }
    auto key = jsoncons::optional<jsoncons::string_view>{};
    if (bracket_start > 0)
    {
        auto key_part = strip(jsoncons::span<char>{line.data(), bracket_start});
        if (!key_part.empty())
        {
            auto rkey = parse_key(key_part);
            if (!rkey)
            {
                return header_result{jsoncons::unexpect, rkey.error()};
            }
            key = *rkey;
        }
    }
    auto bracket_end = find_unquoted_char(line, ']', bracket_start);
    if (bracket_end == jsoncons::string_view::npos)
    {
        return header_result{};
    }

    jsoncons::string_view bracket_content = jsoncons::string_view(line.data() + (bracket_start + 1), bracket_end - (bracket_start + 1));
    if (jsoncons::starts_with(bracket_content, "#"))
    {
        bracket_content = jsoncons::string_view(bracket_content.data() + 1, bracket_content.size() - 1);
    }

    jsoncons::string_view length_str = bracket_content;

    char delimiter = ',';
    if (jsoncons::ends_with(bracket_content, '\t'))
    {
        delimiter = '\t';
        length_str = jsoncons::string_view(bracket_content.data(), bracket_content.size() - 1);
    }
    else if (jsoncons::ends_with(bracket_content, '|'))
    {
        delimiter = '|';
        length_str = jsoncons::string_view(bracket_content.data(), bracket_content.size() - 1);
    }
    else if (jsoncons::ends_with(bracket_content, ','))
    {
        delimiter = ',';
        length_str = jsoncons::string_view(bracket_content.data(), bracket_content.size() - 1);
    }

    std::size_t length{0};
    auto rc = to_integer(length_str.data(), length_str.size(), length);
    if (rc.ec != std::errc{})
    {
        return header_result{jsoncons::unexpect, toon_errc::invalid_value};
    }

    auto after_bracket = strip(jsoncons::span<char>(line.data() + (bracket_end + 1),
        line.size() - (bracket_end + 1)));

    std::vector<jsoncons::string_view> fields;
    if (starts_with(after_bracket, '{'))
    {
        auto brace_end = find_unquoted_char(after_bracket, '}');
        if (brace_end == jsoncons::string_view::npos)
        {
            return header_result{jsoncons::unexpect, toon_errc::unterminated_fields_segment};
        }
        auto fields_content = jsoncons::string_view(after_bracket.data() + 1, brace_end - 1);

        // Parse fields using the delimiter
        parse_delimited_values(fields_content, delimiter, fields);

        after_bracket = jsoncons::span<char>(after_bracket.data() + (brace_end + 1),
            after_bracket.size() - (brace_end + 1));
    }
    if (!starts_with(after_bracket, ':'))
    {
        fields.clear();
    }
    return header_result{jsoncons::in_place, header_info{jsoncons::optional<jsoncons::string_view>{key}, length, delimiter, std::move(fields)}};
}

inline
std::size_t compute_depth_from_indent(std::size_t indent_spaces, std::size_t indent_size)
{
    return static_cast<std::size_t>(std::floor(static_cast<double>(indent_spaces) / static_cast<double>(indent_size)));
}

inline
read_result<std::vector<parsed_line>> read_lines(jsoncons::span<char> raw, 
    const toon_decode_options& options)
{
    using result_type = read_result<std::vector<parsed_line>>;

    std::vector<parsed_line> lines;

    std::size_t indent_size = options.indent();
    bool strict = options.strict();

    std::size_t line_num = 1;
    std::size_t indent = 0;
    std::size_t start = 0;
    bool is_blank_line = true;
    std::size_t trailing_blanks = 0;

    std::size_t i = 0;
    for (; i < raw.size(); ++i)
    {
        char c = raw[i];
        if (c == '\t')
        {
            if (is_blank_line)
            {
                if (strict)
                {
                    return result_type{jsoncons::unexpect, toon_errc::tab_in_indentation, line_num, 0};
                }
                else
                {
                    indent += indent_size;
                    continue;
                }
            }
        }
        else if (c == ' ')
        {
            if (is_blank_line)
            {
                ++indent;
            }
            else
            {
                ++trailing_blanks;
            }
        }
        else
        {
            if (!(c == '\n'))
            {
                is_blank_line = false;
                trailing_blanks = 0;
            }
        }
        if (c == '\n')
        {
            if (strict && !is_blank_line && indent > 0 && indent % indent_size !=0)
            {
                return result_type{jsoncons::unexpect, toon_errc::indent_not_multiple_of_indent_size, line_num, 0};
            }
            std::size_t depth = compute_depth_from_indent(indent, indent_size);
            lines.push_back(parsed_line{depth, indent, jsoncons::span<char>{raw.data()+(start+indent), i-(start+indent+trailing_blanks)}, line_num});
            ++line_num;
            indent = 0;
            is_blank_line = true;
            start = i+1;
            trailing_blanks = 0;
        }
    }
    if (start < i)
    {
        if (strict && !is_blank_line && indent > 0 && indent % indent_size != 0)
        {
            return result_type{jsoncons::unexpect, toon_errc::indent_not_multiple_of_indent_size, line_num, 0};
        }
        std::size_t depth = compute_depth_from_indent(indent, indent_size);
        lines.push_back(parsed_line{depth, indent, jsoncons::span<char>{raw.data()+(start+indent), i-(start+indent+trailing_blanks)}, line_num});
    }

    return result_type{std::move(lines)};
}

inline
jsoncons::expected<void,std::error_code> decode_inline_array(jsoncons::span<char> content, 
    char delimiter,
    std::size_t expected_length,
    const toon_decode_options& options,
    json_visitor& visitor)
{
    using result_type = jsoncons::expected<void,std::error_code>;

    bool strict = options.strict();

    if (content.empty() && expected_length == 0)
    {
        visitor.begin_array();
        visitor.end_array();
        return result_type{};
    }
    visitor.begin_array();
    auto r = parse_delimited_values(content, delimiter, expected_length, strict, visitor);
    if (!r)
    {
        return r;
    }
    visitor.end_array();
    return result_type{};
}

inline
std::pair<std::size_t,char> find_first_unquoted(jsoncons::span<char> line, 
    jsoncons::span<const char> chars)
{
    bool in_quotes = false;
    for (std::size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];
        if (!in_quotes && c == '\"')
        {
            in_quotes = true;
        }
        else if (in_quotes && c == '\\' && i+1 < line.size())
        {
            ++i;
        }
        else if (in_quotes && c == '\"')
        {
            in_quotes = false;
        }
        if (!in_quotes)
        {
            for (auto chr : chars)
            {
                if (c == chr)
                {
                    return std::pair<std::size_t,char>{i, c};
                }
            }
        }
    }

    return std::pair<std::size_t,char>{jsoncons::string_view::npos, ' '};
}

inline
bool is_row_line(jsoncons::span<char> line, char delimiter) 
{
    // Find first occurrence of delimiter or colon (single pass optimization)

    char chars [2] = {delimiter, ':'};
    auto res = find_first_unquoted(line, chars);

    // No special chars found -> row
    if (res.first == jsoncons::string_view::npos)
        return true;

    // First special char is delimiter -> row
    // First special char is colon -> key-value
    return res.second == delimiter;
}

inline
line_result decode_object(const std::vector<parsed_line>& lines,
    std::size_t start_idx,
    std::size_t base_depth,
    const toon_decode_options& options,
    json_visitor& visitor)
{
    bool strict = options.strict();

    visitor.begin_object();

    std::size_t i = start_idx;
    std::size_t expected_depth = (start_idx == 0) ? base_depth : base_depth+1;

    while (i < lines.size())
    {
        const auto& line = lines[i];
        if (line.is_blank())
        {
            ++i;
            continue;
        }

        // Stop if we've dedented below expected depth
        if (line.depth < expected_depth)
        {
            break;
        }

        // Skip lines that are too deeply indented (they belong to nested structures)
        if (line.depth > expected_depth)
        {
            ++i;
            continue;
        }

        auto content = line.content;

        // Check for array header
        auto header_result = parse_header(content);
        if (!header_result)
        {
            return line_result{jsoncons::unexpect, header_result.error(), line.line_num, 0};
        }
        if (*header_result)
        {
            const header_info& header(*(*header_result));
            auto key = header.key;
            if (key)
            {
                // Array field
                visitor.key(*key);
                auto next_i_result = decode_array_from_header(lines, false, i, line.depth, header, options, visitor);
                if (!next_i_result)
                {
                    return line_result(jsoncons::unexpect, next_i_result.error());
                }
                i = *next_i_result;
                continue;
            }
        }

        auto kv = split_key_value(content);
        if (!kv)
        {
            // Invalid line, skip in non-strict mode
            if (strict)
            {
                return line_result{jsoncons::unexpect, kv.error(), line.line_num, 0};
            }
            ++i;
            continue;
        }

        auto key_str = kv->first;
        auto value_str = kv->second;

        auto rkey = parse_key(key_str);
        if (!rkey)
        {
            return line_result{jsoncons::unexpect, rkey.error(), line.line_num, 0};
        }
        auto key = *rkey;

        if (value_str.empty())
        {
            // Nested object
            visitor.key(key);
            auto r = decode_object(lines, i+1, line.depth, options, visitor);
            if (!r)
            {
                return r;
            }
            i = *r;
        }
        else
        {
            // Primitive value
            visitor.key(key);
            auto r = parse_primitive(value_str, visitor);
            if (!r)
            {
                return line_result{jsoncons::unexpect, r.error(), line.line_num, 0};
            }
            ++i;
        }
    }

    visitor.end_object();
    return line_result{i};
}

inline
line_result decode_list_array(const std::vector<parsed_line>& lines,
    std::size_t start_idx,
    std::size_t base_depth,
    std::size_t expected_length,
    const toon_decode_options& options,
    json_visitor& visitor)
{
    bool strict = options.strict();

    visitor.begin_array();

    std::size_t item_depth = base_depth + 1;

    std::size_t row_count = 0;
    std::size_t i = start_idx;
    while (i < lines.size())
    {
        const auto& line = lines[i];
        if (line.is_blank())
        {
            if (strict)
            {
                // In strict mode: blank lines at or above row depth are errors
                // Blank lines dedented below row depth mean array has ended
                if ((i-start_idx) < expected_length)
                {
                    return line_result{jsoncons::unexpect, toon_errc::blank_lines_in_array, line.line_num, 0};
                }
                else
                {
                    break;
                }
            }
            else
            {
                ++i;
                continue;
            }
        }
        // Stop if indented
        if (line.depth < item_depth)
        {
            break;
        }
        // Must start with "-"
        auto content = line.content;
        if (!starts_with(content, '-'))
        {
            break;
        }
        // Remove "- " prefix
        auto item_content = strip(jsoncons::span<char>(content.data()+1, content.size()-1));
        auto item_header_result = parse_header(item_content);
        if (!item_header_result)
        {
            return line_result{jsoncons::unexpect, item_header_result.error(), line.line_num, 0};
        }
        if (*item_header_result)
        {
            const header_info& item_header(*(*item_header_result));
            auto key = item_header.key;
            std::size_t length{item_header.length};
            char item_delim{item_header.delimiter};

            if (!key)
            {
                // - [N]: inline array
                auto it = std::find(item_content.begin(), item_content.end(), ':');
                if (it != item_content.end())
                {
                    auto colon_idx = it - item_content.begin();
                    auto inline_part = strip(jsoncons::span<char>(item_content.data()+(colon_idx+1), item_content.size()-(colon_idx+1)));
                    if (!inline_part.empty() || length == 0)
                    {
                        auto r = decode_inline_array(inline_part, item_delim, length, options, visitor);
                        if (!r)
                        {
                            return line_result{jsoncons::unexpect, r.error(), line.line_num, 0};
                        }
                        ++i;
                        ++row_count;
                        continue;
                    }
                    else 
                    {
                        auto next_i_result = decode_list_array(
                            lines, i + 1, base_depth, length, options, visitor);
                        if (!next_i_result)
                        {
                            return next_i_result;
                        }
                        i = *next_i_result;
                        ++row_count;
                        continue;
                    }
                }
            }
            else
            {
                visitor.begin_object();
                visitor.key(*key);
                auto next_i_result = decode_array_from_header(lines, true, i, line.depth, item_header, options, visitor);
                if (!next_i_result)
                {
                    return next_i_result;
                }
                i = *next_i_result;
                while (i < lines.size() && lines[i].depth == line.depth + 1)
                {
                    const auto& field_line = lines[i];
                    if (field_line.is_blank())
                    {
                        ++i;
                        continue;
                    }
                    auto field_content = field_line.content;

                    // Check for array header
                    auto field_header_result = parse_header(field_content);
                    if (!field_header_result)
                    {
                        return line_result{jsoncons::unexpect, field_header_result.error(), line.line_num, 0};
                    }
                    if (*field_header_result)
                    {
                        const header_info& field_header (*(*field_header_result));
                        auto field_key = field_header.key;

                        visitor.key(*field_key);
                        auto r1 = decode_array_from_header(lines, false, i, field_line.depth, field_header, options, visitor);
                        if (!r1)
                        {
                            return r1;
                        }
                        i = *r1;
                        continue;
                    }

                    auto kv = split_key_value(field_content);
                    if (!kv)
                    {
                        break;
                    }
                    auto field_key_str = kv->first;
                    auto field_value_str = kv->second;
                    auto rfield_key = parse_key(field_key_str);
                    if (!rfield_key)
                    {
                        return line_result{jsoncons::unexpect, rfield_key.error(), field_line.line_num, 0};
                    }
                    auto field_key = *rfield_key;
                    if (field_value_str.empty())
                    {
                        visitor.key(field_key);
                        auto r = decode_object(lines, i + 1, field_line.depth, options, visitor);
                        if (!r)
                        {
                            return line_result{jsoncons::unexpect, r.error()};
                        }
                        i = *r;
                    }
                    else
                    {
                        visitor.key(field_key);
                        auto r = parse_primitive(field_value_str, visitor);
                        if (!r)
                        {
                            return line_result{jsoncons::unexpect, r.error(), field_line.line_num, 0};
                        }
                        ++i;
                    }
                }
                visitor.end_object();
                ++row_count;
                continue;
            }
        }
        // Check if it's an object (has colon)

        auto kv = split_key_value(item_content);
        if (kv)
        {
            // It's an object item
            visitor.begin_object();
            auto key_str = kv->first;
            auto value_str = kv->second;
            auto rkey = parse_key(key_str);
            if (!rkey)
            {
                return line_result{jsoncons::unexpect, rkey.error(), line.line_num, 0};
            }
            auto key = *rkey;
            if (value_str.empty())
            {
                // First field is nested object: fields at depth +2
                visitor.key(key);
                auto r = decode_object(lines, i + 1, line.depth + 1, options, visitor);
                if (!r)
                {
                    return line_result{jsoncons::unexpect, r.error()};
                }
                i = *r;
            }
            else
            {
                // first field is primitive
                visitor.key(key);
                auto r = parse_primitive(value_str, visitor);
                if (!r)
                {
                    return line_result{jsoncons::unexpect, r.error(), line.line_num, 0};
                }
                ++i;
            }
            // Remaining fields at depth + 1
            while (i < lines.size() && lines[i].depth == line.depth + 1)
            {
                const auto& field_line = lines[i];
                if (field_line.is_blank())
                {
                    ++i;
                    continue;
                }
                auto field_content = field_line.content;

                // Check for array header
                auto field_header_result = parse_header(field_content);
                if (!field_header_result)
                {
                    return line_result{jsoncons::unexpect, field_header_result.error(), field_line.line_num, 0};
                }
                if (*field_header_result)
                {
                    const header_info& field_header (*(*field_header_result));
                    auto field_key = field_header.key;
                    visitor.key(*field_key);
                    auto r1 = decode_array_from_header(lines, false, i, field_line.depth, field_header, options, visitor);
                    if (!r1)
                    {
                        return r1;
                    }
                    i = *r1;
                    continue;
                }

                auto field_kv = split_key_value(field_content);
                if (!field_kv)
                {
                    break;
                }
                auto field_key_str = field_kv->first;
                auto field_value_str = field_kv->second;
                auto rfield_key = parse_key(field_key_str);
                if (!rfield_key)
                {
                    return line_result{jsoncons::unexpect, rfield_key.error(), i + 1, 0};
                }
                auto field_key = *rfield_key;
                if (field_value_str.empty())
                {
                    // Nested object
                    visitor.key(field_key);
                    auto r = decode_object(lines, i + 1, field_line.depth, options, visitor);
                    if (!r)
                    {
                        return line_result{jsoncons::unexpect, r.error()};
                    }
                    i = *r;
                }
                else
                {
                    visitor.key(field_key);
                    auto r = parse_primitive(field_value_str, visitor);
                    if (!r)
                    {
                        return line_result{jsoncons::unexpect, r.error(), field_line.line_num, 0};
                    }
                    ++i;
                }
            }
            visitor.end_object();
            ++row_count;
        }
        else
        {
            // Not an object, must be a primitive;
            if (item_content.empty())
            {
                visitor.begin_object();
                visitor.end_object();
                ++row_count;
                ++i;
            }
            else
            {
                auto r = parse_primitive(item_content, visitor);
                if (!r)
                {
                    return line_result{jsoncons::unexpect, r.error(), line.line_num, 0};
                }
                ++row_count;
                ++i;
            }
        }

    }

    visitor.end_array();

    return strict && expected_length != row_count ? line_result{jsoncons::unexpect, toon_errc::list_array_length_mismatch, i+1, 0} : line_result{i};
}

inline
line_result decode_tabular_array(const std::vector<parsed_line>& lines,
    std::size_t start_idx,
    std::size_t base_depth,
    const std::vector<jsoncons::string_view>& fields,
    char delimiter,
    std::size_t expected_length,
    const toon_decode_options& options,
    json_visitor& visitor)
{
    bool strict = options.strict();

    visitor.begin_array();

    std::size_t i = start_idx;
    std::size_t row_depth = base_depth + 1;
    std::size_t row_count =0;

    while (i < lines.size())
    {
        const auto& line = lines[i];
        if (line.is_blank())
        {
            if (strict)
            {
                // In strict mode: blank lines at or above row depth are errors
                // Blank lines dedented below row depth mean array has ended

                if ((i-start_idx) < expected_length)
                {
                    return line_result{jsoncons::unexpect, toon_errc::blank_lines_in_array, line.line_num, 0};
                }
                else
                {
                    break;
                }
            }
            else
            {
                ++i;
                continue;
            }
        }
        if (line.depth < row_depth)
        {
            break;
        }
        if (line.depth > row_depth)
        {
            break;
        }
        auto content = line.content;
        if (is_row_line(content, delimiter))
        {
            auto r = parse_delimited_values(content, delimiter, fields, visitor);
            if (!r)
            {
                return line_result{jsoncons::unexpect, r.error(), line.line_num, 0};
            }
            ++row_count;
            ++i;
        }
        else
        {
            break;
        }
    }

    visitor.end_array();

    return strict && expected_length != row_count ? 
        line_result{jsoncons::unexpect, toon_errc::tabular_array_length_mismatch,i+1,0} : line_result{i};
}

inline
line_result decode_array_from_header(const std::vector<parsed_line>& lines,
    bool list_item,
    std::size_t header_idx,
    std::size_t base_depth,
    const header_info& header_info,
    const toon_decode_options& options,
    json_visitor& visitor)
{
    auto key = header_info.key;
    std::size_t length{header_info.length};
    char delimiter{header_info.delimiter};
    const std::vector<jsoncons::string_view>& fields{header_info.fields};

    const jsoncons::span<char>& header_line{lines[header_idx].content};

    // Check if there's inline content after the colon
    // Use split_key_value to find the colon position (respects quoted strings)

    std::size_t colon_idx = find_unquoted_char(header_line, ':');
    if (colon_idx == jsoncons::string_view::npos)
    {
        return line_result{jsoncons::unexpect, toon_errc::missing_colon_after_key, header_idx+1, 0};
    }
    auto inline_content = strip(jsoncons::span<char>(header_line.data() + (colon_idx + 1), header_line.size() - (colon_idx + 1)));

    if (!inline_content.empty() || (fields.empty() && length == 0))
    {
        auto r = decode_inline_array(inline_content, delimiter, length, options, visitor);
        if (!r)
        {
            return line_result(jsoncons::unexpect, r.error(), header_idx+1, 0);
        }
        return line_result{header_idx + 1};
    }

    // Check for tabular-first list-item object: `- key[N]{fields}:`
    if (!fields.empty())
    {
        if (key && list_item)
        {
            // Tabular array
            // Use base_depth + 1 for the array so rows are at base_depth + 2
            return decode_tabular_array(lines, header_idx + 1, base_depth+1, fields, delimiter, length, options, visitor);
        }
        else
        {
            return decode_tabular_array(lines, header_idx + 1, base_depth, fields, delimiter, length, options, visitor);
        }
    }
    else
    {
        return decode_list_array(lines, header_idx + 1, base_depth, length, options, visitor);
    }
}

template <typename Source=jsoncons::stream_source<char>,typename TempAlloc =std::allocator<char>>
class basic_toon_reader 
{
public:
    using char_type = char;
    using source_type = Source;
    using string_view_type = jsoncons::string_view;
private:
    using char_allocator_type = typename std::allocator_traits<TempAlloc>:: template rebind_alloc<char_type>;

    static constexpr size_t default_max_buffer_size = 16384;

    source_type source_;
    default_json_visitor default_visitor_;
    json_visitor& visitor_;
    toon_decode_options options_;

    // Noncopyable and nonmoveable
    basic_toon_reader(const basic_toon_reader&) = delete;
    basic_toon_reader& operator=(const basic_toon_reader&) = delete;

public:

    template <typename Sourceable>
    basic_toon_reader(Sourceable&& source, 
        const toon_decode_options& options = toon_decode_options{}, 
        const TempAlloc& temp_alloc = TempAlloc())
        : basic_toon_reader(std::forward<Sourceable>(source),
                            default_visitor_,
                            options,
                            temp_alloc)
    {
    }

    template <typename Sourceable>
    basic_toon_reader(Sourceable&& source, 
        json_visitor& visitor, 
        const TempAlloc& temp_alloc = TempAlloc())
        : basic_toon_reader(std::forward<Sourceable>(source),
                            visitor,
                            toon_decode_options{},
                            temp_alloc)
    {
    }

    template <typename Sourceable>
    basic_toon_reader(Sourceable&& source, 
        json_visitor& visitor,
        const toon_decode_options& options, 
        const TempAlloc& /*temp_alloc*/ = TempAlloc())
    : source_(std::forward<Sourceable>(source)),
      visitor_(visitor),
      options_(options)
    {
    }

    void read()
    {
        std::error_code ec;
        auto result = try_read();
        if (!result)
        {
            JSONCONS_THROW(ser_error(result.error().code()));
        }
    }

    jsoncons::expected<void,read_error> try_read()
    {
        using result_type = jsoncons::expected<void, read_error>;

        std::string raw;

        while (!source_.eof())
        {
            auto s = source_.read_buffer();
            raw.append(s.data(), s.size());
        }

        auto r_lines = read_lines(jsoncons::span<char>{&raw[0], raw.size()}, options_);
        if (!r_lines)
        {
            return result_type{jsoncons::unexpect, r_lines.error()};
        }
        const std::vector<parsed_line>& lines{*r_lines};

        std::size_t count = 0;
        parsed_line first_line{};

        for (std::size_t i = 0; i < lines.size() && count < 2; ++i)
        {
            if (!lines[i].is_blank())
            {
                if (count == 0)
                {
                    first_line = lines[i];
                }
                ++count;
            }
        }
        if (count == 0)
        {
            visitor_.begin_object();
            visitor_.end_object();
            return result_type{};
        }

        auto header_result = parse_header(first_line.content);
        if (!header_result)
        {
            return result_type{jsoncons::unexpect, header_result.error(), first_line.line_num, 0};
        }
        if (*header_result && !(*header_result)->key)
        {
            // Root array
            const header_info& header(*(*header_result));
            auto r1 = decode_array_from_header(lines, false, 0, 0, header, options_, visitor_);
            if (!r1)
            {
                return result_type{jsoncons::unexpect, r1.error()};
            }
            return result_type{};
        }

        // Determine root form (Section 5)

        // Check if it's a single primitive
        if (count == 1)
        {
            // Check if it's not a key-value line
            auto kv = split_key_value(first_line.content);
            if (!kv)
            {
                // Not a key-value, check if it's a header
                if (!header_result || !(*header_result))
                {
                    // Single primitive
                    auto r = parse_primitive(first_line.content, visitor_);
                    if (!r)
                    {
                        return result_type{jsoncons::unexpect, r.error()};
                    }
                    return result_type{};
                }
            }
        }

        // Otherwise, root object
        auto r = decode_object(lines, 0, 0, options_, visitor_);
        if (!r)
        {
            return result_type{jsoncons::unexpect, r.error()};
        }
        return result_type{};
    }
};

using toon_string_reader = basic_toon_reader<string_source<char>>;
using toon_stream_reader = basic_toon_reader<stream_source<char>>;

} // namespace toon
} // namespace jsoncons

#endif // JSONCONS_TOON_TOON_READER_HPP


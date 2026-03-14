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
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/ser_utils.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/ser_utils.hpp>
#include <jsoncons/utility/read_number.hpp>
#include <jsoncons/utility/unicode_traits.hpp>
#include <jsoncons/utility/string_utils.hpp>
#include <jsoncons_ext/toon/toon_options.hpp>
#include <jsoncons_ext/toon/toon_error.hpp>

namespace jsoncons {
namespace toon {

enum class parse_number_state{sign,zero,digits,fraction,exponent_sign,exponent_value,err};

inline
jsoncons::expected<void,std::error_code> parse_number_or_string(jsoncons::string_view str, jsoncons::json_visitor& visitor)
{
    using result_type = jsoncons::expected<void,std::error_code>;

    std::string result;

    std::string num_str;
    std::string exponent_str;

    bool neg_value = false;
    bool neg_exp = false;
    bool not_a_number = false;

    std::size_t decimal_places = 0;

    parse_number_state state = parse_number_state::sign;
    for (std::size_t i = 0; i < str.size() && !not_a_number;)
    {
        char c = str[i];
        switch (state)
        {
            case parse_number_state::sign:
                if (c == '-')
                {
                    neg_value = true;
                    ++i;
                }
                if (i < str.size() && str[i] == '0')
                {
                    num_str.push_back('0');
                    state = parse_number_state::zero;
                    if (++i == str.size())
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
                    if ((str.size() - i) == 2 && (str[i] == 'e' || str[i] == 'E') && str[i + 1] == '1')
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
                i = str.size();
                break;
            default:
                not_a_number = true;
                break;
        }
    }

    if (not_a_number)
    {
        visitor.string_value(str);
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
        visitor.string_value(str);
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

        visitor.string_value(str);
    }

    return result_type{};
}

inline
jsoncons::expected<std::string,toon_errc> unescape_string(jsoncons::string_view value)
{
    using result_type = jsoncons::expected<std::string,toon_errc>;

    result_type result{};

    std::string& str{*result};

    std::size_t i = 0;

    while (i < value.size())
    {
        if (value[i] == '\\')
        {
            if (i + 1 >= value.size())
            {
                return result_type{jsoncons::unexpect, toon_errc::invalid_escape_sequence};
            }
            char next_char = value[i + 1];
            if (next_char == 'n')
            {
                str.push_back('\n');
                i += 2;
                continue;
            }
            if (next_char == 't')
            {
                str.push_back('\t');
                i += 2;
                continue;
            }
            if (next_char == 'r')
            {
                str.push_back('\r');
                i += 2;
                continue;
            }
            if (next_char == '\\')
            {
                str.push_back('\\');
                i += 2;
                continue;
            }
            if (next_char == '\"')
            {
                str.push_back('\"');
                i += 2;
                continue;
            }
            return result_type{jsoncons::unexpect, toon_errc::invalid_escape_sequence};
        }
        str.push_back(value[i]);
        ++i;
    }

    return result;
}

struct header_info
{
    jsoncons::optional<std::string> key;
    std::size_t length{0};
    char delimiter{','};
    std::vector<jsoncons::string_view> fields;
};

using header_result = read_result<jsoncons::optional<header_info>>;

using line_result = read_result<std::size_t>;

struct parsed_line
{
    std::size_t depth{0};
    std::size_t indent{0};
    jsoncons::string_view content;
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

struct blank_line_info
{
    std::size_t line_num{0};
    std::size_t indent{0};
    std::size_t depth{0};
};

inline
std::size_t find_unquoted_char(jsoncons::string_view line, 
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
jsoncons::expected<std::pair<jsoncons::string_view,jsoncons::string_view>,std::error_code> split_key_value(jsoncons::string_view line)
{
    using result_type = jsoncons::expected<std::pair<jsoncons::string_view,jsoncons::string_view>,std::error_code>;

    auto colon_idx = find_unquoted_char(line, ':');
    if (colon_idx == jsoncons::string_view::npos)
    {
        return result_type{jsoncons::unexpect, toon_errc::missing_colon};
    }
    return result_type{std::make_pair(
        jsoncons::strip(jsoncons::string_view{line.data(),colon_idx}),
        jsoncons::strip(jsoncons::string_view{line.data()+(colon_idx+1), line.size()-(colon_idx+1)}))
    };
}

line_result decode_array_from_header(const std::vector<parsed_line>& lines,
    std::size_t header_idx,
    std::size_t base_depth,
    const header_info& header_info,
    bool strict,
    json_visitor& visitor);

inline
toon_errc parse_key(jsoncons::string_view key_str, std::string& result)
{
    bool in_quotes{false};
    std::size_t start{0};
    std::size_t end{0};

    bool terminated{false};

    std::size_t i = 0;
    while (i < key_str.size() && key_str[i] == ' ')
    {
        ++start;
    }

    for (;i < key_str.size() && !terminated; ++i)
    {
        char c = key_str[i];
        if (!in_quotes && c == '\"')
        {
            start = i+1;
            in_quotes = true;
        }
        else if (in_quotes && c == '\\')
        {
            ++i;
        }
        else if (in_quotes && c == '\"')
        {
            end = i;
            terminated = true;
            in_quotes = false;
            auto res = unescape_string(jsoncons::string_view(key_str.data() + start, (end-start)));
            if (!res) 
            {
                return res.error();
            }
            result = *res;
            return toon_errc{};
        }
        else if (c != ' ')
        {
            end = i+1;
        }
    }
    if (terminated)
    {
        result = std::string(key_str.data() + start, (end-start));
        return toon_errc{};
    }
    if (in_quotes) // unterminated_quoted_key
    {
        return toon_errc::unterminated_quoted_key;
    }
    result = std::string(key_str.data() + start, (end-start));

    return toon_errc{};
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
jsoncons::expected<void,std::error_code> parse_primitive(jsoncons::string_view token, 
    json_visitor& visitor)
{
    using result_type = jsoncons::expected<void,std::error_code>;

    token = jsoncons::strip(token);

    if (token.empty())
    {
        visitor.string_value(jsoncons::string_view{});
        return result_type{};
    }

    if (jsoncons::starts_with(token, '\"'))
    {
        if (!jsoncons::ends_with(token, '\"') || token.size() < 2)
        {
            return result_type{jsoncons::unexpect, toon_errc::missing_closing_quote};
        }
        auto result = unescape_string(jsoncons::string_view(token.data()+1, token.size()-2));
        if (!result)
        {
            return result_type{jsoncons::unexpect, result.error()};
        }
        visitor.string_value(*result);
        return result_type{};
    }
    if (token == "true")
    {
        visitor.bool_value(true);
        return result_type{};
    }
    if (token == "false")
    {
        visitor.bool_value(false);
        return result_type{};
    }
    if (token == "null")
    {
        visitor.null_value();
        return result_type{};
    }
    return parse_number_or_string(token, visitor);
}

inline 
jsoncons::expected<void,std::error_code> parse_delimited_values(jsoncons::string_view line, 
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
            auto r = parse_primitive(jsoncons::strip(jsoncons::string_view(line.data()+offset, length)), visitor);
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
            auto r = parse_primitive(jsoncons::string_view(line.data()+offset, length+2), visitor);
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
        auto r = parse_primitive(jsoncons::strip(jsoncons::string_view(line.data()+offset, length)), visitor);
        if (!r)
        {
            return result_type{jsoncons::unexpect, r.error()};
        }
        ++num_items;
    }

    return strict && expected_length != num_items ? result_type{jsoncons::unexpect, toon_errc::inline_array_length_mismatch} : result_type{};
}

inline 
line_result parse_delimited_values(jsoncons::string_view line, 
    char delimiter,
    const std::vector<jsoncons::string_view>& fields,
    json_visitor& visitor)
{
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
                return line_result{jsoncons::unexpect, toon_errc::too_many_values_in_row};
            }
            visitor.key(fields[field_index]);
            auto r = parse_primitive(jsoncons::strip(jsoncons::string_view(line.data()+offset, length)), visitor);
            if (!r)
            {
                return line_result{jsoncons::unexpect, r.error()};
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
                return line_result{jsoncons::unexpect, toon_errc::too_many_values_in_row};
            }
            visitor.key(fields[field_index]);
            auto r = parse_primitive(jsoncons::string_view(line.data()+offset, length+2), visitor);
            if (!r)
            {
                return line_result{jsoncons::unexpect, r.error()};
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
            return line_result{jsoncons::unexpect, toon_errc::too_many_values_in_row};
        }
        visitor.key(fields[field_index]);
        auto r = parse_primitive(jsoncons::strip(jsoncons::string_view(line.data()+offset, length)), visitor);
        if (!r)
        {
            return line_result{jsoncons::unexpect, r.error()};
        }
        ++field_index;
    }
    if (field_index != fields.size())
    {
        return line_result{jsoncons::unexpect, toon_errc::too_few_values_in_row};
    }
    visitor.end_object();
    return line_result{};
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
header_result parse_header(jsoncons::string_view line)
{
    auto bracket_start = find_unquoted_char(line, '[');
    if (bracket_start == jsoncons::string_view::npos)
    {
        return header_result{};
    }
    auto key = jsoncons::optional<std::string>{};
    if (bracket_start > 0)
    {
        auto key_part = jsoncons::strip(jsoncons::string_view{line.data(), bracket_start});
        if (!key_part.empty())
        {
            std::string key_str;
            toon_errc ec = parse_key(key_part, key_str);
            if (ec != toon_errc{})
            {
                return header_result{jsoncons::unexpect, ec};
            }
            key = key_str;
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

    auto after_bracket = jsoncons::strip(jsoncons::string_view(line.data() + (bracket_end + 1),
        line.size() - (bracket_end + 1)));

    std::vector<jsoncons::string_view> fields;
    if (jsoncons::starts_with(after_bracket, '{'))
    {
        auto brace_end = find_unquoted_char(after_bracket, '}');
        if (brace_end == jsoncons::string_view::npos)
        {
            return header_result{jsoncons::unexpect, toon_errc::unterminated_fields_segment};
        }
        auto fields_content = jsoncons::string_view(after_bracket.data() + 1, brace_end - 1);

        // Parse fields using the delimiter
        parse_delimited_values(fields_content, delimiter, fields);

        after_bracket = jsoncons::string_view(after_bracket.data() + (brace_end + 1),
            after_bracket.size() - (brace_end + 1));
    }
    if (!jsoncons::starts_with(after_bracket, ':'))
    {
        fields.clear();
    }
    return header_result{jsoncons::in_place, header_info{jsoncons::optional<std::string>{std::move(key)}, length, delimiter, std::move(fields)}};
}

inline
std::size_t compute_depth_from_indent(std::size_t indent_spaces, std::size_t indent_size)
{
    return static_cast<std::size_t>(std::floor(static_cast<double>(indent_spaces) / static_cast<double>(indent_size)));
}

inline
void read_lines(jsoncons::string_view raw, 
    std::size_t indent_size,
    bool strict,
    std::vector<parsed_line>& lines,
    std::vector<blank_line_info>& blank_lines,
    std::error_code& ec)
{
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
                    ec = toon_errc::tab_in_indentation;
                    return;
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
                ec = toon_errc::indent_not_multiple_of_indent_size;
                return;
            }
            std::size_t depth = compute_depth_from_indent(indent, indent_size);
            if (is_blank_line)
            {
                blank_lines.push_back(blank_line_info{line_num,indent,depth});
            }
            lines.push_back(parsed_line{depth, indent, jsoncons::string_view{raw.data()+(start+indent), i-(start+indent+trailing_blanks)}, line_num});
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
            ec = toon_errc::indent_not_multiple_of_indent_size;
            return;
        }
        std::size_t depth = compute_depth_from_indent(indent, indent_size);
        if (is_blank_line)
        {
            blank_lines.push_back(blank_line_info{line_num, indent, depth});
        }
        lines.push_back(parsed_line{depth, indent, jsoncons::string_view{raw.data()+(start+indent), i-(start+indent+trailing_blanks)}, line_num});
    }

}

inline
jsoncons::expected<void,std::error_code> decode_inline_array(jsoncons::string_view content, 
    char delimiter,
    std::size_t expected_length,
    bool strict,
    json_visitor& visitor)
{
    using result_type = jsoncons::expected<void,std::error_code>;

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
std::pair<std::size_t,char> find_first_unquoted(jsoncons::string_view line, 
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
bool is_row_line(jsoncons::string_view line, char delimiter) 
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
jsoncons::expected<void,read_error> decode_object(const std::vector<parsed_line>& lines,
    std::size_t start_idx,
    std::size_t base_depth,
    bool strict,
    json_visitor& visitor)
{
    using result_type = jsoncons::expected<void, read_error>;

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
            return result_type{jsoncons::unexpect, header_result.error()};
        }
        if (*header_result)
        {
            const header_info& header(*(*header_result));
            const jsoncons::optional<std::string>& key(header.key);
            if (key)
            {
                // Array field
                visitor.key(*key);
                auto next_i_result = decode_array_from_header(lines, i, line.depth, header, strict, visitor);
                if (!next_i_result)
                {
                    return result_type(jsoncons::unexpect, next_i_result.error());
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
                return result_type{jsoncons::unexpect, kv.error()};
            }
            ++i;
            continue;
        }

        auto key_str = kv->first;
        auto value_str = kv->second;

        std::string key;
        parse_key(key_str, key);

        if (value_str.empty())
        {
            // Nested object
            visitor.key(key);
            auto r = decode_object(lines, i+1, line.depth, strict, visitor);
            if (!r)
            {
                return result_type{jsoncons::unexpect, r.error()};
            }
            // Skip past nested object
            ++i;
            while (i < lines.size() && lines[i].depth > line.depth)
            {
                ++i;
            }
        }
        else
        {
            // Primitive value
            visitor.key(key);
            auto r = parse_primitive(value_str, visitor);
            if (!r)
            {
                return result_type{jsoncons::unexpect, r.error()};
            }
            ++i;
        }
    }

    visitor.end_object();
    return result_type{};
}

inline
line_result decode_list_array(const std::vector<parsed_line>& lines,
    std::size_t start_idx,
    std::size_t base_depth,
    std::size_t expected_length,
    bool strict,
    json_visitor& visitor)
{
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
                    return line_result{jsoncons::unexpect, toon_errc::blank_lines_in_array};
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
        if (!jsoncons::starts_with(content, "-"))
        {
            break;
        }
        // Remove "- " prefix
        auto item_content = jsoncons::strip(jsoncons::string_view(content.data()+1, content.size()-1));
        auto item_header_result = parse_header(item_content);
        if (!item_header_result)
        {
            return line_result{jsoncons::unexpect, item_header_result.error()};
        }
        if (*item_header_result)
        {
            const header_info& item_header(*(*item_header_result));
            const jsoncons::optional<std::string>& key(item_header.key);
            std::size_t length{item_header.length};
            char item_delim{item_header.delimiter};

            if (!key)
            {
                // - [N]: inline array
                auto colon_idx = item_content.find(':');
                if (colon_idx != jsoncons::string_view::npos)
                {
                    auto inline_part = jsoncons::strip(jsoncons::string_view(item_content.data()+(colon_idx+1), item_content.size()-(colon_idx+1)));
                    if (!inline_part.empty() || length == 0)
                    {
                        auto r = decode_inline_array(inline_part, item_delim, length, strict, visitor);
                        if (!r)
                        {
                            return line_result{jsoncons::unexpect, r.error()};
                        }
                        ++i;
                        ++row_count;
                        continue;
                    }
                    else 
                    {
                        auto next_i_result = decode_list_array(
                            lines, i + 1, base_depth, length, strict, visitor);
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
                auto next_i_result = decode_array_from_header(lines, i, line.depth, item_header, strict, visitor);
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
                        return line_result{jsoncons::unexpect, field_header_result.error()};
                    }
                    if (*field_header_result)
                    {
                        const header_info& field_header (*(*field_header_result));
                        const jsoncons::optional<std::string>& field_key(field_header.key);

                        visitor.key(*field_key);
                        auto r1 = decode_array_from_header(lines, i, field_line.depth, field_header, strict, visitor);
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
                    std::string field_key;
                    parse_key(field_key_str, field_key);
                    if (field_value_str.empty())
                    {
                        visitor.key(field_key);
                        auto r = decode_object(lines, i + 1, field_line.depth, strict, visitor);
                        if (!r)
                        {
                            return line_result{jsoncons::unexpect, r.error()};
                        }
                        ++i;
                        while (i < lines.size() && lines[i].depth > field_line.depth)
                        {
                            ++i;
                        }
                    }
                    else
                    {
                        visitor.key(field_key);
                        auto r = parse_primitive(field_value_str, visitor);
                        if (!r)
                        {
                            return line_result{jsoncons::unexpect, r.error()};
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
            std::string key;
            parse_key(key_str, key);
            if (value_str.empty())
            {
                // First field is nested object: fields at depth +2
                visitor.key(key);
                auto r = decode_object(lines, i + 1, line.depth + 1, strict, visitor);
                if (!r)
                {
                    return line_result{jsoncons::unexpect, r.error()};
                }
                // Skip nested content
                ++i;
                while (i < lines.size() && lines[i].depth > line.depth + 1)
                {
                    ++i;
                }
            }
            else
            {
                // first field is primitive
                visitor.key(key);
                auto r = parse_primitive(value_str, visitor);
                if (!r)
                {
                    return line_result{jsoncons::unexpect, r.error()};
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
                    return line_result{jsoncons::unexpect, field_header_result.error()};
                }
                if (*field_header_result)
                {
                    const header_info& field_header (*(*field_header_result));
                    const auto& field_key{field_header.key};
                    visitor.key(*field_key);
                    auto r1 = decode_array_from_header(lines, i, field_line.depth, field_header, strict, visitor);
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
                std::string field_key;
                parse_key(field_key_str, field_key);
                if (field_value_str.empty())
                {
                    // Nested object
                    visitor.key(field_key);
                    auto r = decode_object(lines, i + 1, field_line.depth, strict, visitor);
                    if (!r)
                    {
                        return line_result{jsoncons::unexpect, r.error()};
                    }

                    ++i;
                    while (i < lines.size() && lines[i].depth > field_line.depth)
                    {
                        ++i;
                    }
                }
                else
                {
                    visitor.key(field_key);
                    auto r = parse_primitive(field_value_str, visitor);
                    if (!r)
                    {
                        return line_result{jsoncons::unexpect, r.error()};
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
                    return line_result{jsoncons::unexpect, r.error()};
                }
                ++row_count;
                ++i;
            }
        }

    }

    visitor.end_array();

    return strict && expected_length != row_count ? line_result{jsoncons::unexpect, toon_errc::list_array_length_mismatch} : line_result{i};
}

inline
line_result decode_tabular_array(const std::vector<parsed_line>& lines,
    std::size_t start_idx,
    std::size_t base_depth,
    const std::vector<jsoncons::string_view>& fields,
    char delimiter,
    std::size_t expected_length,
    bool strict,
    json_visitor& visitor)
{
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
                    return line_result{jsoncons::unexpect, toon_errc::blank_lines_in_array};
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
        jsoncons::string_view content = line.content;
        if (is_row_line(content, delimiter))
        {
            auto r = parse_delimited_values(content, delimiter, fields, visitor);
            if (!r)
            {
                return r;
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

    return strict && expected_length != row_count ? line_result{jsoncons::unexpect, toon_errc::tabular_array_length_mismatch} : line_result{i};
}

inline
line_result decode_array_from_header(const std::vector<parsed_line>& lines,
    std::size_t header_idx,
    std::size_t base_depth,
    const header_info& header_info,
    bool strict,
    json_visitor& visitor)
{
    //const jsoncons::optional<std::string>& key(header_info.key);
    std::size_t length{header_info.length};
    char delimiter{header_info.delimiter};
    const std::vector<jsoncons::string_view>& fields{header_info.fields};

    const jsoncons::string_view& header_line{lines[header_idx].content};

    // Check if there's inline content after the colon
    // Use split_key_value to find the colon position (respects quoted strings)

    std::size_t colon_idx = find_unquoted_char(header_line, ':');
    if (colon_idx == jsoncons::string_view::npos)
    {
        return line_result{jsoncons::unexpect, toon_errc::missing_colon_after_key};
    }
    auto inline_content = jsoncons::strip(jsoncons::string_view(header_line.data() + (colon_idx + 1), header_line.size() - (colon_idx + 1)));

    if (!inline_content.empty() || (fields.empty() && length == 0))
    {
        auto r = decode_inline_array(inline_content, delimiter, length, strict, visitor);
        if (!r)
        {
            return line_result(jsoncons::unexpect, r.error());
        }
        return line_result{header_idx + 1};
    }

    // Check for tabular-first list-item object: `- key[N]{fields}:`
    if (!fields.empty())
    {
        if (base_depth != 0)
        {
            // Tabular array
            // Use base_depth + 1 for the array so rows are at base_depth + 2
            return decode_tabular_array(
                lines, header_idx + 1, base_depth+1, fields, delimiter, length, strict, visitor);
        }
        else
        {
            return decode_tabular_array(
                lines, header_idx + 1, base_depth, fields, delimiter, length, strict, visitor);
        }
    }
    else
    {
        return decode_list_array(
            lines, header_idx + 1, base_depth, length, strict, visitor);
    }
}


inline
line_result decode_array(const std::vector<parsed_line>& lines,
    std::size_t start_idx,
    std::size_t base_depth,
    const header_info& header_info,
    bool strict,
    json_visitor& visitor)
{
    return decode_array_from_header(lines, start_idx, base_depth, header_info, strict, visitor);
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
    std::size_t indent_size_;
    bool strict_;
    std::string raw_;
    std::vector<parsed_line> lines_;
    std::vector<blank_line_info> blank_lines_;

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
      indent_size_(options.indent()),
      strict_(options.strict())
    {
    }

    void read()
    {
        std::error_code ec;
        read(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    void read(std::error_code& ec)
    {
        while (!source_.eof())
        {
            auto s = source_.read_buffer();
            raw_.append(s.data(), s.size());
        }

        read_lines(raw_, indent_size_, strict_, lines_, blank_lines_, ec);
        if (ec)
        {
            return;
        }
        std::vector<parsed_line> non_blank_lines;
        for (const auto& ln : lines_)
        {
            if (!ln.is_blank())
            {
                non_blank_lines.push_back(ln);
            }
        }
        if (non_blank_lines.empty())
        {
            visitor_.begin_object();
            visitor_.end_object();
            return;
        }

        auto header_result = parse_header(non_blank_lines[0].content);
        if (!header_result)
        {
            ec = header_result.error().code();
            return;
        }
        if (*header_result && !(*header_result)->key)
        {
            // Root array
            const header_info& header(*(*header_result));
            auto r1 = decode_array(lines_, 0, 0, header, strict_, visitor_);
            if (!r1)
            {
                ec = r1.error().code();
            }
            return;
        }

        // Determine root form (Section 5)
        const auto& first_line = non_blank_lines[0];

        // Check if it's a single primitive
        if (non_blank_lines.size() == 1)
        {
            auto line_content = first_line.content;
            // Check if it's not a key-value line

            auto kv = split_key_value(line_content);
            if (!kv)
            {
                // Not a key-value, check if it's a header
                if (!header_result || !(*header_result))
                {
                    // Single primitive
                    auto r = parse_primitive(line_content, visitor_);
                    if (!r)
                    {
                        ec = r.error();
                    }
                    return;
                }
            }
        }

        // Otherwise, root object
        auto r = decode_object(lines_, 0, 0, strict_, visitor_);
        if (!r)
        {
            ec = r.error().code();
        }
    }

    const std::vector<parsed_line>& lines() const {return lines_;}
    const std::vector<blank_line_info>& blank_lines() const {return blank_lines_;}
};

using toon_string_reader = basic_toon_reader<string_source<char>>;
using toon_stream_reader = basic_toon_reader<stream_source<char>>;

} // namespace toon
} // namespace jsoncons

#endif // JSONCONS_TOON_TOON_READER_HPP


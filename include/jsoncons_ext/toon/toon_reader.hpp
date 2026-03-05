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

struct header_info
{
    jsoncons::optional<std::string> key;
    std::size_t length{0};
    char delimiter{','};
    std::vector<jsoncons::string_view> fields;
};

using header_info_result = read_result<jsoncons::optional<header_info>>;

using line_result = read_result<std::size_t>;

using void_result = jsoncons::expected<void, read_error>;

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

line_result decode_array_from_header(const std::vector<parsed_line>& lines,
    std::size_t header_idx,
    std::size_t header_depth,
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
        else if (in_quotes && c == '\"')
        {
            end = i;
            terminated = true;
            in_quotes = false;
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
toon_errc parse_primitive(jsoncons::string_view token, json_visitor& visitor)
{
    token = jsoncons::strip(token);

    if (jsoncons::starts_with(token, '\"'))
    {
        if (!jsoncons::ends_with(token, '\"') || token.size() < 2)
        {
            return toon_errc::missing_closing_quote;
        }
        visitor.string_value(jsoncons::string_view(token.data()+1, token.size()-2));
        return toon_errc{};
    }
    if (token == "true")
    {
        visitor.bool_value(true);
        return toon_errc{};
    }
    if (token == "false")
    {
        visitor.bool_value(false);
        return toon_errc{};
    }
    if (token == "null")
    {
        visitor.null_value();
        return toon_errc{};
    }
    
    {
        std::uint64_t u64;
        auto ru64 = jsoncons::to_integer(token.data(), token.size(), u64);
        if (ru64)
        {
            visitor.uint64_value(u64);
            return toon_errc{};
        }
        std::int64_t i64;
        auto ri64 = jsoncons::to_integer(token.data(), token.size(), i64);
        if (ri64)
        {
            visitor.int64_value(i64);
            return toon_errc{};
        }
        double d;
        auto result = jsoncons::decstr_to_double(token.data(), token.size(), d);
        if (result)
        {
            visitor.double_value(d);
            return toon_errc{};
        }
    }
    visitor.string_value(jsoncons::string_view(token.data(), token.size()));

    return toon_errc{};
}

inline 
void parse_delimited_values(jsoncons::string_view line, 
    char delimiter,
    json_visitor& visitor)
{
    bool is_quoted = false;
    std::size_t offset = 0;
    std::size_t length = 0;
    bool is_empty = true;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];

        if (c == delimiter && !is_quoted)
        {
            parse_primitive(jsoncons::strip(jsoncons::string_view(line.data()+offset, length)), visitor);
            offset = i+1;
            length = 0;
            is_empty = false;
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
            parse_primitive(jsoncons::strip(jsoncons::string_view(line.data()+offset, length+2)), visitor);
            while (++i < line.size() && line[i] != delimiter)
            {
            }
            is_quoted = false;
            offset = i+1;
            length = 0;
        }
        else
        {
            ++length;
        }
    }
    if (length > 0 || !is_empty)
    {
        parse_primitive(jsoncons::strip(jsoncons::string_view(line.data()+offset, length)), visitor);
    }
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
            std::cout << "parse_delimited_values.378 key: " << fields[field_index] << "\n";
            visitor.key(fields[field_index]);
            parse_primitive(jsoncons::strip(jsoncons::string_view(line.data()+offset, length)), visitor);
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
            std::cout << "parse_delimited_values.403 key: " << fields[field_index] << "\n";
            visitor.key(fields[field_index]);
            parse_primitive(jsoncons::strip(jsoncons::string_view(line.data()+offset, length+2)), visitor);
            while (++i < line.size() && line[i] != delimiter)
            {
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
    if (length > 0 || !is_empty)
    {
        if (field_index >= fields.size())
        {
            return line_result{jsoncons::unexpect, toon_errc::too_many_values_in_row};
        }
        std::cout << "parse_delimited_values.425 key: " << fields[field_index] << "\n";
        visitor.key(fields[field_index]);
        parse_primitive(jsoncons::strip(jsoncons::string_view(line.data()+offset, length)), visitor);
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
header_info_result parse_header(jsoncons::string_view line)
{
    auto bracket_start = find_unquoted_char(line, '[');
    if (bracket_start == jsoncons::string_view::npos)
    {
        return header_info_result{};
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
                return header_info_result{jsoncons::unexpect, ec};
            }
            key = key_str;
        }
    }
    auto bracket_end = find_unquoted_char(line, ']', bracket_start);
    if (bracket_end == jsoncons::string_view::npos)
    {
        return header_info_result{};
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
        return header_info_result{jsoncons::unexpect, toon_errc::invalid_value};
    }

    auto after_bracket = jsoncons::strip(jsoncons::string_view(line.data() + (bracket_end + 1),
        line.size() - (bracket_end + 1)));

    std::vector<jsoncons::string_view> fields;
    if (jsoncons::starts_with(after_bracket, '{'))
    {
        auto brace_end = find_unquoted_char(after_bracket, '}');
        if (brace_end == jsoncons::string_view::npos)
        {
            return header_info_result{jsoncons::unexpect, toon_errc::unterminated_fields_segment};
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
    return header_info_result{jsoncons::in_place, header_info{jsoncons::optional{std::move(key)}, length, delimiter, std::move(fields)}};
};

inline
std::size_t compute_depth_from_indent(std::size_t indent_spaces, std::size_t indent_size)
{
    return indent_spaces / indent_size;
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
        if (c == ' ')
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
            is_blank_line = false;
            if (!(c == '\n'))
            {
                trailing_blanks = 0;
            }
        }
        if (strict && is_blank_line && c == '\t')
        {
            ec = toon_errc::tab_in_indentation;
            return;
        }
        if (c == '\n')
        {
            if (strict && indent > 0 && indent % indent_size !=0)
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
        std::size_t depth = compute_depth_from_indent(indent, indent_size);
        if (is_blank_line)
        {
            blank_lines.push_back(blank_line_info{line_num,indent,depth});
        }
        lines.push_back(parsed_line{depth, indent, jsoncons::string_view{raw.data()+(start+indent), i-(start+indent+trailing_blanks)}, line_num});
    }

}

inline
void decode_inline_array(jsoncons::string_view content, 
    char delimiter,
    std::size_t expected_length,
    bool strict,
    json_visitor& visitor)
{
    if (content.empty() && expected_length == 0)
    {
        visitor.begin_array();
        visitor.end_array();
    }
    visitor.begin_array();
    parse_delimited_values(content, delimiter, visitor);
    visitor.end_array();

    if (strict && expected_length != 1)
    {
        // error
    }
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
void_result decode_object(const std::vector<parsed_line>& lines,
    std::size_t start_idx,
    std::size_t parent_depth,
    bool strict,
    json_visitor& visitor)
{
    visitor.begin_object();

    std::size_t i = start_idx;
    std::size_t expected_depth = (start_idx == 0) ? parent_depth : parent_depth+1;

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
        auto header_info_result = parse_header(content);
        if (header_info_result && *header_info_result)
        {
            const header_info& header(*(*header_info_result));
            const jsoncons::optional<std::string>& key(header.key);
            if (key)
            {
                // Array field
                std::cout << "decode_object.769 key: " << *key << "\n";
                visitor.key(*key);
                auto r = decode_array_from_header(lines, i, line.depth, header, strict, visitor);
                i = *r;
                continue;
            }
        }

        // Must be a key-value line
        auto colon_idx = content.find(':');
        if (colon_idx == jsoncons::string_view::npos)
        {
            // Invalid line, skip in non-strict mode
            if (strict)
            {
                return void_result{jsoncons::unexpect, toon_errc::invalid_line};
            }
            ++i;
            continue;
        }
        auto key_str = jsoncons::strip(jsoncons::string_view{content.data(), colon_idx});
        auto value_str = jsoncons::strip(jsoncons::string_view(content.data() + (colon_idx + 1), content.size() - (colon_idx + 1)));

        std::string key;
        parse_key(key_str, key);

        if (value_str.empty())
        {
            // Nested object
            std::cout << "decode_object.798 key: " << key << "\n";
            visitor.key(key);
            decode_object(lines, i+1, line.depth, strict, visitor);
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
            std::cout << "decode_object.811 key: " << key << "\n";
            visitor.key(key);
            parse_primitive(value_str, visitor);
            ++i;
        }
    }

    visitor.end_object();
    return void_result{};
}

inline
line_result decode_list_array(const std::vector<parsed_line>& lines,
    std::size_t start_idx,
    std::size_t header_depth,
    char delimiter,
    std::size_t expected_length,
    bool strict,
    json_visitor& visitor)
{
    visitor.begin_array();

    std::size_t row_depth = header_depth + 1;

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

                if (line.depth >= row_depth)
                {
                    return line_result{jsoncons::unexpect, toon_errc::blank_lines_in_arrays};
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
        if (line.depth < row_depth)
        {
            break;
        }
        // Must start with "- "
        auto content = line.content;
        if (!jsoncons::starts_with(content, "- "))
        {
            break;
        }
        auto item_content = jsoncons::strip(jsoncons::string_view(content.data()+2, content.size()-2));
        auto header_info_result = parse_header(item_content);
        if (header_info_result && *header_info_result)
        {
            const header_info& item_header(*(*header_info_result));
            const jsoncons::optional<std::string>& key(item_header.key);
            std::size_t length{item_header.length};
            char item_delim{item_header.delimiter};

            if (!key)
            {
                auto colon_idx = item_content.find(':');
                if (colon_idx != jsoncons::string_view::npos)
                {
                    auto inline_part = jsoncons::strip(jsoncons::string_view(item_content.data()+1, item_content.size()-1));
                    if (!inline_part.empty() || length == 0)
                    {
                        decode_inline_array(inline_part, item_delim, length, strict, visitor);
                        ++i;
                        continue;
                    }
                }
            }
            else
            {
                visitor.begin_object();
                std::cout << "decode_list_array.898 key: " << *key << "\n";
                visitor.key(*key);
                auto r = decode_array_from_header(lines, i, line.depth, item_header, strict, visitor);
                i = *r;
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
                    if (field_header_result && *field_header_result)
                    {
                        const header_info& field_header (*(*field_header_result));
                        const jsoncons::optional<std::string>& field_key(field_header.key);

                        std::cout << "decode_list_array.917 key: " << *field_key << "\n";
                        visitor.key(*field_key);
                        auto r1 = decode_array_from_header(lines, i, field_line.depth, field_header, strict, visitor);
                        i = *r1;
                        continue;
                    }
                    std::size_t colon_idx = find_unquoted_char(field_content, ':');
                    if (colon_idx != jsoncons::string_view::npos)
                    {
                        auto field_key_str = jsoncons::strip(jsoncons::string_view{field_content.data(), colon_idx});
                        auto field_value_str = jsoncons::strip(jsoncons::string_view(field_content.data() + (colon_idx + 1), field_content.size() - (colon_idx + 1)));
                        std::string field_key;
                        parse_key(field_key_str, field_key);
                        if (field_value_str.empty())
                        {
                            std::cout << "decode_list_array.932 key: " << field_key << "\n";
                            visitor.key(field_key);
                            decode_object(
                                lines, i + 1, field_line.depth, strict, visitor
                            );
                            ++i;
                            while (i < lines.size() && lines[i].depth > field_line.depth)
                            {
                                ++i;
                            }
                        }
                        else
                        {
                            std::cout << "decode_list_array.945 key: " << field_key << "\n";
                            visitor.key(field_key);
                            parse_primitive(field_value_str, visitor);
                        }
                    }

                }
                visitor.end_object();
            }
        }
        // Check if it's an object (has colon)

        std::size_t colon_idx = find_unquoted_char(item_content, ':');
        if (colon_idx != jsoncons::string_view::npos)
        {
            // It's an object item
            visitor.begin_object();
            auto key_str = jsoncons::strip(jsoncons::string_view{item_content.data(), colon_idx});
            auto value_str = jsoncons::strip(jsoncons::string_view(item_content.data() + (colon_idx + 1), item_content.size() - (colon_idx + 1)));
            std::string key;
            parse_key(key_str, key);
            if (value_str.empty())
            {
                // First field is nested object: fields at depth +2
                std::cout << "decode_list_array.969 key: " << key << "\n";
                visitor.key(key);
                decode_object(lines, i + 1, line.depth + 1, strict, visitor);
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
                std::cout << "decode_list_array.982 key: " << key << "\n";
                visitor.key(key);
                parse_primitive(value_str, visitor);
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
                if (field_header_result && *field_header_result)
                {
                    const header_info& field_header (*(*field_header_result));
                    const auto& field_key{field_header.key};
                    std::cout << "decode_list_array.1004 key: " << *field_key << "\n";
                    visitor.key(*field_key);
                    auto r1 = decode_array_from_header(lines, i, field_line.depth, field_header, strict, visitor);
                    i = *r1;
                    continue;
                }
                std::size_t colon_idx = find_unquoted_char(field_content, ':');
                if (colon_idx != jsoncons::string_view::npos)
                {
                    auto field_key_str = jsoncons::strip(jsoncons::string_view{field_content.data(), colon_idx});
                    auto field_value_str = jsoncons::strip(jsoncons::string_view(field_content.data() + (colon_idx + 1), field_content.size() - (colon_idx + 1)));
                    std::string field_key;
                    parse_key(field_key_str, field_key);
                    if (field_value_str.empty())
                    {
                        // Nested object
                        std::cout << "decode_list_array.917 key: " << field_key << "\n";
                        visitor.key(field_key);
                        decode_object(
                            lines, i + 1, field_line.depth, strict, visitor
                        );
                        ++i;
                        while (i < lines.size() && lines[i].depth > field_line.depth)
                        {
                            ++i;
                        }
                    }
                    else
                    {
                        visitor.key(field_key);
                        parse_primitive(field_value_str, visitor);
                        ++i;
                    }
                }
                else
                {
                    break;
                }
            }
            visitor.end_object();
        }
        else
        {
            // Not an object, must be a primitive;
            if (item_content.empty())
            {
                visitor.begin_object();
                visitor.end_object();
            }
            else
            {
                parse_primitive(item_content, visitor);
                ++i;
            }
        }
    }

    visitor.end_array();

    return line_result{i};
}

inline
line_result decode_tabular_array(const std::vector<parsed_line>& lines,
    std::size_t start_idx,
    std::size_t header_depth,
    const std::vector<jsoncons::string_view>& fields,
    char delimiter,
    std::size_t expected_length,
    bool strict,
    json_visitor& visitor)
{
    visitor.begin_array();

    std::size_t row_depth = header_depth + 1;

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

                if (line.depth >= row_depth)
                {
                    return line_result{jsoncons::unexpect, toon_errc::blank_lines_in_arrays};
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
            ++i;
        }
        else
        {
            break;
        }
    }

    visitor.end_array();

    return line_result{i};
}

inline
line_result decode_array_from_header(const std::vector<parsed_line>& lines,
    std::size_t header_idx,
    std::size_t header_depth,
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
        // return toon_errc::missing_colon_after_key;
    }
    auto inline_content = jsoncons::strip(jsoncons::string_view(header_line.data() + (colon_idx + 1), header_line.size() - (colon_idx + 1)));

    if (!inline_content.empty() || (fields.empty() && length == 0))
    {
        decode_inline_array(inline_content, delimiter, length, strict, visitor);
        return line_result{header_idx + 1};
    }

    if (!fields.empty())
    {
        // Tabular array
        return decode_tabular_array(
            lines, header_idx + 1, header_depth, fields, delimiter, length, strict, visitor);
    }
    else
    {
        return decode_list_array(
            lines, header_idx + 1, header_depth, delimiter, length, strict, visitor);
    }
}


inline
void decode_array(const std::vector<parsed_line>& lines,
    std::size_t start_idx,
    std::size_t parent_depth,
    const header_info& header_info,
    bool strict,
    json_visitor& visitor)
{
    decode_array_from_header(lines, start_idx, parent_depth, header_info, strict, visitor);
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

        auto header_info_result = parse_header(non_blank_lines[0].content);
        if (header_info_result && *header_info_result && !(*header_info_result)->key)
        {
            // Root array
            const header_info& header(*(*header_info_result));
            decode_array(lines_, 0, 0, header, strict_, visitor_);
            return;
        }

        // Determine root form (Section 5)
        const auto& first_line = non_blank_lines[0];

        // Check if it's a single primitive
        if (non_blank_lines.size() == 1)
        {
            auto line_content = first_line.content;
            // Check if it's not a key-value line
            auto colon_idx = line_content.find(':');
            if (colon_idx == jsoncons::string_view::npos)
            {
                // Not a key-value, check if it's a header
                if (!header_info_result || !(*header_info_result))
                {
                    // Single primitive
                    parse_primitive(line_content, visitor_);
                    return;
                }
            }
        }

        // Otherwise, root object
        decode_object(lines_, 0, 0, strict_, visitor_);
    }

    const std::vector<parsed_line>& lines() const {return lines_;}
    const std::vector<blank_line_info>& blank_lines() const {return blank_lines_;}
};

using toon_string_reader = basic_toon_reader<string_source<char>>;
using toon_stream_reader = basic_toon_reader<stream_source<char>>;

} // namespace toon
} // namespace jsoncons

#endif // JSONCONS_TOON_TOON_READER_HPP


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
#include <jsoncons/ser_util.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/source.hpp>
#include <jsoncons/ser_util.hpp>
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

using header_result = read_result<jsoncons::optional<header_info>>;

using decode_result = read_result<std::size_t>;

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

class line_cursor
{
    static std::vector<blank_line_info> default_blank_lines;

    const std::vector<parsed_line>& lines_;
    const std::vector<blank_line_info>& blank_lines_;
    std::size_t index_{0};

public:
    line_cursor(const std::vector<parsed_line>& lines,
        const std::vector<blank_line_info> blank_lines = default_blank_lines)
      : lines_(lines), blank_lines_(blank_lines)
    {
    }

    const std::vector<blank_line_info>& blank_lines() const
    {
        return blank_lines_;
    }

    const parsed_line& peek() const
    {
        JSONCONS_ASSERT(index_ < lines_.size());
        return lines_[index_];
    }

    const parsed_line& next() 
    {
        JSONCONS_ASSERT(index_ < lines_.size());
        const auto& line = lines_[index_];
        ++index_;
        return line;
    }

    const parsed_line& current() const
    {
        JSONCONS_ASSERT(index_ < lines_.size());
        return lines_[index_];
    }

    void advance()
    {
        ++index_;
    }

    bool at_end() const
    {
        return index_ == lines_.size();
    }

    std::size_t length() const
    {
        return lines_.size();
    }

    const parsed_line& peek_at_depth(std::size_t target_depth) const
    {
        JSONCONS_ASSERT(target_depth < lines_.size());
        return lines_[target_depth];
    }

    bool has_more_at_depth(std::size_t target_depth) const
    {
        return target_depth < lines_.size();
    }

    void skip_deeper_than(std::size_t depth) 
    {
        if (!(depth < lines_.size()))
        {
            return;
        }
        index_ = lines_.size(); 
    }
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
toon_errc parse_key(jsoncons::string_view key_str, std::string& result)
{
    bool in_quotes{false};
    std::size_t start{0};
    std::size_t end{0};

    bool terminated{false};
    for (std::size_t i = 0; i < key_str.size() && !terminated; ++i)
    {
        char c = key_str[i];
        if (start == 0 && c != ' ')
        {
            start = i;
        }
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
            end = i;
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
decode_result parse_delimited_values(jsoncons::string_view line, 
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
                return decode_result{jsoncons::unexpect, toon_errc::too_many_values_in_row};
            }
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
                return decode_result{jsoncons::unexpect, toon_errc::too_many_values_in_row};
            }
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
            return decode_result{jsoncons::unexpect, toon_errc::too_many_values_in_row};
        }
        visitor.key(fields[field_index]);
        parse_primitive(jsoncons::strip(jsoncons::string_view(line.data()+offset, length)), visitor);
        ++field_index;
    }
    if (field_index != fields.size())
    {
        return decode_result{jsoncons::unexpect, toon_errc::too_few_values_in_row};
    }
    visitor.end_object();
    return decode_result{};
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
    return header_result{jsoncons::in_place, header_info{jsoncons::optional{std::move(key)}, length, delimiter, std::move(fields)}};
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
decode_result decode_tabular_array(const std::vector<parsed_line>& lines,
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

    for (std::size_t i = start_idx; i < lines.size(); ++i)
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
                    return decode_result{jsoncons::unexpect, toon_errc::blank_lines_in_arrays};
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
        }
    }

    visitor.end_array();

    return decode_result{1};
}

/*    result = []

    while i < len(lines):
        line = lines[i]

        # Handle blank lines
        if line.is_blank:
            if strict:
                # In strict mode: blank lines at or above row depth are errors
                # Blank lines dedented below row depth mean array has ended
                if line.depth >= row_depth:
                    raise ToonDecodeError("Blank lines not allowed inside arrays")
                else:
                    break
            else:
                # In non-strict mode: ignore all blank lines and continue
                i += 1
                continue

        # Stop if dedented or different depth
        if line.depth < row_depth:
            break
        if line.depth > row_depth:
            # End of tabular rows (might be next key-value)
            break

        content = line.content

        # Disambiguation: check if this is a row or a key-value line
        # A row has no unquoted colon, or delimiter before colon
        if is_row_line(content, delimiter):
            # Parse as row
            tokens = parse_delimited_values(content, delimiter)
            values = [parse_primitive(token) for token in tokens]

            if strict and len(values) != len(fields):
                raise ToonDecodeError(
                    f"Expected {len(fields)} values in row, but got {len(values)}"
                )

            obj = {fields[j]: values[j] for j in range(min(len(fields), len(values)))}
            result.append(obj)
            i += 1
        else:
            # Not a row, end of tabular data
            break

    if strict and len(result) != expected_length:
        raise ToonDecodeError(f"Expected {expected_length} rows, but got {len(result)}")

    return result, i
*/

inline
decode_result decode_array_from_header(const std::vector<parsed_line>& lines,
    std::size_t header_idx,
    std::size_t header_depth,
    const header_info& header_info,
    bool strict,
    json_visitor& visitor)
{
    const jsoncons::optional<std::string>& key(header_info.key);
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
        return decode_result{header_idx + 1};
    }

    if (!fields.empty())
    {
        // Tabular array
        return decode_tabular_array(
            lines, header_idx + 1, header_depth, fields, delimiter, length, strict, visitor);
        
    }

    return decode_result{header_idx + 1};
/*

    # Non-inline array
    if fields is not None:
        # Tabular array
        return decode_tabular_array(
            lines, header_idx + 1, header_depth, fields, delimiter, length, strict
        )
    else:
        # List format (mixed/non-uniform)
        return decode_list_array(lines, header_idx + 1, header_depth, delimiter, length, strict)
*/
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

std::vector<blank_line_info> line_cursor::default_blank_lines = std::vector<blank_line_info>{};

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
        if (header_result && *header_result && !(*header_result)->key)
        {
            const header_info& hdr_info(*(*header_result));
            decode_array(lines_, 0, 0, hdr_info, strict_, visitor_);
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


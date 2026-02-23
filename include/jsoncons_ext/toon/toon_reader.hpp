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
#include <jsoncons/utility/read_number.hpp>
#include <jsoncons/utility/unicode_traits.hpp>
#include <jsoncons/utility/string_utils.hpp>
#include <jsoncons_ext/toon/toon_options.hpp>
#include <jsoncons_ext/toon/toon_error.hpp>

namespace jsoncons {
namespace toon {

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
toon_errc read_key(jsoncons::string_view key_str, std::string& result)
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
void parse_delimited_values(jsoncons::string_view line, 
    char delimiter,
    std::vector<std::string>& tokens)
{
    bool is_quoted = false;
    std::size_t offset = 0;
    std::size_t length = 0;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];

        if (c == delimiter && !is_quoted)
        {
            tokens.emplace_back(line.data()+offset, length);
            offset = i+1;
            length = 0;
        }
        else if (!is_quoted && c == '\"')
        {
            is_quoted = true;
        }
        else if (is_quoted && c == '\\' && i+1 < line.size())
        {
            length += 2;
            ++i;
        }
        else if (is_quoted && c == '\"')
        {
            offset = i+1;
        }
        else
        {
            ++length;
        }
    }
    if (length > 0 || !tokens.empty())
    {
        tokens.emplace_back(line.data()+offset, length);
    }
}

inline
toon_errc parse_header(jsoncons::string_view line, std::vector<std::string>& /*fields*/)
{
    auto bracket_start = find_unquoted_char(line, '{');
    if (bracket_start == jsoncons::string_view::npos)
    {
        return toon_errc{};
    }
    jsoncons::string_view key;
    auto key_part = jsoncons::string_view{line.data() + bracket_start, (line.size() - bracket_start)};

    std::string str;
    toon_errc ec = read_key(key_part, str);
    if (ec != toon_errc{})
    {
        return toon_errc{};
    }
    auto bracket_end = find_unquoted_char(line, '}', bracket_start);
    if (bracket_end == jsoncons::string_view::npos)
    {
        return toon_errc{};
    }

    jsoncons::string_view bracket_content = jsoncons::string_view(line.data() + (bracket_start + 1), bracket_end - (bracket_start + 1));
    if (jsoncons::starts_with(bracket_content, "#"))
    {
        bracket_content = jsoncons::string_view(bracket_content.data()+1, bracket_content.size()-1);
    }

    char delimiter = ',';
    jsoncons::string_view length_str = bracket_content;

    if (jsoncons::ends_with(bracket_content, '\t'))
    {
        delimiter = '\t';
        length_str = jsoncons::string_view(bracket_content.data(), bracket_content.size()-1);
    }
    else if (jsoncons::ends_with(bracket_content, '|'))
    {
        delimiter = '|';
        length_str = jsoncons::string_view(bracket_content.data(), bracket_content.size()-1);
    }
    else if (jsoncons::ends_with(bracket_content, ','))
    {
        delimiter = ',';
        length_str = jsoncons::string_view(bracket_content.data(), bracket_content.size()-1);
    }

    //std::size_t length{0};
    //auto rc = to_integer(length_str.data(), length_str.size(), length);
    //if (rc.ec != std::errc{})
    //{
    //    return toon_errc::invalid_value;
    //}

    auto after_bracket = jsoncons::strip(jsoncons::string_view(line.data() + bracket_end+1, 
        line.size() - (bracket_end+1)));

    if (jsoncons::starts_with(after_bracket, '{'))
    {
        auto brace_end = find_unquoted_char(after_bracket, '}');
        if (brace_end == jsoncons::string_view::npos)
        {
            return toon_errc::unterminated_fields_segment;
        }
        auto fields_content = jsoncons::string_view(after_bracket.data()+1, brace_end);

        bool in_quotes = false;
        std::size_t offset = 0;
        std::size_t len = 0;
        std::vector<std::string> tokens;
        for (size_t i = 0; i < fields_content.size(); ++i)
        {
            char c = fields_content[i];
            if (!in_quotes && c == '\"')
            {
                if (len != 0)
                {
                    tokens.emplace_back(after_bracket.data(), len);
                    len = 0;
                }
                in_quotes = true;
            }
            else if (in_quotes && c == '\"')
            {
                tokens.emplace_back(after_bracket.data()+offset, len);
                len = 0;
                offset = i+1;
            }
        }
        // Parse fields using the delimiter
        std::vector<std::string> field_tokens;
        parse_delimited_values(fields_content, delimiter, tokens);
        for (auto& token : field_tokens)
        {
            token = jsoncons::strip(token);
        }

        after_bracket = jsoncons::string_view(after_bracket.data()+(brace_end + 1), 
            after_bracket.size()-(brace_end + 1));
    }
    return toon_errc{};
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
        read_lines(ec);
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

        std::vector<std::string> fields;
        parse_header(non_blank_lines[0].content, fields);
    }

    const std::vector<parsed_line>& lines() const {return lines_;}
    const std::vector<blank_line_info>& blank_lines() const {return blank_lines_;}

    void read_lines(std::error_code& ec)
    {
        while (!source_.eof())
        {
            auto s = source_.read_buffer();
            raw_.append(s.data(), s.size());
        }

        std::size_t line_num = 1;
        std::size_t indent = 0;
        std::size_t start = 0;
        bool is_blank_line = true;
        std::size_t trailing_blanks = 0;

        std::size_t i = 0;
        for (; i < raw_.size(); ++i)
        {
            char c = raw_[i];
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
            if (strict_ && is_blank_line && c == '\t')
            {
                ec = toon_errc::tab_in_indentation;
                return;
            }
            if (c == '\n')
            {
                if (strict_ && indent > 0 && indent % indent_size_ !=0)
                {
                    ec = toon_errc::indent_not_multiple_of_indent_size;
                    return;
                }
                std::size_t depth = compute_depth_from_indent(indent, indent_size_);
                if (is_blank_line)
                {
                    blank_lines_.push_back(blank_line_info{line_num,indent,depth});
                }
                lines_.push_back(parsed_line{depth, indent, jsoncons::string_view{raw_.data()+(start+indent), i-(start+indent+trailing_blanks)}, line_num});
                ++line_num;
                indent = 0;
                is_blank_line = true;
                start = i+1;
                trailing_blanks = 0;
            }
        }
        if (start < i)
        {
            std::size_t depth = compute_depth_from_indent(indent, indent_size_);
            if (is_blank_line)
            {
                blank_lines_.push_back(blank_line_info{line_num,indent,depth});
            }
            lines_.push_back(parsed_line{depth, indent, jsoncons::string_view{raw_.data()+(start+indent), i-(start+indent+trailing_blanks)}, line_num});
        }

    }

    std::size_t compute_depth_from_indent(std::size_t indent_spaces, std::size_t indent_size) const
    {
        return indent_spaces / indent_size;
    }
};

using toon_string_reader = basic_toon_reader<string_source<char>>;
using toon_stream_reader = basic_toon_reader<stream_source<char>>;

} // namespace toon
} // namespace jsoncons

#endif // JSONCONS_TOON_TOON_READER_HPP


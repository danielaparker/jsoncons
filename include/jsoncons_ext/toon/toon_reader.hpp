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
#include <jsoncons/utility/unicode_traits.hpp>
#include <jsoncons_ext/toon/toon_options.hpp>
#include <jsoncons_ext/toon/toon_error.hpp>

namespace jsoncons {
namespace toon {

struct parsed_line
{
    std::string raw;
    int depth{0};
    int indent{0};
    std::string content;
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
    int indent{0};
    int depth{0};
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
    toon_decode_options options_;
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
        const TempAlloc& temp_alloc = TempAlloc())
    : source_(std::forward<Sourceable>(source)),
      visitor_(visitor),
      options_(options)
    {
        (temp_alloc);
    }

    void read()
    {
        std::error_code ec;
        read(ec);
    }

    void read(std::error_code& ec)
    {
        parse_lines(ec);
        if (ec)
        {
            JSONCONS_THROW(ser_error(ec));
        }
    }

    const std::vector<parsed_line>& lines() const {return lines_;}
    const std::vector<blank_line_info>& blank_lines() const {return blank_lines_;}

    void parse_lines(std::error_code& ec)
    {
        std::string str;
        while (!source_.eof())
        {
            auto s = source_.read_buffer();
            if (ec)
            {
                return;
            }
            str.append(s.data(), s.size());
        }

        std::size_t line_num = 1;
        int indent = 0;
        int depth = 0;
        std::size_t start = 0;
        bool is_blank = true;

        std::size_t i = 0;
        for (; i < str.size(); ++i)
        {
            char c = str[i];
            if (is_blank && c == ' ')
            {
                ++indent;
            }

            if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n'))
            {
                is_blank = false;
            }
            if (str[i] == '\n')
            {
                if (is_blank)
                {
                    blank_lines_.push_back(blank_line_info{line_num,indent,depth});
                }
                lines_.push_back(parsed_line{std::string{str.data(), start, i-start},
                    depth, indent, std::string{}, line_num});
                ++line_num;
                indent = 0;
                is_blank = true;
                start = i;
            }
        }
        if (start < i)
        {
            if (is_blank)
            {
                blank_lines_.push_back(blank_line_info{line_num,indent,depth});
            }
            lines_.push_back(parsed_line{std::string{str.data(), start, i-start},
                depth, indent, std::string{}, line_num});
        }

    }
};

using toon_string_reader = basic_toon_reader<string_source<char>>;
using toon_stream_reader = basic_toon_reader<stream_source<char>>;

} // namespace toon
} // namespace jsoncons

#endif // JSONCONS_TOON_TOON_READER_HPP


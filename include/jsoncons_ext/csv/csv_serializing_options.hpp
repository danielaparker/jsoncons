// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_SERIALIZING_OPTIONS_HPP
#define JSONCONS_CSV_CSV_SERIALIZING_OPTIONS_HPP

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <istream>
#include <ostream>
#include <cstdlib>
#include <limits>
#include <cwchar>

namespace jsoncons { namespace csv {

namespace detail {
    JSONCONS_DEFINE_LITERAL(string_literal,"string")
    JSONCONS_DEFINE_LITERAL(integer_literal,"integer")
    JSONCONS_DEFINE_LITERAL(float_literal,"float")
    JSONCONS_DEFINE_LITERAL(boolean_literal,"boolean")
}

enum class csv_column_type
{
    string_t,integer_t,float_t,boolean_t,repeat_t
};

enum class quote_style_type
{
    all,minimal,none,nonnumeric
};

typedef quote_style_type quote_styles;

enum class mapping_type
{
    n_rows, 
    n_objects, 
    m_columns
};

enum class column_state {sequence,label};

struct csv_type_info
{
    csv_type_info() = default;
    csv_type_info(const csv_type_info&) = default;
    csv_type_info(csv_type_info&&) = default;

    csv_type_info(csv_column_type ctype, size_t lev, size_t repcount = 0)
    {
        col_type = ctype;
        level = lev;
        rep_count = repcount;
    }

    csv_column_type col_type;
    size_t level;
    size_t rep_count;
};

template <class CharT,class Allocator=std::allocator<CharT>>
class basic_csv_serializing_options
{
    typedef CharT char_type;
    typedef Allocator allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT> char_allocator_type;
    typedef std::basic_string<CharT,std::char_traits<CharT>,char_allocator_type> string_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<string_type> string_allocator_type;
    typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<csv_type_info> csv_type_info_allocator_type;

    chars_format floating_point_format_;
    uint8_t precision_;
    bool assume_header_;
    bool ignore_empty_values_;
    bool ignore_empty_lines_;
    bool trim_leading_;
    bool trim_trailing_;
    bool trim_leading_inside_quotes_;
    bool trim_trailing_inside_quotes_;
    bool unquoted_empty_value_is_null_;
    CharT field_delimiter_;
    bool has_subfield_delimiter_;
    CharT subfield_delimiter_;
    CharT quote_char_;
    CharT quote_escape_char_;
    CharT comment_starter_;
    quote_style_type quote_style_;
    std::pair<mapping_type,bool> mapping_;
    unsigned long max_lines_;
    size_t header_lines_;
    string_type line_delimiter_;
    bool infer_types_;

    std::vector<string_type,string_allocator_type> column_names_;
    std::vector<csv_type_info,csv_type_info_allocator_type> column_types_;
    std::vector<string_type,string_allocator_type> column_defaults_;
public:
    static const size_t default_indent = 4;

//  Constructors

    basic_csv_serializing_options()
        :
        floating_point_format_(chars_format::hex),
        precision_(0),
        assume_header_(false),
        ignore_empty_values_(false),
        ignore_empty_lines_(true),
        trim_leading_(false),
        trim_trailing_(false),
        trim_leading_inside_quotes_(false),
        trim_trailing_inside_quotes_(false),
        unquoted_empty_value_is_null_(false),
        field_delimiter_(','),
        has_subfield_delimiter_(false),
        subfield_delimiter_(','),
        quote_char_('\"'),
        quote_escape_char_('\"'),
        comment_starter_('\0'),
        quote_style_(quote_style_type::minimal),
        mapping_({mapping_type::n_rows,false}),
        max_lines_((std::numeric_limits<unsigned long>::max)()),
        header_lines_(0),
        infer_types_(true)
    {
        line_delimiter_.push_back('\n');
    }

//  Properties

    chars_format floating_point_format() const
    {
        return floating_point_format_;
    }

    basic_csv_serializing_options<CharT>& floating_point_format(chars_format value)
    {
        floating_point_format_ = value;
        return *this;
    }

    uint8_t precision() const
    {
        return precision_;
    }

    basic_csv_serializing_options<CharT>& precision(uint8_t value)
    {
        precision_ = value;
        return *this;
    }

    size_t header_lines() const
    {
        return (assume_header_ && header_lines_ <= 1) ? 1 : header_lines_;
    }

    basic_csv_serializing_options& header_lines(size_t value)
    {
        header_lines_ = value;
        return *this;
    }

    bool assume_header() const
    {
        return assume_header_;
    }

    basic_csv_serializing_options& assume_header(bool value)
    {
        assume_header_ = value;
        return *this;
    }

    bool ignore_empty_values() const
    {
        return ignore_empty_values_;
    }

    basic_csv_serializing_options& ignore_empty_values(bool value)
    {
        ignore_empty_values_ = value;
        return *this;
    }

    bool ignore_empty_lines() const
    {
        return ignore_empty_lines_;
    }

    basic_csv_serializing_options& ignore_empty_lines(bool value)
    {
        ignore_empty_lines_ = value;
        return *this;
    }

    bool trim_leading() const
    {
        return trim_leading_;
    }

    basic_csv_serializing_options& trim_leading(bool value)
    {
        trim_leading_ = value;
        return *this;
    }

    bool trim_trailing() const
    {
        return trim_trailing_;
    }

    basic_csv_serializing_options& trim_trailing(bool value)
    {
        trim_trailing_ = value;
        return *this;
    }

    bool trim_leading_inside_quotes() const
    {
        return trim_leading_inside_quotes_;
    }

    basic_csv_serializing_options& trim_leading_inside_quotes(bool value)
    {
        trim_leading_inside_quotes_ = value;
        return *this;
    }

    bool trim_trailing_inside_quotes() const
    {
        return trim_trailing_inside_quotes_;
    }

    basic_csv_serializing_options& trim_trailing_inside_quotes(bool value)
    {
        trim_trailing_inside_quotes_ = value;
        return *this;
    }

    bool trim() const
    {
        return trim_leading_ && trim_trailing_;
    }

    basic_csv_serializing_options& trim(bool value)
    {
        trim_leading_ = value;
        trim_trailing_ = value;
        return *this;
    }

    bool trim_inside_quotes() const
    {
        return trim_leading_inside_quotes_ && trim_trailing_inside_quotes_;
    }

    basic_csv_serializing_options& trim_inside_quotes(bool value)
    {
        trim_leading_inside_quotes_ = value;
        trim_trailing_inside_quotes_ = value;
        return *this;
    }

    bool unquoted_empty_value_is_null() const
    {
        return unquoted_empty_value_is_null_;
    }

    basic_csv_serializing_options& unquoted_empty_value_is_null(bool value)
    {
        unquoted_empty_value_is_null_ = value;
        return *this;
    }

    std::vector<string_type,string_allocator_type> column_names() const
    {
        return column_names_;
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    basic_csv_serializing_options& column_names(const std::vector<string_type,string_allocator_type>& value)
    {
        column_names_ = value;
        return *this;
    }

    basic_csv_serializing_options& column_defaults(const std::vector<string_type,string_allocator_type>& value)
    {
        column_defaults_ = value;
        return *this;
    }

    basic_csv_serializing_options& column_types(const std::vector<string_type,string_allocator_type>& value)
    {
        if (value.size() > 0)
        {
            column_types_.reserve(value.size());
            for (size_t i = 0; i < value.size(); ++i)
            {
                if (value[i] == detail::string_literal<CharT>()())
                {
                    column_types_.emplace_back(csv_column_type::string_t,0);
                }
                else if (value[i] == detail::integer_literal<CharT>()())
                {
                    column_types_.emplace_back(csv_column_type::integer_t,0);
                }
                else if (value[i] == detail::float_literal<CharT>()())
                {
                    column_types_.emplace_back(csv_column_type::float_t,0);
                }
                else if (value[i] == detail::boolean_literal<CharT>()())
                {
                    column_types_.emplace_back(csv_column_type::boolean_t,0);
                }
            }
        }
        return *this;
    }
#endif
    basic_csv_serializing_options& column_names(const string_type& names)
    {
        column_names_ = parse_column_names(names);
        return *this;
    }

    std::vector<csv_type_info,csv_type_info_allocator_type> column_types() const
    {
        return column_types_;
    }

    basic_csv_serializing_options& column_types(const string_type& types)
    {
        column_types_ = parse_column_types(types);
        return *this;
    }

    std::vector<string_type,string_allocator_type> column_defaults() const
    {
        return column_defaults_;
    }

    basic_csv_serializing_options& column_defaults(const string_type& defaults)
    {
        column_defaults_ = parse_column_names(defaults);
        return *this;
    }

    CharT field_delimiter() const
    {
        return field_delimiter_;
    }

    std::pair<CharT,bool> subfield_delimiter() const
    {
        return std::make_pair(subfield_delimiter_,has_subfield_delimiter_);
    }

    basic_csv_serializing_options& field_delimiter(CharT value)
    {
        field_delimiter_ = value;
        return *this;
    }

    basic_csv_serializing_options& subfield_delimiter(CharT value)
    {
        has_subfield_delimiter_ = true;
        subfield_delimiter_ = value;
        return *this;
    }

    string_type line_delimiter() const
    {
        return line_delimiter_;
    }

    basic_csv_serializing_options& line_delimiter(string_type value)
    {
        line_delimiter_ = value;
        return *this;
    }

    CharT quote_char() const
    {
        return quote_char_;
    }

    basic_csv_serializing_options& quote_char(CharT value)
    {
        quote_char_ = value;
        return *this;
    }

    bool infer_types() const
    {
        return infer_types_;
    }

    basic_csv_serializing_options& infer_types(bool value)
    {
        infer_types_ = value;
        return *this;
    }

    CharT quote_escape_char() const
    {
        return quote_escape_char_;
    }

    basic_csv_serializing_options& quote_escape_char(CharT value)
    {
        quote_escape_char_ = value;
        return *this;
    }

    CharT comment_starter() const
    {
        return comment_starter_;
    }

    basic_csv_serializing_options& comment_starter(CharT value)
    {
        comment_starter_ = value;
        return *this;
    }

    quote_style_type quote_style() const
    {
        return quote_style_;
    }

    mapping_type mapping() const
    {
        return mapping_.second ? (mapping_.first) : (assume_header() || column_names_.size() > 0 ? mapping_type::n_objects : mapping_type::n_rows);
    }

    basic_csv_serializing_options& quote_style(quote_style_type value)
    {
        quote_style_ = value;
        return *this;
    }

    basic_csv_serializing_options& mapping(mapping_type value)
    {
        mapping_ = {value,true};
        return *this;
    }

    unsigned long max_lines() const
    {
        return max_lines_;
    }

    basic_csv_serializing_options& max_lines(unsigned long value)
    {
        max_lines_ = value;
        return *this;
    }

    static std::vector<string_type,string_allocator_type> parse_column_names(const string_type& names)
    {
        std::vector<string_type,string_allocator_type> column_names;

        column_state state = column_state::sequence;
        string_type buffer;

        auto p = names.begin();
        while (p != names.end())
        {
            switch (state)
            {
            case column_state::sequence:
                {
                    switch (*p)
                    {
                    case ' ': case '\t':case '\r': case '\n':
                        ++p;
                        break;
                    default:
                        buffer.clear();
                        state = column_state::label;
                        break;
                    }
                    break;
                }
            case column_state::label:
                {
                    switch (*p)
                    {
                    case ',':
                        column_names.push_back(buffer);
                        buffer.clear();
                        ++p;
                        state = column_state::sequence;
                        break;
                    default:
                        buffer.push_back(*p);
                        ++p;
                        break;
                    }
                    break;
                }
            }
        }
        if (state == column_state::label)
        {
            column_names.push_back(buffer);
            buffer.clear();
        }
        return column_names;
    }

    static std::vector<csv_type_info,csv_type_info_allocator_type> parse_column_types(const string_type& types)
    {
        typedef typename std::allocator_traits<allocator_type>:: template rebind_alloc<std::pair<const string_type,csv_column_type>> pair_allocator_type;

        const std::unordered_map<string_type,csv_column_type, std::hash<string_type>,std::equal_to<string_type>,pair_allocator_type> type_dictionary =
        {

            {detail::string_literal<char_type>(),csv_column_type::string_t},
            {detail::integer_literal<char_type>(),csv_column_type::integer_t},
            {detail::float_literal<char_type>(),csv_column_type::float_t},
            {detail::boolean_literal<char_type>(),csv_column_type::boolean_t}
        };

        std::vector<csv_type_info,csv_type_info_allocator_type> column_types;

        column_state state = column_state::sequence;
        int depth = 0;
        string_type buffer;

        auto p = types.begin();
        while (p != types.end())
        {
            switch (state)
            {
            case column_state::sequence:
                {
                    switch (*p)
                    {
                    case ' ': case '\t':case '\r': case '\n':
                        ++p;
                        break;
                    case '[':
                        ++depth;
                        ++p;
                        break;
                    case ']':
                        JSONCONS_ASSERT(depth > 0);
                        --depth;
                        ++p;
                        break;
                    case '*':
                        {
                            JSONCONS_ASSERT(column_types.size() != 0);
                            size_t offset = 0;
                            size_t level = column_types.size() > 0 ? column_types.back().level: 0;
                            if (level > 0)
                            {
                                for (auto it = column_types.rbegin();
                                     it != column_types.rend() && level == it->level;
                                     ++it)
                                {
                                    ++offset;
                                }
                            }
                            else
                            {
                                offset = 1;
                            }
                            column_types.emplace_back(csv_column_type::repeat_t,depth,offset);
                            ++p;
                            break;
                        }
                    default:
                        buffer.clear();
                        state = column_state::label;
                        break;
                    }
                    break;
                }
            case column_state::label:
                {
                    switch (*p)
                    {
                    case '*':
                        {
                            auto it = type_dictionary.find(buffer);
                            if (it != type_dictionary.end())
                            {
                                column_types.emplace_back(it->second,depth);
                                buffer.clear();
                            }
                            else
                            {
                                JSONCONS_ASSERT(false);
                            }
                            state = column_state::sequence;
                        }
                        break;
                    case ',':
                        {
                            auto it = type_dictionary.find(buffer);
                            if (it != type_dictionary.end())
                            {
                                column_types.emplace_back(it->second,depth);
                                buffer.clear();
                            }
                            else
                            {
                                JSONCONS_ASSERT(false);
                            }
                            ++p;
                            state = column_state::sequence;
                        }
                        break;
                    case ']':
                        {
                            JSONCONS_ASSERT(depth > 0);
                            auto it = type_dictionary.find(buffer);
                            if (it != type_dictionary.end())
                            {
                                column_types.emplace_back(it->second,depth);
                                buffer.clear();
                            }
                            else
                            {
                                JSONCONS_ASSERT(false);
                            }
                            --depth;
                            ++p;
                            state = column_state::sequence;
                        }
                        break;
                    default:
                        {
                            buffer.push_back(*p);
                            ++p;
                        }
                        break;
                    }
                    break;
                }
            }
        }
        if (state == column_state::label)
        {
            auto it = type_dictionary.find(buffer);
            if (it != type_dictionary.end())
            {
                column_types.emplace_back(it->second,depth);
                buffer.clear();
            }
            else
            {
                JSONCONS_ASSERT(false);
            }
        }
        return column_types;
    }

};

typedef basic_csv_serializing_options<char> csv_serializing_options;
typedef basic_csv_serializing_options<wchar_t> wcsv_serializing_options;

#if !defined(JSONCONS_NO_DEPRECATED)
typedef basic_csv_serializing_options<char> csv_parameters;
typedef basic_csv_serializing_options<wchar_t> wcsv_parameters;
#endif


}}
#endif

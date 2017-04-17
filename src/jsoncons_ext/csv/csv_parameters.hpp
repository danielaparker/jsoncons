// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_PARAMETERS_HPP
#define JSONCONS_CSV_CSV_PARAMETERS_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <cstdlib>
#include <limits>
#include <cwchar>

namespace jsoncons { namespace csv {

template <class CharT>
struct json_csv_parser_traits
{
#if !defined(JSONCONS_HAS_STRING_VIEW)
    typedef Basic_string_view_<CharT> string_view_type;
#else
    typedef std::basic_string_view<CharT> string_view_type;
#endif

    static string_view_type string_literal() 
    {
        static const CharT data[] = {'s','t','r','i','n','g'};
        return string_view_type{data,sizeof(data)/sizeof(CharT)};
    }

    static string_view_type integer_literal() 
    {
        static const CharT data[] = {'i','n','t','e','g','e','r'};
        return string_view_type{data,sizeof(data)/sizeof(CharT)};
    }

    static string_view_type float_literal() 
    {
        static const CharT data[] = {'f','l','o','a','t'};
        return string_view_type{data,sizeof(data)/sizeof(CharT)};
    }

    static string_view_type boolean_literal() 
    {
        static const CharT data[] = {'b','o','o','l','e','a','n'};
        return string_view_type{data,sizeof(data)/sizeof(CharT)};
    }
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

template <class CharT>
class basic_csv_parameters
{
public:
    static const size_t default_indent = 4;

//  Constructors

    basic_csv_parameters()
        :
        assume_header_(false),
        ignore_empty_values_(false),
        trim_leading_(false),
        trim_trailing_(false),
        trim_leading_inside_quotes_(false),
        trim_trailing_inside_quotes_(false),
        unquoted_empty_value_is_null_(false),
        field_delimiter_(','),
        quote_char_('\"'),
        quote_escape_char_('\"'),
        comment_starter_('\0'),
        quote_style_(quote_style_type::minimal),
        mapping_({mapping_type::n_rows,false}),
        max_lines_((std::numeric_limits<unsigned long>::max)()),
        header_lines_(0)
    {
        line_delimiter_.push_back('\n');
    }

//  Properties

    size_t header_lines() const
    {
        return (assume_header_ && header_lines_ <= 1) ? 1 : header_lines_;
    }

    basic_csv_parameters<CharT>& header_lines(size_t value)
    {
        header_lines_ = value;
        return *this;
    }

    bool assume_header() const
    {
        return assume_header_;
    }

    basic_csv_parameters<CharT>& assume_header(bool value)
    {
        assume_header_ = value;
        return *this;
    }

    bool ignore_empty_values() const
    {
        return ignore_empty_values_;
    }

    basic_csv_parameters<CharT>& ignore_empty_values(bool value)
    {
        ignore_empty_values_ = value;
        return *this;
    }

    bool trim_leading() const
    {
        return trim_leading_;
    }

    basic_csv_parameters<CharT>& trim_leading(bool value)
    {
        trim_leading_ = value;
        return *this;
    }

    bool trim_trailing() const
    {
        return trim_trailing_;
    }

    basic_csv_parameters<CharT>& trim_trailing(bool value)
    {
        trim_trailing_ = value;
        return *this;
    }

    bool trim_leading_inside_quotes() const
    {
        return trim_leading_inside_quotes_;
    }

    basic_csv_parameters<CharT>& trim_leading_inside_quotes(bool value)
    {
        trim_leading_inside_quotes_ = value;
        return *this;
    }

    bool trim_trailing_inside_quotes() const
    {
        return trim_trailing_inside_quotes_;
    }

    basic_csv_parameters<CharT>& trim_trailing_inside_quotes(bool value)
    {
        trim_trailing_inside_quotes_ = value;
        return *this;
    }

    bool trim() const
    {
        return trim_leading_ && trim_trailing_;
    }

    basic_csv_parameters<CharT>& trim(bool value)
    {
        trim_leading_ = value;
        trim_trailing_ = value;
        return *this;
    }

    bool trim_inside_quotes() const
    {
        return trim_leading_inside_quotes_ && trim_trailing_inside_quotes_;
    }

    basic_csv_parameters<CharT>& trim_inside_quotes(bool value)
    {
        trim_leading_inside_quotes_ = value;
        trim_trailing_inside_quotes_ = value;
        return *this;
    }

    bool unquoted_empty_value_is_null() const
    {
        return unquoted_empty_value_is_null_;
    }

    basic_csv_parameters<CharT>& unquoted_empty_value_is_null(bool value)
    {
        unquoted_empty_value_is_null_ = value;
        return *this;
    }

    std::vector<std::basic_string<CharT>> column_names() const
    {
        return column_names_;
    }

    basic_csv_parameters<CharT>& column_names(const std::vector<std::basic_string<CharT>>& value)
    {
        column_names_ = value;
        return *this;
    }

    std::vector<std::basic_string<CharT>> column_types() const
    {
        return column_types_;
    }

    basic_csv_parameters<CharT>& column_types(const std::vector<std::basic_string<CharT>>& value)
    {
        column_types_ = value;
        return *this;
    }

    enum class column_state {sequence,label};

    basic_csv_parameters<CharT>& column_types(const std::basic_string<CharT>& types)
    {
        std::vector<std::basic_string<CharT>> column_types;

        column_state state = column_state::sequence;
        int depth = 0;
        std::basic_string<CharT> buffer;

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
                        if (buffer == json_csv_parser_traits<CharT>::string_literal() ||
                            buffer == json_csv_parser_traits<CharT>::integer_literal() ||
                            buffer == json_csv_parser_traits<CharT>::float_literal() ||
                            buffer == json_csv_parser_traits<CharT>::boolean_literal())
                        {
                            column_types.push_back(buffer);
                            buffer.clear();
                        }
                        else
                        {
                            JSONCONS_ASSERT(false);
                        }
                        ++p;
                        state = column_state::sequence;
                        break;
                    case ']':
                        JSONCONS_ASSERT(depth > 0);
                        --depth;
                        if (buffer == json_csv_parser_traits<CharT>::string_literal() ||
                            buffer == json_csv_parser_traits<CharT>::integer_literal() ||
                            buffer == json_csv_parser_traits<CharT>::float_literal() ||
                            buffer == json_csv_parser_traits<CharT>::boolean_literal())
                        {
                            column_types.push_back(buffer);
                            buffer.clear();
                        }
                        else
                        {
                            JSONCONS_ASSERT(false);
                        }
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
            if (buffer == json_csv_parser_traits<CharT>::string_literal() ||
                buffer == json_csv_parser_traits<CharT>::integer_literal() ||
                buffer == json_csv_parser_traits<CharT>::float_literal() ||
                buffer == json_csv_parser_traits<CharT>::boolean_literal())
            {
                column_types.push_back(buffer);
                buffer.clear();
            }
            else
            {
                JSONCONS_ASSERT(false);
            }
        }
        column_types_ = column_types;
        return *this;
    }

    std::vector<std::basic_string<CharT>> column_defaults() const
    {
        return column_defaults_;
    }

    basic_csv_parameters<CharT>& column_defaults(const std::vector<std::basic_string<CharT>>& value)
    {
        column_defaults_ = value;
        return *this;
    }

    CharT field_delimiter() const
    {
        return field_delimiter_;
    }

    basic_csv_parameters<CharT>& field_delimiter(CharT value)
    {
        field_delimiter_ = value;
        return *this;
    }

    std::basic_string<CharT> line_delimiter() const
    {
        return line_delimiter_;
    }

    basic_csv_parameters<CharT>& line_delimiter(std::basic_string<CharT> value)
    {
        line_delimiter_ = value;
        return *this;
    }

    CharT quote_char() const
    {
        return quote_char_;
    }

    basic_csv_parameters<CharT>& quote_char(CharT value)
    {
        quote_char_ = value;
        return *this;
    }

    CharT quote_escape_char() const
    {
        return quote_escape_char_;
    }

    basic_csv_parameters<CharT>& quote_escape_char(CharT value)
    {
        quote_escape_char_ = value;
        return *this;
    }

    CharT comment_starter() const
    {
        return comment_starter_;
    }

    basic_csv_parameters<CharT>& comment_starter(CharT value)
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

    basic_csv_parameters<CharT>& quote_style(quote_style_type value)
    {
        quote_style_ = value;
        return *this;
    }

    basic_csv_parameters<CharT>& mapping(mapping_type value)
    {
        mapping_ = {value,true};
        return *this;
    }

    unsigned long max_lines() const
    {
        return max_lines_;
    }

    basic_csv_parameters<CharT>& max_lines(unsigned long value)
    {
        max_lines_ = value;
        return *this;
    }

private:
    bool assume_header_;
    bool ignore_empty_values_;
    bool trim_leading_;
    bool trim_trailing_;
    bool trim_leading_inside_quotes_;
    bool trim_trailing_inside_quotes_;
    bool unquoted_empty_value_is_null_;
    CharT field_delimiter_;
    CharT quote_char_;
    CharT quote_escape_char_;
    CharT comment_starter_;
    quote_style_type quote_style_;
    std::pair<mapping_type,bool> mapping_;
    unsigned long max_lines_;
    size_t header_lines_;
    std::basic_string<CharT> line_delimiter_;
    std::basic_string<CharT> header_;
    std::basic_string<CharT> data_types_;
    std::basic_string<CharT> default_values_;
    std::vector<std::basic_string<CharT>> column_names_;
    std::vector<std::basic_string<CharT>> column_types_;
    std::vector<std::basic_string<CharT>> column_defaults_;
};

typedef basic_csv_parameters<char> csv_parameters;
typedef basic_csv_parameters<wchar_t> wcsv_parameters;

}}
#endif

// Copyright 2013-2025 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_PARSER_HPP
#define JSONCONS_CSV_CSV_PARSER_HPP

#include <cctype>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory> // std::allocator
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/detail/parse_number.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_filter.hpp>
#include <jsoncons/json_reader.hpp>
#include <jsoncons/json_type.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/semantic_tag.hpp>
#include <jsoncons/ser_context.hpp>
#include <jsoncons/staj_event.hpp>

#include <jsoncons_ext/csv/csv_error.hpp>
#include <jsoncons_ext/csv/csv_options.hpp>

namespace jsoncons { 
namespace csv {

enum class csv_mode 
{
    initial,
    header,
    data,
    subfields
};

enum class csv_parse_state 
{
    start,
    cr, 
    expect_comment_or_record,
    expect_record,
    end_record,
    no_more_records,
    comment,
    between_values,
    quoted_string,
    unquoted_string,
    before_unquoted_string,
    escaped_value,
    minus, 
    zero,  
    integer,
    fraction,
    exp1,
    exp2,
    exp3,
    accept,
    before_unquoted_field,
    before_unquoted_field_tail, 
    before_unquoted_field_tail1,
    before_last_unquoted_field,
    before_last_unquoted_field_tail,
    before_unquoted_subfield,
    before_unquoted_subfield_tail,
    before_quoted_subfield,
    before_quoted_subfield_tail,
    before_quoted_field,
    before_quoted_field_tail,
    before_last_quoted_field,
    before_last_quoted_field_tail,
    done
};

enum class cached_state
{
    begin_object,
    end_object,
    begin_array,
    end_array,
    name,
    item,
    done
};

struct default_csv_parsing
{
    bool operator()(csv_errc, const ser_context&) noexcept
    {
        return false;
    }
};

namespace detail {

    template <typename CharT,typename TempAllocator >
    class parse_event
    {
        using temp_allocator_type = TempAllocator;
        using string_view_type = typename basic_json_visitor<CharT>::string_view_type;
        using char_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<CharT>;
        using byte_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<uint8_t>;                  
        using string_type = std::basic_string<CharT,std::char_traits<CharT>,char_allocator_type>;
        using byte_string_type = basic_byte_string<byte_allocator_type>;

        staj_event_type event_type;
        string_type string_value;
        byte_string_type byte_string_value;
        union
        {
            bool bool_value;
            int64_t int64_value;
            uint64_t uint64_value;
            double double_value;
        };
        semantic_tag tag;
    public:
        parse_event(staj_event_type event_type, semantic_tag tag, const TempAllocator& alloc)
            : event_type(event_type), 
              string_value(alloc),
              byte_string_value(alloc),
              tag(tag)
        {
        }

        parse_event(const string_view_type& value, semantic_tag tag, const TempAllocator& alloc)
            : event_type(staj_event_type::string_value), 
              string_value(value.data(),value.length(),alloc), 
              byte_string_value(alloc),
              tag(tag)
        {
        }

        parse_event(const byte_string_view& value, semantic_tag tag, const TempAllocator& alloc)
            : event_type(staj_event_type::byte_string_value), 
              string_value(alloc),
              byte_string_value(value.data(),value.size(),alloc), 
              tag(tag)
        {
        }

        parse_event(bool value, semantic_tag tag, const TempAllocator& alloc)
            : event_type(staj_event_type::bool_value), 
              string_value(alloc),
              byte_string_value(alloc),
              bool_value(value), 
              tag(tag)
        {
        }

        parse_event(int64_t value, semantic_tag tag, const TempAllocator& alloc)
            : event_type(staj_event_type::int64_value), 
              string_value(alloc),
              byte_string_value(alloc),
              int64_value(value), 
              tag(tag)
        {
        }

        parse_event(uint64_t value, semantic_tag tag, const TempAllocator& alloc)
            : event_type(staj_event_type::uint64_value), 
              string_value(alloc),
              byte_string_value(alloc),
              uint64_value(value), 
              tag(tag)
        {
        }

        parse_event(double value, semantic_tag tag, const TempAllocator& alloc)
            : event_type(staj_event_type::double_value), 
              string_value(alloc),
              byte_string_value(alloc),
              double_value(value), 
              tag(tag)
        {
        }

        parse_event(const parse_event&) = default;
        parse_event(parse_event&&) = default;
        parse_event& operator=(const parse_event&) = default;
        parse_event& operator=(parse_event&&) = default;

        void replay(basic_json_visitor<CharT>& visitor) const
        {
            switch (event_type)
            {
                case staj_event_type::begin_array:
                    visitor.begin_array(tag, ser_context());
                    break;
                case staj_event_type::end_array:
                    visitor.end_array(ser_context());
                    break;
                case staj_event_type::string_value:
                    visitor.string_value(string_value, tag, ser_context());
                    break;
                case staj_event_type::byte_string_value:
                    visitor.byte_string_value(byte_string_value, tag, ser_context());
                    break;
                case staj_event_type::null_value:
                    visitor.null_value(tag, ser_context());
                    break;
                case staj_event_type::bool_value:
                    visitor.bool_value(bool_value, tag, ser_context());
                    break;
                case staj_event_type::int64_value:
                    visitor.int64_value(int64_value, tag, ser_context());
                    break;
                case staj_event_type::uint64_value:
                    visitor.uint64_value(uint64_value, tag, ser_context());
                    break;
                case staj_event_type::double_value:
                    visitor.double_value(double_value, tag, ser_context());
                    break;
                default:
                    break;
            }
        }
    };

    template <typename CharT,typename TempAllocator >
    class m_columns_filter : public basic_json_visitor<CharT>
    {
    public:
        using string_view_type = typename basic_json_visitor<CharT>::string_view_type;
        using char_type = CharT;
        using temp_allocator_type = TempAllocator;

        using char_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<CharT>;
        using string_type = std::basic_string<CharT,std::char_traits<CharT>,char_allocator_type>;

        using string_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<string_type>;
        using parse_event_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<parse_event<CharT,TempAllocator>>;
        using parse_event_vector_type = std::vector<parse_event<CharT,TempAllocator>, parse_event_allocator_type>;
        using parse_event_vector_allocator_type = typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<parse_event_vector_type>;
    private:
        TempAllocator alloc_;
        std::size_t name_index_{0};
        int level_{0};
        int level2_{0};
        cached_state state_{cached_state::begin_object};
        std::size_t column_index_{0};
        std::size_t row_index_{0};

        std::vector<string_type, string_allocator_type> column_names_;
        std::vector<parse_event_vector_type,parse_event_vector_allocator_type> cached_events_;
    public:

        m_columns_filter(const TempAllocator& alloc)
            : alloc_(alloc),
              column_names_(alloc),
              cached_events_(alloc)
        {
        }

        void reset()
        {
            name_index_ = 0;
            level_ = 0;
            level2_ = 0;
            state_ = cached_state::begin_object;
            column_index_ = 0;
            row_index_ = 0;
            column_names_.clear();
            cached_events_.clear();
        }

        bool done() const
        {
            return state_ == cached_state::done;
        }

        void initialize(const std::vector<string_type, string_allocator_type>& column_names)
        {
            for (const auto& name : column_names)
            {
                column_names_.push_back(name);
                cached_events_.emplace_back(alloc_);
            }
            name_index_ = 0;
            level_ = 0;
            level2_ = 0;
            column_index_ = 0;
            row_index_ = 0;
            state_ = cached_state::begin_object;
        }

        void skip_column()
        {
            ++name_index_;
        }
        
        int level() const
        {
            return static_cast<int>(level_);
        }

        bool replay_parse_events(basic_json_visitor<CharT>& visitor, bool cursor_mode, int mark_level)
        {
            bool more = true;
            while (more)
            {
                switch (state_)
                {
                    case cached_state::begin_object:
                        visitor.begin_object(semantic_tag::none, ser_context());
                        ++level_;
                        more = !cursor_mode;
                        column_index_ = 0;
                        state_ = cached_state::name;
                        break;
                    case cached_state::end_object:
                        visitor.end_object(ser_context());
                        more = !cursor_mode;
                        if (level_ == mark_level)
                        {
                            more = false;
                        }
                        --level_;
                        state_ = cached_state::done;
                        break;
                    case cached_state::name:
                        if (column_index_ < column_names_.size())
                        {
                            visitor.key(column_names_[column_index_], ser_context());
                            more = !cursor_mode;
                            state_ = cached_state::begin_array;
                        }
                        else
                        {
                            state_ = cached_state::end_object;
                        }
                        break;
                    case cached_state::begin_array:
                        visitor.begin_array(semantic_tag::none, ser_context());
                        ++level_;
                        more = !cursor_mode;
                        row_index_ = 0;
                        state_ = cached_state::item;
                        break;
                    case cached_state::end_array:
                        visitor.end_array(ser_context());
                        more = !cursor_mode;
                        if (level_ == mark_level)
                        {
                            more = false;
                        }
                        --level_;
                        ++column_index_;
                        state_ = cached_state::name;
                        break;
                    case cached_state::item:
                        if (row_index_ < cached_events_[column_index_].size())
                        {
                            cached_events_[column_index_][row_index_].replay(visitor);
                            more = !cursor_mode;
                            ++row_index_;
                        }
                        else
                        {
                            state_ = cached_state::end_array;
                        }
                        break;
                    default:
                        more = false;
                        break;
                }
            }
            return more;
        }

        void visit_flush() override
        {
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_begin_object(semantic_tag, const ser_context&, std::error_code& ec) override
        {
            ec = csv_errc::invalid_parse_state;
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_end_object(const ser_context&, std::error_code& ec) override
        {
            ec = csv_errc::invalid_parse_state;
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_begin_array(semantic_tag tag, const ser_context&, std::error_code&) override
        {
            if (name_index_ < column_names_.size())
            {
                cached_events_[name_index_].emplace_back(staj_event_type::begin_array, tag, alloc_);
                
                ++level2_;
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_end_array(const ser_context&, std::error_code&) override
        {
            if (level2_ > 0)
            {
                cached_events_[name_index_].emplace_back(staj_event_type::end_array, semantic_tag::none, alloc_);
                ++name_index_;
                --level2_;
            }
            else
            {
                name_index_ = 0;
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_key(const string_view_type&, const ser_context&, std::error_code& ec) override
        {
            ec = csv_errc::invalid_parse_state;
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_null(semantic_tag tag, const ser_context&, std::error_code&) override
        {
            if (name_index_ < column_names_.size())
            {
                cached_events_[name_index_].emplace_back(staj_event_type::null_value, tag, alloc_);
                if (level2_ == 0)
                {
                    ++name_index_;
                }
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_string(const string_view_type& value, semantic_tag tag, const ser_context&, std::error_code&) override
        {
            if (name_index_ < column_names_.size())
            {
                cached_events_[name_index_].emplace_back(value, tag, alloc_);

                if (level2_ == 0)
                {
                    ++name_index_;
                }
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_byte_string(const byte_string_view& value,
                                  semantic_tag tag,
                                  const ser_context&,
                                  std::error_code&) override
        {
            if (name_index_ < column_names_.size())
            {
                cached_events_[name_index_].emplace_back(value, tag, alloc_);
                if (level2_ == 0)
                {
                    ++name_index_;
                }
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_double(double value,
                             semantic_tag tag, 
                             const ser_context&,
                             std::error_code&) override
        {
            if (name_index_ < column_names_.size())
            {
                cached_events_[name_index_].emplace_back(value, tag, alloc_);
                if (level2_ == 0)
                {
                    ++name_index_;
                }
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_int64(int64_t value,
                            semantic_tag tag,
                            const ser_context&,
                            std::error_code&) override
        {
            if (name_index_ < column_names_.size())
            {
                cached_events_[name_index_].emplace_back(value, tag, alloc_);
                if (level2_ == 0)
                {
                    ++name_index_;
                }
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_uint64(uint64_t value,
                             semantic_tag tag,
                             const ser_context&,
                             std::error_code&) override
        {
            if (name_index_ < column_names_.size())
            {
                cached_events_[name_index_].emplace_back(value, tag, alloc_);
                if (level2_ == 0)
                {
                    ++name_index_;
                }
            }
            JSONCONS_VISITOR_RETURN;
        }

        JSONCONS_VISITOR_RETURN_TYPE visit_bool(bool value, semantic_tag tag, const ser_context&, std::error_code&) override
        {
            if (name_index_ < column_names_.size())
            {
                cached_events_[name_index_].emplace_back(value, tag, alloc_);
                if (level2_ == 0)
                {
                    ++name_index_;
                }
            }
            JSONCONS_VISITOR_RETURN;
        }
    };

} // namespace detail

template <typename CharT,typename TempAllocator =std::allocator<char>>
class basic_csv_parser : public ser_context
{
public:
    using string_view_type = jsoncons::basic_string_view<CharT>;
    using char_type = CharT;
private:
    struct string_maps_to_double
    {
        string_view_type s;

        bool operator()(const std::pair<string_view_type,double>& val) const
        {
            return val.first == s;
        }
    };

    using temp_allocator_type = TempAllocator;
    typedef typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<CharT> char_allocator_type;
    using string_type = std::basic_string<CharT,std::char_traits<CharT>,char_allocator_type>;
    typedef typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<string_type> string_allocator_type;
    typedef typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<csv_mode> csv_mode_allocator_type;
    typedef typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<csv_type_info> csv_type_info_allocator_type;
    typedef typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<std::vector<string_type,string_allocator_type>> string_vector_allocator_type;
    typedef typename std::allocator_traits<temp_allocator_type>:: template rebind_alloc<csv_parse_state> csv_parse_state_allocator_type;

    static constexpr int default_depth = 3;

    temp_allocator_type alloc_;
    csv_parse_state state_;
    std::function<bool(csv_errc,const ser_context&)> err_handler_;
    std::size_t column_{1};
    std::size_t line_{1};
    int nesting_depth_{default_depth};
    bool assume_header_;
    char_type comment_starter_;
    char_type field_delimiter_;
    std::size_t header_lines_;
    bool ignore_empty_values_;
    bool ignore_empty_lines_;
    bool infer_types_;
    bool lossless_number_; 
    csv_mapping_kind mapping_kind_;
    std::size_t max_lines_;
    char_type quote_char_;
    char_type quote_escape_char_;
    char_type subfield_delimiter_;
    bool trim_leading_;
    bool trim_leading_inside_quotes_;
    bool trim_trailing_;
    bool trim_trailing_inside_quotes_;
    bool unquoted_empty_value_is_null_;
    std::size_t min_column_names_{0};
    std::size_t column_index_{0};
    int level_{0};
    std::size_t depth_{0};
    std::size_t offset_{0};
    jsoncons::detail::chars_to to_double_; 
    const CharT* begin_input_{nullptr};
    const CharT* input_end_{nullptr};
    const CharT* input_ptr_{nullptr};
    bool more_{true};
    std::size_t header_line_{1};
    bool cursor_mode_{false};
    bool actual_cursor_mode_{false};
    int mark_level_{0};
    std::size_t header_line_offset_{0};

    detail::m_columns_filter<CharT,TempAllocator> m_columns_filter_;
    std::vector<csv_mode,csv_mode_allocator_type> stack_;
    std::vector<string_type,string_allocator_type> column_names_;
    std::vector<csv_type_info,csv_type_info_allocator_type> column_types_;
    std::vector<string_type,string_allocator_type> column_defaults_;
    std::vector<csv_parse_state,csv_parse_state_allocator_type> state_stack_;
    string_type buffer_;
    std::vector<std::pair<std::basic_string<char_type>,double>> string_double_map_;

public:
    basic_csv_parser(const TempAllocator& alloc = TempAllocator())
       : basic_csv_parser(basic_csv_decode_options<CharT>(), 
                          default_csv_parsing(),
                          alloc)
    {
    }

    basic_csv_parser(const basic_csv_decode_options<CharT>& options,
                     const TempAllocator& alloc = TempAllocator())
        : basic_csv_parser(options, 
                           default_csv_parsing(),
                           alloc)
    {
    }

    basic_csv_parser(std::function<bool(csv_errc,const ser_context&)> err_handler,
                     const TempAllocator& alloc = TempAllocator())
        : basic_csv_parser(basic_csv_decode_options<CharT>(), 
                           err_handler,
                           alloc)
    {
    }

    basic_csv_parser(const basic_csv_decode_options<CharT>& options,
                     std::function<bool(csv_errc,const ser_context&)> err_handler,
                     const TempAllocator& alloc = TempAllocator())
       : alloc_(alloc),
         state_(csv_parse_state::start),
         err_handler_(err_handler),
         assume_header_(options.assume_header()),                  
         comment_starter_(options.comment_starter()),
         field_delimiter_(options.field_delimiter()),
         header_lines_(options.header_lines()),
         ignore_empty_values_(options.ignore_empty_values()),
         ignore_empty_lines_(options.ignore_empty_lines()),
         infer_types_(options.infer_types()),
         lossless_number_(options.lossless_number()), 
         mapping_kind_(options.mapping_kind()),
         max_lines_(options.max_lines()),
         quote_char_(options.quote_char()),
         quote_escape_char_(options.quote_escape_char()),
         subfield_delimiter_(options.subfield_delimiter()),
         trim_leading_(options.trim_leading()),
         trim_leading_inside_quotes_(options.trim_leading_inside_quotes()),
         trim_trailing_(options.trim_trailing()),
         trim_trailing_inside_quotes_(options.trim_trailing_inside_quotes()),
         unquoted_empty_value_is_null_(options.unquoted_empty_value_is_null()),
         m_columns_filter_(alloc),
         stack_(alloc),
         column_names_(alloc),
         column_types_(alloc),
         column_defaults_(alloc),
         state_stack_(alloc),
         buffer_(alloc)
    {
        if (options.enable_str_to_nan())
        {
            string_double_map_.emplace_back(options.nan_to_str(),std::nan(""));
        }
        if (options.enable_str_to_inf())
        {
            string_double_map_.emplace_back(options.inf_to_str(),std::numeric_limits<double>::infinity());
        }
        if (options.enable_str_to_neginf())
        {
            string_double_map_.emplace_back(options.neginf_to_str(),-std::numeric_limits<double>::infinity());
        }

        jsoncons::csv::detail::parse_column_types(options.column_types(), column_types_);
        jsoncons::csv::detail::parse_column_names(options.column_defaults(), column_defaults_);
        jsoncons::csv::detail::parse_column_names(options.column_names(), column_names_);
        min_column_names_ = column_names_.size();
        initialize();
    }

    ~basic_csv_parser() noexcept
    {
    }

    void cursor_mode(bool value)
    {
        actual_cursor_mode_ = value;
        cursor_mode_ = (mapping_kind_ == csv_mapping_kind::m_columns) ? false : value;
    }

    int level() const
    {
        return level_;
    }

    int mark_level() const 
    {
        return mark_level_;
    }

    void mark_level(int value)
    {
        mark_level_ = value;
    }

    bool done() const
    {
        return state_ == csv_parse_state::done;
    }

    bool accept() const
    {
        return state_ == csv_parse_state::accept || state_ == csv_parse_state::done;
    }

    bool stopped() const
    {
        return !more_;
    }

    bool source_exhausted() const
    {
        return input_ptr_ == input_end_;
    }

    const std::vector<string_type,string_allocator_type>& column_labels() const
    {
        return column_names_;
    }

    void reinitialize()
    {
        state_ = csv_parse_state::start;
        column_ = 1;
        line_ = 1;
        nesting_depth_ = default_depth;
        column_index_ = 0;
        level_ = 0;
        depth_ = 0;
        offset_ = 0;
        begin_input_ = nullptr;
        input_end_ = nullptr;
        input_ptr_ = nullptr;
        more_ = true;
        header_line_ = 1;
        m_columns_filter_.reset();
        stack_.clear();
        column_names_.erase(column_names_.begin() + min_column_names_, column_names_.end());
        state_stack_.clear();
        buffer_.clear();

        initialize();
    }

    void restart()
    {
        more_ = true;
    }

    void parse_some(basic_json_visitor<CharT>& visitor)
    {
        std::error_code ec;
        parse_some(visitor, ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            JSONCONS_THROW(ser_error(ec,line_,column_));
        }
    }

    void parse_some(basic_json_visitor<CharT>& visitor, std::error_code& ec)
    {
        basic_json_visitor<CharT>& local_visitor = (mapping_kind_ == csv_mapping_kind::m_columns) 
            ? m_columns_filter_ : visitor;

        switch (mapping_kind_)
        {
            case csv_mapping_kind::m_columns:
                cursor_mode_ = false;
                break;
            default:
                break;
        } 

        const CharT* local_input_end = input_end_;

        if (input_ptr_ == local_input_end && more_)
        {
            switch (state_)
            {
                case csv_parse_state::start:
                    ec = csv_errc::source_error;
                    more_ = false;
                    return;
                case csv_parse_state::before_unquoted_field:
                case csv_parse_state::before_last_unquoted_field:
                    end_unquoted_string_value(local_visitor, ec);
                    state_ = csv_parse_state::before_last_unquoted_field_tail;
                    break;
                case csv_parse_state::before_last_unquoted_field_tail:
                    if (stack_.back() == csv_mode::subfields)
                    {
                        stack_.pop_back();
                        local_visitor.end_array(*this, ec);
                        more_ = !cursor_mode_;
                        if (mapping_kind_ != csv_mapping_kind::m_columns)
                        {
                            if (level() == mark_level_)
                            {
                                more_ = false;
                            }
                        }
                        --level_;
                    }
                    ++column_index_;
                    state_ = csv_parse_state::end_record;
                    break;
                case csv_parse_state::before_unquoted_string: 
                    buffer_.clear();
                    JSONCONS_FALLTHROUGH;
                case csv_parse_state::unquoted_string: 
                    if (trim_leading_ || trim_trailing_)
                    {
                        trim_string_buffer(trim_leading_,trim_trailing_);
                    }
                    if (ignore_empty_values_ && buffer_.empty())
                    {
                        state_ = csv_parse_state::end_record;
                    }
                    else
                    {
                        before_value(local_visitor, ec);
                        state_ = csv_parse_state::before_unquoted_field;
                    }
                    break;
                case csv_parse_state::before_last_quoted_field:
                    end_quoted_string_value(local_visitor, ec);
                    ++column_index_;
                    state_ = csv_parse_state::end_record;
                    break;
                case csv_parse_state::escaped_value:
                    if (quote_escape_char_ == quote_char_)
                    {
                        if (!(ignore_empty_values_ && buffer_.empty()))
                        {
                            before_value(local_visitor, ec);
                            ++column_;
                            state_ = csv_parse_state::before_last_quoted_field;
                        }
                        else
                        {
                            state_ = csv_parse_state::end_record;
                        }
                    }
                    else
                    {
                        ec = csv_errc::invalid_escaped_char;
                        more_ = false;
                        return;
                    }
                    break;
                case csv_parse_state::end_record:
                    if (column_index_ > 0)
                    {
                        end_record(local_visitor, ec);
                    }
                    state_ = csv_parse_state::no_more_records;
                    break;
                case csv_parse_state::no_more_records: 
                    switch (stack_.back()) 
                    {
                        case csv_mode::header:
                            stack_.pop_back();
                            break;
                        case csv_mode::data:
                            stack_.pop_back();
                            break;
                        default:
                            break;
                    }
                    local_visitor.end_array(*this, ec);
                    more_ = !cursor_mode_;
                    if (mapping_kind_ != csv_mapping_kind::m_columns)
                    {
                        if (level() == mark_level_)
                        {
                            more_ = false;
                        }
                    }
                    --level_;
                    
                    if (mapping_kind_ == csv_mapping_kind::m_columns)
                    {
                        if (!m_columns_filter_.done())
                        {
                            more_ = m_columns_filter_.replay_parse_events(visitor, actual_cursor_mode_, mark_level_);
                        }
                        else
                        {
                            state_ = csv_parse_state::accept;
                        }
                    }
                    else
                    {
                        state_ = csv_parse_state::accept;
                    }
                    break;
                case csv_parse_state::accept:
                    if (!(stack_.size() == 1 && stack_.back() == csv_mode::initial))
                    {
                        err_handler_(csv_errc::unexpected_eof, *this);
                        ec = csv_errc::unexpected_eof;
                        more_ = false;
                        return;
                    }
                    stack_.pop_back();
                    local_visitor.flush();
                    state_ = csv_parse_state::done;
                    more_ = false;
                    return;
                default:
                    state_ = csv_parse_state::end_record;
                    break;
            }
        }

        for (; (input_ptr_ < local_input_end) && more_;)
        {
            CharT curr_char = *input_ptr_;

            switch (state_) 
            {
                case csv_parse_state::cr:
                    ++line_;
                    column_ = 1;
                    switch (*input_ptr_)
                    {
                        case '\n':
                            ++input_ptr_;
                            state_ = pop_state();
                            break;
                        default:
                            state_ = pop_state();
                            break;
                    }
                    break;
                case csv_parse_state::start:
                    if (mapping_kind_ != csv_mapping_kind::m_columns)
                    {
                        local_visitor.begin_array(semantic_tag::none, *this, ec);
                        more_ = !cursor_mode_;
                        ++level_;
                    }
                    if (assume_header_ && mapping_kind_ == csv_mapping_kind::n_rows && !column_names_.empty())
                    {
                        column_index_ = 0; 
                        local_visitor.begin_array(semantic_tag::none, *this, ec);
                        ++level_;
                        more_ = !cursor_mode_;
                        state_ = csv_parse_state::expect_comment_or_record;
                    }
                    else
                    {
                        state_ = csv_parse_state::expect_comment_or_record;
                    }
                    break;
                case csv_parse_state::comment: 
                    switch (curr_char)
                    {
                        case '\n':
                        {
                            ++line_;
                            if (stack_.back() == csv_mode::header)
                            {
                                ++header_line_offset_;
                            }
                            column_ = 1;
                            state_ = csv_parse_state::expect_comment_or_record;
                            break;
                        }
                        case '\r':
                            ++line_;
                            if (stack_.back() == csv_mode::header)
                            {
                                ++header_line_offset_;
                            }
                            column_ = 1;
                            state_ = csv_parse_state::expect_comment_or_record;
                            push_state(state_);
                            state_ = csv_parse_state::cr;
                            break;
                        default:
                            ++column_;
                            break;
                    }
                    ++input_ptr_;
                    break;
                
                case csv_parse_state::expect_comment_or_record:
                    buffer_.clear();
                    if (curr_char == comment_starter_)
                    {
                        state_ = csv_parse_state::comment;
                        ++column_;
                        ++input_ptr_;
                    }
                    else
                    {
                        state_ = csv_parse_state::expect_record;
                    }
                    break;
                case csv_parse_state::quoted_string: 
                    {
                        if (curr_char == quote_escape_char_)
                        {
                            state_ = csv_parse_state::escaped_value;
                        }
                        else if (curr_char == quote_char_)
                        {
                            state_ = csv_parse_state::between_values;
                        }
                        else
                        {
                            buffer_.push_back(static_cast<CharT>(curr_char));
                        }
                    }
                    ++column_;
                    ++input_ptr_;
                    break;
                case csv_parse_state::escaped_value: 
                    {
                        if (curr_char == quote_char_)
                        {
                            buffer_.push_back(static_cast<CharT>(curr_char));
                            state_ = csv_parse_state::quoted_string;
                            ++column_;
                            ++input_ptr_;
                        }
                        else if (quote_escape_char_ == quote_char_)
                        {
                            state_ = csv_parse_state::between_values;
                        }
                        else
                        {
                            ec = csv_errc::invalid_escaped_char;
                            more_ = false;
                            return;
                        }
                    }
                    break;
                case csv_parse_state::between_values:
                    switch (curr_char)
                    {
                        case '\r':
                        case '\n':
                        {
                            if (trim_leading_ || trim_trailing_)
                            {
                                trim_string_buffer(trim_leading_,trim_trailing_);
                            }
                            if (!(ignore_empty_values_ && buffer_.empty()))
                            {
                                before_value(local_visitor, ec);
                                state_ = csv_parse_state::before_last_quoted_field;
                            }
                            else
                            {
                                state_ = csv_parse_state::end_record;
                            }
                            break;
                        }
                        default:
                            if (curr_char == field_delimiter_)
                            {
                                if (trim_leading_ || trim_trailing_)
                                {
                                    trim_string_buffer(trim_leading_,trim_trailing_);
                                }
                                before_value(local_visitor, ec);
                                state_ = csv_parse_state::before_quoted_field;
                            }
                            else if (subfield_delimiter_ != char_type() && curr_char == subfield_delimiter_)
                            {
                                if (trim_leading_ || trim_trailing_)
                                {
                                    trim_string_buffer(trim_leading_,trim_trailing_);
                                }
                                before_value(local_visitor, ec);
                                state_ = csv_parse_state::before_quoted_subfield;
                            }
                            else if (curr_char == ' ' || curr_char == '\t')
                            {
                                ++column_;
                                ++input_ptr_;
                            }
                            else
                            {
                                ec = csv_errc::unexpected_char_between_fields;
                                more_ = false;
                                return;
                            }
                            break;
                    }
                    break;
                case csv_parse_state::before_unquoted_string: 
                {
                    buffer_.clear();
                    state_ = csv_parse_state::unquoted_string;
                    break;
                }
                case csv_parse_state::before_unquoted_field:
                    end_unquoted_string_value(local_visitor, ec);
                    state_ = csv_parse_state::before_unquoted_field_tail;
                    break;
                case csv_parse_state::before_unquoted_field_tail:
                {
                    if (stack_.back() == csv_mode::subfields)
                    {
                        stack_.pop_back();
                        local_visitor.end_array(*this, ec);
                        more_ = !cursor_mode_;
                        if (mapping_kind_ != csv_mapping_kind::m_columns)
                        {
                            if (level() == mark_level_)
                            {
                                more_ = false;
                            }
                        }
                        --level_;
                    }
                    ++column_index_;
                    state_ = csv_parse_state::before_unquoted_string;
                    ++column_;
                    ++input_ptr_;
                    break;
                }
                case csv_parse_state::before_unquoted_field_tail1:
                {
                    if (stack_.back() == csv_mode::subfields)
                    {
                        stack_.pop_back();
                        local_visitor.end_array(*this, ec);
                        more_ = !cursor_mode_;
                        if (mapping_kind_ != csv_mapping_kind::m_columns)
                        {
                            if (level() == mark_level_)
                            {
                                more_ = false;
                            }
                        }
                        --level_;
                    }
                    state_ = csv_parse_state::end_record;
                    ++column_;
                    ++input_ptr_;
                    break;
                }

                case csv_parse_state::before_last_unquoted_field:
                    end_unquoted_string_value(local_visitor, ec);
                    state_ = csv_parse_state::before_last_unquoted_field_tail;
                    break;

                case csv_parse_state::before_last_unquoted_field_tail:
                    if (stack_.back() == csv_mode::subfields)
                    {
                        stack_.pop_back();
                        local_visitor.end_array(*this, ec);
                        more_ = !cursor_mode_;
                        if (mapping_kind_ != csv_mapping_kind::m_columns)
                        {
                            if (level() == mark_level_)
                            {
                                more_ = false;
                            }
                        }
                        --level_;
                    }
                    ++column_index_;
                    state_ = csv_parse_state::end_record;
                    break;

                case csv_parse_state::before_unquoted_subfield:
                    if (stack_.back() == csv_mode::data)
                    {
                        stack_.push_back(csv_mode::subfields);
                        local_visitor.begin_array(semantic_tag::none, *this, ec);
                        more_ = !cursor_mode_;
                        ++level_;
                    }
                    state_ = csv_parse_state::before_unquoted_subfield_tail;
                    break; 
                case csv_parse_state::before_unquoted_subfield_tail:
                    end_unquoted_string_value(local_visitor, ec);
                    state_ = csv_parse_state::before_unquoted_string;
                    ++column_;
                    ++input_ptr_;
                    break;
                case csv_parse_state::before_quoted_field:
                    end_quoted_string_value(local_visitor, ec);
                    state_ = csv_parse_state::before_unquoted_field_tail; // return to unquoted
                    break;
                case csv_parse_state::before_quoted_subfield:
                    if (stack_.back() == csv_mode::data)
                    {
                        stack_.push_back(csv_mode::subfields);
                        local_visitor.begin_array(semantic_tag::none, *this, ec);
                        more_ = !cursor_mode_;
                        ++level_;
                    }
                    state_ = csv_parse_state::before_quoted_subfield_tail;
                    break; 
                case csv_parse_state::before_quoted_subfield_tail:
                    end_quoted_string_value(local_visitor, ec);
                    state_ = csv_parse_state::before_unquoted_string;
                    ++column_;
                    ++input_ptr_;
                    break;
                case csv_parse_state::before_last_quoted_field:
                    end_quoted_string_value(local_visitor, ec);
                    state_ = csv_parse_state::before_last_quoted_field_tail;
                    break;
                case csv_parse_state::before_last_quoted_field_tail:
                    if (stack_.back() == csv_mode::subfields)
                    {
                        stack_.pop_back();
                        local_visitor.end_array(*this, ec);
                        more_ = !cursor_mode_;
                        if (mapping_kind_ != csv_mapping_kind::m_columns)
                        {
                            if (level() == mark_level_)
                            {
                                more_ = false;
                            }
                        }
                        --level_;
                    }
                    ++column_index_;
                    state_ = csv_parse_state::end_record;
                    break;
                case csv_parse_state::unquoted_string: 
                {
                    switch (curr_char)
                    {
                        case '\n':
                        case '\r':
                        {
                            if (trim_leading_ || trim_trailing_)
                            {
                                trim_string_buffer(trim_leading_,trim_trailing_);
                            }
                            if (!(ignore_empty_values_ && buffer_.empty()))
                            {
                                before_value(local_visitor, ec);
                                state_ = csv_parse_state::before_last_unquoted_field;
                            }
                            else
                            {
                                state_ = csv_parse_state::end_record;
                            }
                            break;
                        }
                        default:
                            if (curr_char == field_delimiter_)
                            {
                                if (trim_leading_ || trim_trailing_)
                                {
                                    trim_string_buffer(trim_leading_,trim_trailing_);
                                }
                                before_value(local_visitor, ec);
                                state_ = csv_parse_state::before_unquoted_field;
                            }
                            else if (subfield_delimiter_ != char_type() && curr_char == subfield_delimiter_)
                            {
                                if (trim_leading_ || trim_trailing_)
                                {
                                    trim_string_buffer(trim_leading_,trim_trailing_);
                                }
                                before_value(local_visitor, ec);
                                state_ = csv_parse_state::before_unquoted_subfield;
                            }
                            else if (curr_char == quote_char_)
                            {
                                buffer_.clear();
                                state_ = csv_parse_state::quoted_string;
                                ++column_;
                                ++input_ptr_;
                            }
                            else
                            {
                                buffer_.push_back(static_cast<CharT>(curr_char));
                                ++column_;
                                ++input_ptr_;
                            }
                            break;
                    }
                    break;
                }
                case csv_parse_state::expect_record: 
                {
                    switch (curr_char)
                    {
                        case '\n':
                        {
                            if (!ignore_empty_lines_)
                            {
                                begin_record(local_visitor, ec);
                                state_ = csv_parse_state::end_record;
                            }
                            else
                            {
                                ++line_;
                                column_ = 1;
                                state_ = csv_parse_state::expect_comment_or_record;
                                ++input_ptr_;
                            }
                            break;
                        }
                        case '\r':
                            if (!ignore_empty_lines_)
                            {
                                begin_record(local_visitor, ec);
                                state_ = csv_parse_state::end_record;
                            }
                            else
                            {
                                ++input_ptr_;
                                push_state(state_);
                                state_ = csv_parse_state::cr;
                            }
                            break;
                        case ' ':
                        case '\t':
                            if (!trim_leading_)
                            {
                                buffer_.push_back(static_cast<CharT>(curr_char));
                                begin_record(local_visitor, ec);
                                state_ = csv_parse_state::unquoted_string;
                            }
                            ++column_;
                            ++input_ptr_;
                            break;
                        default:
                            begin_record(local_visitor, ec);
                            if (curr_char == quote_char_)
                            {
                                buffer_.clear();
                                state_ = csv_parse_state::quoted_string;
                                ++column_;
                                ++input_ptr_;
                            }
                            else
                            {
                                state_ = csv_parse_state::unquoted_string;
                            }
                            break;
                        }
                    break;
                    }
                case csv_parse_state::end_record: 
                {
                    switch (curr_char)
                    {
                        case '\n':
                        {
                            ++line_;
                            column_ = 1;
                            state_ = csv_parse_state::expect_comment_or_record;
                            end_record(local_visitor, ec);
                            ++input_ptr_;
                            break;
                        }
                        case '\r':
                            ++line_;
                            column_ = 1;
                            state_ = csv_parse_state::expect_comment_or_record;
                            end_record(local_visitor, ec);
                            push_state(state_);
                            state_ = csv_parse_state::cr;
                            ++input_ptr_;
                            break;
                        case ' ':
                        case '\t':
                            ++column_;
                            ++input_ptr_;
                            break;
                        default:
                            err_handler_(csv_errc::syntax_error, *this);
                            ec = csv_errc::syntax_error;
                            more_ = false;
                            return;
                        }
                    break;
                }
                default:
                    err_handler_(csv_errc::invalid_parse_state, *this);
                    ec = csv_errc::invalid_parse_state;
                    more_ = false;
                    return;
            }
            if (line_ > max_lines_)
            {
                state_ = csv_parse_state::done;
                more_ = false;
            }
        }
    }

    void finish_parse()
    {
        std::error_code ec;
        finish_parse(ec);
        if (JSONCONS_UNLIKELY(ec))
        {
            JSONCONS_THROW(ser_error(ec,line_,column_));
        }
    }

    void finish_parse(std::error_code& ec)
    {
        while (more_)
        {
            parse_some(ec);
        }
    }

    csv_parse_state state() const
    {
        return state_;
    }

    void update(const string_view_type sv)
    {
        update(sv.data(),sv.length());
    }

    void update(const CharT* data, std::size_t length)
    {
        begin_input_ = data;
        input_end_ = data + length;
        input_ptr_ = begin_input_;
    }

    std::size_t line() const override
    {
        return line_;
    }

    std::size_t column() const override
    {
        return column_;
    }

private:
    void initialize()
    {
        stack_.reserve(default_depth);
        stack_.push_back(csv_mode::initial);
        stack_.push_back((header_lines_ > 0) ? csv_mode::header : csv_mode::data);
    }

    // name
    void before_value(basic_json_visitor<CharT>& visitor, 
        std::error_code& ec)
    {
        switch (stack_.back())
        {
            case csv_mode::header:
                if (trim_leading_inside_quotes_ || trim_trailing_inside_quotes_)
                {
                    trim_string_buffer(trim_leading_inside_quotes_,trim_trailing_inside_quotes_);
                }
                if (line_ == (header_line_+header_line_offset_) && column_index_ >= min_column_names_)
                {
                    column_names_.push_back(buffer_);
                    if (assume_header_ && mapping_kind_ == csv_mapping_kind::n_rows)
                    {
                        visitor.string_value(buffer_, semantic_tag::none, *this, ec);
                        more_ = !cursor_mode_;
                    }
                }
                break;
            case csv_mode::data:
                if (mapping_kind_ == csv_mapping_kind::n_objects)
                {
                    if (!(ignore_empty_values_ && buffer_.empty()))
                    {
                        if (column_index_ < column_names_.size() + offset_)
                        {
                            visitor.key(column_names_[column_index_ - offset_], *this, ec);
                            more_ = !cursor_mode_;
                        }
                    }
                }
                break;
            default:
                break;
        }
    }

    // begin_array or begin_record
    void begin_record(basic_json_visitor<CharT>& visitor, std::error_code& ec)
    {
        offset_ = 0;

        if (stack_.back() == csv_mode::header && line_ > (header_lines_+header_line_offset_))
        {
            stack_.back() = csv_mode::data;
        }
        switch (stack_.back())
        {
            case csv_mode::header:
                switch (mapping_kind_)
                {
                    case csv_mapping_kind::n_rows:
                        if (assume_header_ && line_ == (header_line_+header_line_offset_))
                        {
                            visitor.begin_array(semantic_tag::none, *this, ec);
                            more_ = !cursor_mode_;
                            ++level_;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case csv_mode::data:
                switch (mapping_kind_)
                {
                    case csv_mapping_kind::n_rows:
                        visitor.begin_array(semantic_tag::none, *this, ec);
                        more_ = !cursor_mode_;
                        ++level_;
                        break;
                    case csv_mapping_kind::n_objects:
                        visitor.begin_object(semantic_tag::none, *this, ec);
                        more_ = !cursor_mode_;
                        ++level_;
                        break;
                    case csv_mapping_kind::m_columns:
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    // end_array, begin_array, string_value (headers)
    void end_record(basic_json_visitor<CharT>& visitor, std::error_code& ec)
    {
        if (!column_types_.empty())
        {
            switch (mapping_kind_)
            {
                case csv_mapping_kind::n_rows:
                case csv_mapping_kind::n_objects:
                    if (depth_ > 0)
                    {
                        visitor.end_array(*this, ec);
                        more_ = !cursor_mode_;
                        if (level_ == mark_level_)
                        {
                            more_ = false;
                        }
                        --level_;
                        depth_ = 0;
                    }
                    break;
                case csv_mapping_kind::m_columns:
                    if (depth_ > 0)
                    {
                        visitor.end_array(*this, ec);
                        more_ = !cursor_mode_;
                        --level_;
                        depth_ = 0;
                    }
                    break;
                default:
                    break;
            }
        }
        switch (stack_.back())
        {
            case csv_mode::header:
                if (line_ >= header_lines_)
                {
                    stack_.back() = csv_mode::data;
                }
                switch (mapping_kind_)
                {
                    case csv_mapping_kind::n_rows:
                        if (assume_header_)
                        {
                            visitor.end_array(*this, ec);
                            more_ = !cursor_mode_;
                            if (level() == mark_level_)
                            {
                                more_ = false;
                            }
                            --level_;
                        }
                        break;
                    case csv_mapping_kind::m_columns:
                        m_columns_filter_.initialize(column_names_);
                        break;
                    default:
                        break;
                }
                break;
            case csv_mode::data:
            case csv_mode::subfields:
            {
                switch (mapping_kind_)
                {
                    case csv_mapping_kind::n_rows:
                        visitor.end_array(*this, ec);
                        more_ = !cursor_mode_;
                        if (level_ == mark_level_)
                        {
                            more_ = false;
                        }
                        --level_;
                        break;
                    case csv_mapping_kind::n_objects:
                        visitor.end_object(*this, ec);
                        more_ = !cursor_mode_;
                        if (level_ == mark_level_)
                        {
                            more_ = false;
                        }
                        --level_;
                        break;
                    case csv_mapping_kind::m_columns:
                        visitor.end_array(*this, ec);
                        more_ = !cursor_mode_;
                        --level_;
                        break;
                }
                break;
            }
            default:
                break;
        }
        column_index_ = 0;
    }

    void trim_string_buffer(bool trim_leading, bool trim_trailing)
    {
        std::size_t start = 0;
        std::size_t length = buffer_.length();
        if (trim_leading)
        {
            bool done = false;
            while (!done && start < buffer_.length())
            {
                if ((buffer_[start] < 256) && std::isspace(buffer_[start]))
                {
                    ++start;
                }
                else
                {
                    done = true;
                }
            }
        }
        if (trim_trailing)
        {
            bool done = false;
            while (!done && length > 0)
            {
                if ((buffer_[length-1] < 256) && std::isspace(buffer_[length-1]))
                {
                    --length;
                }
                else
                {
                    done = true;
                }
            }
        }
        if (start != 0 || length != buffer_.size())
        {
            // Do not use buffer_.substr(...), as this won't preserve the allocator state.
            buffer_.resize(length);
            buffer_.erase(0, start);
        }
    }

    /*
        end_array, begin_array, xxx_value (end_value)
    */
    void end_unquoted_string_value(basic_json_visitor<CharT>& visitor, std::error_code& ec) 
    {
        switch (stack_.back())
        {
            case csv_mode::data:
            case csv_mode::subfields:
                switch (mapping_kind_)
                {
                case csv_mapping_kind::n_rows:
                    if (unquoted_empty_value_is_null_ && buffer_.length() == 0)
                    {
                        visitor.null_value(semantic_tag::none, *this, ec);
                        more_ = !cursor_mode_;
                    }
                    else
                    {
                        end_value(visitor, infer_types_, ec);
                    }
                    break;
                case csv_mapping_kind::n_objects:
                    if (!(ignore_empty_values_ && buffer_.empty()))
                    {
                        if (column_index_ < column_names_.size() + offset_)
                        {
                            if (unquoted_empty_value_is_null_ && buffer_.length() == 0)
                            {
                                visitor.null_value(semantic_tag::none, *this, ec);
                                more_ = !cursor_mode_;
                            }
                            else
                            {
                                end_value(visitor, infer_types_, ec);
                            }
                        }
                        else if (depth_ > 0)
                        {
                            if (unquoted_empty_value_is_null_ && buffer_.length() == 0)
                            {
                                visitor.null_value(semantic_tag::none, *this, ec);
                                more_ = !cursor_mode_;
                            }
                            else
                            {
                                end_value(visitor, infer_types_, ec);
                            }
                        }
                    }
                    break;
                case csv_mapping_kind::m_columns:
                    if (!(ignore_empty_values_ && buffer_.empty()))
                    {
                        end_value(visitor, infer_types_, ec);
                    }
                    else
                    {
                        m_columns_filter_.skip_column();
                    }
                    break;
                }
                break;
            default:
                break;
        }
    }

    void end_quoted_string_value(basic_json_visitor<CharT>& visitor, 
        std::error_code& ec) 
    {
        switch (stack_.back())
        {
            case csv_mode::data:
            case csv_mode::subfields:
                if (trim_leading_inside_quotes_ || trim_trailing_inside_quotes_)
                {
                    trim_string_buffer(trim_leading_inside_quotes_,trim_trailing_inside_quotes_);
                }
                switch (mapping_kind_)
                {
                case csv_mapping_kind::n_rows:
                    end_value(visitor, false, ec);
                    break;
                case csv_mapping_kind::n_objects:
                    if (!(ignore_empty_values_ && buffer_.empty()))
                    {
                        if (column_index_ < column_names_.size() + offset_)
                        {
                            if (unquoted_empty_value_is_null_ && buffer_.length() == 0)
                            {
                                visitor.null_value(semantic_tag::none, *this, ec);
                                more_ = !cursor_mode_;
                            }
                            else 
                            {
                                end_value(visitor, false, ec);
                            }
                        }
                        else if (depth_ > 0)
                        {
                            if (unquoted_empty_value_is_null_ && buffer_.length() == 0)
                            {
                                visitor.null_value(semantic_tag::none, *this, ec);
                                more_ = !cursor_mode_;
                            }
                            else
                            {
                                end_value(visitor, false, ec);
                            }
                        }
                    }
                    break;
                case csv_mapping_kind::m_columns:
                    if (!(ignore_empty_values_ && buffer_.empty()))
                    {
                        end_value(visitor, false, ec);
                    }
                    else
                    {
                        m_columns_filter_.skip_column();
                    }
                    break;
                }
                break;
            default:
                break;
        }
    }

    void end_value(basic_json_visitor<CharT>& visitor, 
        bool infer_types, std::error_code&  ec)
    {
        auto it = std::find_if(string_double_map_.begin(), string_double_map_.end(), string_maps_to_double{ buffer_ });
        if (it != string_double_map_.end())
        {
            visitor.double_value((*it).second, semantic_tag::none, *this, ec);
            more_ = !cursor_mode_;
        }
        else if (column_index_ < column_types_.size() + offset_)
        {
            if (column_types_[column_index_ - offset_].col_type == csv_column_type::repeat_t)
            {
                offset_ = offset_ + column_types_[column_index_ - offset_].rep_count;
                if (column_index_ - offset_ + 1 < column_types_.size())
                {
                    if (column_index_ == offset_ || depth_ > column_types_[column_index_-offset_].level)
                    {
                        visitor.end_array(*this, ec);
                        more_ = !cursor_mode_;
                        if (mapping_kind_ != csv_mapping_kind::m_columns)
                        {
                            if (level() == mark_level_)
                            {
                                more_ = false;
                            }
                        }
                        --level_;
                    }
                    depth_ = column_index_ == offset_ ? 0 : column_types_[column_index_ - offset_].level;
                }
            }
            if (depth_ < column_types_[column_index_ - offset_].level)
            {
                visitor.begin_array(semantic_tag::none, *this, ec);
                more_ = !cursor_mode_;
                depth_ = column_types_[column_index_ - offset_].level;
                ++level_;
            }
            else if (depth_ > column_types_[column_index_ - offset_].level)
            {
                visitor.end_array(*this, ec);
                more_ = !cursor_mode_;
                if (mapping_kind_ != csv_mapping_kind::m_columns)
                {
                    if (level() == mark_level_)
                    {
                        more_ = false;
                    }
                }
                --level_;
                depth_ = column_types_[column_index_ - offset_].level;
            }
            switch (column_types_[column_index_ - offset_].col_type)
            {
                case csv_column_type::integer_t:
                    {
                        std::basic_istringstream<CharT,std::char_traits<CharT>,char_allocator_type> iss{buffer_};
                        int64_t val;
                        iss >> val;
                        if (!iss.fail())
                        {
                            visitor.int64_value(val, semantic_tag::none, *this, ec);
                            more_ = !cursor_mode_;
                        }
                        else
                        {
                            if (column_index_ - offset_ < column_defaults_.size() && column_defaults_[column_index_ - offset_].length() > 0)
                            {
                                basic_json_parser<CharT,temp_allocator_type> parser(alloc_);
                                parser.update(column_defaults_[column_index_ - offset_].data(),column_defaults_[column_index_ - offset_].length());
                                parser.parse_some(visitor);
                                parser.finish_parse(visitor);
                            }
                            else
                            {
                                visitor.null_value(semantic_tag::none, *this, ec);
                                more_ = !cursor_mode_;
                            }
                        }
                    }
                    break;
                case csv_column_type::float_t:
                    {
                        if (lossless_number_)
                        {
                            visitor.string_value(buffer_,semantic_tag::bigdec, *this, ec);
                            more_ = !cursor_mode_;
                        }
                        else
                        {
                            std::basic_istringstream<CharT, std::char_traits<CharT>, char_allocator_type> iss{ buffer_ };
                            double val;
                            iss >> val;
                            if (!iss.fail())
                            {
                                visitor.double_value(val, semantic_tag::none, *this, ec);
                                more_ = !cursor_mode_;
                            }
                            else
                            {
                                if (column_index_ - offset_ < column_defaults_.size() && column_defaults_[column_index_ - offset_].length() > 0)
                                {
                                    basic_json_parser<CharT,temp_allocator_type> parser(alloc_);
                                    parser.update(column_defaults_[column_index_ - offset_].data(),column_defaults_[column_index_ - offset_].length());
                                    parser.parse_some(visitor);
                                    parser.finish_parse(visitor);
                                }
                                else
                                {
                                    visitor.null_value(semantic_tag::none, *this, ec);
                                    more_ = !cursor_mode_;
                                }
                            }
                        }
                    }
                    break;
                case csv_column_type::boolean_t:
                    {
                        if (buffer_.length() == 1 && buffer_[0] == '0')
                        {
                            visitor.bool_value(false, semantic_tag::none, *this, ec);
                            more_ = !cursor_mode_;
                        }
                        else if (buffer_.length() == 1 && buffer_[0] == '1')
                        {
                            visitor.bool_value(true, semantic_tag::none, *this, ec);
                            more_ = !cursor_mode_;
                        }
                        else if (buffer_.length() == 5 && ((buffer_[0] == 'f' || buffer_[0] == 'F') && (buffer_[1] == 'a' || buffer_[1] == 'A') && (buffer_[2] == 'l' || buffer_[2] == 'L') && (buffer_[3] == 's' || buffer_[3] == 'S') && (buffer_[4] == 'e' || buffer_[4] == 'E')))
                        {
                            visitor.bool_value(false, semantic_tag::none, *this, ec);
                            more_ = !cursor_mode_;
                        }
                        else if (buffer_.length() == 4 && ((buffer_[0] == 't' || buffer_[0] == 'T') && (buffer_[1] == 'r' || buffer_[1] == 'R') && (buffer_[2] == 'u' || buffer_[2] == 'U') && (buffer_[3] == 'e' || buffer_[3] == 'E')))
                        {
                            visitor.bool_value(true, semantic_tag::none, *this, ec);
                            more_ = !cursor_mode_;
                        }
                        else
                        {
                            if (column_index_ - offset_ < column_defaults_.size() && column_defaults_[column_index_ - offset_].length() > 0)
                            {
                                basic_json_parser<CharT,temp_allocator_type> parser(alloc_);
                                parser.update(column_defaults_[column_index_ - offset_].data(),column_defaults_[column_index_ - offset_].length());
                                parser.parse_some(visitor);
                                parser.finish_parse(visitor);
                            }
                            else
                            {
                                visitor.null_value(semantic_tag::none, *this, ec);
                                more_ = !cursor_mode_;
                            }
                        }
                    }
                    break;
                default:
                    if (buffer_.length() > 0)
                    {
                        visitor.string_value(buffer_, semantic_tag::none, *this, ec);
                        more_ = !cursor_mode_;
                    }
                    else
                    {
                        if (column_index_ < column_defaults_.size() + offset_ && column_defaults_[column_index_ - offset_].length() > 0)
                        {
                            basic_json_parser<CharT,temp_allocator_type> parser(alloc_);
                            parser.update(column_defaults_[column_index_ - offset_].data(),column_defaults_[column_index_ - offset_].length());
                            parser.parse_some(visitor);
                            parser.finish_parse(visitor);
                        }
                        else
                        {
                            visitor.string_value(string_view_type(), semantic_tag::none, *this, ec);
                            more_ = !cursor_mode_;
                        }
                    }
                    break;  
            }
        }
        else
        {
            if (infer_types)
            {
                end_value_with_numeric_check(visitor, ec);
            }
            else
            {
                visitor.string_value(buffer_, semantic_tag::none, *this, ec);
                more_ = !cursor_mode_;
            }
        }
    }

    enum class numeric_check_state 
    {
        initial,
        null,
        boolean_true,
        boolean_false,
        minus,
        zero,
        integer,
        fraction1,
        fraction,
        exp1,
        exp,
        not_a_number
    };

    /*
        xxx_value 
    */
    void end_value_with_numeric_check(basic_json_visitor<CharT>& visitor, 
        std::error_code& ec)
    {
        numeric_check_state state = numeric_check_state::initial;
        bool is_negative = false;
        //int precision = 0;
        //uint8_t decimal_places = 0;

        auto last = buffer_.end();

        std::string buffer;
        for (auto p = buffer_.begin(); state != numeric_check_state::not_a_number && p != last; ++p)
        {
            switch (state)
            {
                case numeric_check_state::initial:
                {
                    switch (*p)
                    {
                    case 'n':case 'N':
                        if ((last-p) == 4 && (p[1] == 'u' || p[1] == 'U') && (p[2] == 'l' || p[2] == 'L') && (p[3] == 'l' || p[3] == 'L'))
                        {
                            state = numeric_check_state::null;
                        }
                        else
                        {
                            state = numeric_check_state::not_a_number;
                        }
                        break;
                    case 't':case 'T':
                        if ((last-p) == 4 && (p[1] == 'r' || p[1] == 'R') && (p[2] == 'u' || p[2] == 'U') && (p[3] == 'e' || p[3] == 'U'))
                        {
                            state = numeric_check_state::boolean_true;
                        }
                        else
                        {
                            state = numeric_check_state::not_a_number;
                        }
                        break;
                    case 'f':case 'F':
                        if ((last-p) == 5 && (p[1] == 'a' || p[1] == 'A') && (p[2] == 'l' || p[2] == 'L') && (p[3] == 's' || p[3] == 'S') && (p[4] == 'e' || p[4] == 'E'))
                        {
                            state = numeric_check_state::boolean_false;
                        }
                        else
                        {
                            state = numeric_check_state::not_a_number;
                        }
                        break;
                    case '-':
                        is_negative = true;
                        buffer.push_back(*p);
                        state = numeric_check_state::minus;
                        break;
                    case '0':
                        //++precision;
                        buffer.push_back(*p);
                        state = numeric_check_state::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                        //++precision;
                        buffer.push_back(*p);
                        state = numeric_check_state::integer;
                        break;
                    default:
                        state = numeric_check_state::not_a_number;
                        break;
                    }
                    break;
                }
                case numeric_check_state::zero:
                {
                    switch (*p)
                    {
                    case '.':
                        buffer.push_back(to_double_.get_decimal_point());
                        state = numeric_check_state::fraction1;
                        break;
                    case 'e':case 'E':
                        buffer.push_back(*p);
                        state = numeric_check_state::exp1;
                        break;
                    default:
                        state = numeric_check_state::not_a_number;
                        break;
                    }
                    break;
                }
                case numeric_check_state::integer:
                {
                    switch (*p)
                    {
                    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                        //++precision;
                        buffer.push_back(*p);
                        break;
                    case '.':
                        buffer.push_back(to_double_.get_decimal_point());
                        state = numeric_check_state::fraction1;
                        break;
                    case 'e':case 'E':
                        buffer.push_back(*p);
                        state = numeric_check_state::exp1;
                        break;
                    default:
                        state = numeric_check_state::not_a_number;
                        break;
                    }
                    break;
                }
                case numeric_check_state::minus:
                {
                    switch (*p)
                    {
                    case '0':
                        //++precision;
                        buffer.push_back(*p);
                        state = numeric_check_state::zero;
                        break;
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                        //++precision;
                        buffer.push_back(*p);
                        state = numeric_check_state::integer;
                        break;
                    default:
                        state = numeric_check_state::not_a_number;
                        break;
                    }
                    break;
                }
                case numeric_check_state::fraction1:
                {
                    switch (*p)
                    {
                    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                        //++precision;
                        //++decimal_places;
                        buffer.push_back(*p);
                        state = numeric_check_state::fraction;
                        break;
                    default:
                        state = numeric_check_state::not_a_number;
                        break;
                    }
                    break;
                }
                case numeric_check_state::fraction:
                {
                    switch (*p)
                    {
                    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                        //++precision;
                        //++decimal_places;
                        buffer.push_back(*p);
                        break;
                    case 'e':case 'E':
                        buffer.push_back(*p);
                        state = numeric_check_state::exp1;
                        break;
                    default:
                        state = numeric_check_state::not_a_number;
                        break;
                    }
                    break;
                }
                case numeric_check_state::exp1:
                {
                    switch (*p)
                    {
                    case '-':
                        buffer.push_back(*p);
                        break;
                    case '+':
                        break;
                    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                        state = numeric_check_state::exp;
                        buffer.push_back(*p);
                        break;
                    default:
                        state = numeric_check_state::not_a_number;
                        break;
                    }
                    break;
                }
                case numeric_check_state::exp:
                {
                    switch (*p)
                    {
                    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                        buffer.push_back(*p);
                        break;
                    default:
                        state = numeric_check_state::not_a_number;
                        break;
                    }
                    break;
                }
                default:
                    break;
            }
        }

        switch (state)
        {
            case numeric_check_state::null:
                visitor.null_value(semantic_tag::none, *this, ec);
                more_ = !cursor_mode_;
                break;
            case numeric_check_state::boolean_true:
                visitor.bool_value(true, semantic_tag::none, *this, ec);
                more_ = !cursor_mode_;
                break;
            case numeric_check_state::boolean_false:
                visitor.bool_value(false, semantic_tag::none, *this, ec);
                more_ = !cursor_mode_;
                break;
            case numeric_check_state::zero:
            case numeric_check_state::integer:
            {
                if (is_negative)
                {
                    int64_t val{ 0 };
                    auto result = jsoncons::detail::dec_to_integer(buffer_.data(), buffer_.length(), val);
                    if (result)
                    {
                        visitor.int64_value(val, semantic_tag::none, *this, ec);
                        more_ = !cursor_mode_;
                    }
                    else // Must be overflow
                    {
                        visitor.string_value(buffer_, semantic_tag::bigint, *this, ec);
                        more_ = !cursor_mode_;
                    }
                }
                else
                {
                    uint64_t val{ 0 };
                    auto result = jsoncons::detail::dec_to_integer(buffer_.data(), buffer_.length(), val);
                    if (result)
                    {
                        visitor.uint64_value(val, semantic_tag::none, *this, ec);
                        more_ = !cursor_mode_;
                    }
                    else if (result.ec == jsoncons::detail::to_integer_errc::overflow)
                    {
                        visitor.string_value(buffer_, semantic_tag::bigint, *this, ec);
                        more_ = !cursor_mode_;
                    }
                    else
                    {
                        ec = result.ec;
                        more_ = false;
                        return;
                    }
                }
                break;
            }
            case numeric_check_state::fraction:
            case numeric_check_state::exp:
            {
                if (lossless_number_)
                {
                    visitor.string_value(buffer_,semantic_tag::bigdec, *this, ec);
                    more_ = !cursor_mode_;
                }
                else
                {
                    double d = to_double_(buffer.c_str(), buffer.length());
                    visitor.double_value(d, semantic_tag::none, *this, ec);
                    more_ = !cursor_mode_;
                }
                break;
            }
            default:
            {
                visitor.string_value(buffer_, semantic_tag::none, *this, ec);
                more_ = !cursor_mode_;
                break;
            }
        }
    } 

    void push_state(csv_parse_state state)
    {
        state_stack_.push_back(state);
    }

    csv_parse_state pop_state()
    {
        JSONCONS_ASSERT(!state_stack_.empty())
        csv_parse_state state = state_stack_.back();
        state_stack_.pop_back();
        return state;
    }
};

using csv_parser = basic_csv_parser<char>;
using wcsv_parser = basic_csv_parser<wchar_t>;

}}

#endif


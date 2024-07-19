// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CSV_CSV_ENCODER_HPP
#define JSONCONS_CSV_CSV_ENCODER_HPP

#include <array> // std::array
#include <string>
#include <vector>
#include <ostream>
#include <utility> // std::move
#include <unordered_map> // std::unordered_map
#include <memory> // std::allocator
#include <limits> // std::numeric_limits
#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_visitor.hpp>
#include <jsoncons/detail/write_number.hpp>
#include <jsoncons_ext/csv/csv_options.hpp>
#include <jsoncons/sink.hpp>

namespace jsoncons { namespace csv {

template <typename CharT,typename Sink=jsoncons::stream_sink<CharT>,typename Allocator=std::allocator<char>>
class basic_csv_encoder final : public basic_json_visitor<CharT>
{
public:
    using char_type = CharT;
    using typename basic_json_visitor<CharT>::string_view_type;
    using sink_type = Sink;

    using allocator_type = Allocator;
    using char_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<CharT>;
    using string_type = std::basic_string<CharT, std::char_traits<CharT>, char_allocator_type>;
    using string_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<string_type>;
    using string_string_allocator_type = typename std::allocator_traits<allocator_type>:: template rebind_alloc<std::pair<const string_type,string_type>>;

private:
    static jsoncons::basic_string_view<CharT> null_constant()
    {
        static jsoncons::basic_string_view<CharT> k = JSONCONS_STRING_VIEW_CONSTANT(CharT,"null");
        return k;
    }
    static jsoncons::basic_string_view<CharT> true_constant()
    {
        static jsoncons::basic_string_view<CharT> k = JSONCONS_STRING_VIEW_CONSTANT(CharT,"true");
        return k;
    }
    static jsoncons::basic_string_view<CharT> false_constant()
    {
        static jsoncons::basic_string_view<CharT> k = JSONCONS_STRING_VIEW_CONSTANT(CharT,"false");
        return k;
    }

    enum class stack_item_kind
    {
        row_mapping,
        column_mapping,
        object,
        row,
        column,
        object_multi_valued_field,
        row_multi_valued_field,
        column_multi_valued_field
    };

    struct stack_item
    {
        stack_item_kind item_kind_;
        std::size_t count_;

        stack_item(stack_item_kind item_kind) noexcept
           : item_kind_(item_kind), count_(0)
        {
        }

        bool is_object() const
        {
            return item_kind_ == stack_item_kind::object;
        }

        stack_item_kind item_kind() const
        {
            return item_kind_;
        }
    };

    Sink sink_;
    const basic_csv_encode_options<CharT> options_;
    allocator_type alloc_;

    std::vector<stack_item> stack_;
    jsoncons::detail::write_double fp_;
    std::vector<string_type,string_allocator_type> strings_buffer_;

    std::unordered_map<string_type,string_type, std::hash<string_type>,std::equal_to<string_type>,string_string_allocator_type> buffered_line_;
    string_type name_;
    std::size_t column_index_;
    std::vector<std::size_t> row_counts_;

    // Noncopyable and nonmoveable
    basic_csv_encoder(const basic_csv_encoder&) = delete;
    basic_csv_encoder& operator=(const basic_csv_encoder&) = delete;
public:
    basic_csv_encoder(Sink&& sink, 
                      const Allocator& alloc = Allocator())
       : basic_csv_encoder(std::forward<Sink>(sink), basic_csv_encode_options<CharT>(), alloc)
    {
    }

    basic_csv_encoder(Sink&& sink,
                      const basic_csv_encode_options<CharT>& options, 
                      const Allocator& alloc = Allocator())
      : sink_(std::forward<Sink>(sink)),
        options_(options),
        alloc_(alloc),
        stack_(),
        fp_(options.float_format(), options.precision()),
        column_index_(0)
    {
        jsoncons::csv::detail::parse_column_names(options.column_names(), strings_buffer_);
    }

    ~basic_csv_encoder() noexcept
    {
        JSONCONS_TRY
        {
            sink_.flush();
        }
        JSONCONS_CATCH(...)
        {
        }
    }

    void reset()
    {
        stack_.clear();
        strings_buffer_.clear();
        buffered_line_.clear();
        name_.clear();
        column_index_ = 0;
        row_counts_.clear();
    }

    void reset(Sink&& sink)
    {
        sink_ = std::move(sink);
        reset();
    }

private:

    template <typename AnyWriter>
    void escape_string(const CharT* s,
                       std::size_t length,
                       CharT quote_char, CharT quote_escape_char,
                       AnyWriter& sink)
    {
        const CharT* begin = s;
        const CharT* end = s + length;
        for (const CharT* it = begin; it != end; ++it)
        {
            CharT c = *it;
            if (c == quote_char)
            {
                sink.push_back(quote_escape_char); 
                sink.push_back(quote_char);
            }
            else
            {
                sink.push_back(c);
            }
        }
    }

    void visit_flush() override
    {
        sink_.flush();
    }

    bool visit_begin_object(semantic_tag, const ser_context&, std::error_code& ec) override
    {
        if (stack_.empty())
        {
            stack_.emplace_back(stack_item_kind::column_mapping);
            return true;
        }
        switch (stack_.back().item_kind_)
        {
            case stack_item_kind::row_mapping:
                stack_.emplace_back(stack_item_kind::object);
                return true;
            default: // error
                ec = csv_errc::source_error;
                return false;
        }
    }

    bool visit_end_object(const ser_context&, std::error_code&) override
    {
        JSONCONS_ASSERT(!stack_.empty());

        switch (stack_.back().item_kind_)
        {
            case stack_item_kind::object:
                if (stack_[0].count_ == 0)
                {
                    for (std::size_t i = 0; i < strings_buffer_.size(); ++i)
                    {
                        if (i > 0)
                        {
                            sink_.push_back(options_.field_delimiter());
                        }
                        sink_.append(strings_buffer_[i].data(),
                                      strings_buffer_[i].length());
                    }
                    sink_.append(options_.line_delimiter().data(),
                                  options_.line_delimiter().length());
                }
                for (std::size_t i = 0; i < strings_buffer_.size(); ++i)
                {
                    if (i > 0)
                    {
                        sink_.push_back(options_.field_delimiter());
                    }
                    auto it = buffered_line_.find(strings_buffer_[i]);
                    if (it != buffered_line_.end())
                    {
                        sink_.append(it->second.data(),it->second.length());
                        it->second.clear();
                    }
                }
                sink_.append(options_.line_delimiter().data(), options_.line_delimiter().length());
                break;
            case stack_item_kind::column_mapping:
             {
                 for (const auto& item : strings_buffer_)
                 {
                     sink_.append(item.data(), item.size());
                     sink_.append(options_.line_delimiter().data(), options_.line_delimiter().length());
                 }
                 break;
             }
            default:
                break;
        }
        stack_.pop_back();
        if (!stack_.empty())
        {
            end_value();
        }
        return true;
    }

    bool visit_begin_array(semantic_tag, const ser_context&, std::error_code& ec) override
    {
        if (stack_.empty())
        {
            stack_.emplace_back(stack_item_kind::row_mapping);
            return true;
        }
        switch (stack_.back().item_kind_)
        {
            case stack_item_kind::row_mapping:
                stack_.emplace_back(stack_item_kind::row);
                if (stack_[0].count_ == 0)
                {
                    for (std::size_t i = 0; i < strings_buffer_.size(); ++i)
                    {
                        if (i > 0)
                        {
                            sink_.push_back(options_.field_delimiter());
                        }
                        sink_.append(strings_buffer_[i].data(),strings_buffer_[i].length());
                    }
                    if (strings_buffer_.size() > 0)
                    {
                        sink_.append(options_.line_delimiter().data(),
                                      options_.line_delimiter().length());
                    }
                }
                return true;
            case stack_item_kind::object:
                stack_.emplace_back(stack_item_kind::object_multi_valued_field);
                return true;
            case stack_item_kind::column_mapping:
                stack_.emplace_back(stack_item_kind::column);
                row_counts_.push_back(1);
                if (strings_buffer_.size() <= row_counts_.back())
                {
                    strings_buffer_.emplace_back();
                }
                return true;
            case stack_item_kind::column:
            {
                if (strings_buffer_.size() <= row_counts_.back())
                {
                    strings_buffer_.emplace_back();
                }                
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                begin_value(bo);
                stack_.emplace_back(stack_item_kind::column_multi_valued_field);
                return true;
            }
            case stack_item_kind::row:
                begin_value(sink_);
                stack_.emplace_back(stack_item_kind::row_multi_valued_field);
                return true;
            default: // error
                ec = csv_errc::source_error;
                return false;
        }
    }

    bool visit_end_array(const ser_context&, std::error_code&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        switch (stack_.back().item_kind_)
        {
            case stack_item_kind::row:
                sink_.append(options_.line_delimiter().data(),
                              options_.line_delimiter().length());
                break;
            case stack_item_kind::column:
                ++column_index_;
                break;
            default:
                break;
        }
        stack_.pop_back();

        if (!stack_.empty())
        {
            end_value();
        }
        return true;
    }

    bool visit_key(const string_view_type& name, const ser_context&, std::error_code&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        switch (stack_.back().item_kind_)
        {
            case stack_item_kind::object:
            {
                name_ = string_type(name);
                buffered_line_[string_type(name)] = std::basic_string<CharT>();
                if (stack_[0].count_ == 0 && options_.column_names().size() == 0)
                {
                    strings_buffer_.emplace_back(name);
                }
                break;
            }
            case stack_item_kind::column_mapping:
            {
                if (strings_buffer_.empty())
                {
                    strings_buffer_.emplace_back(name);
                }
                else
                {
                    strings_buffer_[0].push_back(options_.field_delimiter());
                    strings_buffer_[0].append(string_type(name));
                }
                break;
            }
            default:
                break;
        }
        return true;
    }

    bool visit_null(semantic_tag, const ser_context&, std::error_code&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        switch (stack_.back().item_kind_)
        {
            case stack_item_kind::object:
            case stack_item_kind::object_multi_valued_field:
            {
                auto it = buffered_line_.find(name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::string_sink<std::basic_string<CharT>> bo(s);
                    write_null_value(bo);
                    bo.flush();
                    if (!it->second.empty() && options_.subfield_delimiter() != char_type())
                    {
                        it->second.push_back(options_.subfield_delimiter());
                    }
                    it->second.append(s);
                }
                break;
            }
            case stack_item_kind::row:
            case stack_item_kind::row_multi_valued_field:
                write_null_value(sink_);
                break;
            case stack_item_kind::column:
            {
                if (strings_buffer_.size() <= row_counts_.back())
                {
                    strings_buffer_.emplace_back();
                }
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                write_null_value(bo);
                break;
            }
            case stack_item_kind::column_multi_valued_field:
            {
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                write_null_value(bo);
                break;
            }
            default:
                break;
        }
        return true;
    }

    bool visit_string(const string_view_type& sv, semantic_tag, const ser_context&, std::error_code&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        switch (stack_.back().item_kind_)
        {
            case stack_item_kind::object:
            case stack_item_kind::object_multi_valued_field:
            {
                auto it = buffered_line_.find(name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::string_sink<std::basic_string<CharT>> bo(s);
                    write_string_value(sv,bo);
                    bo.flush();
                    if (!it->second.empty() && options_.subfield_delimiter() != char_type())
                    {
                        it->second.push_back(options_.subfield_delimiter());
                    }
                    it->second.append(s);
                }
                break;
            }
            case stack_item_kind::row:
            case stack_item_kind::row_multi_valued_field:
                write_string_value(sv,sink_);
                break;
            case stack_item_kind::column:
            {
                if (strings_buffer_.size() <= row_counts_.back())
                {
                    strings_buffer_.emplace_back();
                }
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                write_string_value(sv,bo);
                break;
            }
            case stack_item_kind::column_multi_valued_field:
            {
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                write_string_value(sv,bo);
                break;
            }
            default:
                break;
        }
        return true;
    }

    bool visit_byte_string(const byte_string_view& b, 
                              semantic_tag tag, 
                              const ser_context& context,
                              std::error_code& ec) override
    {
        byte_string_chars_format encoding_hint;
        switch (tag)
        {
            case semantic_tag::base16:
                encoding_hint = byte_string_chars_format::base16;
                break;
            case semantic_tag::base64:
                encoding_hint = byte_string_chars_format::base64;
                break;
            case semantic_tag::base64url:
                encoding_hint = byte_string_chars_format::base64url;
                break;
            default:
                encoding_hint = byte_string_chars_format::none;
                break;
        }
        byte_string_chars_format format = jsoncons::detail::resolve_byte_string_chars_format(encoding_hint,byte_string_chars_format::none,byte_string_chars_format::base64url);

        std::basic_string<CharT> s;
        switch (format)
        {
            case byte_string_chars_format::base16:
            {
                encode_base16(b.begin(),b.end(),s);
                visit_string(s, semantic_tag::none, context, ec);
                break;
            }
            case byte_string_chars_format::base64:
            {
                encode_base64(b.begin(),b.end(),s);
                visit_string(s, semantic_tag::none, context, ec);
                break;
            }
            case byte_string_chars_format::base64url:
            {
                encode_base64url(b.begin(),b.end(),s);
                visit_string(s, semantic_tag::none, context, ec);
                break;
            }
            default:
            {
                JSONCONS_UNREACHABLE();
            }
        }

        return true;
    }

    bool visit_double(double val, 
                         semantic_tag, 
                         const ser_context& context,
                         std::error_code& ec) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        switch (stack_.back().item_kind_)
        {
            case stack_item_kind::object:
            case stack_item_kind::object_multi_valued_field:
            {
                auto it = buffered_line_.find(name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::string_sink<std::basic_string<CharT>> bo(s);
                    write_double_value(val, context, bo, ec);
                    bo.flush();
                    if (!it->second.empty() && options_.subfield_delimiter() != char_type())
                    {
                        it->second.push_back(options_.subfield_delimiter());
                    }
                    it->second.append(s);
                }
                break;
            }
            case stack_item_kind::row:
            case stack_item_kind::row_multi_valued_field:
                write_double_value(val, context, sink_, ec);
                break;
            case stack_item_kind::column:
            {
                if (strings_buffer_.size() <= row_counts_.back())
                {
                    strings_buffer_.emplace_back();
                }
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                write_double_value(val, context, bo, ec);
                break;
            }
            case stack_item_kind::column_multi_valued_field:
            {
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                write_double_value(val, context, bo, ec);
                break;
            }
            default:
                break;
        }
        return true;
    }

    bool visit_int64(int64_t val, 
                        semantic_tag, 
                        const ser_context&,
                        std::error_code&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        switch (stack_.back().item_kind_)
        {
            case stack_item_kind::object:
            case stack_item_kind::object_multi_valued_field:
            {
                auto it = buffered_line_.find(name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::string_sink<std::basic_string<CharT>> bo(s);
                    write_int64_value(val,bo);
                    bo.flush();
                    if (!it->second.empty() && options_.subfield_delimiter() != char_type())
                    {
                        it->second.push_back(options_.subfield_delimiter());
                    }
                    it->second.append(s);
                }
                break;
            }
            case stack_item_kind::row:
            case stack_item_kind::row_multi_valued_field:
                write_int64_value(val,sink_);
                break;
            case stack_item_kind::column:
            {
                if (strings_buffer_.size() <= row_counts_.back())
                {
                    strings_buffer_.emplace_back();
                }
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                write_int64_value(val, bo);
                break;
            }
            case stack_item_kind::column_multi_valued_field:
            {
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                write_int64_value(val, bo);
                break;
            }
            default:
                break;
        }
        return true;
    }

    bool visit_uint64(uint64_t val, 
                      semantic_tag, 
                      const ser_context&,
                      std::error_code&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        switch (stack_.back().item_kind_)
        {
            case stack_item_kind::object:
            case stack_item_kind::object_multi_valued_field:
            {
                auto it = buffered_line_.find(name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::string_sink<std::basic_string<CharT>> bo(s);
                    write_uint64_value(val, bo);
                    bo.flush();
                    if (!it->second.empty() && options_.subfield_delimiter() != char_type())
                    {
                        it->second.push_back(options_.subfield_delimiter());
                    }
                    it->second.append(s);
                }
                break;
            }
            case stack_item_kind::row:
            case stack_item_kind::row_multi_valued_field:
                write_uint64_value(val,sink_);
                break;
            case stack_item_kind::column:
            {
                if (strings_buffer_.size() <= row_counts_.back())
                {
                    strings_buffer_.emplace_back();
                }
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                write_uint64_value(val, bo);
                break;
            }
            case stack_item_kind::column_multi_valued_field:
            {
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                write_uint64_value(val, bo);
                break;
            }
            default:
                break;
        }
        return true;
    }

    bool visit_bool(bool val, semantic_tag, const ser_context&, std::error_code&) override
    {
        JSONCONS_ASSERT(!stack_.empty());
        switch (stack_.back().item_kind_)
        {
            case stack_item_kind::object:
            case stack_item_kind::object_multi_valued_field:
            {
                auto it = buffered_line_.find(name_);
                if (it != buffered_line_.end())
                {
                    std::basic_string<CharT> s;
                    jsoncons::string_sink<std::basic_string<CharT>> bo(s);
                    write_bool_value(val,bo);
                    bo.flush();
                    if (!it->second.empty() && options_.subfield_delimiter() != char_type())
                    {
                        it->second.push_back(options_.subfield_delimiter());
                    }
                    it->second.append(s);
                }
                break;
            }
            case stack_item_kind::row:
            case stack_item_kind::row_multi_valued_field:
                write_bool_value(val,sink_);
                break;
            case stack_item_kind::column:
            {
                if (strings_buffer_.size() <= row_counts_.back())
                {
                    strings_buffer_.emplace_back();
                }
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                write_bool_value(val, bo);
                break;
            }
            case stack_item_kind::column_multi_valued_field:
            {
                jsoncons::string_sink<std::basic_string<CharT>> bo(strings_buffer_[row_counts_.back()]);
                write_bool_value(val, bo);
                break;
            }
            default:
                break;
        }
        return true;
    }

    template <typename AnyWriter>
    bool do_string_value(const CharT* s, std::size_t length, AnyWriter& sink)
    {
        bool quote = false;
        if (options_.quote_style() == quote_style_kind::all || options_.quote_style() == quote_style_kind::nonnumeric ||
            (options_.quote_style() == quote_style_kind::minimal &&
            (std::char_traits<CharT>::find(s, length, options_.field_delimiter()) != nullptr || std::char_traits<CharT>::find(s, length, options_.quote_char()) != nullptr)))
        {
            quote = true;
            sink.push_back(options_.quote_char());
        }
        escape_string(s, length, options_.quote_char(), options_.quote_escape_char(), sink);
        if (quote)
        {
            sink.push_back(options_.quote_char());
        }

        return true;
    }

    template <typename AnyWriter>
    void write_string_value(const string_view_type& value, AnyWriter& sink)
    {
        begin_value(sink);
        do_string_value(value.data(),value.length(),sink);
        end_value();
    }

    template <typename AnyWriter>
    void write_double_value(double val, const ser_context& context, AnyWriter& sink, std::error_code& ec)
    {
        begin_value(sink);

        if (!std::isfinite(val))
        {
            if ((std::isnan)(val))
            {
                if (options_.enable_nan_to_num())
                {
                    sink.append(options_.nan_to_num().data(), options_.nan_to_num().length());
                }
                else if (options_.enable_nan_to_str())
                {
                    visit_string(options_.nan_to_str(), semantic_tag::none, context, ec);
                }
                else
                {
                    sink.append(null_constant().data(), null_constant().size());
                }
            }
            else if (val == std::numeric_limits<double>::infinity())
            {
                if (options_.enable_inf_to_num())
                {
                    sink.append(options_.inf_to_num().data(), options_.inf_to_num().length());
                }
                else if (options_.enable_inf_to_str())
                {
                    visit_string(options_.inf_to_str(), semantic_tag::none, context, ec);
                }
                else
                {
                    sink.append(null_constant().data(), null_constant().size());
                }
            }
            else
            {
                if (options_.enable_neginf_to_num())
                {
                    sink.append(options_.neginf_to_num().data(), options_.neginf_to_num().length());
                }
                else if (options_.enable_neginf_to_str())
                {
                    visit_string(options_.neginf_to_str(), semantic_tag::none, context, ec);
                }
                else
                {
                    sink.append(null_constant().data(), null_constant().size());
                }
            }
        }
        else
        {
            fp_(val, sink);
        }

        end_value();

    }

    template <typename AnyWriter>
    void write_int64_value(int64_t val, AnyWriter& sink)
    {
        begin_value(sink);

        jsoncons::detail::from_integer(val,sink);

        end_value();
    }

    template <typename AnyWriter>
    void write_uint64_value(uint64_t val, AnyWriter& sink)
    {
        begin_value(sink);

        jsoncons::detail::from_integer(val,sink);

        end_value();
    }

    template <typename AnyWriter>
    void write_bool_value(bool val, AnyWriter& sink) 
    {
        begin_value(sink);

        if (val)
        {
            sink.append(true_constant().data(), true_constant().size());
        }
        else
        {
            sink.append(false_constant().data(), false_constant().size());
        }

        end_value();
    }
 
    template <typename AnyWriter>
    bool write_null_value(AnyWriter& sink) 
    {
        begin_value(sink);
        sink.append(null_constant().data(), null_constant().size());
        end_value();
        return true;
    }

    template <typename AnyWriter>
    void begin_value(AnyWriter& sink)
    {
        JSONCONS_ASSERT(!stack_.empty());
        switch (stack_.back().item_kind_)
        {
            case stack_item_kind::row:
                if (stack_.back().count_ > 0)
                {
                    sink.push_back(options_.field_delimiter());
                }
                break;
            case stack_item_kind::column:
            {
                if (row_counts_.size() >= 3)
                {
                    for (std::size_t i = row_counts_.size()-2; i-- > 0;)
                    {
                        if (row_counts_[i] <= row_counts_.back())
                        {
                            sink.push_back(options_.field_delimiter());
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                if (column_index_ > 0)
                {
                    sink.push_back(options_.field_delimiter());
                }
                break;
            }
            case stack_item_kind::row_multi_valued_field:
            case stack_item_kind::column_multi_valued_field:
                if (stack_.back().count_ > 0 && options_.subfield_delimiter() != char_type())
                {
                    sink.push_back(options_.subfield_delimiter());
                }
                break;
            default:
                break;
        }
    }

    void end_value()
    {
        JSONCONS_ASSERT(!stack_.empty());
        switch(stack_.back().item_kind_)
        {
            case stack_item_kind::row:
            {
                ++stack_.back().count_;
                break;
            }
            case stack_item_kind::column:
            {
                ++row_counts_.back();
                break;
            }
            default:
                ++stack_.back().count_;
                break;
        }
    }
};

using csv_stream_encoder = basic_csv_encoder<char>;
using csv_string_encoder = basic_csv_encoder<char,jsoncons::string_sink<std::string>>;
using csv_wstream_encoder = basic_csv_encoder<wchar_t>;
using wcsv_string_encoder = basic_csv_encoder<wchar_t,jsoncons::string_sink<std::wstring>>;

}}

#endif

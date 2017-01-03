// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSONQUERY_HPP
#define JSONCONS_JSONPATH_JSONQUERY_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <jsoncons/json.hpp>
#include "jsonpath_filter.hpp"
#include "jsonpath_error_category.hpp"

namespace jsoncons { namespace jsonpath {

template<class CharT>
bool try_string_to_index(const CharT *s, size_t length, size_t* value, bool* positive)
{
    static const size_t max_value = (std::numeric_limits<size_t>::max)();
    static const size_t max_value_div_10 = max_value / 10;

    size_t start = 0;
    size_t n = 0;
    if (length > 0)
    {
        if (s[start] == '-')
        {
            *positive = false;
            ++start;
        }
        else
        {
            *positive = true;
        }
    }
    if (length > start)
    {
        for (size_t i = start; i < length; ++i)
        {
            CharT c = s[i];
            switch (c)
            {
            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                {
                    size_t x = c - '0';
                    if (n > max_value_div_10)
                    {
                        return false;
                    }
                    n = n * 10;
                    if (n > max_value - x)
                    {
                        return false;
                    }

                    n += x;
                }
                break;
            default:
                return false;
                break;
            }
        }
        *value = n;
        return true;
    }
    else
    {
        return false;
    }
}

template<class Json>
Json json_query(const Json& root, typename Json::string_view_type path)
{
    jsonpath_evaluator<Json,const Json&,const Json*> evaluator;
    evaluator.evaluate(root,path.data(),path.length());
    return evaluator.get_values();
}

template<class Json, class T>
void json_replace(Json& root, typename Json::string_view_type path, T&& new_value)
{
    jsonpath_evaluator<Json,Json&,Json*> evaluator;
    evaluator.evaluate(root,path.data(),path.length());
    evaluator.replace(std::forward<T&&>(new_value));
}

enum class states 
{
    start,
    cr,
    lf,
    expect_dot_or_left_bracket,
    expect_unquoted_name_or_left_bracket,
    unquoted_name,
    left_bracket_single_quoted_string,
    left_bracket_double_quoted_string,
    left_bracket,
    left_bracket_start,
    left_bracket_end,
    left_bracket_end2,
    left_bracket_step,
    left_bracket_step2,
    expect_comma_or_right_bracket,
    dot
};

template<class Json,
         class JsonReference=const Json&,
         class JsonPointer=const Json*>
class jsonpath_evaluator : private basic_parsing_context<typename Json::char_type>
{
private:
    typedef typename Json::char_type char_type;
    typedef typename Json::char_traits_type char_traits_type;
    typedef std::basic_string<char_type,char_traits_type> string_type;
    typedef typename Json::key_storage_type key_storage_type;
    typedef typename Json::string_view_type string_view_type;
    typedef JsonReference json_reference;
    typedef JsonPointer json_pointer;
    typedef std::vector<json_pointer> node_set;

    static string_view_type length_literal() 
    {
        static const char_type data[] = {'l','e','n','g','t','h'};
        return string_view_type{data,sizeof(data)/sizeof(char_type)};
    }

    class selector
    {
    public:
        virtual ~selector()
        {
        }
        virtual void select(json_reference context, std::vector<json_pointer>& nodes, std::vector<std::shared_ptr<Json>>& temp_json_values) = 0;
    };

    class expr_selector : public selector
    {
    private:
         jsonpath_filter_expr<Json> result_;
    public:
        expr_selector(const jsonpath_filter_expr<Json>& result)
            : result_(result)
        {
        }

        void select(json_reference context, std::vector<json_pointer>& nodes, std::vector<std::shared_ptr<Json>>& temp_json_values) override
        {
            auto index = result_.eval(context);
            if (index.template is<size_t>())
            {
                size_t start = index. template as<size_t>();
                if (context.is_array() && start < context.size())
                {
                    nodes.push_back(std::addressof(context[start]));
                }
            }
            else if (index.is_string())
            {
                name_selector selector(index.as_string_view(),true);
                selector.select(context, nodes, temp_json_values);
            }
        }
    };

    class filter_selector : public selector
    {
    private:
         jsonpath_filter_expr<Json> result_;
    public:
        filter_selector(const jsonpath_filter_expr<Json>& result)
            : result_(result)
        {
        }

        void select(json_reference context, std::vector<json_pointer>& nodes, std::vector<std::shared_ptr<Json>>&) override
        {
            if (context.is_array())
            {
                for (json_reference element : context.array_range())
                {
                    if (result_.exists(element))
                    {
                        nodes.push_back(std::addressof(element));
                    }
                }
            }
        }
    };

    class name_selector : public selector
    {
    private:
        string_type name_;
        bool positive_start_;
    public:
        name_selector(string_view_type name, bool positive_start)
            : name_(name), positive_start_(positive_start)
        {
        }

        void select(json_reference context,
            std::vector<json_pointer>& nodes,
            std::vector<std::shared_ptr<Json>>& temp_json_values) override
        {
            if (context.is_object() && context.count(name_) > 0)
            {
                nodes.push_back(std::addressof(context.at(name_)));
            }
            else if (context.is_array())
            {
                size_t pos = 0;
                if (try_string_to_index(name_.data(), name_.size(), &pos, &positive_start_))
                {
                    size_t index = positive_start_ ? pos : context.size() - pos;
                    if (index < context.size())
                    {
                        nodes.push_back(std::addressof(context[index]));
                    }
                }
            }
            else if (context.is_string())
            {
                size_t pos = 0;
                string_view_type s = context.as_string_view();
                if (try_string_to_index(name_.data(), name_.size(), &pos, &positive_start_))
                {
                    size_t index = positive_start_ ? pos : s.size() - pos;
                    auto sequence = json_text_traits<char_type>::sequence_at(s.data(), s.data() + s.size(), index);
                    if (sequence.second > 0)
                    {
                        auto temp = std::make_shared<Json>(sequence.first,sequence.second);
                        temp_json_values.push_back(temp);
                        nodes.push_back(temp.get());
                    }
                }
            }
        }
    };

    class array_slice_selector : public selector
    {
    private:
        size_t start_;
        bool positive_start_;
        size_t end_;
        bool positive_end_;
        bool undefined_end_;
        size_t step_;
        bool positive_step_;
    public:
        array_slice_selector(size_t start, bool positive_start, 
                             size_t end, bool positive_end,
                             size_t step, bool positive_step,
                             bool undefined_end)
            : start_(start), positive_start_(positive_start),
              end_(end), positive_end_(positive_end),undefined_end_(undefined_end),
              step_(step), positive_step_(positive_step) 
        {
        }

        void select(json_reference context,
            std::vector<json_pointer>& nodes,
            std::vector<std::shared_ptr<Json>>&) override
        {
            if (positive_step_)
            {
                end_array_slice1(context,nodes);
            }
            else
            {
                end_array_slice2(context,nodes);
            }
        }

        void end_array_slice1(json_reference context,
                              std::vector<json_pointer>& nodes)
        {
            if (context.is_array())
            {
                size_t start = positive_start_ ? start_ : context.size() - start_;
                size_t end;
                if (!undefined_end_)
                {
                    end = positive_end_ ? end_ : context.size() - end_;
                }
                else
                {
                    end = context.size();
                }
                for (size_t j = start; j < end; j += step_)
                {
                    if (j < context.size())
                    {
                        nodes.push_back(std::addressof(context[j]));
                    }
                }
            }
        }

        void end_array_slice2(json_reference context,
                              std::vector<json_pointer>& nodes)
        {
            if (context.is_array())
            {
                size_t start = positive_start_ ? start_ : context.size() - start_;
                size_t end;
                if (!undefined_end_)
                {
                    end = positive_end_ ? end_ : context.size() - end_;
                }
                else
                {
                    end = context.size();
                }

                size_t j = end + step_ - 1;
                while (j > (start+step_-1))
                {
                    j -= step_;
                    if (j < context.size())
                    {
                        nodes.push_back(std::addressof(context[j]));
                    }
                }
            }
        }
    };

    basic_default_parse_error_handler<char_type> default_err_handler_;
    basic_parse_error_handler<char_type> *err_handler_;
    states state_;
    string_type buffer_;
    size_t start_;
    bool positive_start_;
    size_t end_;
    bool positive_end_;
    bool undefined_end_;
    size_t step_;
    bool positive_step_;
    std::vector<node_set> stack_;
    bool recursive_descent_;
    std::vector<json_pointer> nodes_;
    std::vector<std::shared_ptr<Json>> temp_json_values_;
    size_t line_;
    size_t column_;
    const char_type* begin_input_;
    const char_type* end_input_;
    const char_type* p_;
    std::vector<std::shared_ptr<selector>> selectors_;

public:
    jsonpath_evaluator()
        : err_handler_(&default_err_handler_),
          state_(states::start),
          start_(0), positive_start_(true), 
          end_(0), positive_end_(true), undefined_end_(false),
          step_(0), positive_step_(true),
          recursive_descent_(false),
          line_(0), column_(0),
          begin_input_(nullptr), end_input_(nullptr),
          p_(nullptr)
    {
    }

    Json get_values() const
    {
        Json result = Json::make_array();

        if (stack_.size() > 0)
        {
            for (size_t i = 0; i < stack_.back().size(); ++i)
            {
                json_pointer p = stack_.back()[i];
                result.add(*p);
            }
        }
        return result;
    }

    template <class T>
    void replace(T&& new_value)
    {
        if (stack_.size() > 0)
        {
            for (size_t i = 0; i < stack_.back().size(); ++i)
            {
                *(stack_.back()[i]) = new_value;
            }
        }
    }

    void evaluate(json_reference root, string_view_type path)
    {
        evaluate(root,path.data(),path.length());
    }
    void evaluate(json_reference root, const char_type* path)
    {
        evaluate(root,path,char_traits_type::length(path));
    }

    void evaluate(json_reference root, const char_type* path, size_t length)
    {
        states pre_line_break_state = states::start;

        begin_input_ = path;
        end_input_ = path + length;
        p_ = begin_input_;

        line_ = 1;
        column_ = 1;
        state_ = states::start;

        recursive_descent_ = false;

        clear_index();

        while (p_ < end_input_)
        {
            switch (state_)
            {
            case states::cr:
                ++line_;
                column_ = 1;
                switch (*p_)
                {
                case '\n':
                    state_ = pre_line_break_state;
                    ++p_;
                    ++column_;
                    break;
                default:
                    state_ = pre_line_break_state;
                    break;
                }
                break;
            case states::lf:
                ++line_;
                column_ = 1;
                state_ = pre_line_break_state;
                break;
            case states::start: 
                switch (*p_)
                {
                case ' ':case '\t':
                    break;
                case '$':
                case '@':
                    {
                        node_set v;
                        v.push_back(std::addressof(root));
                        stack_.push_back(v);
                        state_ = states::expect_dot_or_left_bracket;
                    }
                    break;
                default:
                    err_handler_->fatal_error(jsonpath_parser_errc::expected_root, *this);
                    break;
                };
                ++p_;
                ++column_;
                break;
            case states::dot:
                switch (*p_)
                {
                case '.':
                    recursive_descent_ = true;
                    ++p_;
                    ++column_;
                    state_ = states::expect_unquoted_name_or_left_bracket;
                    break;
                default:
                    state_ = states::expect_unquoted_name_or_left_bracket;
                    break;
                }
                break;
            case states::expect_unquoted_name_or_left_bracket:
                switch (*p_)
                {
                case '.':
                    err_handler_->fatal_error(jsonpath_parser_errc::expected_name, *this);
                    ++p_;
                    ++column_;
                    break;
                case '*':
                    end_all();
                    transfer_nodes();
                    state_ = states::expect_dot_or_left_bracket;
                    ++p_;
                    ++column_;
                    break;
                case '[':
                    state_ = states::left_bracket;
                    ++p_;
                    ++column_;
                    break;
                default:
                    buffer_.clear();
                    state_ = states::unquoted_name;
                    break;
                }
                break;
            case states::expect_dot_or_left_bracket: 
                switch (*p_)
                {
                case ' ':case '\t':
                    break;
                case '.':
                    state_ = states::dot;
                    break;
                case '[':
                    state_ = states::left_bracket;
                    break;
                default:
                    err_handler_->fatal_error(jsonpath_parser_errc::expected_separator, *this);
                    break;
                };
                ++p_;
                ++column_;
                break;
            case states::expect_comma_or_right_bracket:
                switch (*p_)
                {
                case ',':
                    state_ = states::left_bracket;
                    break;
                case ']':
                    apply_selectors();
                    transfer_nodes();
                    state_ = states::expect_dot_or_left_bracket;
                    break;
                case ' ':case '\t':
                    break;
                default:
                    err_handler_->fatal_error(jsonpath_parser_errc::expected_right_bracket, *this);
                    break;
                }
                ++p_;
                ++column_;
                break;
            case states::left_bracket:
                switch (*p_)
                {
                case ' ':case '\t':
                    ++p_;
                    ++column_;
                    break;
                case '(':
                    {
                        jsonpath_filter_parser<Json> parser(line_,column_);
                        auto result = parser.parse(p_,end_input_,&p_);
                        line_ = parser.line();
                        column_ = parser.column();
                        selectors_.push_back(std::make_shared<expr_selector>(result));
                        state_ = states::expect_comma_or_right_bracket;
                    }
                    break;
                case '?':
                    {
                        jsonpath_filter_parser<Json> parser(line_,column_);
                        auto result = parser.parse(p_,end_input_,&p_);
                        line_ = parser.line();
                        column_ = parser.column();
                        selectors_.push_back(std::make_shared<filter_selector>(result));
                        state_ = states::expect_comma_or_right_bracket;
                    }
                    break;                   
                case ':':
                    clear_index();
                    state_ = states::left_bracket_end;
                    ++p_;
                    ++column_;
                    break;
                case '*':
                    end_all();
                    state_ = states::expect_comma_or_right_bracket;
                    ++p_;
                    ++column_;
                    break;
                case '\'':
                    state_ = states::left_bracket_single_quoted_string;
                    ++p_;
                    ++column_;
                    break;
                case '\"':
                    state_ = states::left_bracket_double_quoted_string;
                    ++p_;
                    ++column_;
                    break;
                default:
                    clear_index();
                    buffer_.push_back(*p_);
                    state_ = states::left_bracket_start;
                    ++p_;
                    ++column_;
                    break;
                }
                break;
            case states::left_bracket_start:
                switch (*p_)
                {
                case ':':
                    if (!try_string_to_index(buffer_.data(), buffer_.size(), &start_, &positive_start_))
                    {
                        err_handler_->fatal_error(jsonpath_parser_errc::expected_index, *this);
                    }
                    state_ = states::left_bracket_end;
                    break;
                case ',':
                    selectors_.push_back(std::make_shared<name_selector>(buffer_,positive_start_));
                    buffer_.clear();
                    state_ = states::left_bracket;
                    break;
                case ']':
                    selectors_.push_back(std::make_shared<name_selector>(buffer_,positive_start_));
                    buffer_.clear();
                    apply_selectors();
                    transfer_nodes();
                    state_ = states::expect_dot_or_left_bracket;
                    break;
                default:
                    buffer_.push_back(*p_);
                    break;
                }
                ++p_;
                ++column_;
                break;
            case states::left_bracket_end:
                switch (*p_)
                {
                case '-':
                    positive_end_ = false;
                    state_ = states::left_bracket_end2;
                    break;
                case ':':
                    step_ = 0;
                    state_ = states::left_bracket_step;
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    undefined_end_ = false;
                    end_ = static_cast<size_t>(*p_-'0');
                    state_ = states::left_bracket_end2;
                    break;
                case ',':
                    selectors_.push_back(std::make_shared<array_slice_selector>(start_,positive_start_,end_,positive_end_,step_,positive_step_,undefined_end_));
                    state_ = states::left_bracket;
                    break;
                case ']':
                    selectors_.push_back(std::make_shared<array_slice_selector>(start_,positive_start_,end_,positive_end_,step_,positive_step_,undefined_end_));
                    apply_selectors();
                    transfer_nodes();
                    state_ = states::expect_dot_or_left_bracket;
                    break;
                }
                ++p_;
                ++column_;
                break;
            case states::left_bracket_end2:
                switch (*p_)
                {
                case ':':
                    step_ = 0;
                    state_ = states::left_bracket_step;
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    undefined_end_ = false;
                    end_ = end_*10 + static_cast<size_t>(*p_-'0');
                    break;
                case ',':
                    selectors_.push_back(std::make_shared<array_slice_selector>(start_,positive_start_,end_,positive_end_,step_,positive_step_,undefined_end_));
                    state_ = states::left_bracket;
                    break;
                case ']':
                    selectors_.push_back(std::make_shared<array_slice_selector>(start_,positive_start_,end_,positive_end_,step_,positive_step_,undefined_end_));
                    apply_selectors();
                    transfer_nodes();
                    state_ = states::expect_dot_or_left_bracket;
                    break;
                }
                ++p_;
                ++column_;
                break;
            case states::left_bracket_step:
                switch (*p_)
                {
                case '-':
                    positive_step_ = false;
                    state_ = states::left_bracket_step2;
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    step_ = static_cast<size_t>(*p_-'0');
                    state_ = states::left_bracket_step2;
                    break;
                case ',':
                    selectors_.push_back(std::make_shared<array_slice_selector>(start_,positive_start_,end_,positive_end_,step_,positive_step_,undefined_end_));
                    state_ = states::left_bracket;
                    break;
                case ']':
                    selectors_.push_back(std::make_shared<array_slice_selector>(start_,positive_start_,end_,positive_end_,step_,positive_step_,undefined_end_));
                    apply_selectors();
                    transfer_nodes();
                    state_ = states::expect_dot_or_left_bracket;
                    break;
                }
                ++p_;
                ++column_;
                break;
            case states::left_bracket_step2:
                switch (*p_)
                {
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    step_ = step_*10 + static_cast<size_t>(*p_-'0');
                    break;
                case ',':
                    selectors_.push_back(std::make_shared<array_slice_selector>(start_,positive_start_,end_,positive_end_,step_,positive_step_,undefined_end_));
                    state_ = states::left_bracket;
                    break;
                case ']':
                    selectors_.push_back(std::make_shared<array_slice_selector>(start_,positive_start_,end_,positive_end_,step_,positive_step_,undefined_end_));
                    apply_selectors();
                    transfer_nodes();
                    state_ = states::expect_dot_or_left_bracket;
                    break;
                }
                ++p_;
                ++column_;
                break;
            case states::unquoted_name: 
                switch (*p_)
                {
                case '[':
                    apply_unquoted_string(buffer_);
                    transfer_nodes();
                    start_ = 0;
                    state_ = states::left_bracket;
                    break;
                case '.':
                    apply_unquoted_string(buffer_);
                    transfer_nodes();
                    state_ = states::dot;
                    break;
                case ' ':case '\t':
                    apply_unquoted_string(buffer_);
                    transfer_nodes();
                    state_ = states::expect_dot_or_left_bracket;
                    break;
                case '\r':
                    apply_unquoted_string(buffer_);
                    transfer_nodes();
                    pre_line_break_state = states::expect_dot_or_left_bracket;
                    state_= states::cr;
                    break;
                case '\n':
                    apply_unquoted_string(buffer_);
                    transfer_nodes();
                    pre_line_break_state = states::expect_dot_or_left_bracket;
                    state_= states::lf;
                    break;
                default:
                    buffer_.push_back(*p_);
                    break;
                };
                ++p_;
                ++column_;
                break;
            case states::left_bracket_single_quoted_string: 
                switch (*p_)
                {
                case '\'':
                    selectors_.push_back(std::make_shared<name_selector>(buffer_,positive_start_));
                    buffer_.clear();
                    state_ = states::expect_comma_or_right_bracket;
                    break;
                case '\\':
                    buffer_.push_back(*p_);
                    if (p_+1 < end_input_)
                    {
                        ++p_;
                        ++column_;
                        buffer_.push_back(*p_);
                    }
                    break;
                default:
                    buffer_.push_back(*p_);
                    break;
                };
                ++p_;
                ++column_;
                break;
            case states::left_bracket_double_quoted_string: 
                switch (*p_)
                {
                case '\"':
                    selectors_.push_back(std::make_shared<name_selector>(buffer_,positive_start_));
                    buffer_.clear();
                    state_ = states::expect_comma_or_right_bracket;
                    break;
                case '\\':
                    buffer_.push_back(*p_);
                    if (p_+1 < end_input_)
                    {
                        ++p_;
                        ++column_;
                        buffer_.push_back(*p_);
                    }
                    break;
                default:
                    buffer_.push_back(*p_);
                    break;
                };
                ++p_;
                ++column_;
                break;
            default:
                ++p_;
                ++column_;
                break;
            }
        }
        switch (state_)
        {
        case states::unquoted_name: 
            {
                apply_unquoted_string(buffer_);
                transfer_nodes();
            }
            break;
        default:
            break;
        }
    }

    void clear_index()
    {
        buffer_.clear();
        start_ = 0;
        positive_start_ = true;
        end_ = 0;
        positive_end_ = true;
        undefined_end_ = true;
        step_ = 1;
        positive_step_ = true;
    }

    void end_all()
    {
        for (size_t i = 0; i < stack_.back().size(); ++i)
        {
            json_pointer p = stack_.back()[i];
            if (p->is_array())
            {
                for (auto it = p->array_range().begin(); it != p->array_range().end(); ++it)
                {
                    nodes_.push_back(std::addressof(*it));
                }
            }
            else if (p->is_object())
            {
                for (auto it = p->object_range().begin(); it != p->object_range().end(); ++it)
                {
                    nodes_.push_back(std::addressof(it->value()));
                }
            }

        }
        start_ = 0;
    }

    void apply_unquoted_string(string_view_type name)
    {
        if (name.length() > 0)
        {
            for (size_t i = 0; i < stack_.back().size(); ++i)
            {
                apply_unquoted_string(*(stack_.back()[i]), name);
            }
        }
        buffer_.clear();
    }

    void apply_unquoted_string(json_reference context, string_view_type name)
    {
        if (context.is_object())
        {
            if (context.count(name) > 0)
            {
                nodes_.push_back(std::addressof(context.at(name)));
            }
            if (recursive_descent_)
            {
                for (auto it = context.object_range().begin(); it != context.object_range().end(); ++it)
                {
                    if (it->value().is_object() || it->value().is_array())
                    {
                        apply_unquoted_string(it->value(), name);
                    }
                }
            }
        }
        else if (context.is_array())
        {
            size_t pos = 0;
            if (try_string_to_index(name.data(),name.size(),&pos, &positive_start_))
            {
                size_t index = positive_start_ ? pos : context.size() - pos;
                if (index < context.size())
                {
                    nodes_.push_back(std::addressof(context[index]));
                }
            }
            else if (name == length_literal() && context.size() > 0)
            {
                auto temp = std::make_shared<Json>(context.size());
                temp_json_values_.push_back(temp);
                nodes_.push_back(temp.get());
            }
            if (recursive_descent_)
            {
                for (auto it = context.array_range().begin(); it != context.array_range().end(); ++it)
                {
                    if (it->is_object() || it->is_array())
                    {
                        apply_unquoted_string(*it, name);
                    }
                }
            }
        }
        else if (context.is_string())
        {
            string_view_type s = context.as_string_view();
            size_t pos = 0;
            if (try_string_to_index(name.data(),name.size(),&pos, &positive_start_))
            {
                auto sequence = json_text_traits<char_type>::sequence_at(s.data(), s.data() + s.size(), pos);
                if (sequence.second > 0)
                {
                    auto temp = std::make_shared<Json>(sequence.first,sequence.second);
                    temp_json_values_.push_back(temp);
                    nodes_.push_back(temp.get());
                }
            }
            else if (name == length_literal() && s.size() > 0)
            {
                size_t count = json_text_traits<char_type>::codepoint_count(s.data(),s.data()+s.size());
                auto temp = std::make_shared<Json>(count);
                temp_json_values_.push_back(temp);
                nodes_.push_back(temp.get());
            }
        }
    }

    void apply_selectors()
    {
        if (selectors_.size() > 0)
        {
            for (size_t i = 0; i < stack_.back().size(); ++i)
            {
                apply_selectors(*(stack_.back()[i]));
            }
            selectors_.clear();
        }
    }

    void apply_selectors(json_reference context)
    {
        for (const auto& selector : selectors_)
        {
            selector->select(context,nodes_,temp_json_values_);
        }
        if (recursive_descent_)
        {
            if (context.is_object())
            {
                for (auto it = context.object_range().begin(); it != context.object_range().end(); ++it)
                {
                    if (it->value().is_object() || it->value().is_array())
                    {
                        apply_selectors(it->value());
                    }
                }
            }
            else if (context.is_array())
            {
                for (auto it = context.array_range().begin(); it != context.array_range().end(); ++it)
                {
                    if (it->is_object() || it->is_array())
                    {
                        apply_selectors(*it);
                    }
                }
            }
        }
    }

    void transfer_nodes()
    {
        stack_.push_back(nodes_);
        nodes_.clear();
        recursive_descent_ = false;
    }

    size_t do_line_number() const override
    {
        return line_;
    }

    size_t do_column_number() const override
    {
        return column_;
    }

    char_type do_current_char() const override
    {
        return 0; //p_ < end_input_? *p_ : 0;
    }

};

}}

#endif

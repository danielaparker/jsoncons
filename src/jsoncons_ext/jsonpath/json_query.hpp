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
#include "jsoncons/json.hpp"
#include "jsonpath_filter.hpp"
#include "jsonpath_error_category.hpp"

namespace jsoncons { namespace jsonpath {

    template<typename CharT>
    bool try_string_to_index(const CharT *s, size_t length, size_t* value)
    {
        static const size_t max_value = std::numeric_limits<size_t>::max JSONCONS_NO_MACRO_EXP();
        static const size_t max_value_div_10 = max_value / 10;

        size_t n = 0;
        for (size_t i = 0; i < length; ++i)
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

    template <typename CharT>
    struct json_jsonpath_traits
    {
    };

    template <>
    struct json_jsonpath_traits<char>
    {
        static const std::string length_literal() {return "length";};
    };

    template <>
    struct json_jsonpath_traits<wchar_t> // assume utf16
    {
        static const std::wstring length_literal() {return L"length";};
    };

// here

template<class JsonT>
JsonT json_query(const JsonT& root, const typename JsonT::char_type* path, size_t length)
{
    jsonpath_evaluator<JsonT> evaluator;
    evaluator.evaluate(root,path,length);
    return evaluator.get_values();
}

template<class JsonT>
JsonT json_query(const JsonT& root, const typename JsonT::string_type& path)
{
    return json_query(root,path.data(),path.length());
}

template<class JsonT>
JsonT json_query(const JsonT& root, const typename JsonT::char_type* path)
{
    return json_query(root,path,std::char_traits<typename JsonT::char_type>::length(path));
}

enum class states 
{
    start,
    cr,
    lf,
    expect_separator,
    expect_unquoted_name,
    unquoted_name,
    single_quoted_name,
    double_quoted_name,
    left_bracket,
    left_bracket_start,
    left_bracket_end,
    left_bracket_end2,
    left_bracket_step,
    left_bracket_step2,
    expect_right_bracket,
    dot
};

template<class JsonT>
class jsonpath_evaluator : private basic_parsing_context<typename JsonT::char_type>
{
private:
    typedef typename JsonT::char_type char_type;
    typedef typename JsonT::string_type string_type;
    typedef const JsonT* cjson_ptr;
    typedef std::vector<cjson_ptr> node_set;

    basic_parse_error_handler<char_type> *err_handler_;
    states state_;
    string_type buffer_;
    size_t start_;
    size_t end_;
    size_t step_;
    bool positive_start_;
    bool positive_end_;
    bool positive_step_;
    bool end_undefined_;
    std::vector<node_set> stack_;
    bool recursive_descent_;
    std::vector<cjson_ptr> nodes_;
    std::vector<std::shared_ptr<JsonT>> temp_;
    size_t line_;
    size_t column_;
    const char_type* begin_input_;
    const char_type* end_input_;
    const char_type* p_;
    states pre_line_break_state_;

    void transfer_nodes()
    {
        stack_.push_back(nodes_);
        nodes_.clear();
    }

public:
    jsonpath_evaluator()
        : err_handler_(std::addressof(basic_default_parse_error_handler<char_type>::instance()))
    {
    }

    JsonT get_values() const
    {
        JsonT result = JsonT::make_array();

        if (stack_.size() > 0)
        {
            for (size_t i = 0; i < stack_.back().size(); ++i)
            {
                cjson_ptr p = stack_.back()[i];
                result.add(*p);
            }
        }
        return result;
    }

    void evaluate(const JsonT& root, const string_type& path)
    {
        evaluate(root,path.data(),path.length());
    }
    void evaluate(const JsonT& root, const char_type* path)
    {
        evaluate(root,path,std::char_traits<char_type>::length(path));
    }

    void evaluate(const JsonT& root, const char_type* path, size_t length)
    {
        begin_input_ = path;
        end_input_ = path + length;
        p_ = begin_input_;

        line_ = 1;
        column_ = 1;
        state_ = states::start;
        buffer_.clear();
        start_ = 0;
        end_ = 0;
        step_ = 1;
        recursive_descent_ = false;
        positive_start_ = true;
        positive_end_ = true;
        positive_step_ = true;
        end_undefined_ = false;

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
                    state_ = pre_line_break_state_;
                    ++p_;
                    ++column_;
                    break;
                default:
                    state_ = pre_line_break_state_;
                    break;
                }
                break;
            case states::lf:
                ++line_;
                column_ = 1;
                state_ = pre_line_break_state_;
                break;
            case states::start: 
                switch (*p_)
                {
                case '\r':
                    pre_line_break_state_ = state_;
                    state_ = states::cr;
                    break;
                case '\n':
                    pre_line_break_state_ = state_;
                    state_ = states::lf;
                    break;
                case ' ':case '\t':
                    ++p_;
                    ++column_;
                    break;
                case '$':
                case '@':
                    {
                        node_set v;
                        v.push_back(std::addressof(root));
                        stack_.push_back(v);
                        state_ = states::expect_separator;
                    }
                    break;
                default:
                    err_handler_->fatal_error(std::error_code(jsonpath_parser_errc::expected_root, jsonpath_error_category()), *this);
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
                    state_ = states::expect_unquoted_name;
                    break;
                default:
                    state_ = states::expect_unquoted_name;
                    break;
                }
                break;
            case states::expect_unquoted_name:
                switch (*p_)
                {
                case '\r':
                    pre_line_break_state_ = state_;
                    state_ = states::cr;
                    break;
                case '\n':
                    pre_line_break_state_ = state_;
                    state_ = states::lf;
                    break;
                case '.':
                    err_handler_->fatal_error(std::error_code(jsonpath_parser_errc::expected_name, jsonpath_error_category()), *this);
                    ++p_;
                    ++column_;
                    break;
                case '*':
                    end_all();
                    transfer_nodes();
                    state_ = states::expect_separator;
                    ++p_;
                    ++column_;
                    break;
                default:
                    state_ = states::unquoted_name;
                    break;
                }
                break;
            case states::expect_separator: 
                switch (*p_)
                {
                case '\r':
                    pre_line_break_state_ = state_;
                    state_ = states::cr;
                    break;
                case '\n':
                    pre_line_break_state_ = state_;
                    state_ = states::lf;
                    break;
                case ' ':case '\t':
                    ++p_;
                    ++column_;
                    break;
                case '.':
                    state_ = states::dot;
                    break;
                case '[':
                    state_ = states::left_bracket;
                    break;
                default:
                    err_handler_->fatal_error(std::error_code(jsonpath_parser_errc::expected_separator, jsonpath_error_category()), *this);
                    break;
                };
                ++p_;
                ++column_;
                break;
            case states::expect_right_bracket:
                switch (*p_)
                {
                case '\r':
                    pre_line_break_state_ = state_;
                    state_ = states::cr;
                    break;
                case '\n':
                    pre_line_break_state_ = state_;
                    state_ = states::lf;
                    break;
                case ',':
                    state_ = states::left_bracket;
                    break;
                case ']':
                    transfer_nodes();
                    state_ = states::expect_separator;
                    break;
                case ' ':case '\t':
                    break;
                default:
                    err_handler_->fatal_error(std::error_code(jsonpath_parser_errc::expected_right_bracket, jsonpath_error_category()), *this);
                    break;
                }
                ++p_;
                ++column_;
                break;
            case states::left_bracket_step:
                switch (*p_)
                {
                case '\r':
                    pre_line_break_state_ = state_;
                    state_ = states::cr;
                    break;
                case '\n':
                    pre_line_break_state_ = state_;
                    state_ = states::lf;
                    break;
                case '-':
                    positive_step_ = false;
                    state_ = states::left_bracket_step2;
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    step_ = static_cast<size_t>(*p_-'0');
                    state_ = states::left_bracket_step2;
                    break;
                case ']':
                    end_array_slice();
                    transfer_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                ++p_;
                ++column_;
                break;
            case states::left_bracket_step2:
                switch (*p_)
                {
                case '\r':
                    pre_line_break_state_ = state_;
                    state_ = states::cr;
                    break;
                case '\n':
                    pre_line_break_state_ = state_;
                    state_ = states::lf;
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    step_ = step_*10 + static_cast<size_t>(*p_-'0');
                    break;
                case ']':
                    end_array_slice();
                    transfer_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                ++p_;
                ++column_;
                break;
            case states::left_bracket_end:
                switch (*p_)
                {
                case '\r':
                    pre_line_break_state_ = state_;
                    state_ = states::cr;
                    break;
                case '\n':
                    pre_line_break_state_ = state_;
                    state_ = states::lf;
                    break;
                case '-':
                    positive_end_ = false;
                    state_ = states::left_bracket_end2;
                    break;
                case ':':
                    step_ = 0;
                    state_ = states::left_bracket_step;
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    end_undefined_ = false;
                    end_ = static_cast<size_t>(*p_-'0');
                    state_ = states::left_bracket_end2;
                    break;
                case ']':
                    end_array_slice();
                    transfer_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                ++p_;
                ++column_;
                break;
            case states::left_bracket_end2:
                switch (*p_)
                {
                case '\r':
                    pre_line_break_state_ = state_;
                    state_ = states::cr;
                    break;
                case '\n':
                    pre_line_break_state_ = state_;
                    state_ = states::lf;
                    break;
                case ':':
                    step_ = 0;
                    state_ = states::left_bracket_step;
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    end_undefined_ = false;
                    end_ = end_*10 + static_cast<size_t>(*p_-'0');
                    break;
                case ']':
                    end_array_slice();
                    transfer_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                ++p_;
                ++column_;
                break;
            case states::left_bracket_start:
                switch (*p_)
                {
                case '\r':
                    pre_line_break_state_ = state_;
                    state_ = states::cr;
                    break;
                case '\n':
                    pre_line_break_state_ = state_;
                    state_ = states::lf;
                    break;
                case ':':
                    step_ = 1;
                    end_undefined_ = true;
                    state_ = states::left_bracket_end;
                    break;
                case ',':
                    find_elements();
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    start_ = start_*10 + static_cast<size_t>(*p_-'0');
                    break;
                case ']':
                    find_elements();
                    transfer_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                ++p_;
                ++column_;
                break;
            case states::left_bracket:
                switch (*p_)
                {
                case '\r':
                    pre_line_break_state_ = state_;
                    state_ = states::cr;
                    break;
                case '\n':
                    pre_line_break_state_ = state_;
                    state_ = states::lf;
                    break;
                case ' ':case '\t':
                    ++p_;
                    ++column_;
                    break;
                case '(':
                    {
                        if (stack_.back().size() == 1)
                        {
                            jsonpath_filter_parser<JsonT> parser(&p_,&line_,&column_);
                            parser.parse(p_,end_input_);
                            auto index = parser.eval(*(stack_.back()[0]));
                            if (index.template is<size_t>())
                            {
                                start_ = index. template as<size_t>();
                                find_elements();
                            }
                            else if (index.is_string())
                            {
                                find(index.as_string());
                            }
                        }
                        else
                        {
                            ++p_;
                            ++column_;
                        }
                    }
                    break;
                case '?':
                    {
                        jsonpath_filter_parser<JsonT> parser(&p_,&line_,&column_);
                        parser.parse(p_,end_input_);
                        nodes_.clear();
                        for (size_t j = 0; j < stack_.back().size(); ++j)
                        {
                            accept(*(stack_.back()[j]),parser);
                        }
                    }
                    break;
                    
                case ':':
                    step_ = 1;
                    end_undefined_ = true;
                    state_ = states::left_bracket_end;
                    ++p_;
                    ++column_;
                    break;
                case ',':
                    find_elements();
                    ++p_;
                    ++column_;
                    break;
                case '-':
                    positive_start_ = false;
                    state_ = states::left_bracket_start;
                    ++p_;
                    ++column_;
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    start_ = static_cast<size_t>(*p_-'0');
                    state_ = states::left_bracket_start;
                    ++p_;
                    ++column_;
                    break;
                case ']':
                    //find_elements();
                    transfer_nodes();
                    state_ = states::expect_separator;
                    ++p_;
                    ++column_;
                    break;
                case '*':
                    end_all();
                    //transfer_nodes();
                    state_ = states::expect_right_bracket;
                    ++p_;
                    ++column_;
                    break;
                case '\'':
                    state_ = states::single_quoted_name;
                    ++p_;
                    ++column_;
                    break;
                case '\"':
                    state_ = states::double_quoted_name;
                    ++p_;
                    ++column_;
                    break;
                default:
                    ++p_;
                    ++column_;
                    break;
                }
                break;
            case states::unquoted_name: 
                switch (*p_)
                {
                case '\r':
                    pre_line_break_state_ = state_;
                    state_ = states::cr;
                    break;
                case '\n':
                    pre_line_break_state_ = state_;
                    state_ = states::lf;
                    break;
                case '[':
                    find(buffer_);
                    buffer_.clear();
                    transfer_nodes();
                    start_ = 0;
                    state_ = states::left_bracket;
                    break;
                case '.':
                    find(buffer_);
                    buffer_.clear();
                    transfer_nodes();
                    state_ = states::dot;
                    break;
                case ' ':case '\t':
                    break;
                default:
                    buffer_.push_back(*p_);
                    break;
                };
                ++p_;
                ++column_;
                break;
            case states::single_quoted_name: 
                switch (*p_)
                {
                case '\'':
                    find(buffer_);
                    buffer_.clear();
                    state_ = states::expect_right_bracket;
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
            case states::double_quoted_name: 
                switch (*p_)
                {
                case '\"':
                    find(buffer_);
                    buffer_.clear();
                    state_ = states::expect_right_bracket;
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
                find(buffer_);
                buffer_.clear();
                transfer_nodes();
            }
            break;
        default:
            break;
        }
    }

    void accept(const JsonT& val,
                jsonpath_filter_parser<JsonT>& filter)
    {
        if (val.is_object())
        {
            if (recursive_descent_ && val.is_object())
            {
                for (auto it = val.members().begin(); it != val.members().end(); ++it)
                {
                    accept(it->value(),filter);
                }
            }
            if (filter.exists(val))
            {
                nodes_.push_back(std::addressof(val));
            }
        }
        else if (val.is_array())
        {
            for (auto it = val.elements().begin(); it != val.elements().end(); ++it)
            {
                accept(*it,filter);
            }
        }
    }

   

    void end_all()
    {
        for (size_t i = 0; i < stack_.back().size(); ++i)
        {
            cjson_ptr p = stack_.back()[i];
            if (p->is_array())
            {
                for (auto it = p->elements().begin(); it != p->elements().end(); ++it)
                {
                    nodes_.push_back(std::addressof(*it));
                }
            }
            else if (p->is_object())
            {
                for (auto it = p->members().begin(); it != p->members().end(); ++it)
                {
                    nodes_.push_back(std::addressof(it->value()));
                }
            }

        }
        start_ = 0;
    }

    void find_elements()
    {
        for (size_t i = 0; i < stack_.back().size(); ++i)
        {
            cjson_ptr p = stack_.back()[i];
            if (p->is_array() && start_ < p->size())
            {
                nodes_.push_back(std::addressof((*p)[start_]));
            }
        }
        start_ = 0;
    }

    void end_array_slice()
    {
        if (positive_step_)
        {
            end_array_slice1();
        }
        else
        {
            end_array_slice2();
        }
        start_ = 0;
        end_ = 0;
        step_ = 1;
        positive_start_ = positive_end_ = positive_step_ = true;
        end_undefined_ = true;
    }

    void end_array_slice1()
    {
        for (size_t i = 0; i < stack_.back().size(); ++i)
        {
            cjson_ptr p = stack_.back()[i];
            if (p->is_array())
            {
                size_t start = positive_start_ ? start_ : p->size() - start_;
                size_t end;
                if (!end_undefined_)
                {
                    end = positive_end_ ? end_ : p->size() - end_;
                }
                else
                {
                    end = p->size();
                }
                for (size_t j = start; j < end; j += step_)
                {
                    if (p->is_array() && j < p->size())
                    {
                        nodes_.push_back(std::addressof((*p)[j]));
                    }
                }
            }
        }
    }

    void end_array_slice2()
    {
        for (size_t i = 0; i < stack_.back().size(); ++i)
        {
            cjson_ptr p = stack_.back()[i];
            size_t start = positive_start_ ? start_ : p->size() - start_;
            size_t end;
            if (!end_undefined_)
            {
                end = positive_end_ ? end_ : p->size() - end_;
            }
            else
            {
                end = p->size();
            }

            size_t j = end + step_ - 1;
            while (j > (start+step_-1))
            {
                j -= step_;
                if (p->is_array() && j < p->size())
                {
                    nodes_.push_back(std::addressof((*p)[j]));
                }
            }
        }
    }

    void find(const string_type& name)
    {
        if (name.length() > 0)
        {
            for (size_t i = 0; i < stack_.back().size(); ++i)
            {
                find1(*(stack_.back()[i]), name);
            }
            recursive_descent_ = false;
        }
    }

    void find1(const JsonT& context_val, const string_type& name)
    {
        if (context_val.is_object())
        {
            if (context_val.count(name) > 0)
            {
                nodes_.push_back(std::addressof(context_val.at(name)));
            }
            if (recursive_descent_)
            {
                for (auto it = context_val.members().begin(); it != context_val.members().end(); ++it)
                {
                    if (it->value().is_object() || it->value().is_array())
                    {
                        find1(it->value(), name);
                    }
                }
            }
        }
        else if (context_val.is_array())
        {
            size_t index = 0;
            if (try_string_to_index(name.data(),name.size(),&index))
            {
                if (index < context_val.size())
                {
                    nodes_.push_back(std::addressof(context_val[index]));
                }
            }
            else if (name == json_jsonpath_traits<char_type>::length_literal() && context_val.size() > 0)
            {
                auto q = std::make_shared<JsonT>(context_val.size());
                temp_.push_back(q);
                nodes_.push_back(q.get());
            }
            if (recursive_descent_)
            {
                for (auto it = context_val.elements().begin(); it != context_val.elements().end(); ++it)
                {
                    if (it->is_object() || it->is_array())
                    {
                        find1(*it, name);
                    }
                }
            }
        }
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

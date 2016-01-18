// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

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

    template <typename Char>
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

template<typename Char, class Alloc>
basic_json<Char,Alloc> json_query(const basic_json<Char, Alloc>& root, const std::basic_string<Char>& path)
{
    return json_query(root,path.c_str(),path.length());
}

template<typename Char, class Alloc>
basic_json<Char,Alloc> json_query(const basic_json<Char, Alloc>& root, const Char* path)
{
    return json_query(root,path,std::char_traits<Char>::length(path));
}

template<typename Char, class Alloc>
basic_json<Char,Alloc> json_query(const basic_json<Char, Alloc>& root, const Char* path, size_t length)
{
    jsonpath_evaluator<Char,Alloc> evaluator;
    evaluator.evaluate(root,path,length);
    return evaluator.get_values();
}

namespace states {
    enum states_t {
        start,
        cr,
        lf,
        expect_separator,
        member_name,
        quoted_string,
        left_bracket,
        left_bracket_start,
        left_bracket_end,
        left_bracket_end2,
        left_bracket_step,
        left_bracket_step2,
        expect_right_bracket,
        dot
    };
};

template<typename Char, class Alloc>
class jsonpath_evaluator : private basic_parsing_context<Char>
{
private:
    typedef const basic_json<Char,Alloc>* cjson_ptr;
    typedef std::vector<cjson_ptr> node_set;

    basic_parse_error_handler<Char> *err_handler_;
    states::states_t state_;
    std::basic_string<Char> buffer_;
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
    std::vector<std::shared_ptr<basic_json<Char,Alloc>>> temp_;
    size_t line_;
    size_t column_;
    const Char* begin_input_;
    const Char* end_input_;
    const Char* p_;

    void end_nodes()
    {
        stack_.push_back(nodes_);
        nodes_.clear();
    }

public:
    jsonpath_evaluator()
        : err_handler_(std::addressof(basic_default_parse_error_handler<Char>::instance()))
    {
    }

    basic_json<Char,Alloc> get_values() const
    {
        basic_json<Char,Alloc> result = basic_json<Char,Alloc>::make_array();

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

    void evaluate(const basic_json<Char, Alloc>& root, const std::basic_string<Char>& path)
    {
        evaluate(root,path.c_str(),path.length());
    }
    void evaluate(const basic_json<Char, Alloc>& root, const Char* path)
    {
        evaluate(root,path,std::char_traits<Char>::length(path));
    }
    void evaluate(const basic_json<Char, Alloc>& root, const Char* path, size_t length)
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

        for (; p_ < end_input_; ++p_)
        {
handle_state:
            Char c = *p_;
            switch (state_)
            {
            case states::start: 
                switch (c)
                {
                case '$':
                case '@':
                    {
                        node_set v;
                        v.push_back(std::addressof(root));
                        stack_.push_back(v);
                        state_ = states::expect_separator;
                    }
                    break;
                case ' ':case '\n':case '\r':case '\t':
                    break;
                default:
                    err_handler_->fatal_error(std::error_code(jsonpath_parser_errc::expected_root, jsonpath_error_category()), *this);
                    break;
                };
                break;
            case states::dot:
                switch (c)
                {
                case '.':
                    recursive_descent_ = true;
                    break;
				case '*':
                    end_all();
                    end_nodes();
                    break;
                default:
                    state_ = states::member_name;
                    goto handle_state;
                    break;
                }
                break;
            case states::expect_separator: 
                switch (c)
                {
                case '.':
                    state_ = states::dot;
                    break;
                case '[':
                    state_ = states::left_bracket;
                    break;
                };
                break;
            case states::expect_right_bracket:
                switch (c)
                {
                case ',':
                    state_ = states::left_bracket;
                    break;
                case ']':
                    end_nodes();
                    state_ = states::expect_separator;
                    break;
                case ' ':case '\n':case '\r':case '\t':
                    break;
                default:
                    err_handler_->fatal_error(std::error_code(jsonpath_parser_errc::expected_right_bracket, jsonpath_error_category()), *this);
                    break;
                }
                break;
            case states::left_bracket_step:
                switch (c)
                {
                case '-':
                    positive_step_ = false;
                    state_ = states::left_bracket_step2;
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    step_ = static_cast<size_t>(c-'0');
                    state_ = states::left_bracket_step2;
                    break;
                case ']':
                    end_array_slice();
                    end_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                break;
            case states::left_bracket_step2:
                switch (c)
                {
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    step_ = step_*10 + static_cast<size_t>(c-'0');
                    break;
                case ']':
                    end_array_slice();
                    end_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                break;
            case states::left_bracket_end:
                switch (c)
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
                    end_undefined_ = false;
                    end_ = static_cast<size_t>(c-'0');
                    state_ = states::left_bracket_end2;
                    break;
                case ']':
                    end_array_slice();
                    end_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                break;
            case states::left_bracket_end2:
                switch (c)
                {
                case ':':
                    step_ = 0;
                    state_ = states::left_bracket_step;
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    end_undefined_ = false;
                    end_ = end_*10 + static_cast<size_t>(c-'0');
                    break;
                case ']':
                    end_array_slice();
                    end_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                break;
            case states::left_bracket_start:
                switch (c)
                {
                case ':':
                    step_ = 1;
                    end_undefined_ = true;
                    state_ = states::left_bracket_end;
                    break;
                case ',':
                    find_elements();
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    start_ = start_*10 + static_cast<size_t>(c-'0');
                    break;
                case ']':
                    find_elements();
                    end_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                break;
            case states::left_bracket:
                switch (c)
                {
				case ' ':case '\n':case '\r':case '\t':
					break;
				case '(':
                    {
                        if (stack_.back().size() == 1)
                        {
                            jsonpath_filter_parser<Char,Alloc> parser(line_,column_);
                            parser.parse(path,p_ - begin_input_,length,line_,column_);
                            //auto filter = parser.get_filter();
                            //auto index = filter->evaluate(*(stack_.back()[0]));
                            auto index = parser.eval(*(stack_.back()[0]));
                            if (index.is<size_t>())
                            {
                                start_ = index.as<size_t>();
                                find_elements();
                            }
                            else if (index.is_string())
                            {
                                find(index.as_string());
                            }
                            p_ = begin_input_ + parser.index();
                            goto handle_state;
                        }
                    }
                    break;
                case '?':
                    {
                        jsonpath_filter_parser<Char,Alloc> parser(line_,column_);
                        parser.parse(path,p_ - begin_input_,length,line_,column_);
                        nodes_.clear();
                        for (size_t j = 0; j < stack_.back().size(); ++j)
                        {
                            accept(*(stack_.back()[j]),parser);
                        }
                        p_ = begin_input_ + parser.index();
                        goto handle_state;
                    }
                    break;
                    
                case ':':
                    step_ = 1;
                    end_undefined_ = true;
                    state_ = states::left_bracket_end;
                    break;
                case ',':
                    find_elements();
                    break;
                case '-':
                    positive_start_ = false;
                    state_ = states::left_bracket_start;
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    start_ = static_cast<size_t>(c-'0');
                    state_ = states::left_bracket_start;
                    break;
                case ']':
                    //find_elements();
                    end_nodes();
                    state_ = states::expect_separator;
                    break;
                case '*':
                    end_all();
                    //end_nodes();
                    state_ = states::expect_right_bracket;
                    break;
                case '\'':
                    state_ = states::quoted_string;
                    break;
                }
                break;
            case states::member_name: 
                switch (c)
                {
                case '[':
					find(buffer_);
                    buffer_.clear();
                    end_nodes();
					start_ = 0;
                    state_ = states::left_bracket;
                    break;
                case '.':
                    find(buffer_);
                    buffer_.clear();
                    end_nodes();
                    state_ = states::dot;
                    break;
                case ' ':case '\n':case '\r':case '\t':
                    break;
                default:
                    buffer_.push_back(c);
                    break;
                };
                break;
            case states::quoted_string: 
                switch (c)
                {
                case '\'':
                    find(buffer_);
                    buffer_.clear();
                    state_ = states::expect_right_bracket;
                    break;
                default:
                    buffer_.push_back(c);
                    break;
                };
                break;
            }
        }
        switch (state_)
        {
        case states::member_name: 
            {
                find(buffer_);
                buffer_.clear();
                end_nodes();
            }
            break;
        }
    }
    void accept(const basic_json<Char,Alloc>& val,
                jsonpath_filter_parser<Char,Alloc>& filter)
    {
        if (val.is_object())
        {
            if (recursive_descent_ && val.is_object())
            {
                for (auto it = val.begin_members(); it != val.end_members(); ++it)
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
            for (auto it = val.begin_elements(); it != val.end_elements(); ++it)
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
                for (auto it = p->begin_elements(); it != p->end_elements(); ++it)
                {
                    nodes_.push_back(std::addressof(*it));
                }
            }
            else if (p->is_object())
            {
                for (auto it = p->begin_members(); it != p->end_members(); ++it)
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

    void find(const std::basic_string<Char>& name)
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

    void find1(const basic_json<Char,Alloc>& context_val, const std::basic_string<Char>& name)
    {
        if (context_val.is_object())
        {
            if (context_val.has_member(name))
            {
                nodes_.push_back(std::addressof(context_val.get(name)));
            }
            if (recursive_descent_)
            {
                for (auto it = context_val.begin_members(); it != context_val.end_members(); ++it)
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
            if (try_string_to_index(name.c_str(),name.size(),&index))
            {
                if (index < context_val.size())
                {
                    nodes_.push_back(std::addressof(context_val[index]));
                }
			}
            else if (name == json_jsonpath_traits<Char>::length_literal() && context_val.size() > 0)
            {
                auto q = std::make_shared<basic_json<Char,Alloc>>(context_val.size());
                temp_.push_back(q);
                nodes_.push_back(q.get());
            }
            if (recursive_descent_)
            {
                for (auto it = context_val.begin_elements(); it != context_val.end_elements(); ++it)
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

    Char do_current_char() const override
    {
        return 0; //p_ < end_input_? *p_ : 0;
    }

};

}}

#endif

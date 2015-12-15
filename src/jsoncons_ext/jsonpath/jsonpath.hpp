// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSONPATH_JSONPATH_HPP
#define JSONCONS_JSONPATH_JSONPATH_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json1.hpp"
#include "jsoncons/json_input_handler.hpp"
#include "jsoncons/json_structures.hpp"
#include "jsonpath_filter.hpp"

namespace jsoncons { namespace jsonpath {

namespace states {
    enum states_t {
        start,
        expect_separator,
        string,
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
std::vector<basic_json<Char,Alloc>> json_query(const basic_json<Char, Alloc>& root, const std::basic_string<char>& path)
{
    jsonpath_evaluator<Char,Alloc> evaluator;
    evaluator.evaluate(root,path);
    return evaluator.get_values();
}

template<typename Char, class Alloc>
std::vector<basic_json<Char,Alloc>> json_query(const basic_json<Char, Alloc>& root, const Char* path)
{
    jsonpath_evaluator<Char,Alloc> evaluator;
    evaluator.evaluate(root,path);
    return evaluator.get_values();
}

template<typename Char, class Alloc>
std::vector<basic_json<Char,Alloc>> json_query(const basic_json<Char, Alloc>& root, const Char* path, size_t length)
{
    jsonpath_evaluator<Char,Alloc> evaluator;
    evaluator.evaluate(root,path,length);
    return evaluator.get_values();
}

template<typename Char, class Alloc>
class jsonpath_evaluator
{
private:
    typedef const basic_json<Char,Alloc>* cjson_ptr;
    typedef std::vector<cjson_ptr> node_set;

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

    void end_nodes()
    {
        stack_.push_back(nodes_);
        nodes_.clear();
    }

public:
    jsonpath_evaluator()
    {
    }

    std::vector<basic_json<Char,Alloc>> get_values() const
    {
        std::vector<basic_json<Char,Alloc>> result;

        if (stack_.size() > 0)
        {
            for (size_t i = 0; i < stack_.back().size(); ++i)
            {
                cjson_ptr p = stack_.back()[i];
                result.push_back(*p);
            }
        }
        return result;
    }

    std::vector<const basic_json<Char,Alloc>*> get_value_ptrs() const
    {
        return stack_.size() > 0 ? stack_.back() : std::vector<cjson_ptr>();
    }

    void evaluate(const basic_json<Char, Alloc>& root, const std::basic_string<Char>& path)
    {
        evaluate(root,path.c_str(),path.length());
    }
    void evaluate(const basic_json<Char, Alloc>& root, const Char* path)
    {
        evaluate(root,path,std::char_traits<Char>::length(path));
    }
    void evaluate(const basic_json<Char, Alloc>& root, const Char* path, size_t path_length)
    {
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

        for (size_t i = 0; i < path_length; ++i)
        {
            Char c = path[i];
handle_state:
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
                };
                break;
            case states::dot:
                switch (c)
                {
                case '.':
                    recursive_descent_ = true;
                    break;
                default:
                    state_ = states::string;
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
                    end_element_index();
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    start_ = start_*10 + static_cast<size_t>(c-'0');
                    break;
                case ']':
                    end_element_index();
                    end_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                break;
            case states::left_bracket:
                switch (c)
                {
                case '?':
					{
                        jsonpath_filter_parser<Char,Alloc> parser;
                        parser.parse(path,i,path_length);
                        auto filter = parser.get_filter();
                        nodes_.clear();
						for (size_t j = 0; j < stack_.back().size(); ++j)
						{
	                        accept(*(stack_.back()[j]),*filter);
						}
                        end_nodes();
						i += parser.index();
					}
                    break;
                case ':':
                    step_ = 1;
                    end_undefined_ = true;
                    state_ = states::left_bracket_end;
                    break;
                case ',':
                    end_element_index();
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
                    end_element_index();
                    end_nodes();
                    state_ = states::expect_separator;
                    break;
                case '*':
                    end_all();
                    end_nodes();
                    state_ = states::expect_right_bracket;
                    break;
                case '\'':
                    state_ = states::quoted_string;
                    break;
                }
                break;
            case states::string: 
                switch (c)
                {
                case '[':
					end_member_name();
                    end_nodes();
					start_ = 0;
                    state_ = states::left_bracket;
                    break;
                case '.':
                    end_member_name();
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
                    end_member_name();
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
        case states::string: 
            {
                end_member_name();
                end_nodes();
            }
            break;
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

    void end_element_index()
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

    void end_member_name()
    {
		if (buffer_.length() > 0)
		{
            if (recursive_descent_)
            {
                end_member_name2();
                recursive_descent_ = false;
            }
            else
            {
                end_member_name1();
            }
			buffer_.clear();
		}
    }

    void end_member_name1()
    {
        for (size_t i = 0; i < stack_.back().size(); ++i)
        {
            cjson_ptr p = stack_.back()[i];
            if (p->has_member(buffer_))
            {
                nodes_.push_back(std::addressof(p->get(buffer_)));
            }
        }
    }

    void end_member_name2()
    {
        for (size_t i = 0; i < stack_.back().size(); ++i)
        {
            cjson_ptr p = stack_.back()[i];
            end_member_name2(*p);
        }
    }

    void accept(const basic_json<Char,Alloc>& val,
                jsonpath_filter<Char,Alloc>& filter)
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
            if (filter.accept(val))
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

    void end_member_name2(const basic_json<Char,Alloc>& val)
    {
        if (val.is_object())
        {
            if (val.has_member(buffer_))
            {
                nodes_.push_back(std::addressof(val.get(buffer_)));
            }
            for (auto it = val.begin_members(); it != val.end_members(); ++it)
            {
                end_member_name2(it->value());
            }
        }
		else if (val.is_array())
		{
            for (auto it = val.begin_elements(); it != val.end_elements(); ++it)
            {
                end_member_name2(*it);
            }
		}
    }
};

}}

#endif

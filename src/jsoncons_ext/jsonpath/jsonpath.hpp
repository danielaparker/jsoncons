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

namespace jsoncons { namespace jsonpath {

namespace states {
    enum states_t {
        start,
        expect_separator,
        string,
        quoted_string,
        left_bracket,
        left_bracket_end,
        left_bracket_step,
        expect_right_bracket,
        dot
    };
};

template<typename Char, class Alloc>
std::vector<basic_json<Char,Alloc>> jsonpath_query(const basic_json<Char, Alloc>& root, const std::basic_string<char>& path)
{
    jsonpath_evaluator<Char,Alloc> evaluator;
    evaluator.evaluate(root,path);
    return evaluator.get_result();
}

template<typename Char, class Alloc>
std::vector<basic_json<Char,Alloc>> jsonpath_query(const basic_json<Char, Alloc>& root, const Char* path)
{
    jsonpath_evaluator<Char,Alloc> evaluator;
    evaluator.evaluate(root,path);
    return evaluator.get_result();
}

template<typename Char, class Alloc>
std::vector<basic_json<Char,Alloc>> jsonpath_query(const basic_json<Char, Alloc>& root, const Char* path, size_t length)
{
    jsonpath_evaluator<Char,Alloc> evaluator;
    evaluator.evaluate(root,path,length);
    return evaluator.get_result();
}

template<typename Char, class Alloc>
class jsonpath_evaluator
{
private:
    typedef const basic_json<Char,Alloc>* cjson_ptr;
    typedef std::vector<cjson_ptr> node_set;

    states::states_t state_;
    std::basic_string<Char> buffer_;
    size_t index_;
    size_t index_end_;
    size_t index_step_;
    std::vector<node_set> stack_;
    std::vector<basic_json<Char,Alloc>> result_;
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

    std::vector<basic_json<Char,Alloc>> get_result()
    {
        return std::move(result_);
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
        index_ = 0;
        index_end_ = 0;
        index_step_ = 0;
        recursive_descent_ = false;

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
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    index_step_ = index_step_*10 + static_cast<size_t>(c-'0');
                    break;
                case ']':
                    end_element_index2();
                    end_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                break;
            case states::left_bracket_end:
                switch (c)
                {
                case ':':
                    end_element_index2();
                    state_ = states::left_bracket_step;
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    index_end_ = index_end_*10 + static_cast<size_t>(c-'0');
                    break;
                case ']':
                    end_element_index2();
                    end_nodes();
                    state_ = states::expect_separator;
                    break;
                }
                break;
            case states::left_bracket:
                switch (c)
                {
                case ':':
                    state_ = states::left_bracket_end;
                    break;
                case ',':
                    end_element_index();
                    break;
                case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                    index_ = index_*10 + static_cast<size_t>(c-'0');
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
					index_ = 0;
                    state_ = states::left_bracket;
                    break;
                case '.':
                    end_member_name();
                    end_nodes();
                    state_ = states::dot;
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
        if (stack_.size() > 0)
        {
            for (size_t i = 0; i < stack_.back().size(); ++i)
            {
                cjson_ptr p = stack_.back()[i];
                result_.push_back(*p);
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
        index_ = 0;
    }

    void end_element_index()
    {
        for (size_t i = 0; i < stack_.back().size(); ++i)
        {
            cjson_ptr p = stack_.back()[i];
            if (p->is_array() && index_ < p->size())
            {
                nodes_.push_back(std::addressof((*p)[index_]));
            }
        }
        index_ = 0;
    }

    void end_element_index2()
    {
        for (size_t i = 0; i < stack_.back().size(); ++i)
        {
            cjson_ptr p = stack_.back()[i];
            for (size_t j = index_; j < index_end_; ++j)
            {
                if (p->is_array() && j < p->size())
                {
                    nodes_.push_back(std::addressof((*p)[j]));
                }
            }
        }
        index_ = 0;
        index_end_ = 0;
        index_step_ = 1;
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
    }
};

}}

#endif

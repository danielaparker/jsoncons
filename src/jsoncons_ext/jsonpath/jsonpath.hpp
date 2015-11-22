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
        left_bracket
    };
};

template<typename Char, class Alloc>
basic_json<Char,Alloc> jsonpath_query(const basic_json<Char, Alloc>& root, const std::basic_string<char>& path)
{
    jsonpath_evaluator<Char,Alloc> evaluator;
    evaluator.evaluate(root,path);
    return evaluator.get_result();
}

template<typename Char, class Alloc>
basic_json<Char,Alloc> jsonpath_query(const basic_json<Char, Alloc>& root, const Char* path)
{
    jsonpath_evaluator<Char,Alloc> evaluator;
    evaluator.evaluate(root,path);
    return evaluator.get_result();
}

template<typename Char, class Alloc>
basic_json<Char,Alloc> jsonpath_query(const basic_json<Char, Alloc>& root, const Char* path, size_t length)
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
    std::vector<node_set> stack_;
    basic_json<Char,Alloc> result_;

public:
    jsonpath_evaluator()
    {
        result_ = basic_json<Char,Alloc>::array();
    }

    basic_json<Char,Alloc> get_result()
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

        for (size_t i = 0; i < path_length; ++i)
        {
            Char c = path[i];
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
            case states::expect_separator: 
                switch (c)
                {
                case '.':
                    buffer_.clear();
                    state_ = states::string;
                    break;
                case '[':
                    index_ = 0;
                    state_ = states::left_bracket;
                    break;
                };
                break;
            case states::left_bracket:
                switch (c)
                {
                case '0':case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:
                    index_ = index_*10 + static_cast<size_t>(c-'0');
                    break;
                case ']':
                    end_element_index();
                    state_ = states::string;
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
					index_ = 0;
                    state_ = states::left_bracket;
                    break;
                case '.':
                    {
                        end_member_name();
                        state_ = states::string;
                    }
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
                    state_ = states::expect_separator;
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
            }
            break;
        }
        if (stack_.size() > 0)
        {
            for (size_t i = 0; i < stack_.back().size(); ++i)
            {
                cjson_ptr p = stack_.back()[i];
                result_.add(*p);
            }
        }
    }

    void end_element_index()
    {
        node_set v;
        for (size_t i = 0; i < stack_.back().size(); ++i)
        {
            cjson_ptr p = stack_.back()[i];
            if (p->is_array() && index_ < p->size())
            {
                v.push_back(std::addressof((*p)[index_]));
            }
        }
        stack_.push_back(v);
        index_ = 0;
    }

    void end_member_name()
    {
		if (buffer_.length() > 0)
		{
			node_set v;
			for (size_t i = 0; i < stack_.back().size(); ++i)
			{
				cjson_ptr p = stack_.back()[i];
				if (p->has_member(buffer_))
				{
					v.push_back(std::addressof(p->get(buffer_)));
				}
			}
			stack_.push_back(v);
			buffer_.clear();
		}
    }
};

}}

#endif

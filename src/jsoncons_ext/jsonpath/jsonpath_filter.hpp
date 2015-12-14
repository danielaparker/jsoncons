// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSONPATH_FILTER_HPP
#define JSONCONS_JSONPATH_FILTER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include "jsoncons/jsoncons.hpp"
#include "jsoncons/json1.hpp"
#include "jsoncons/json2.hpp"

namespace jsoncons { namespace jsonpath {

template <class Char,class Alloc>
class jsonpath_evaluator;

namespace filter_states {
    enum filter_states_t {
        start,
        grouping,
        expect_path_or_value,
        string,
        integer,
        path,
        value,
        oper
    };
}


template<typename Char, class Alloc>
class expression
{
public:
    virtual void evaluate(const basic_json<Char,Alloc>& parent) = 0;
    virtual bool lt(const expression& parent) const = 0;
    virtual bool lt(const basic_json<Char,Alloc>& parent) const = 0;
    virtual bool gt(const expression& parent) const = 0;
    virtual bool gt(const basic_json<Char,Alloc>& parent) const = 0;

    static bool lt(const basic_json<Char,Alloc>& lhs, const basic_json<Char,Alloc>& rhs)
    {
        bool result = false;
        if (lhs.is<unsigned long long>() && rhs.is<unsigned long long>())
        {
            result = lhs.as<unsigned long long>() < rhs.as<unsigned long long>();
        }
        else if (lhs.is<long long>() && rhs.is<long long>())
        {
            result = lhs.as<long long>() < rhs.as<long long>();
        }
        else if (lhs.is<double>() || rhs.is<double>())
        {
            result = lhs.as<double>() < rhs.as<double>();
        }
        return result;
    }

    static bool gt(const basic_json<Char,Alloc>& lhs, const basic_json<Char,Alloc>& rhs)
    {
        bool result = false;
        if (lhs.is<unsigned long long>() && rhs.is<unsigned long long>())
        {
            result = lhs.as<unsigned long long>() > rhs.as<unsigned long long>();
        }
        else if (lhs.is<long long>() && rhs.is<long long>())
        {
            result = lhs.as<long long>() > rhs.as<long long>();
        }
        else if ((lhs.is<double>() && rhs.is_number()) || (lhs.is_number() && rhs.is<double>()))
        {
            result = lhs.as<double>() > rhs.as<double>();
        }
        return result;
    }
};

template<typename Char, class Alloc>
class value_expression : public expression<Char,Alloc>
{
    basic_json<Char,Alloc> value_;
public:
    template <class T>
    value_expression(T value)
        : value_(value)
    {
    }

    void evaluate(const basic_json<Char,Alloc>& parent) override
    {
    }

    bool lt(const expression<Char,Alloc>& rhs) const override
    {
        return rhs.gt(value_);
    }

    bool lt(const basic_json<Char,Alloc>& rhs) const override
    {
        return expression<Char,Alloc>::lt(value_,rhs);
    }

    bool gt(const expression<Char,Alloc>& rhs) const override
    {
        return rhs.lt(value_);
    }

    bool gt(const basic_json<Char,Alloc>& rhs) const override
    {
        return expression<Char,Alloc>::gt(value_,rhs);
    }
};

template<typename Char, class Alloc>
class path_expression : public expression<Char,Alloc>
{
    std::basic_string<Char> path_;
    std::vector<const basic_json<Char,Alloc>*> nodes_;
public:
    path_expression(const std::basic_string<Char>& path)
        : path_(path)
    {
    }

    void evaluate(const basic_json<Char,Alloc>& parent) override
    {
        jsonpath_evaluator<Char,Alloc> evaluator;
        evaluator.evaluate(parent,path_);
        nodes_ = evaluator.get_value_ptrs();
    }

    bool lt(const basic_json<Char,Alloc>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = expression<Char,Alloc>::lt(*nodes_[i],rhs);
            }
        }
        return result;
    }

    bool lt(const expression<Char,Alloc>& parent) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = parent.gt(*nodes_[i]);
            }
        }
        return result;
    }

    bool gt(const basic_json<Char,Alloc>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = expression<Char,Alloc>::gt(*nodes_[i],rhs);
            }
        }
        return result;
    }

    bool gt(const expression<Char,Alloc>& parent) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = parent.lt(*nodes_[i]);
            }
        }
        return result;
    }
};

namespace operators {
enum operators_t
{
    none,
    eq,
    lt,
    gt,
    lte,
    gte
};}

template<typename Char, class Alloc>
class jsonpath_filter
{
public:
    std::shared_ptr<expression<Char,Alloc>> lhs_;
    operators::operators_t operator_;
    std::shared_ptr<expression<Char,Alloc>> rhs_;

    jsonpath_filter()
    {
    }

    bool has_lhs() const
    {
        return lhs_.get() != nullptr;
    }

    bool has_rhs() const
    {
        return rhs_.get() != nullptr;
    }

    bool has_operator() const
    {
        return operator_ != operators::none;
    }

    bool accept(const basic_json<Char,Alloc>& arg) const
    {
        lhs_->evaluate(arg);
        rhs_->evaluate(arg);
        switch (operator_)
        {
        case operators::gt:
            return lhs_->gt(*rhs_);
        case operators::lt:
            return lhs_->lt(*rhs_);
        }
        return false;
    }
};

template<typename Char, class Alloc>
class jsonpath_filter_parser
{
    size_t index_;
    filter_states::filter_states_t state_;
    std::vector<jsonpath_filter<Char,Alloc>> stack_;
    jsonpath_filter<Char,Alloc> filter_;
    std::basic_string<Char> buffer_;
public:
    void parse(const Char* p, size_t start, size_t length)
    {
        int prev_c = 0;
        index_ = start;
        state_ = filter_states::start;
        bool done = false;
        for (; !done && index_ < length; ++index_)
        {
            int c = p[index_];
handle_state:
            switch (state_)
            {
            case filter_states::start:
                switch (c)
                {
                case '(':
                    state_ = filter_states::grouping;
                    stack_.push_back(jsonpath_filter<Char,Alloc>());
                    break;
                case ']':
                    done = true;
                    break;
                }
                break;
            case filter_states::oper:
                switch (c)
                {
                case '=':
                    switch (prev_c)
                    {
                    case '=':
                        {
                            stack_.back().operator_ = operators::eq;
                            state_ = filter_states::expect_path_or_value;
                        }
                        break;
                    case '>':
                        {
                            stack_.back().operator_ = operators::gte;
                            state_ = filter_states::expect_path_or_value;
                        }
                        break;
                    case '<':
                        {
                            stack_.back().operator_ = operators::lte;
                            state_ = filter_states::expect_path_or_value;
                        }
                        break;
                    }
                    break;
                default:
                    switch (prev_c)
                    {
                    case '<':
                        {
                            stack_.back().operator_ = operators::lt;
                            state_ = filter_states::expect_path_or_value;
                        }
                        break;
                    case '>':
                        {
                            stack_.back().operator_ = operators::gt;
                            state_ = filter_states::expect_path_or_value;
                        }
                        break;
                    }
                    break;
                }
                break;
            case filter_states::integer: 
                {
                    switch (c)
                    {
                    case '<':
                    case '>':
                        {
                            if (buffer_.length() > 0)
                            {
                                auto val = basic_json<Char, Alloc>::parse_string(buffer_);
                                if (!stack_.back().has_lhs())
                                {
                                    stack_.back().lhs_ = std::make_shared<value_expression<Char, Alloc>>(val);
                                }
                                buffer_.clear();
                            }
                            state_ = filter_states::oper;
                            goto handle_state;
                        }
                        break;
                    case ')':
                        if (buffer_.length() > 0)
                        {
                            auto val = basic_json<Char,Alloc>::parse_string(buffer_);
                            if (!stack_.back().has_lhs())
                            {
                                stack_.back().lhs_ = std::make_shared<value_expression<Char,Alloc>>(val);
                            }
                            else if (!stack_.back().has_rhs())
                            {
                                stack_.back().rhs_ = std::make_shared<value_expression<Char,Alloc>>(val);
                            }
                            buffer_.clear();
                        }
						if (stack_.size() == 1)
						{
							filter_ = stack_[0];
						}
                        stack_.pop_back();
                        state_ = filter_states::start;
                        break;
                    case ' ':case '\n':case '\r':case '\t':
                        if (buffer_.length() > 0)
						{
                            auto val = basic_json<Char,Alloc>::parse_string(buffer_);
                            if (!stack_.back().has_lhs())
                            {
                                stack_.back().lhs_ = std::make_shared<value_expression<Char,Alloc>>(val);
                            }
                            else if (!stack_.back().has_rhs())
                            {
                                stack_.back().rhs_ = std::make_shared<value_expression<Char,Alloc>>(val);
                            }
							buffer_.clear();
						}
                        break; 
                    case '0': 
                    case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8': case '9':
                        buffer_.push_back(c);
                        break;
                    }
                }
                break;
            case filter_states::string: 
                switch (c)
                {
                    if (buffer_.length() > 0)
                    {
                        auto val = basic_json<Char,Alloc>::parse_string(buffer_);
                        if (!stack_.back().has_lhs())
                        {
                            stack_.back().lhs_ = std::make_shared<value_expression<Char,Alloc>>(val);
                        }
                        else if (!stack_.back().has_rhs())
                        {
                            stack_.back().rhs_ = std::make_shared<value_expression<Char,Alloc>>(val);
                        }
                        buffer_.clear();
                    }
                    if (stack_.size() == 1)
                    {
                        filter_ = stack_[0];
                        done = true;
                    }
                    stack_.pop_back();
                    state_ = filter_states::start;
                    break;
                case '<':
                    stack_.back().lhs_ = std::make_shared<path_expression<Char,Alloc>>(buffer_);
                    buffer_.clear();
                    stack_.back().operator_ = operators::lt;
                    state_ = filter_states::expect_path_or_value;
                    break;
                case '>':
                    stack_.back().lhs_ = std::make_shared<path_expression<Char,Alloc>>(buffer_);
                    buffer_.clear();
                    stack_.back().operator_ = operators::gt;
                    break;
                default:
                    buffer_.push_back(c);
                    break;
                };
                break;
            case filter_states::expect_path_or_value: 
                switch (c)
                {
                case '@':
                    buffer_.push_back(c);
                    state_ = filter_states::path;
                    break;
				case ' ':case '\n':case '\r':case '\t':
					break;
                default: 
                    buffer_.push_back(c);
                    state_ = filter_states::integer;
                    break;
                };
                break;
            case filter_states::path: 
                switch (c)
                {
                case '<':
                    if (buffer_.length() > 0)
                    {
                        if (!stack_.back().has_lhs())
                        {
                            stack_.back().lhs_ = std::make_shared<path_expression<Char,Alloc>>(buffer_);
                        }
                        else if (!stack_.back().has_rhs())
                        {
                            stack_.back().rhs_ = std::make_shared<path_expression<Char,Alloc>>(buffer_);
                        }
                        buffer_.clear();
                    }
                    stack_.back().operator_ = operators::lt;
                    state_ = filter_states::expect_path_or_value;
                    break;
                case '>':
                    if (buffer_.length() > 0)
                    {
                        if (!stack_.back().has_lhs())
                        {
                            stack_.back().lhs_ = std::make_shared<path_expression<Char,Alloc>>(buffer_);
                        }
                        else if (!stack_.back().has_rhs())
                        {
                            stack_.back().rhs_ = std::make_shared<path_expression<Char,Alloc>>(buffer_);
                        }
                        buffer_.clear();
                    }
                    stack_.back().operator_ = operators::gt;
                    state_ = filter_states::expect_path_or_value;
                    break;
                case ')':
                    if (buffer_.length() > 0)
                    {
                        if (!stack_.back().has_lhs())
                        {
                            stack_.back().lhs_ = std::make_shared<path_expression<Char,Alloc>>(buffer_);
                        }
                        else if (!stack_.back().has_rhs())
                        {
                            stack_.back().rhs_ = std::make_shared<path_expression<Char,Alloc>>(buffer_);
                        }
                        buffer_.clear();
                    }
                    if (stack_.size() == 1)
                    {
                        filter_ = stack_[0];
                    }
                    stack_.pop_back();
                    state_ = filter_states::start;
                    break;
                default:
                    buffer_.push_back(c);
                    break;
                };
                break;
            case filter_states::grouping:
                switch (c)
                {
                case '@':
                    buffer_.push_back(c);
                    state_ = filter_states::path;
                    break;
                case ')':
                    state_ = filter_states::grouping;
                    if (stack_.size() == 1)
                    {
                        filter_ = stack_[0];
                        done = true;
                    }
                    stack_.pop_back();
                    break;
                default: 
                    buffer_.push_back(c);
                    state_ = filter_states::integer;
                    break;
                }
                break;
            }
            prev_c = c;
        }
    }

    jsonpath_filter<Char,Alloc> get_filter() 
    {
        return filter_;
    }

    size_t index() const
    {
        return index_;
    }
};


}}
#endif
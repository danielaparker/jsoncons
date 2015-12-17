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
        expect_right_round_bracket,
        expect_oper_or_right_round_bracket,
        expect_path_or_value,
        quoted_text,
        unquoted_text,
        path,
        value,
        oper
    };
}

template<typename Char, class Alloc>
class expression
{
public:
    virtual void evaluate(const basic_json<Char,Alloc>& context_node) = 0;
    virtual bool evaluate_single_node() const = 0;
    virtual bool eq(const expression& rhs) const = 0;
    virtual bool eq(const basic_json<Char,Alloc>& rhs) const = 0;
    virtual bool ne(const expression& rhs) const = 0;
    virtual bool ne(const basic_json<Char,Alloc>& rhs) const = 0;
    virtual bool ampamp(const expression& rhs) const = 0;
    virtual bool ampamp(const basic_json<Char,Alloc>& rhs) const = 0;
    virtual bool pipepipe(const expression& rhs) const = 0;
    virtual bool pipepipe(const basic_json<Char,Alloc>& rhs) const = 0;
    virtual bool lt(const expression& rhs) const = 0;
    virtual bool lt(const basic_json<Char,Alloc>& rhs) const = 0;
    virtual bool gt(const expression& rhs) const = 0;
    virtual bool gt(const basic_json<Char,Alloc>& rhs) const = 0;

    static bool evaluate_single_node(const basic_json<Char,Alloc>& node)
    {
        bool result = false;
        if (node.is_bool())
        {
            result = node.as_bool();
        }
        else 
        {
            result = node.is_empty();
        }
        return result;
    }

    static bool ampamp(const basic_json<Char,Alloc>& lhs, const basic_json<Char,Alloc>& rhs)
    {
        return evaluate_single_node(lhs) && evaluate_single_node(rhs);
    }

    static bool pipepipe(const basic_json<Char,Alloc>& lhs, const basic_json<Char,Alloc>& rhs)
    {
        return evaluate_single_node(lhs) || evaluate_single_node(rhs);
    }

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
        else if (lhs.is_string() && rhs.is_string())
        {
            result = lhs.as_string() < rhs.as_string();
        }
        return result;
    }

    static bool gt(const basic_json<Char,Alloc>& lhs, const basic_json<Char,Alloc>& rhs)
    {
        return lt(rhs,lhs);
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

    void evaluate(const basic_json<Char,Alloc>& context_node) override
    {
    }

    bool evaluate_single_node() const override
    {
        return expression<Char, Alloc>::evaluate_single_node(value_);
    }

    bool eq(const expression<Char,Alloc>& rhs) const override
    {
        return rhs.eq(value_);
    }

    bool eq(const basic_json<Char,Alloc>& rhs) const override
    {
        return value_ == rhs;
    }

    bool ne(const expression<Char,Alloc>& rhs) const override
    {
        return rhs.ne(value_);
    }
    bool ne(const basic_json<Char,Alloc>& rhs) const override
    {
        return value_ != rhs;
    }
    bool ampamp(const expression& rhs) const override
    {
        return rhs.ampamp(value_);
    }
    bool ampamp(const basic_json<Char,Alloc>& rhs) const override
    {
        return expression<Char, Alloc>::ampamp(value_,rhs);
    }
    bool pipepipe(const expression& rhs) const override
    {
        return rhs.pipepipe(value_);
    }
    bool pipepipe(const basic_json<Char,Alloc>& rhs) const override
    {
        return expression<Char, Alloc>::pipepipe(value_,rhs);
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

    void evaluate(const basic_json<Char,Alloc>& context_node) override
    {
        jsonpath_evaluator<Char,Alloc> evaluator;
        evaluator.evaluate(context_node,path_);
        nodes_ = evaluator.get_nodes();
    }

    bool evaluate_single_node() const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = expression<Char,Alloc>::evaluate_single_node(*nodes_[i]);
            }
        }
        return result;
    }

    bool eq(const expression<Char,Alloc>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.eq(*nodes_[i]);
            }
        }
        return result;
    }

    bool eq(const basic_json<Char,Alloc>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = *nodes_[i] == rhs;
            }
        }
        return result;
    }

    bool ne(const expression& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.ne(*nodes_[i]);
            }
        }
        return result;

    }
    bool ne(const basic_json<Char,Alloc>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = *nodes_[i] != rhs;
            }
        }
        return result;
    }
    bool ampamp(const expression& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.ampamp(*nodes_[i]);
            }
        }
        return result;
    }
    bool ampamp(const basic_json<Char,Alloc>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = expression<Char,Alloc>::ampamp(*nodes_[i],rhs);
            }
        }
        return result;
    }
    bool pipepipe(const expression& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.pipepipe(*nodes_[i]);
            }
        }
        return result;
    }
    bool pipepipe(const basic_json<Char,Alloc>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = expression<Char,Alloc>::pipepipe(*nodes_[i],rhs);
            }
        }
        return result;
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

    bool lt(const expression<Char,Alloc>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.gt(*nodes_[i]);
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

    bool gt(const expression<Char,Alloc>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.lt(*nodes_[i]);
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
    ne,
    ampamp,
    pipepipe,
    lt,
    gt,
    lte,
    gte
};}

template<typename Char, class Alloc>
class jsonpath_filter : public expression<Char,Alloc>
{
public:
    std::shared_ptr<expression<Char,Alloc>> lhs_;
    operators::operators_t operator_;
    std::shared_ptr<expression<Char,Alloc>> rhs_;

    jsonpath_filter()
		: operator_(operators::none)
    {
    }

    void evaluate(const basic_json<Char,Alloc>& context_node) override
    {
        lhs_->evaluate(context_node);
        if (operator_ != operators::none)
        {
            rhs_->evaluate(context_node);
        }
    }
    bool evaluate_single_node() const override
    {
        return lhs_->evaluate_single_node();
    }
    bool eq(const expression& rhs) const override
    {
        return false;
    }
    bool eq(const basic_json<Char,Alloc>& rhs) const override
    {
        return false;
    }

    bool ne(const expression& rhs) const override
    {
        return false;
    }
    bool ne(const basic_json<Char,Alloc>& rhs) const override
    {
        return false;
    }
    bool ampamp(const expression& rhs) const override
    {
        return rhs.ampamp(basic_json<Char,Alloc>(compare()));
    }
    bool ampamp(const basic_json<Char,Alloc>& rhs) const override
    {
        return (rhs.is<bool>() && rhs.as<bool>()) && compare();
    }
    bool pipepipe(const expression& rhs) const override
    {
        return rhs.pipepipe(basic_json<Char,Alloc>(compare()));
    }
    bool pipepipe(const basic_json<Char,Alloc>& rhs) const override
    {
        return (rhs.is<bool>() && rhs.as<bool>()) || compare();
    }

    bool lt(const expression& rhs) const override
    {
        return false;
    }
    bool lt(const basic_json<Char,Alloc>& rhs) const override
    {
        return false;
    }
    bool gt(const expression& rhs) const override
    {
        return false;
    }
    bool gt(const basic_json<Char,Alloc>& rhs) const override
    {
        return false;
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
		if (operator_ != operators::none)
		{
			rhs_->evaluate(arg);
		}
        return compare();
    }

    bool compare() const
    {
        switch (operator_)
        {
        case operators::eq:
            return lhs_->eq(*rhs_);
        case operators::ne:
            return lhs_->ne(*rhs_);
        case operators::ampamp:
            return lhs_->ampamp(*rhs_);
        case operators::pipepipe:
            return lhs_->pipepipe(*rhs_);
        case operators::gt:
            return lhs_->gt(*rhs_);
        case operators::lt:
            return lhs_->lt(*rhs_);
        case operators::gte:
            return lhs_->gt(*rhs_) || lhs_->eq(*rhs_);
        case operators::lte:
            return lhs_->lt(*rhs_) || lhs_->eq(*rhs_);
        case operators::none:
            return lhs_->evaluate_single_node();
			break;
        }
        return false;
    }
};

template<typename Char, class Alloc>
class jsonpath_filter_parser
{
    size_t index_;
    filter_states::filter_states_t state_;
    std::vector<std::shared_ptr<jsonpath_filter<Char,Alloc>>> stack_;
    std::shared_ptr<jsonpath_filter<Char,Alloc>> filter_;
    std::basic_string<Char> buffer_;
public:
    void parse(const Char* p, size_t start, size_t length)
    {
        index_ = start;
        state_ = filter_states::start;
        bool done = false;
        while (!done && index_ < length)
        {
            int c = p[index_];
handle_state:
            switch (state_)
            {
            case filter_states::start:
                switch (c)
                {
                case '(':
                    state_ = filter_states::expect_path_or_value;
                    stack_.push_back(std::make_shared<jsonpath_filter<Char,Alloc>>());
                    break;
                case ')':
                    if (stack_.size() == 1)
                    {
                        filter_ = stack_.back();
                        stack_.pop_back();
                        done = true;
                    }
                    else 
                    {
                        auto expr = stack_.back();
                        stack_.pop_back();

                        if (!stack_.back()->has_lhs())
                        {
                            stack_.back()->lhs_ = expr;
                        }
                        else if (!stack_.back()->has_rhs())
                        {
                            stack_.back()->rhs_ = expr;
                        }
                    }
                    break;
                }
                break;
            case filter_states::oper:
                switch (c)
                {
                case '!':
                    if (index_+1 < length && p[index_+1] == '=')
                    {
                        stack_.back()->operator_ = operators::ne;
                        ++index_;
                        state_ = filter_states::expect_path_or_value;
                    }
                    break;
                case '&':
                    if (index_+1 < length && p[index_+1] == '&')
                    {
                        stack_.back()->operator_ = operators::ampamp;
                        ++index_;
                        state_ = filter_states::expect_path_or_value;
                    }
                    break;
                case '|':
                    if (index_+1 < length && p[index_+1] == '|')
                    {
                        stack_.back()->operator_ = operators::pipepipe;
                        ++index_;
                        state_ = filter_states::expect_path_or_value;
                    }
                    break;
                case '=':
                    if (index_+1 < length && p[index_+1] == '=')
                    {
                        stack_.back()->operator_ = operators::eq;
                        ++index_;
                        state_ = filter_states::expect_path_or_value;
                    }
                    break;
                case '>':
                    if (index_+1 < length && p[index_+1] == '=')
                    {
                        stack_.back()->operator_ = operators::gte;
                        ++index_;
                        state_ = filter_states::expect_path_or_value;
                    }
                    else
                    {
                        stack_.back()->operator_ = operators::gt;
                        state_ = filter_states::expect_path_or_value;
                    }
                    break;
                case '<':
                    if (index_+1 < length && p[index_+1] == '=')
                    {
                        stack_.back()->operator_ = operators::lte;
                        ++index_;
                        state_ = filter_states::expect_path_or_value;
                    }
                    else
                    {
                        stack_.back()->operator_ = operators::lt;
                        state_ = filter_states::expect_path_or_value;
                    }
                    break;
                case ' ':case '\n':case '\r':case '\t':
                    break;
                default:
                    JSONCONS_THROW_EXCEPTION("Invalid filter.");
                    break;

                }
                break;
            case filter_states::unquoted_text: 
                {
                    switch (c)
                    {
                    case '<':
                    case '>':
                    case '!':
                    case '=':
                    case '&':
                    case '|':
                        {
                            if (buffer_.length() > 0)
                            {
                                auto val = basic_json<Char, Alloc>::parse_string(buffer_);
                                if (stack_.back()->has_lhs())
                                {
                                    JSONCONS_THROW_EXCEPTION("Invalid filter.");
                                }
                                stack_.back()->lhs_ = std::make_shared<value_expression<Char, Alloc>>(val);
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
                            if (!stack_.back()->has_lhs())
                            {
                                stack_.back()->lhs_ = std::make_shared<value_expression<Char,Alloc>>(val);
                            }
                            else if (!stack_.back()->has_rhs())
                            {
                                stack_.back()->rhs_ = std::make_shared<value_expression<Char,Alloc>>(val);
                            }
                            else
                            {
                                JSONCONS_THROW_EXCEPTION("Invalid filter.");
                            }
                            buffer_.clear();
                        }
						if (stack_.size() == 1)
						{
							filter_ = stack_.back();
                            state_ = filter_states::start;
                            stack_.pop_back();
                            done = true;
						}
                        else
                        {
                            auto expr = stack_.back();
                            stack_.pop_back();

                            if (!stack_.back()->has_lhs())
                            {
                                stack_.back()->lhs_ = expr;
                            }
                            else if (!stack_.back()->has_rhs())
                            {
                                stack_.back()->rhs_ = expr;
                            }
                            else
                            {
                                JSONCONS_THROW_EXCEPTION("Invalid filter.");
                            }
                            state_ = filter_states::expect_path_or_value;
                        }
                        break;
                    case ' ':case '\n':case '\r':case '\t':
                        if (buffer_.length() > 0)
						{
                            auto val = basic_json<Char,Alloc>::parse_string(buffer_);
                            if (!stack_.back()->has_lhs())
                            {
                                stack_.back()->lhs_ = std::make_shared<value_expression<Char,Alloc>>(val);
                            }
                            else if (!stack_.back()->has_rhs())
                            {
                                stack_.back()->rhs_ = std::make_shared<value_expression<Char,Alloc>>(val);
                            }
                            else
                            {
                                JSONCONS_THROW_EXCEPTION("Invalid filter.");
                            }
							buffer_.clear();
						}
                        break; 
                    default: 
                        buffer_.push_back(c);
                        break;
                    }
                }
                break;
            case filter_states::quoted_text: 
                {
                    switch (c)
                    {                   
                    case '\'':
                        buffer_.push_back('\"');
                        if (buffer_.length() > 0)
                        {
                            auto val = basic_json<Char,Alloc>::parse_string(buffer_);
                            if (!stack_.back()->has_lhs())
                            {
                                stack_.back()->lhs_ = std::make_shared<value_expression<Char,Alloc>>(val);
                                state_ = filter_states::oper;
                            }
                            else if (!stack_.back()->has_rhs())
                            {
                                stack_.back()->rhs_ = std::make_shared<value_expression<Char,Alloc>>(val);
                                state_ = filter_states::expect_right_round_bracket;
                            }
                            else
                            {
                                JSONCONS_THROW_EXCEPTION("Invalid filter.");
                            }
                            buffer_.clear();
                        }
                        break;

                    default: 
                        buffer_.push_back(c);
                        break;
                    }
                }
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
                case '\'':
                    buffer_.push_back('\"');
                    state_ = filter_states::quoted_text;
                    break;
                case '(':
                    stack_.push_back(std::make_shared<jsonpath_filter<Char,Alloc>>());
                    break;
                case ')':
                    if (stack_.size() == 1)
                    {
                        filter_ = stack_.back();
                        stack_.pop_back();
                        done = true;
                        state_ = filter_states::start;
                    }
                    else 
                    {
                        auto expr = stack_.back();
                        stack_.pop_back();

                        if (!stack_.back()->has_lhs())
                        {
                            stack_.back()->lhs_ = expr;
                        }
                        else if (!stack_.back()->has_rhs())
                        {
                            stack_.back()->rhs_ = expr;
                        }
                        else
                        {
                            JSONCONS_THROW_EXCEPTION("Invalid filter.");
                        }
                    }
                    break;
                case '<':
                case '>':
                case '!':
                case '=':
                case '&':
                case '|':
                    {
                        state_ = filter_states::oper;
                        goto handle_state;
                    }
                    break;
                default: 
                    state_ = filter_states::unquoted_text;
                    goto handle_state;
                    break;
                };
                break;
            case filter_states::expect_oper_or_right_round_bracket: 
                switch (c)
                {
                case ' ':case '\n':case '\r':case '\t':
                    break;
                case ')':
                    if (stack_.size() == 1)
                    {
                        filter_ = stack_.back();
                        stack_.pop_back();
                        done = true;
                        state_ = filter_states::start;
                    }
                    else 
                    {
                        auto expr = stack_.back();
                        stack_.pop_back();

                        if (!stack_.back()->has_lhs())
                        {
                            stack_.back()->lhs_ = expr;
                        }
                        else if (!stack_.back()->has_rhs())
                        {
                            stack_.back()->rhs_ = expr;
                        }
                        else
                        {
                            JSONCONS_THROW_EXCEPTION("Invalid filter.");
                        }
                    }
                    break;
                case '<':
                case '>':
                case '!':
                case '=':
                case '&':
                case '|':
                    {
                        state_ = filter_states::oper;
                        goto handle_state;
                    }
                    break;
                default: 
                    JSONCONS_THROW_EXCEPTION("Invalid filter.");
                    break;
                };
                break;
            case filter_states::expect_right_round_bracket: 
                switch (c)
                {
                case ' ':case '\n':case '\r':case '\t':
                    break;
                case ')':
                    if (stack_.size() == 1)
                    {
                        filter_ = stack_.back();
                        stack_.pop_back();
                        done = true;
                        state_ = filter_states::start;
                    }
                    else 
                    {
                        auto expr = stack_.back();
                        stack_.pop_back();

                        if (!stack_.back()->has_lhs())
                        {
                            stack_.back()->lhs_ = expr;
                        }
                        else if (!stack_.back()->has_rhs())
                        {
                            stack_.back()->rhs_ = expr;
                        }
                        else
                        {
                            JSONCONS_THROW_EXCEPTION("Invalid filter.");
                        }
                        state_ = filter_states::expect_oper_or_right_round_bracket;
                    }
                    break;
                default: 
                    JSONCONS_THROW_EXCEPTION("Invalid filter.");
                    break;
                };
                break;
            case filter_states::path: 
                switch (c)
                {
                case '<':
                case '>':
                case '!':
                case '=':
                case '&':
                case '|':
                    {
                        if (buffer_.length() > 0)
                        {
                            if (stack_.back()->has_lhs())
                            {
                                JSONCONS_THROW_EXCEPTION("Invalid filter.");
                            }
                            stack_.back()->lhs_ = std::make_shared<path_expression<Char,Alloc>>(buffer_);
                            buffer_.clear();
                        }
                        state_ = filter_states::oper;
                        goto handle_state;
                    }
                    break;
                case ')':
                    if (buffer_.length() > 0)
                    {
                        if (!stack_.back()->has_lhs())
                        {
                            stack_.back()->lhs_ = std::make_shared<path_expression<Char,Alloc>>(buffer_);
                        }
                        else if (!stack_.back()->has_rhs())
                        {
                            stack_.back()->rhs_ = std::make_shared<path_expression<Char,Alloc>>(buffer_);
                        }
                        else
                        {
                            JSONCONS_THROW_EXCEPTION("Invalid filter.");
                        }
                        buffer_.clear();
                    }
                    if (stack_.size() == 1)
                    {
                        filter_ = stack_.back();
                        stack_.pop_back();
                        state_ = filter_states::start;
                        done = true;
                    }
                    else
                    {
                        auto expr = stack_.back();
                        stack_.pop_back();

                        if (!stack_.back()->has_lhs())
                        {
                            stack_.back()->lhs_ = expr;
                        }
                        else if (!stack_.back()->has_rhs())
                        {
                            stack_.back()->rhs_ = expr;
                        }
                        else
                        {
                            JSONCONS_THROW_EXCEPTION("Invalid filter.");
                        }
                        state_ = filter_states::expect_path_or_value;
                    }
                     break;
                default:
                    buffer_.push_back(c);
                    break;
                };
                break;
            }
            ++index_;
        }
        if (stack_.size() != 0)
        {
            JSONCONS_THROW_EXCEPTION("Invalid filter.");
        }
    }

    std::shared_ptr<jsonpath_filter<Char,Alloc>> get_filter() 
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
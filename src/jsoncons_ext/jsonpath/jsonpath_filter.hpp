// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_FILTER_HPP
#define JSONCONS_JSONPATH_FILTER_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <regex>
#include "jsoncons/json.hpp"
#include "jsonpath_error_category.hpp"

namespace jsoncons { namespace jsonpath {

template <class JsonT>
class jsonpath_evaluator;

enum class filter_states
{
    start,
    cr,
    lf,
    expect_right_round_bracket,
    expect_oper_or_right_round_bracket,
    expect_path_or_value,
    expect_regex,
    regex,
    single_quoted_text,
    double_quoted_text,
    unquoted_text,
    path,
    value,
    oper
};

enum class token_types
{
    left_paren,
    right_paren,
    term,
    eq,
    ne,
    regex,
    ampamp,
    pipepipe,
    lt,
    gt,
    lte,
    gte,
    plus,
    minus,
    exclaim,
    done
};

template <class JsonT>
class term
{
public:
    typedef typename JsonT::string_type string_type;
    typedef typename JsonT::char_type char_type;

    virtual void initialize(const JsonT& context_node)
    {
    }
    virtual bool accept_single_node() const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual JsonT evaluate_single_node() const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool exclaim() const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool eq(const term& rhs) const 
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool eq(const JsonT& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool ne(const term& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool ne(const JsonT& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool regex(const term& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool regex2(const string_type& subject) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool ampamp(const term& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool ampamp(const JsonT& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool pipepipe(const term& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool pipepipe(const JsonT& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool lt(const term& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool lt(const JsonT& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool gt(const term& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual bool gt(const JsonT& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual JsonT minus(const term& rhs) const 
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual JsonT  minus(const JsonT& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual JsonT  unary_minus() const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual JsonT plus(const term& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
    virtual JsonT  plus(const JsonT& rhs) const
    {
        throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unsupported_operator, jsonpath_error_category()),1,1);
    }
};

template <class JsonT>
class token
{
    token_types type_;
    std::shared_ptr<term<JsonT>> term_ptr_;
public:
    token(token_types type)
        : type_(type)
    {
    }
    token(token_types type, std::shared_ptr<term<JsonT>> term_ptr)
        : type_(type), term_ptr_(term_ptr)
    {
    }
    token(const token& t)
        : type_(t.type_), term_ptr_(t.term_ptr_)
    {
    }

    token_types type() const
    {
        return type_;
    }

    std::shared_ptr<term<JsonT>> term_ptr()
    {
        return term_ptr_;
    }

    void initialize(const JsonT& context_node)
    {
        if (term_ptr_.get() != nullptr)
        {
            term_ptr_->initialize(context_node);
        }
    }
};

template <class JsonT>
class token_stream
{
    std::vector<token<JsonT>>& tokens_;
    size_t index_;
public:
    token_stream(std::vector<token<JsonT>>& tokens)
        : tokens_(tokens), index_(0)
    {
    }

    token<JsonT> get()
    {
        static token<JsonT> done = token<JsonT>(token_types::done);
        return index_ < tokens_.size() ? tokens_[index_++] : done;
    }
    void putback()
    {
        --index_;
    }
};

template <class JsonT>
bool ampamp(const JsonT& lhs, const JsonT& rhs)
{
    return lhs.as_bool() && rhs.as_bool();
}

template <class JsonT>
bool pipepipe(const JsonT& lhs, const JsonT& rhs)
{
    return lhs.as_bool() || rhs.as_bool();
}

template <class JsonT>
bool lt(const JsonT& lhs, const JsonT& rhs)
{
    bool result = false;
    if (lhs. template is<unsigned long long>() && rhs. template is<unsigned long long>())
    {
        result = lhs. template as<unsigned long long>() < rhs. template as<unsigned long long>();
    }
    else if (lhs. template is<long long>() && rhs. template is<long long>())
    {
        result = lhs. template as<long long>() < rhs. template as<long long>();
    }
    else if ((lhs.is_number() && rhs.is_double()) || (lhs.is_double() && rhs.is_number()))
    {
        result = lhs.as_double() < rhs.as_double();
    }
    else if (lhs.is_string() && rhs.is_string())
    {
        result = lhs.as_string() < rhs.as_string();
    }
    return result;
}

template <class JsonT>
bool gt(const JsonT& lhs, const JsonT& rhs)
{
    return lt(rhs,lhs);
}

template <class JsonT>
JsonT plus(const JsonT& lhs, const JsonT& rhs)
{
    JsonT result = jsoncons::null_type();
    if (lhs.is_integer() && rhs.is_integer())
    {
        result = ((lhs.as_integer() + rhs.as_integer()));
    }
    else if ((lhs.is_number() && rhs.is_double()) || (lhs.is_double() && rhs.is_number()))
    {
        result = (lhs.as_double() + rhs.as_double());
    }
    else if (lhs.is_uinteger() && rhs.is_uinteger())
    {
        result = (lhs.as_uinteger() + rhs.as_uinteger());
    }
    return result;
}

template <class JsonT>
JsonT unary_minus(const JsonT& lhs)
{
    JsonT result = jsoncons::null_type();
    if (lhs.is_integer())
    {
        result = -lhs.as_integer();
    }
    else if (lhs.is_double())
    {
        result = -lhs.as_double();
    }
    return result;
}

template <class JsonT>
JsonT minus(const JsonT& lhs, const JsonT& rhs)
{
    JsonT result = jsoncons::null_type();
    if (lhs.is_integer() && rhs.is_integer())
    {
        result = ((lhs.as_integer() - rhs.as_integer()));
    }
    else if ((lhs.is_number() && rhs.is_double()) || (lhs.is_double() && rhs.is_number()))
    {
        result = (lhs.as_double() - rhs.as_double());
    }
    else if (lhs.is_uinteger() && rhs.is_uinteger() && lt(rhs,lhs))
    {
        result = (lhs.as_uinteger() - rhs.as_uinteger());
    }
    return result;
}

template <class JsonT>
class value_term : public term<JsonT>
{
    JsonT value_;
public:
    template <class T>
    value_term(const T& value)
        : value_(value)
    {
    }

    bool accept_single_node() const override
    {
        return value_.as_bool();
    }

    JsonT evaluate_single_node() const override
    {
        return value_;
    }

    bool exclaim() const override
    {
        return !value_.as_bool();
    }

    bool eq(const term<JsonT>& rhs) const override
    {
        return rhs.eq(value_);
    }

    bool eq(const JsonT& rhs) const override
    {
        return value_ == rhs;
    }

    bool ne(const term<JsonT>& rhs) const override
    {
        return rhs.ne(value_);
    }
    bool ne(const JsonT& rhs) const override
    {
        return value_ != rhs;
    }
    bool regex(const term<JsonT>& rhs) const override
    {
        return rhs.regex2(value_.as_string());
    }
    bool ampamp(const term<JsonT>& rhs) const override
    {
        return rhs.ampamp(value_);
    }
    bool ampamp(const JsonT& rhs) const override
    {
        return jsoncons::jsonpath::ampamp(value_,rhs);
    }
    bool pipepipe(const term<JsonT>& rhs) const override
    {
        return rhs.pipepipe(value_);
    }
    bool pipepipe(const JsonT& rhs) const override
    {
        return jsoncons::jsonpath::pipepipe(value_,rhs);
    }

    bool lt(const term<JsonT>& rhs) const override
    {
        return rhs.gt(value_);
    }

    bool lt(const JsonT& rhs) const override
    {
        return jsoncons::jsonpath::lt(value_,rhs);
    }

    bool gt(const term<JsonT>& rhs) const override
    {
        return rhs.lt(value_);
    }

    bool gt(const JsonT& rhs) const override
    {
        return jsoncons::jsonpath::gt(value_,rhs);
    }

    JsonT minus(const term<JsonT>& rhs) const override
    {
        return jsoncons::jsonpath::plus(rhs.unary_minus(),value_);
    }

    JsonT minus(const JsonT& rhs) const override
    {
        return jsoncons::jsonpath::minus(value_,rhs);
    }

    JsonT unary_minus() const override
    {
        return jsoncons::jsonpath::unary_minus(value_);
    }

    JsonT plus(const term<JsonT>& rhs) const override
    {
        return rhs.plus(value_);
    }

    JsonT plus(const JsonT& rhs) const override
    {
        return jsoncons::jsonpath::plus(value_,rhs);
    }
};

template <class JsonT>
class regex_term : public term<JsonT>
{
    typedef typename JsonT::char_type char_type;
    typedef typename JsonT::string_type string_type;
    string_type pattern_;
    std::regex::flag_type flags_;
public:
    regex_term(const string_type& pattern, std::regex::flag_type flags)
        : pattern_(pattern), flags_(flags)
    {
    }

    bool regex2(const string_type& subject) const override
    {
        std::basic_regex<char_type> pattern(pattern_,
                                       flags_);
        return std::regex_match(subject, pattern);
    }
};

template <class JsonT>
class path_term : public term<JsonT>
{
    typedef typename JsonT::string_type string_type;

    string_type path_;
    JsonT nodes_;
public:
    path_term(const string_type& path)
        : path_(path)
    {
    }

    void initialize(const JsonT& context_node) override
    {
        jsonpath_evaluator<JsonT> evaluator;
        evaluator.evaluate(context_node,path_);
        nodes_ = evaluator.get_values();
    }

    bool accept_single_node() const override
    {
        return nodes_.size() != 0;
    }

    JsonT evaluate_single_node() const override
    {
        return nodes_.size() == 1 ? nodes_[0] : nodes_;
    }

    bool exclaim() const override
    {
        return nodes_.size() == 0;
    }

    bool eq(const term<JsonT>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.eq(nodes_[i]);
            }
        }
        return result;
    }

    bool eq(const JsonT& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = nodes_[i] == rhs;
            }
        }
        return result;
    }

    bool ne(const term<JsonT>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.ne(nodes_[i]);
            }
        }
        return result;

    }
    bool ne(const JsonT& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = nodes_[i] != rhs;
            }
        }
        return result;
    }
    bool regex(const term<JsonT>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.regex2(nodes_[i].as_string());
            }
        }
        return result;
    }
    bool ampamp(const term<JsonT>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.ampamp(nodes_[i]);
            }
        }
        return result;
    }
    bool ampamp(const JsonT& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = jsoncons::jsonpath::ampamp(nodes_[i],rhs);
            }
        }
        return result;
    }
    bool pipepipe(const term<JsonT>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.pipepipe(nodes_[i]);
            }
        }
        return result;
    }
    bool pipepipe(const JsonT& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = jsoncons::jsonpath::pipepipe(nodes_[i],rhs);
            }
        }
        return result;
    }

    bool lt(const JsonT& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = jsoncons::jsonpath::lt(nodes_[i],rhs);
            }
        }
        return result;
    }

    bool lt(const term<JsonT>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.gt(nodes_[i]);
            }
        }
        return result;
    }

    bool gt(const JsonT& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = jsoncons::jsonpath::gt(nodes_[i],rhs);
            }
        }
        return result;
    }

    bool gt(const term<JsonT>& rhs) const override
    {
        bool result = false;
        if (nodes_.size() > 0)
        {
            result = true;
            for (size_t i = 0; result && i < nodes_.size(); ++i)
            {
                result = rhs.lt(nodes_[i]);
            }
        }
        return result;
    }

    JsonT minus(const JsonT& rhs) const override
    {
        return nodes_.size() == 1 ? jsoncons::jsonpath::minus(nodes_[0],rhs) : jsoncons::null_type();
    }

    JsonT minus(const term<JsonT>& rhs) const override
    {

        return nodes_.size() == 1 ? jsoncons::jsonpath::plus(rhs.unary_minus(),nodes_[0]) : jsoncons::null_type();
    }

    JsonT unary_minus() const override
    {
        return nodes_.size() == 1 ? jsoncons::jsonpath::unary_minus(nodes_[0]) : jsoncons::null_type();
    }

    JsonT plus(const JsonT& rhs) const override
    {
        static auto a_null = jsoncons::null_type();
        return nodes_.size() == 1 ? jsoncons::jsonpath::plus(nodes_[0],rhs) : a_null;
    }

    JsonT plus(const term<JsonT>& rhs) const override
    {
        static auto a_null = jsoncons::null_type();
        return nodes_.size() == 1 ? rhs.plus(nodes_[0]) : a_null;
    }
};

template <class JsonT>
class jsonpath_filter_parser
{
    typedef typename JsonT::string_type string_type;
    typedef typename JsonT::char_type char_type;

    size_t& line_;
    size_t& column_;
    filter_states state_;
    string_type buffer_;
    std::vector<token<JsonT>> tokens_;
    int depth_;
    const char_type* begin_input_;
    const char_type* end_input_;
    const char_type*& p_;
    filter_states pre_line_break_state_;
public:
    jsonpath_filter_parser(const char_type** expr, size_t* line,size_t* column)
        : line_(*line), column_(*column),p_(*expr)
    {
    }

    bool exists(const JsonT& context_node)
    {
        for (auto it=tokens_.begin(); it != tokens_.end(); ++it)
        {
            it->initialize(context_node);
        }
        bool result = false;

        token_stream<JsonT> ts(tokens_);
        auto e = expression(ts);
        result = e->accept_single_node();

        return result;
    }

    JsonT eval(const JsonT& context_node)
    {
        try
        {
            for (auto it=tokens_.begin(); it != tokens_.end(); ++it)
            {
                it->initialize(context_node);
            }
       
            token_stream<JsonT> ts(tokens_);
            auto e = expression(ts);
            JsonT result = e->evaluate_single_node();

            return result;
        }
        catch (const parse_exception& e)
        {
            throw parse_exception(e.code(),line_,column_);
        }
    }

    std::shared_ptr<term<JsonT>> primary(token_stream<JsonT>& ts)
    {
        auto t = ts.get();

        switch (t.type())
        {
        case token_types::left_paren:
        {
            auto expr = expression(ts);
            t = ts.get();
            if (t.type() != token_types::right_paren)
            {
                throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_expected_right_brace, jsonpath_error_category()),line_,column_);
            }
            return expr;
        }
        case token_types::term:
            return t.term_ptr();
        case token_types::exclaim:
        {
            JsonT val = primary(ts)->exclaim();
            auto expr = std::make_shared<value_term<JsonT>>(val);
            return expr;
        }
        case token_types::minus:
        {
            JsonT val = primary(ts)->unary_minus();
            auto expr = std::make_shared<value_term<JsonT>>(val);
            return expr;
        }
        default:
            throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_expected_primary, jsonpath_error_category()),line_,column_);
        }
    }

    std::shared_ptr<term<JsonT>> expression(token_stream<JsonT>& ts)
    {
        auto left = make_term(ts);
        auto t = ts.get();
        while (true)
        {
            switch (t.type())
            {
            case token_types::plus:
            {
                JsonT val = left->plus(*(make_term(ts)));
                left = std::make_shared<value_term<JsonT>>(val);
                t = ts.get();
            }
                break;
            case token_types::minus:
            {
                JsonT val = left->minus(*(make_term(ts)));
                left = std::make_shared<value_term<JsonT>>(val);
                t = ts.get();
            }
                break;
            default:
                ts.putback();
                return left;
            }
        }
        return left;
    }

    std::shared_ptr<term<JsonT>> make_term(token_stream<JsonT>& ts)
    {
        auto left = primary(ts);
        auto t = ts.get();
        while (true)
        {
            switch (t.type())
            {
            case token_types::eq:
            {
                bool e = left->eq(*(primary(ts)));
                JsonT val(e);
                left = std::make_shared<value_term<JsonT>>(val);
                t = ts.get();
            }
                break;
            case token_types::ne:
            {
                bool e = left->ne(*(primary(ts)));
                JsonT val(e);
                left = std::make_shared<value_term<JsonT>>(val);
                t = ts.get();
            }
                break;
            case token_types::regex:
                {
                    bool e = left->regex(*(primary(ts)));
                    JsonT val(e);
                    left = std::make_shared<value_term<JsonT>>(val);
                    t = ts.get();
                }
                break;
            case token_types::ampamp:
                {
                    bool e = left->ampamp(*(primary(ts)));
                    JsonT val(e);
                    left = std::make_shared<value_term<JsonT>>(val);
                    t = ts.get();
                }
                break;
            case token_types::pipepipe:
                {
                    bool e = left->pipepipe(*(primary(ts)));
                    JsonT val(e);
                    left = std::make_shared<value_term<JsonT>>(val);
                    t = ts.get();
                }
                break;
            case token_types::lt:
                {
                    bool e = left->lt(*(primary(ts)));
                    JsonT val(e);
                    left = std::make_shared<value_term<JsonT>>(val);
                    t = ts.get();
                }
                break;
            case token_types::gt:
                {
                    bool e = left->gt(*(primary(ts)));
                    JsonT val(e);
                    left = std::make_shared<value_term<JsonT>>(val);
                    t = ts.get();
                }
                break;
            case token_types::lte:
                {
                    bool e = left->lt(*(primary(ts))) || left->eq(*(primary(ts)));
                    JsonT val(e);
                    left = std::make_shared<value_term<JsonT>>(val);
                    t = ts.get();
                }
                break;
            case token_types::gte:
                {
                    bool e = left->gt(*(primary(ts))) || left->eq(*(primary(ts)));
                    JsonT val(e);
                    left = std::make_shared<value_term<JsonT>>(val);
                    t = ts.get();
                }
                break;
            default:
                ts.putback();
                return left;
            }
        }
    }

    void parse(const char_type* expr, size_t length)
    {
        parse(expr,expr+length);
    }

    void parse(const char_type* expr, const char_type* end_expr)
    {
        p_ = expr;
        end_input_ = end_expr;
        depth_ = 0;
        tokens_.clear();
        state_ = filter_states::start;
        bool done = false;
        while (!done && p_ < end_input_)
        {
            switch (state_)
            {
            case filter_states::cr:
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
            case filter_states::lf:
                ++line_;
                column_ = 1;
                state_ = pre_line_break_state_;
                break;
            case filter_states::start:
                switch (*p_)
                {
                case '\r':
                case '\n':
                    pre_line_break_state_ = state_;
                    state_ = filter_states::lf;
                    break;
                case '(':
                    state_ = filter_states::expect_path_or_value;
                    ++depth_;
                    tokens_.push_back(token<JsonT>(token_types::left_paren));
                    break;
                case ')':
                    tokens_.push_back(token<JsonT>(token_types::right_paren));
                    if (--depth_ == 0)
                    {
                        done = true;
                    }
                    break;
                }
                ++p_;
                ++column_;
                break;
            case filter_states::oper:
                switch (*p_)
                {
                case '\r':
                case '\n':
                    ++line_;
                    column_ = 1;
                    state_ = pre_line_break_state_;
                    break;
                case '!':
                    if (p_+1  < end_input_ && *(p_+1) == '=')
                    {
                        ++p_;
                        ++column_;
                        state_ = filter_states::expect_path_or_value;
                        tokens_.push_back(token<JsonT>(token_types::ne));
                    }
                    else
                    {
                        state_ = filter_states::expect_path_or_value;
                        tokens_.push_back(token<JsonT>(token_types::exclaim));
                    }
                    break;
                case '&':
                    if (p_+1  < end_input_ && *(p_+1) == '&')
                    {
                        ++p_;
                        ++column_;
                        state_ = filter_states::expect_path_or_value;
                        tokens_.push_back(token<JsonT>(token_types::ampamp));
                    }
                    break;
                case '|':
                    if (p_+1  < end_input_ && *(p_+1) == '|')
                    {
                        ++p_;
                        ++column_;
                        state_ = filter_states::expect_path_or_value;
                        tokens_.push_back(token<JsonT>(token_types::pipepipe));
                    }
                    break;
                case '=':
                    if (p_+1  < end_input_ && *(p_+1) == '=')
                    {
                        ++p_;
                        ++column_;
                        state_ = filter_states::expect_path_or_value;
                        tokens_.push_back(token<JsonT>(token_types::eq));
                    }
                    else if (p_+1  < end_input_ && *(p_+1) == '~')
                    {
                        ++p_;
                        ++column_;
                        state_ = filter_states::expect_regex;
                        tokens_.push_back(token<JsonT>(token_types::regex));
                    }
                    break;
                case '>':
                    if (p_+1  < end_input_ && *(p_+1) == '=')
                    {
                        ++p_;
                        ++column_;
                        state_ = filter_states::expect_path_or_value;
                        tokens_.push_back(token<JsonT>(token_types::gte));
                    }
                    else
                    {
                        state_ = filter_states::expect_path_or_value;
                        tokens_.push_back(token<JsonT>(token_types::gt));
                    }
                    break;
                case '<':
                    if (p_+1  < end_input_ && *(p_+1) == '=')
                    {
                        ++p_;
                        ++column_;
                        state_ = filter_states::expect_path_or_value;
                        tokens_.push_back(token<JsonT>(token_types::lte));
                    }
                    else
                    {
                        state_ = filter_states::expect_path_or_value;
                        tokens_.push_back(token<JsonT>(token_types::lt));
                    }
                    break;
                case '+':
                    state_ = filter_states::expect_path_or_value;
                    tokens_.push_back(token<JsonT>(token_types::plus));
                    break;
                case '-':
                    state_ = filter_states::expect_path_or_value;
                    tokens_.push_back(token<JsonT>(token_types::minus));
                    break;
                case ' ':case '\t':
                    break;
                default:
                    throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter, jsonpath_error_category()),line_,column_);
                    break;

                }
                ++p_;
                ++column_;
                break;
            case filter_states::unquoted_text: 
                {
                    switch (*p_)
                    {
                    case '\r':
                    case '\n':
                        ++line_;
                        column_ = 1;
                        state_ = pre_line_break_state_;
                        break;
                    case '<':
                    case '>':
                    case '!':
                    case '=':
                    case '&':
                    case '|':
                    case '+':
                    case '-':
                        {
                            if (buffer_.length() > 0)
                            {
                                try
                                {
                                    auto val = JsonT::parse(buffer_);
                                    tokens_.push_back(token<JsonT>(token_types::term,std::make_shared<value_term<JsonT>>(val)));
                                }
                                catch (const parse_exception& e)
                                {
                                    throw parse_exception(e.code(),line_,column_);
                                }
                                buffer_.clear();
                            }
                            state_ = filter_states::oper;
                        }
                        break;
                    case ')':
                        if (buffer_.length() > 0)
                        {
                            try
                            {
                                auto val = JsonT::parse(buffer_);
                                tokens_.push_back(token<JsonT>(token_types::term,std::make_shared<value_term<JsonT>>(val)));
                            }
                            catch (const parse_exception& e)
                            {
                                throw parse_exception(e.code(),line_,column_);
                            }
                            buffer_.clear();
                        }
                        tokens_.push_back(token<JsonT>(token_types::right_paren));
                        if (--depth_ == 0)
                        {
                            state_ = filter_states::start;
                            done = true;
                        }
                        else
                        {
                            state_ = filter_states::expect_path_or_value;
                        }
                        ++p_;
                        ++column_;
                        break;
                    case ' ':case '\t':
                        if (buffer_.length() > 0)
                        {
                            try
                            {
                                auto val = JsonT::parse(buffer_);
                                tokens_.push_back(token<JsonT>(token_types::term,std::make_shared<value_term<JsonT>>(val)));
                            }
                            catch (const parse_exception& e)
                            {
                                throw parse_exception(e.code(),line_,column_);
                            }
                            buffer_.clear();
                        }
                        ++p_;
                        ++column_;
                        break; 
                    default: 
                        buffer_.push_back(*p_);
                        ++p_;
                        ++column_;
                        break;
                    }
                }
                break;
            case filter_states::single_quoted_text: 
                {
                    switch (*p_)
                    {                   
                    case '\r':
                    case '\n':
                        ++line_;
                        column_ = 1;
                        state_ = pre_line_break_state_;
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
                    case '\'':
                        buffer_.push_back('\"');
                        //if (buffer_.length() > 0)
                        {
                            try
                            {
                                auto val = JsonT::parse(buffer_);
                                tokens_.push_back(token<JsonT>(token_types::term,std::make_shared<value_term<JsonT>>(val)));
                            }
                            catch (const parse_exception& e)
                            {
                                throw parse_exception(e.code(),line_,column_);
                            }
                            buffer_.clear();
                        }
                        state_ = filter_states::expect_path_or_value;
                        break;

                    default: 
                        buffer_.push_back(*p_);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case filter_states::double_quoted_text: 
                {
                    switch (*p_)
                    {                   
                    case '\r':
                    case '\n':
                        ++line_;
                        column_ = 1;
                        state_ = pre_line_break_state_;
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
                    case '\"':
                        buffer_.push_back(*p_);
                        //if (buffer_.length() > 0)
                        {
                            try
                            {
                                auto val = JsonT::parse(buffer_);
                                tokens_.push_back(token<JsonT>(token_types::term,std::make_shared<value_term<JsonT>>(val)));
                            }
                            catch (const parse_exception& e)
                            {
                                throw parse_exception(e.code(),line_,column_);
                            }
                            buffer_.clear();
                        }
                        state_ = filter_states::expect_path_or_value;
                        break;

                    default: 
                        buffer_.push_back(*p_);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            case filter_states::expect_path_or_value: 
                switch (*p_)
                {
                case '\r':
                case '\n':
                    ++line_;
                    column_ = 1;
                    state_ = pre_line_break_state_;
                    break;
                case '<':
                case '>':
                case '!':
                case '=':
                case '&':
                case '|':
                case '+':
                case '-':
                    state_ = filter_states::oper;
                    // don't increment
                    break;
                case '@':
                    buffer_.push_back(*p_);
                    state_ = filter_states::path;
                    ++p_;
                    ++column_;
                    break;
                case ' ':case '\t':
                    ++p_;
                    ++column_;
                    break;
                case '\'':
                    buffer_.push_back('\"');
                    state_ = filter_states::single_quoted_text;
                    ++p_;
                    ++column_;
                    break;
                case '\"':
                    buffer_.push_back(*p_);
                    state_ = filter_states::double_quoted_text;
                    ++p_;
                    ++column_;
                    break;
                case '(':
                    ++depth_;
                    tokens_.push_back(token<JsonT>(token_types::left_paren));
                    ++p_;
                    ++column_;
                    break;
                case ')':
                    tokens_.push_back(token<JsonT>(token_types::right_paren));
                    if (--depth_ == 0)
                    {
                        done = true;
                        state_ = filter_states::start;
                    }
                    ++p_;
                    ++column_;
                    break;
                default: 
                    // don't increment
                    state_ = filter_states::unquoted_text;
                    break;
                };
                break;
            case filter_states::expect_oper_or_right_round_bracket: 
                switch (*p_)
                {
                case '\r':
                case '\n':
                    ++line_;
                    column_ = 1;
                    state_ = pre_line_break_state_;
                    break;
                case ' ':case '\t':
                    break;
                case ')':
                    tokens_.push_back(token<JsonT>(token_types::right_paren));
                    if (--depth_ == 0)
                    {
                        done = true;
                        state_ = filter_states::start;
                    }
                    break;
                case '<':
                case '>':
                case '!':
                case '=':
                case '&':
                case '|':
                case '+':
                case '-':
                    {
                        state_ = filter_states::oper;
                        // don't increment p
                    }
                    break;
                default: 
                    throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter, jsonpath_error_category()),line_,column_);
                    break;
                };
                break;
            case filter_states::expect_right_round_bracket: 
                switch (*p_)
                {
                case '\r':
                case '\n':
                    ++line_;
                    column_ = 1;
                    state_ = pre_line_break_state_;
                    break;
                case ' ':case '\t':
                    break;
                case ')':
                    tokens_.push_back(token<JsonT>(token_types::right_paren));
                    if (--depth_ == 0)
                    {
                        done = true;
                        state_ = filter_states::start;
                    }
                    else 
                    {
                        state_ = filter_states::expect_oper_or_right_round_bracket;
                    }
                    break;
                default: 
                    throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter, jsonpath_error_category()),line_,column_);
                    break;
                };
                ++p_;
                ++column_;
                break;
            case filter_states::path: 
                switch (*p_)
                {
                case '\r':
                case '\n':
                    ++line_;
                    column_ = 1;
                    state_ = pre_line_break_state_;
                    break;
                case '<':
                case '>':
                case '!':
                case '=':
                case '&':
                case '|':
                case '+':
                case '-':
                    {
                        if (buffer_.length() > 0)
                        {
                            tokens_.push_back(token<JsonT>(token_types::term,std::make_shared<path_term<JsonT>>(buffer_)));
                            buffer_.clear();
                        }
                        state_ = filter_states::oper;
                        // don't increment
                    }
                    break;
                case ')':
                    if (buffer_.length() > 0)
                    {
                        tokens_.push_back(token<JsonT>(token_types::term,std::make_shared<path_term<JsonT>>(buffer_)));
                        tokens_.push_back(token<JsonT>(token_types::right_paren));
                        buffer_.clear();
                    }
                    if (--depth_ == 0)
                    {
                        state_ = filter_states::start;
                        done = true;
                    }
                    else
                    {
                        state_ = filter_states::expect_path_or_value;
                    }
                    ++p_;
                    ++column_;
                    break;
                default:
                    buffer_.push_back(*p_);
                    ++p_;
                    ++column_;
                    break;
                };
                break;
            case filter_states::expect_regex: 
                switch (*p_)
                {
                case '\r':
                case '\n':
                    ++line_;
                    column_ = 1;
                    state_ = pre_line_break_state_;
                    break;
                case '/':
                    state_ = filter_states::regex;
                    break;
                case ' ':case '\t':
                    break;
                default: 
                    throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_expected_slash, jsonpath_error_category()),line_,column_);
                    break;
                };
                ++p_;
                ++column_;
                break;
            case filter_states::regex: 
                {
                    switch (*p_)
                    {                   
                    case '\r':
                    case '\n':
                        ++line_;
                        column_ = 1;
                        state_ = pre_line_break_state_;
                        break;
                    case '/':
                        //if (buffer_.length() > 0)
                        {
                            std::regex::flag_type flags = std::regex_constants::ECMAScript; 
                            if (p_+1  < end_input_ && *(p_+1) == 'i')
                            {
                                ++p_;
                                ++column_;
                                flags |= std::regex_constants::icase;
                            }
                            tokens_.push_back(token<JsonT>(token_types::term,std::make_shared<regex_term<JsonT>>(buffer_,flags)));
                            buffer_.clear();
                        }
                        state_ = filter_states::expect_path_or_value;
                        break;

                    default: 
                        buffer_.push_back(*p_);
                        break;
                    }
                }
                ++p_;
                ++column_;
                break;
            default:
                ++p_;
                ++column_;
                break;
            }
        }
        if (depth_ != 0)
        {
            throw parse_exception(std::error_code(jsonpath_parser_errc::invalid_filter_unbalanced_paren, jsonpath_error_category()),line_,column_);
        }
    }
};


}}
#endif
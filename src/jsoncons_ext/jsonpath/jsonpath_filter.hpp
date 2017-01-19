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
#include <jsoncons/json.hpp>
#include "jsonpath_error_category.hpp"

namespace jsoncons { namespace jsonpath {

template <class Json,
          class JsonReference,
          class JsonPointer>
class jsonpath_evaluator;

enum class filter_states
{
    start,
    cr,
    lf,
    expect_right_round_bracket,
    expect_oper_or_right_round_bracket,
    expect_path_or_value_or_unary_op,
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
    operand,
    unary_minus,
    exclaim,
    lparen,
    rparen,
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
    mult,
    div
};

inline
size_t precedence(token_types val)
{
    switch (val)
    {
    case token_types::exclaim:
    case token_types::unary_minus:
        return 1;
    case token_types::regex:
        return 2;
    case token_types::mult:
    case token_types::div:
        return 3;
    case token_types::plus:
    case token_types::minus:
        return 4;
    case token_types::lt:
    case token_types::gt:
    case token_types::lte:
    case token_types::gte:
        return 5;
    case token_types::eq:
    case token_types::ne:
        return 6;
    case token_types::ampamp:
        return 7;
    case token_types::pipepipe:
        return 8;
    default:
        return 0;
    }
}

inline
bool is_right_associative(token_types val)
{
    switch (val)
    {
    case token_types::exclaim:
    case token_types::unary_minus:
        return true;
    default:
        return false;
    }
}

inline
bool is_operator(token_types val)
{
    switch (val)
    {
    case token_types::eq:
    case token_types::ne:
    case token_types::regex:
    case token_types::ampamp:
    case token_types::pipepipe:
    case token_types::lt:
    case token_types::gt:
    case token_types::lte:
    case token_types::gte:
    case token_types::plus:
    case token_types::minus:
    case token_types::mult:
    case token_types::div:
    case token_types::exclaim:
    case token_types::unary_minus:
        return true;
    default:
        return false;
    }
}

inline
bool is_operand(token_types val)
{
    return val == token_types::operand;
}

template <class Json>
class term
{
public:
    typedef typename Json::string_type string_type;
    typedef typename Json::char_type char_type;

    virtual ~term() {}

    virtual void initialize(const Json&)
    {
    }
    virtual bool accept_single_node() const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json evaluate_single_node() const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool exclaim() const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool eq(const term&) const 
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool eq(const Json&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool ne(const term&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool ne(const Json&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool regex(const term&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool regex2(const string_type&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool ampamp(const term&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool ampamp(const Json&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool pipepipe(const term&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool pipepipe(const Json&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool lt(const term&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool lt(const Json&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool gt(const term&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool gt(const Json&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }

    virtual Json minus(const term&) const 
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json  minus(const Json&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }

    virtual Json left_minus(const term&) const 
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json left_minus(const Json&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }

    virtual Json unary_minus() const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json plus(const term&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json  plus(const Json&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json mult(const term&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json mult(const Json&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }

    virtual Json div(const term&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json div(const Json&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }

    virtual Json left_div(const term&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json left_div(const Json&) const
    {
        throw parse_exception(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
};

template <class Json>
class token
{
    token_types type_;
    std::shared_ptr<term<Json>> operand_ptr_;
public:
    token(token_types type)
        : type_(type)
    {
    }
    token(token_types type, std::shared_ptr<term<Json>> term_ptr)
        : type_(type), operand_ptr_(term_ptr)
    {
    }
    token(const token& t)
        : type_(t.type_), operand_ptr_(t.operand_ptr_)
    {
    }

    token<Json>& operator=(const token<Json>& val)
    {
        type_ = val.type_;
        operand_ptr_ = val.operand_ptr_;
        return *this;
    }

    token_types type_id() const
    {
        return type_;
    }

    const term<Json>& operand()
    {
        JSONCONS_ASSERT(type_ == token_types::operand && operand_ptr_ != nullptr);
        return *operand_ptr_;
    }

    void initialize(const Json& context_node)
    {
        if (operand_ptr_.get() != nullptr)
        {
            operand_ptr_->initialize(context_node);
        }
    }
};

template <class Json>
bool ampamp(const Json& lhs, const Json& rhs)
{
    return lhs.as_bool() && rhs.as_bool();
}

template <class Json>
bool pipepipe(const Json& lhs, const Json& rhs)
{
    return lhs.as_bool() || rhs.as_bool();
}

template <class Json>
bool lt(const Json& lhs, const Json& rhs)
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
        result = lhs.as_string_view() < rhs.as_string_view();
    }
    return result;
}

template <class Json>
bool gt(const Json& lhs, const Json& rhs)
{
    return lt(rhs,lhs);
}

template <class Json>
Json plus(const Json& lhs, const Json& rhs)
{
    Json result = Json(jsoncons::null_type());
    if (lhs.is_integer() && rhs.is_integer())
    {
        result = Json(((lhs.as_integer() + rhs.as_integer())));
    }
    else if ((lhs.is_number() && rhs.is_double()) || (lhs.is_double() && rhs.is_number()))
    {
        result = Json((lhs.as_double() + rhs.as_double()));
    }
    else if (lhs.is_uinteger() && rhs.is_uinteger())
    {
        result = Json((lhs.as_uinteger() + rhs.as_uinteger()));
    }
    return result;
}

template <class Json>
Json mult(const Json& lhs, const Json& rhs)
{
    Json result = Json(jsoncons::null_type());
    if (lhs.is_integer() && rhs.is_integer())
    {
        result = Json(((lhs.as_integer() * rhs.as_integer())));
    }
    else if ((lhs.is_number() && rhs.is_double()) || (lhs.is_double() && rhs.is_number()))
    {
        result = Json((lhs.as_double() * rhs.as_double()));
    }
    else if (lhs.is_uinteger() && rhs.is_uinteger())
    {
        result = Json((lhs.as_uinteger() * rhs.as_uinteger()));
    }
    return result;
}

template <class Json>
Json div(const Json& lhs, const Json& rhs)
{
    Json result = Json(jsoncons::null_type());
    if (lhs.is_integer() && rhs.is_integer())
    {
        result = Json((double)(lhs.as_integer() / (double)rhs.as_integer()));
    }
    else if ((lhs.is_number() && rhs.is_double()) || (lhs.is_double() && rhs.is_number()))
    {
        result = Json((lhs.as_double() / rhs.as_double()));
    }
    else if (lhs.is_uinteger() && rhs.is_uinteger())
    {
        result = Json((double)(lhs.as_uinteger() / (double)rhs.as_uinteger()));
    }
    return result;
}

template <class Json>
Json unary_minus(const Json& lhs)
{
    Json result = Json::null();
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

template <class Json>
Json minus(const Json& lhs, const Json& rhs)
{
    Json result = Json::null();
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

template <class Json>
class value_term : public term<Json>
{
    Json value_;
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

    Json evaluate_single_node() const override
    {
        return value_;
    }

    bool exclaim() const override
    {
        return !value_.as_bool();
    }

    bool eq(const term<Json>& rhs) const override
    {
        return rhs.eq(value_);
    }

    bool eq(const Json& rhs) const override
    {
        return value_ == rhs;
    }

    bool ne(const term<Json>& rhs) const override
    {
        return rhs.ne(value_);
    }
    bool ne(const Json& rhs) const override
    {
        return value_ != rhs;
    }
    bool regex(const term<Json>& rhs) const override
    {
        return rhs.regex2(value_.as_string());
    }
    bool ampamp(const term<Json>& rhs) const override
    {
        return rhs.ampamp(value_);
    }
    bool ampamp(const Json& rhs) const override
    {
        return jsoncons::jsonpath::ampamp(value_,rhs);
    }
    bool pipepipe(const term<Json>& rhs) const override
    {
        return rhs.pipepipe(value_);
    }
    bool pipepipe(const Json& rhs) const override
    {
        return jsoncons::jsonpath::pipepipe(value_,rhs);
    }

    bool lt(const term<Json>& rhs) const override
    {
        return rhs.gt(value_);
    }
    bool lt(const Json& rhs) const override
    {
        return jsoncons::jsonpath::lt(value_,rhs);
    }

    bool gt(const term<Json>& rhs) const override
    {
        return rhs.lt(value_);
    }
    bool gt(const Json& rhs) const override
    {
        return jsoncons::jsonpath::gt(value_,rhs);
    }

    Json minus(const term<Json>& rhs) const override
    {
        return rhs.left_minus(value_);
    }
    Json minus(const Json& rhs) const override
    {
        return jsoncons::jsonpath::minus(value_,rhs);
    }

    Json left_minus(const term<Json>& lhs) const override
    {
        return lhs.minus(value_);
    }
    Json left_minus(const Json& lhs) const override
    {
        return jsoncons::jsonpath::minus(lhs,value_);
    }

    Json unary_minus() const override
    {
        return jsoncons::jsonpath::unary_minus(value_);
    }

    Json plus(const term<Json>& rhs) const override
    {
        return rhs.plus(value_);
    }

    Json plus(const Json& rhs) const override
    {
        return jsoncons::jsonpath::plus(value_,rhs);
    }
    Json mult(const term<Json>& rhs) const override
    {
        return rhs.mult(value_);
    }

    Json mult(const Json& rhs) const override
    {
        return jsoncons::jsonpath::mult(value_,rhs);
    }

    Json div(const term<Json>& rhs) const override
    {
        return rhs.left_div(value_);
    }
    Json div(const Json& rhs) const override
    {
        return jsoncons::jsonpath::div(value_,rhs);
    }

    Json left_div(const term<Json>& lhs) const override
    {
        return lhs.div(value_);
    }
    Json left_div(const Json& lhs) const override
    {
        return jsoncons::jsonpath::div(lhs,value_);
    }
};

template <class Json>
class regex_term : public term<Json>
{
    typedef typename Json::char_type char_type;
    typedef typename Json::string_type string_type;
    const std::basic_regex<char_type> pattern_;
public:
    regex_term(const string_type& pattern, std::regex::flag_type flags)
        : pattern_(pattern,flags)
    {
    }

    bool regex2(const string_type& subject) const override
    {
        return std::regex_match(subject, pattern_);
    }
};

template <class Json>
class path_term : public term<Json>
{
    typedef typename Json::string_type string_type;

    string_type path_;
    Json nodes_;
public:
    path_term(const string_type& path)
        : path_(path)
    {
    }

    void initialize(const Json& context_node) override
    {
        jsonpath_evaluator<Json,const Json&,const Json*> evaluator;
        evaluator.evaluate(context_node,path_);
        nodes_ = evaluator.get_values();
    }

    bool accept_single_node() const override
    {
        return nodes_.size() != 0;
    }

    Json evaluate_single_node() const override
    {
        return nodes_.size() == 1 ? nodes_[0] : nodes_;
    }

    bool exclaim() const override
    {
        return nodes_.size() == 0;
    }

    bool eq(const term<Json>& rhs) const override
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

    bool eq(const Json& rhs) const override
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

    bool ne(const term<Json>& rhs) const override
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
    bool ne(const Json& rhs) const override
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
    bool regex(const term<Json>& rhs) const override
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
    bool ampamp(const term<Json>& rhs) const override
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
    bool ampamp(const Json& rhs) const override
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
    bool pipepipe(const term<Json>& rhs) const override
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
    bool pipepipe(const Json& rhs) const override
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

    bool lt(const Json& rhs) const override
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

    bool lt(const term<Json>& rhs) const override
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

    bool gt(const Json& rhs) const override
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

    bool gt(const term<Json>& rhs) const override
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

    Json minus(const term<Json>& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? rhs.left_minus(nodes_[0]) : a_null;
    }
    Json minus(const Json& rhs) const override
    {
        return nodes_.size() == 1 ? jsoncons::jsonpath::minus(nodes_[0],rhs) : Json(jsoncons::null_type());
    }

    Json left_minus(const term<Json>& lhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? lhs.left_minus(nodes_[0]) : a_null;
    }
    Json left_minus(const Json& lhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? jsoncons::jsonpath::minus(lhs,nodes_[0]) : a_null;
    }

    Json unary_minus() const override
    {
        return nodes_.size() == 1 ? jsoncons::jsonpath::unary_minus(nodes_[0]) : Json::null();
    }

    Json plus(const Json& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? jsoncons::jsonpath::plus(nodes_[0],rhs) : a_null;
    }

    Json plus(const term<Json>& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? rhs.plus(nodes_[0]) : a_null;
    }

    Json mult(const Json& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? jsoncons::jsonpath::mult(nodes_[0],rhs) : a_null;
    }

    Json mult(const term<Json>& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? rhs.mult(nodes_[0]) : a_null;
    }

    Json div(const term<Json>& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? rhs.left_div(nodes_[0]) : a_null;
    }
    Json div(const Json& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? jsoncons::jsonpath::div(nodes_[0],rhs) : a_null;
    }

    Json left_div(const term<Json>& lhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? lhs.left_div(nodes_[0]) : a_null;
    }
    Json left_div(const Json& lhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? jsoncons::jsonpath::div(lhs, nodes_[0]) : a_null;
    }
};

template <class Json>
token<Json> evaluate(const Json& context, std::vector<token<Json>>& tokens)
{
    for (auto it= tokens.begin(); it != tokens.end(); ++it)
    {
        it->initialize(context);
    }
    std::vector<token<Json>> stack;
    for (auto t : tokens)
    {
        if (is_operand(t.type_id()))
        {
            stack.push_back(t);
        }
        else if (is_operator(t.type_id()))
        {
            auto rhs = stack.back();
            stack.pop_back();
            switch (t.type_id())
            {
                case token_types::exclaim:
                {
                    Json val = rhs.operand().exclaim();
                    stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    break;
                }
                case token_types::unary_minus:
                {
                    Json val = rhs.operand().unary_minus();
                    stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    break;
                }
                case token_types::plus:
                {
                    auto lhs = stack.back();
                    stack.pop_back();
                    Json val = lhs.operand().plus(rhs.operand());
                    stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    break;
                }
                case token_types::minus:
                {
                    auto lhs = stack.back();
                    stack.pop_back();
                    Json val = lhs.operand().minus(rhs.operand());
                    stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    break;
                }
                case token_types::mult:
                {
                    auto lhs = stack.back();
                    stack.pop_back();
                    Json val = lhs.operand().mult(rhs.operand());
                    stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    break;
                }
                case token_types::div:
                {
                    auto lhs = stack.back();
                    stack.pop_back();
                    Json val = lhs.operand().div(rhs.operand());
                    stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    break;
                }
                case token_types::eq:
                {
                    auto lhs = stack.back();
                    stack.pop_back();
                    bool e = lhs.operand().eq(rhs.operand());
                    Json val(e);
                    stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                }
                    break;
                case token_types::ne:
                {
                    auto lhs = stack.back();
                    stack.pop_back();
                    bool e = lhs.operand().ne(rhs.operand());
                    Json val(e);
                    stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                }
                    break;
                case token_types::regex:
                    {
                        auto lhs = stack.back();
                        stack.pop_back();
                        bool e = lhs.operand().regex(rhs.operand());
                        Json val(e);
                        stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    }
                    break;
                case token_types::ampamp:
                    {
                        auto lhs = stack.back();
                        stack.pop_back();
                        bool e = lhs.operand().ampamp(rhs.operand());
                        Json val(e);
                        stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    }
                    break;
                case token_types::pipepipe:
                    {
                        auto lhs = stack.back();
                        stack.pop_back();
                        bool e = lhs.operand().pipepipe(rhs.operand());
                        Json val(e);
                        stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    }
                    break;
                case token_types::lt:
                    {
                        auto lhs = stack.back();
                        stack.pop_back();
                        bool e = lhs.operand().lt(rhs.operand());
                        Json val(e);
                        stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    }
                    break;
                case token_types::gt:
                    {
                        auto lhs = stack.back();
                        stack.pop_back();
                        bool e = lhs.operand().gt(rhs.operand());
                        Json val(e);
                        stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    }
                    break;
                case token_types::lte:
                    {
                        auto lhs = stack.back();
                        stack.pop_back();
                        bool e = lhs.operand().lt(rhs.operand()) || lhs.operand().eq(rhs.operand());
                        Json val(e);
                        stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    }
                    break;
                case token_types::gte:
                {
                    auto lhs = stack.back();
                    stack.pop_back();
                    bool e = lhs.operand().gt(rhs.operand()) || lhs.operand().eq(rhs.operand());
                    Json val(e);
                    stack.push_back(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                    break;
                }
                default:
                    {
                        //throw std::runtime_error("op not found");
                        break;
                    }
            }
        }
    }
    if (stack.size() != 1)
    {
        throw std::runtime_error("Invalid state");
    }

    return stack.back();
}

template <class Json>
class jsonpath_filter_expr
{
public:
    std::vector<token<Json>> tokens_;
    size_t line_;
    size_t column_;
public:

    jsonpath_filter_expr(const std::vector<token<Json>>& tokens, size_t line, size_t column)
        : tokens_(tokens), line_(line), column_(column)
    {
    }

    Json eval(const Json& context_node)
    {
        try
        {
            auto t = evaluate(context_node,tokens_);

            return t.operand().evaluate_single_node();

        }
        catch (const parse_exception& e)
        {
            throw parse_exception(e.code(),line_,column_);
        }
    }

    bool exists(const Json& context_node)
    {
        try
        {
            auto t = evaluate(context_node,tokens_);
            return t.operand().accept_single_node();
        }
        catch (const parse_exception& e)
        {
            throw parse_exception(e.code(),line_,column_);
        }
    }
};

template <class Json>
class jsonpath_filter_parser
{
    typedef typename Json::string_type string_type;
    typedef typename Json::char_type char_type;

    std::vector<token<Json>> output_stack_;
    std::vector<token<Json>> operator_stack_;
    size_t line_;
    size_t column_;
public:
    jsonpath_filter_parser()
        : line_(1), column_(1)
    {
    }
    jsonpath_filter_parser(size_t line, size_t column)
        : line_(line), column_(column)
    {
    }

    size_t line() const
    {
        return line_;
    }

    size_t column() const
    {
        return column_;
    }

    jsonpath_filter_expr<Json> parse(const char_type* p, size_t length, const char_type** end_ptr)
    {
        return parse(p,p+length, end_ptr);
    }

    void add_token(token<Json> token)
    {
        if (is_operand(token.type_id()))
        {
            output_stack_.push_back(token);
        }
        else if (token.type_id() == token_types::lparen)
        {
            operator_stack_.push_back(token);
        }
        else if (token.type_id() == token_types::rparen)
        {
            auto it = operator_stack_.rbegin();
            while (it != operator_stack_.rend() && it->type_id() != token_types::lparen)
            {
                output_stack_.push_back(*it);
                ++it;
            }
            if (it == operator_stack_.rend())
            {
                throw std::runtime_error("Unbalanced parenthesis");
            }
            operator_stack_.erase(it.base(),operator_stack_.end());
            operator_stack_.pop_back();
        }
        else if (is_operator(token.type_id()))
        {
            if (operator_stack_.empty() || operator_stack_.back().type_id() == token_types::lparen)
            {
                operator_stack_.push_back(token);
            }
            else if (precedence(token.type_id()) < precedence(operator_stack_.back().type_id())
                     || (precedence(token.type_id()) == precedence(operator_stack_.back().type_id()) && is_right_associative(token.type_id())))
            {
                operator_stack_.push_back(token);
            }
            else
            {
                auto it = operator_stack_.rbegin();
                while (it != operator_stack_.rend() && is_operator(it->type_id())
                       && (precedence(token.type_id()) > precedence(it->type_id())
                     || (precedence(token.type_id()) == precedence(it->type_id()) && is_right_associative(token.type_id()))))
                {
                    output_stack_.push_back(*it);
                    ++it;
                }

                operator_stack_.erase(it.base(),operator_stack_.end());
                operator_stack_.push_back(token);
            }
        }
    }

    jsonpath_filter_expr<Json> parse(const char_type* p, const char_type* end_expr, const char_type** end_ptr)
    {
        output_stack_.clear();
        operator_stack_.clear();

        string_type buffer;
        filter_states pre_line_break_state = filter_states::start;

        int depth = 0;
        filter_states state = filter_states::start;
        bool done = false;
        while (!done && p < end_expr)
        {
            switch (state)
            {
            case filter_states::cr:
                ++line_;
                column_ = 1;
                switch (*p)
                {
                case '\n':
                    state = pre_line_break_state;
                    ++p;
                    ++column_;
                    break;
                default:
                    state = pre_line_break_state;
                    break;
                }
                break;
            case filter_states::lf:
                ++line_;
                column_ = 1;
                state = pre_line_break_state;
                break;
            case filter_states::start:
                switch (*p)
                {
                case '\r':
                case '\n':
                    pre_line_break_state = state;
                    state = filter_states::lf;
                    break;
                case '(':
                    state = filter_states::expect_path_or_value_or_unary_op;
                    ++depth;
                    add_token(token<Json>(token_types::lparen));
                    break;
                case ')':
                    add_token(token<Json>(token_types::rparen));
                    if (--depth == 0)
                    {
                        done = true;
                    }
                    break;
                }
                ++p;
                ++column_;
                break;
            case filter_states::oper:
                switch (*p)
                {
                case '\r':
                case '\n':
                    ++line_;
                    column_ = 1;
                    state = pre_line_break_state;
                    break;
                case '!':
                    if (p+1  < end_expr && *(p+1) == '=')
                    {
                        ++p;
                        ++column_;
                        state = filter_states::expect_path_or_value_or_unary_op;
                        add_token(token<Json>(token_types::ne));
                    }
                    else
                    {
                        throw parse_exception(jsonpath_parser_errc::unexpected_operator,line_,column_);
                    }
                    break;
                case '&':
                    if (p+1  < end_expr && *(p+1) == '&')
                    {
                        ++p;
                        ++column_;
                        state = filter_states::expect_path_or_value_or_unary_op;
                        add_token(token<Json>(token_types::ampamp));
                    }
                    break;
                case '|':
                    if (p+1  < end_expr && *(p+1) == '|')
                    {
                        ++p;
                        ++column_;
                        state = filter_states::expect_path_or_value_or_unary_op;
                        add_token(token<Json>(token_types::pipepipe));
                    }
                    break;
                case '=':
                    if (p+1  < end_expr && *(p+1) == '=')
                    {
                        ++p;
                        ++column_;
                        state = filter_states::expect_path_or_value_or_unary_op;
                        add_token(token<Json>(token_types::eq));
                    }
                    else if (p+1  < end_expr && *(p+1) == '~')
                    {
                        ++p;
                        ++column_;
                        state = filter_states::expect_regex;
                        add_token(token<Json>(token_types::regex));
                    }
                    break;
                case '>':
                    if (p+1  < end_expr && *(p+1) == '=')
                    {
                        ++p;
                        ++column_;
                        state = filter_states::expect_path_or_value_or_unary_op;
                        add_token(token<Json>(token_types::gte));
                    }
                    else
                    {
                        state = filter_states::expect_path_or_value_or_unary_op;
                        add_token(token<Json>(token_types::gt));
                    }
                    break;
                case '<':
                    if (p+1  < end_expr && *(p+1) == '=')
                    {
                        ++p;
                        ++column_;
                        state = filter_states::expect_path_or_value_or_unary_op;
                        add_token(token<Json>(token_types::lte));
                    }
                    else
                    {
                        state = filter_states::expect_path_or_value_or_unary_op;
                        add_token(token<Json>(token_types::lt));
                    }
                    break;
                case '+':
                    state = filter_states::expect_path_or_value_or_unary_op;
                    add_token(token<Json>(token_types::plus));
                    break;
                case '-':
                    state = filter_states::expect_path_or_value_or_unary_op;
                    add_token(token<Json>(token_types::minus));
                    break;
                case '*':
                    state = filter_states::expect_path_or_value_or_unary_op;
                    add_token(token<Json>(token_types::mult));
                    break;
                case '/':
                    state = filter_states::expect_path_or_value_or_unary_op;
                    add_token(token<Json>(token_types::div));
                    break;
                case ' ':case '\t':
                    break;
                default:
                    throw parse_exception(jsonpath_parser_errc::invalid_filter,line_,column_);
                    break;

                }
                ++p;
                ++column_;
                break;
            case filter_states::unquoted_text: 
                {
                    switch (*p)
                    {
                    case '\r':
                    case '\n':
                        ++line_;
                        column_ = 1;
                        state = pre_line_break_state;
                        break;
                    case '<':
                    case '>':
                    case '!':
                    case '=':
                    case '&':
                    case '|':
                    case '+':
                    case '-':
                    case '*':
                    case '/':
                        {
                            if (buffer.length() > 0)
                            {
                                try
                                {
                                    auto val = Json::parse(buffer);
                                    add_token(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                                }
                                catch (const parse_exception& e)
                                {
                                    throw parse_exception(e.code(),line_,column_);
                                }
                                buffer.clear();
                            }
                            state = filter_states::oper;
                        }
                        break;
                    case ')':
                        if (buffer.length() > 0)
                        {
                            try
                            {
                                auto val = Json::parse(buffer);
                                add_token(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                            }
                            catch (const parse_exception& e)
                            {
                                throw parse_exception(e.code(),line_,column_);
                            }
                            buffer.clear();
                        }
                        add_token(token<Json>(token_types::rparen));
                        if (--depth == 0)
                        {
                            state = filter_states::start;
                            done = true;
                        }
                        else
                        {
                            state = filter_states::expect_path_or_value_or_unary_op;
                        }
                        ++p;
                        ++column_;
                        break;
                    case ' ':case '\t':
                        if (buffer.length() > 0)
                        {
                            try
                            {
                                auto val = Json::parse(buffer);
                                add_token(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                            }
                            catch (const parse_exception& e)
                            {
                                throw parse_exception(e.code(),line_,column_);
                            }
                            buffer.clear();
                        }
                        ++p;
                        ++column_;
                        break; 
                    default: 
                        buffer.push_back(*p);
                        ++p;
                        ++column_;
                        break;
                    }
                }
                break;
            case filter_states::single_quoted_text: 
                {
                    switch (*p)
                    {                   
                    case '\r':
                    case '\n':
                        ++line_;
                        column_ = 1;
                        state = pre_line_break_state;
                        break;
                    case '\\':
                        buffer.push_back(*p);
                        if (p+1 < end_expr)
                        {
                            ++p;
                            ++column_;
                            buffer.push_back(*p);
                        }
                        break;
                    case '\'':
                        buffer.push_back('\"');
                        //if (buffer.length() > 0)
                        {
                            try
                            {
                                auto val = Json::parse(buffer);
                                add_token(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                            }
                            catch (const parse_exception& e)
                            {
                                throw parse_exception(e.code(),line_,column_);
                            }
                            buffer.clear();
                        }
                        state = filter_states::expect_path_or_value_or_unary_op;
                        break;

                    default: 
                        buffer.push_back(*p);
                        break;
                    }
                }
                ++p;
                ++column_;
                break;
            case filter_states::double_quoted_text: 
                {
                    switch (*p)
                    {                   
                    case '\r':
                    case '\n':
                        ++line_;
                        column_ = 1;
                        state = pre_line_break_state;
                        break;
                    case '\\':
                        buffer.push_back(*p);
                        if (p+1 < end_expr)
                        {
                            ++p;
                            ++column_;
                            buffer.push_back(*p);
                        }
                        break;
                    case '\"':
                        buffer.push_back(*p);
                        //if (buffer.length() > 0)
                        {
                            try
                            {
                                auto val = Json::parse(buffer);
                                add_token(token<Json>(token_types::operand,std::make_shared<value_term<Json>>(val)));
                            }
                            catch (const parse_exception& e)
                            {
                                throw parse_exception(e.code(),line_,column_);
                            }
                            buffer.clear();
                        }
                        state = filter_states::expect_path_or_value_or_unary_op;
                        break;

                    default: 
                        buffer.push_back(*p);
                        break;
                    }
                }
                ++p;
                ++column_;
                break;
            case filter_states::expect_path_or_value_or_unary_op: 
                switch (*p)
                {
                case '\r':
                case '\n':
                    ++line_;
                    column_ = 1;
                    state = pre_line_break_state;
                    break;
                case '!':
                    add_token(token<Json>(token_types::exclaim));
                    ++p;
                    ++column_;
                    break;
                case '-':
                    add_token(token<Json>(token_types::unary_minus));
                    ++p;
                    ++column_;
                    break;
                case '@':
                    buffer.push_back(*p);
                    state = filter_states::path;
                    ++p;
                    ++column_;
                    break;
                case ' ':case '\t':
                    ++p;
                    ++column_;
                    break;
                case '\'':
                    buffer.push_back('\"');
                    state = filter_states::single_quoted_text;
                    ++p;
                    ++column_;
                    break;
                case '\"':
                    buffer.push_back(*p);
                    state = filter_states::double_quoted_text;
                    ++p;
                    ++column_;
                    break;
                case '(':
                    ++depth;
                    add_token(token<Json>(token_types::lparen));
                    ++p;
                    ++column_;
                    break;
                case ')':
                    add_token(token<Json>(token_types::rparen));
                    if (--depth == 0)
                    {
                        done = true;
                        state = filter_states::start;
                    }
                    ++p;
                    ++column_;
                    break;
                default: 
                    // don't increment
                    state = filter_states::unquoted_text;
                    break;
                };
                break;
            case filter_states::expect_oper_or_right_round_bracket: 
                switch (*p)
                {
                case '\r':
                case '\n':
                    ++line_;
                    column_ = 1;
                    state = pre_line_break_state;
                    break;
                case ' ':case '\t':
                    ++p;
                    ++column_;
                    break;
                case ')':
                    add_token(token<Json>(token_types::rparen));
                    if (--depth == 0)
                    {
                        done = true;
                        state = filter_states::start;
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
                case '*':
                case '/':
                    {
                        state = filter_states::oper;
                        // don't increment p
                    }
                    break;
                default: 
                    throw parse_exception(jsonpath_parser_errc::invalid_filter,line_,column_);
                    break;
                };
                break;
            case filter_states::expect_right_round_bracket: 
                switch (*p)
                {
                case '\r':
                case '\n':
                    ++line_;
                    column_ = 1;
                    state = pre_line_break_state;
                    break;
                case ' ':case '\t':
                    break;
                case ')':
                    add_token(token<Json>(token_types::rparen));
                    if (--depth == 0)
                    {
                        done = true;
                        state = filter_states::start;
                    }
                    else 
                    {
                        state = filter_states::expect_oper_or_right_round_bracket;
                    }
                    break;
                default: 
                    throw parse_exception(jsonpath_parser_errc::invalid_filter,line_,column_);
                    break;
                };
                ++p;
                ++column_;
                break;
            case filter_states::path: 
                switch (*p)
                {
                case '\r':
                case '\n':
                    ++line_;
                    column_ = 1;
                    state = pre_line_break_state;
                    break;
                case '<':
                case '>':
                case '!':
                case '=':
                case '&':
                case '|':
                case '+':
                case '-':
                case '*':
                case '/':
                    {
                        if (buffer.length() > 0)
                        {
                            add_token(token<Json>(token_types::operand,std::make_shared<path_term<Json>>(buffer)));
                            buffer.clear();
                        }
                        state = filter_states::oper;
                        // don't increment
                    }
                    break;
                case ')':
                    if (buffer.length() > 0)
                    {
                        add_token(token<Json>(token_types::operand,std::make_shared<path_term<Json>>(buffer)));
                        add_token(token<Json>(token_types::rparen));
                        buffer.clear();
                    }
                    if (--depth == 0)
                    {
                        state = filter_states::start;
                        done = true;
                    }
                    else
                    {
                        state = filter_states::expect_path_or_value_or_unary_op;
                    }
                    ++p;
                    ++column_;
                    break;
                default:
                    buffer.push_back(*p);
                    ++p;
                    ++column_;
                    break;
                };
                break;
            case filter_states::expect_regex: 
                switch (*p)
                {
                case '\r':
                case '\n':
                    ++line_;
                    column_ = 1;
                    state = pre_line_break_state;
                    break;
                case '/':
                    state = filter_states::regex;
                    break;
                case ' ':case '\t':
                    break;
                default: 
                    throw parse_exception(jsonpath_parser_errc::invalid_filter_expected_slash,line_,column_);
                    break;
                };
                ++p;
                ++column_;
                break;
            case filter_states::regex: 
                {
                    switch (*p)
                    {                   
                    case '\r':
                    case '\n':
                        ++line_;
                        column_ = 1;
                        state = pre_line_break_state;
                        break;
                    case '/':
                        //if (buffer.length() > 0)
                        {
                            std::regex::flag_type flags = std::regex_constants::ECMAScript; 
                            if (p+1  < end_expr && *(p+1) == 'i')
                            {
                                ++p;
                                ++column_;
                                flags |= std::regex_constants::icase;
                            }
                            add_token(token<Json>(token_types::operand,std::make_shared<regex_term<Json>>(buffer,flags)));
                            buffer.clear();
                        }
                        state = filter_states::expect_path_or_value_or_unary_op;
                        break;

                    default: 
                        buffer.push_back(*p);
                        break;
                    }
                }
                ++p;
                ++column_;
                break;
            default:
                ++p;
                ++column_;
                break;
            }
        }
        if (depth != 0)
        {
            throw parse_exception(jsonpath_parser_errc::invalid_filter_unbalanced_paren,line_,column_);
        }
        *end_ptr = p;

        return jsonpath_filter_expr<Json>(output_stack_,line_,column_);
    }
};


}}
#endif
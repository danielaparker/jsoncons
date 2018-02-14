// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_FILTER_HPP
#define JSONCONS_JSONPATH_FILTER_HPP

#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <regex>
#include <functional>
#include <cmath> 
#include <jsoncons/json.hpp>
#include "jsonpath_error_category.hpp"

namespace jsoncons { namespace jsonpath { namespace detail {

JSONCONS_DEFINE_LITERAL(eqtilde_literal,"=~")
JSONCONS_DEFINE_LITERAL(star_literal,"*")
JSONCONS_DEFINE_LITERAL(forwardslash_literal,"/")
JSONCONS_DEFINE_LITERAL(plus_literal,"+")
JSONCONS_DEFINE_LITERAL(minus_literal,"-")
JSONCONS_DEFINE_LITERAL(lt_literal,"<")
JSONCONS_DEFINE_LITERAL(lte_literal,"<=")
JSONCONS_DEFINE_LITERAL(gt_literal,">")
JSONCONS_DEFINE_LITERAL(gte_literal,">=")
JSONCONS_DEFINE_LITERAL(eq_literal,"==")
JSONCONS_DEFINE_LITERAL(ne_literal,"!=")
JSONCONS_DEFINE_LITERAL(ampamp_literal,"&&")
JSONCONS_DEFINE_LITERAL(pipepipe_literal,"||")
JSONCONS_DEFINE_LITERAL(max_literal,"max")
JSONCONS_DEFINE_LITERAL(min_literal,"min")

template<class Json>
struct PathConstructor
{
    typedef typename Json::char_type char_type;
    typedef typename Json::string_view_type string_view_type;
    typedef typename Json::string_type string_type;

    string_type operator()(const string_type& path, size_t index) const
    {
        char_type buf[255];
        char_type* p = buf;
        do
        {
            *p++ = static_cast<char_type>(48 + index % 10);
        } while (index /= 10);

        string_type s;
        s.append(path);
        s.push_back('[');
        while (--p >= buf)
        {
            s.push_back(*p);
        }
        s.push_back(']');
        return s;
    }

    string_type operator()(const string_type& path, const string_view_type& sv) const
    {
        string_type s;
        s.append(path);
        s.push_back('[');
        s.push_back('\'');
        s.append(sv.data(),sv.length());
        s.push_back('\'');
        s.push_back(']');
        return s;
    }
};

template<class Json>
struct VoidPathConstructor
{
    typedef typename Json::char_type char_type;
    typedef typename Json::string_view_type string_view_type;
    typedef typename Json::string_type string_type;

    string_type operator()(const string_type&, size_t) const
    {
        return string_type{};
    }

    string_type operator()(const string_type&, string_view_type) const
    {
        return string_type{};
    }
};

template <class Json,
          class JsonReference,
          class PathCons>
class jsonpath_evaluator;

enum class filter_state
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
    oper,
    function_argument,
    done
};

enum class token_type 
{
    operand,
    unary_operator,
    binary_operator,
    lparen,
    rparen
};

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
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json evaluate_single_node() const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool exclaim() const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool eq_term(const term&) const 
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool eq(const Json&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool ne_term(const term&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool ne(const Json&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool regex_term(const term&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool regex2(const string_type&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool ampamp_term(const term&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool ampamp(const Json&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool pipepipe_term(const term&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool pipepipe(const Json&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool lt_term(const term&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool lt(const Json&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool gt_term(const term&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual bool gt(const Json&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }

    virtual Json minus_term(const term&) const 
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json minus(const Json&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }

    virtual Json left_minus(const Json&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }

    virtual Json unary_minus() const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json plus_term(const term&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json plus(const Json&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json mult_term(const term&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json mult(const Json&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }

    virtual Json div_term(const term&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
    virtual Json div(const Json&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }

    virtual Json left_div(const Json&) const
    {
        throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,1,1);
    }
};

template <class Json>
struct operator_properties
{
    typedef std::function<Json(const term<Json>&, const term<Json>&)> operator_type;

    size_t precedence_level;
    bool is_right_associative;
    operator_type op;
};

template <class Json>
struct function_properties
{
    typedef std::function<Json(const term<Json>&)> function_type;

    size_t precedence_level;
    bool is_right_associative;
    bool is_aggregate;
    function_type op;
};

template <class Json>
class token
{
    token_type type_;
    size_t precedence_level_;
    bool is_right_associative_;
    bool is_aggregate_;
    std::shared_ptr<term<Json>> operand_ptr_;
    std::function<Json(const term<Json>&)> unary_operator_;
    std::function<Json(const term<Json>&, const term<Json>&)> operator_;
public:
    typedef std::function<Json(const term<Json>&)> unary_operator_type;
    typedef std::function<Json(const term<Json>&, const term<Json>&)> operator_type;

    Json operator()(const term<Json>& a)
    {
        return unary_operator_(a);
    }

    Json operator()(const term<Json>& a, const term<Json>& b)
    {
        return operator_(a,b);
    }

    token(token_type type)
        : type_(type),precedence_level_(0),is_right_associative_(false),is_aggregate_(false)
    {
    }
    token(token_type type, std::shared_ptr<term<Json>> term_ptr)
        : type_(type),precedence_level_(0),is_right_associative_(false),is_aggregate_(false),operand_ptr_(term_ptr)
    {
    }
    token(size_t precedence_level, 
          bool is_right_associative,
          std::function<Json(const term<Json>&)> unary_operator)
        : type_(token_type::unary_operator), 
          precedence_level_(precedence_level), 
          is_right_associative_(is_right_associative),
          is_aggregate_(false), 
          unary_operator_(unary_operator)
    {
    }
    token(const operator_properties<Json>& properties)
        : type_(token_type::binary_operator), 
          precedence_level_(properties.precedence_level), 
          is_right_associative_(properties.is_right_associative),
          is_aggregate_(false), 
          operator_(properties.op)
    {
    }
    token(const function_properties<Json>& properties)
        : type_(token_type::unary_operator), 
          precedence_level_(properties.precedence_level), 
          is_right_associative_(properties.is_right_associative), 
          is_aggregate_(properties.is_aggregate),
          unary_operator_(properties.op)
    {
    }
    token(const token& t) = default;
    //token(token&& t) = default;

    token<Json>& operator=(const token<Json>& val) = default;
    //token<Json>& operator=(token<Json>&& val) = default;

    bool is_operator() const
    {
        return is_unary_operator() || is_binary_operator(); 
    }

    bool is_unary_operator() const
    {
        return type_ == token_type::unary_operator; 
    }

    bool is_binary_operator() const
    {
        return type_ == token_type::binary_operator; 
    }

    bool is_operand() const
    {
        return type_ == token_type::operand; 
    }

    bool is_lparen() const
    {
        return type_ == token_type::lparen; 
    }

    bool is_rparen() const
    {
        return type_ == token_type::rparen; 
    }

    size_t precedence_level() const
    {
        return precedence_level_;
    }

    bool is_right_associative() const
    {
        return is_right_associative_;
    }

    bool is_aggregate() const
    {
        return is_aggregate_;
    }

    const term<Json>& operand()
    {
        JSONCONS_ASSERT(type_ == token_type::operand && operand_ptr_ != nullptr);
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
class value_term final : public term<Json>
{
    Json value_;
public:
    template <class T>
    value_term(const T& val)
        : value_(val)
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

    bool eq_term(const term<Json>& rhs) const override
    {
        return rhs.eq(value_);
    }

    bool eq(const Json& rhs) const override
    {
        return value_ == rhs;
    }

    bool ne_term(const term<Json>& rhs) const override
    {
        return rhs.ne(value_);
    }
    bool ne(const Json& rhs) const override
    {
        return value_ != rhs;
    }
    bool regex_term(const term<Json>& rhs) const override
    {
        return rhs.regex2(value_.as_string());
    }
    bool ampamp_term(const term<Json>& rhs) const override
    {
        return rhs.ampamp(value_);
    }
    bool ampamp(const Json& rhs) const override
    {
        return jsoncons::jsonpath::detail::ampamp(value_,rhs);
    }
    bool pipepipe_term(const term<Json>& rhs) const override
    {
        return rhs.pipepipe(value_);
    }
    bool pipepipe(const Json& rhs) const override
    {
        return jsoncons::jsonpath::detail::pipepipe(value_,rhs);
    }

    bool lt_term(const term<Json>& rhs) const override
    {
        return rhs.gt(value_);
    }
    bool lt(const Json& rhs) const override
    {
        return jsoncons::jsonpath::detail::lt(value_,rhs);
    }

    bool gt_term(const term<Json>& rhs) const override
    {
        return rhs.lt(value_);
    }
    bool gt(const Json& rhs) const override
    {
        return jsoncons::jsonpath::detail::gt(value_,rhs);
    }

    Json minus_term(const term<Json>& rhs) const override
    {
        return rhs.left_minus(value_);
    }
    Json minus(const Json& rhs) const override
    {
        return jsoncons::jsonpath::detail::minus(value_,rhs);
    }

    Json left_minus(const Json& lhs) const override
    {
        return jsoncons::jsonpath::detail::minus(lhs,value_);
    }

    Json unary_minus() const override
    {
        return jsoncons::jsonpath::detail::unary_minus(value_);
    }

    Json plus_term(const term<Json>& rhs) const override
    {
        return rhs.plus(value_);
    }

    Json plus(const Json& rhs) const override
    {
        return jsoncons::jsonpath::detail::plus(value_,rhs);
    }
    Json mult_term(const term<Json>& rhs) const override
    {
        return rhs.mult(value_);
    }

    Json mult(const Json& rhs) const override
    {
        return jsoncons::jsonpath::detail::mult(value_,rhs);
    }

    Json div_term(const term<Json>& rhs) const override
    {
        return rhs.left_div(value_);
    }
    Json div(const Json& rhs) const override
    {
        return jsoncons::jsonpath::detail::div(value_,rhs);
    }

    Json left_div(const Json& lhs) const override
    {
        return jsoncons::jsonpath::detail::div(lhs,value_);
    }
};

template <class Json>
class regex_term final : public term<Json>
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
class path_term final : public term<Json>
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
        jsonpath_evaluator<Json,const Json&,VoidPathConstructor<Json>> evaluator;
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

    bool eq_term(const term<Json>& rhs) const override
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

    bool ne_term(const term<Json>& rhs) const override
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
    bool regex_term(const term<Json>& rhs) const override
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
    bool ampamp_term(const term<Json>& rhs) const override
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
                result = jsoncons::jsonpath::detail::ampamp(nodes_[i],rhs);
            }
        }
        return result;
    }
    bool pipepipe_term(const term<Json>& rhs) const override
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
                result = jsoncons::jsonpath::detail::pipepipe(nodes_[i],rhs);
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
                result = jsoncons::jsonpath::detail::lt(nodes_[i],rhs);
            }
        }
        return result;
    }

    bool lt_term(const term<Json>& rhs) const override
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
                result = jsoncons::jsonpath::detail::gt(nodes_[i],rhs);
            }
        }
        return result;
    }

    bool gt_term(const term<Json>& rhs) const override
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

    Json minus_term(const term<Json>& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? rhs.left_minus(nodes_[0]) : a_null;
    }
    Json minus(const Json& rhs) const override
    {
        return nodes_.size() == 1 ? jsoncons::jsonpath::detail::minus(nodes_[0],rhs) : Json(jsoncons::null_type());
    }

    Json left_minus(const Json& lhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? jsoncons::jsonpath::detail::minus(lhs,nodes_[0]) : a_null;
    }

    Json unary_minus() const override
    {
        return nodes_.size() == 1 ? jsoncons::jsonpath::detail::unary_minus(nodes_[0]) : Json::null();
    }

    Json plus_term(const term<Json>& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? rhs.plus(nodes_[0]) : a_null;
    }
    Json plus(const Json& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? jsoncons::jsonpath::detail::plus(nodes_[0],rhs) : a_null;
    }

    Json mult_term(const term<Json>& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? rhs.mult(nodes_[0]) : a_null;
    }
    Json mult(const Json& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? jsoncons::jsonpath::detail::mult(nodes_[0],rhs) : a_null;
    }

    Json div_term(const term<Json>& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? rhs.left_div(nodes_[0]) : a_null;
    }
    Json div(const Json& rhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? jsoncons::jsonpath::detail::div(nodes_[0],rhs) : a_null;
    }

    Json left_div(const Json& lhs) const override
    {
        static auto a_null = Json(jsoncons::null_type());
        return nodes_.size() == 1 ? jsoncons::jsonpath::detail::div(lhs, nodes_[0]) : a_null;
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
        if (t.is_operand())
        {
            stack.push_back(t);
        }
        else if (t.is_unary_operator())
        {
            auto rhs = stack.back();
            stack.pop_back();
            Json val = t(rhs.operand());
            stack.push_back(token<Json>(token_type::operand,std::make_shared<value_term<Json>>(val)));
        }
        else if (t.is_binary_operator())
        {
            auto rhs = stack.back();
            stack.pop_back();
            auto lhs = stack.back();
            stack.pop_back();
            Json val = t(lhs.operand(), rhs.operand());
            stack.push_back(token<Json>(token_type::operand,std::make_shared<value_term<Json>>(val)));
        }
    }
    if (stack.size() != 1)
    {
        JSONCONS_THROW(json_exception_impl<std::runtime_error>("Invalid state"));
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
        catch (const parse_error& e)
        {
            throw parse_error(e.code(),line_,column_);
        }
    }

    bool exists(const Json& context_node)
    {
        try
        {
            auto t = evaluate(context_node,tokens_);
            return t.operand().accept_single_node();
        }
        catch (const parse_error& e)
        {
            throw parse_error(e.code(),line_,column_);
        }
    }
};

template <class Json>
class jsonpath_filter_parser
{
    typedef typename Json::string_type string_type;
    typedef typename Json::string_view_type string_view_type;
    typedef typename Json::char_type char_type;

    std::vector<token<Json>> output_stack_;
    std::vector<token<Json>> operator_stack_;
    std::vector<filter_state> state_stack_;

    size_t line_;
    size_t column_;

    static const operator_properties<Json> op_properties_[];

    class function_table
    {
        typedef std::map<string_type,function_properties<Json>> function_dictionary;

        const function_dictionary functions_ =
        {
            {
                max_literal<char_type>(),{1,true,true,[](const term<Json>& term)
                      {
                          Json a = term.evaluate_single_node();

                          double v = std::numeric_limits<double>::lowest();
                          for (const auto& elem : a.array_range())
                          {
                              double x = elem. template as<double>();
                              if (x > v)
                              {
                                  v = x;
                              }
                          }
                          return v;
                      }
                }
            },
            {
                min_literal<char_type>(),{1,true,true,[](const term<Json>& term) 
                      {
                          Json a = term.evaluate_single_node();

                          double v = (std::numeric_limits<double>::max)(); 
                          for (const auto& elem : a.array_range())
                          {
                              double x = elem. template as<double>();
                              if (x < v)
                              {
                                  v = x;
                              }
                          }
                          return v;
                      }
                }
            }
        };

    public:

        typename function_dictionary::const_iterator find(const string_type& key) const
        {
            return functions_.find(key);
        }
        typename function_dictionary::const_iterator end() const
        {
            return functions_.end();
        }
    };

    class binary_operator_table
    {
        typedef std::map<string_type,operator_properties<Json>> binary_operator_map;

        const binary_operator_map operators =
        {
            {eqtilde_literal<char_type>(),{2,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.regex_term(b)); }}},
            {star_literal<char_type>(),{3,false,[](const term<Json>& a, const term<Json>& b) {return a.mult_term(b); }}},
            {forwardslash_literal<char_type>(),{3,false,[](const term<Json>& a, const term<Json>& b) {return a.div_term(b); }}},
            {plus_literal<char_type>(),{4,false,[](const term<Json>& a, const term<Json>& b) {return a.plus_term(b); }}},
            {minus_literal<char_type>(),{4,false,[](const term<Json>& a, const term<Json>& b) {return a.minus_term(b); }}},
            {lt_literal<char_type>(),{5,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.lt_term(b)); }}},
            {lte_literal<char_type>(),{5,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.lt_term(b) || a.eq_term(b)); }}},
            {gt_literal<char_type>(),{5,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.gt_term(b)); }}},
            {gte_literal<char_type>(),{5,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.gt_term(b) || a.eq_term(b)); }}},
            {eq_literal<char_type>(),{6,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.eq_term(b)); }}},
            {ne_literal<char_type>(),{6,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.ne_term(b)); }}},
            {ampamp_literal<char_type>(),{7,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.ampamp_term(b)); }}},
            {pipepipe_literal<char_type>(),{8,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.pipepipe_term(b)); }}}
        };

    public:
        typename binary_operator_map::const_iterator find(const string_type& key) const
        {
            return operators.find(key);
        }
        typename binary_operator_map::const_iterator end() const
        {
            return operators.end();
        }
    };

    function_table functions_;
    binary_operator_table binary_operators_;

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

    jsonpath_filter_expr<Json> parse(const Json& root, const char_type* p, size_t length, const char_type** end_ptr)
    {
        return parse(root, p,p+length, end_ptr);
    }

    void push_state(filter_state state)
    {
        state_stack_.push_back(state);
    }

    filter_state pop_state()
    {
        if (state_stack_.empty())
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Invalid state"));
        }
        filter_state state = state_stack_.back();
        state_stack_.pop_back();
        return state;
    }

    void add_token(token<Json> token)
    {
        if (token.is_operand())
        {
            output_stack_.push_back(token);
        }
        else if (token.is_lparen())
        {
            operator_stack_.push_back(token);
        }
        else if (token.is_rparen())
        {
            auto it = operator_stack_.rbegin();
            while (it != operator_stack_.rend() && !it->is_lparen())
            {
                output_stack_.push_back(*it);
                ++it;
            }
            if (it == operator_stack_.rend())
            {
                JSONCONS_THROW(json_exception_impl<std::runtime_error>("Unbalanced parenthesis"));
            }
            operator_stack_.erase(it.base(),operator_stack_.end());
            operator_stack_.pop_back();
        }
        else if (token.is_operator())
        {
            if (operator_stack_.empty() || operator_stack_.back().is_lparen())
            {
                operator_stack_.push_back(token);
            }
            else if (token.precedence_level() < operator_stack_.back().precedence_level()
                     || (token.precedence_level() == operator_stack_.back().precedence_level() && token.is_right_associative()))
            {
                operator_stack_.push_back(token);
            }
            else
            {
                auto it = operator_stack_.rbegin();
                while (it != operator_stack_.rend() && it->is_operator()
                       && (token.precedence_level() > it->precedence_level()
                     || (token.precedence_level() == it->precedence_level() && token.is_right_associative())))
                {
                    output_stack_.push_back(*it);
                    ++it;
                }

                operator_stack_.erase(it.base(),operator_stack_.end());
                operator_stack_.push_back(token);
            }
        }
    }

    jsonpath_filter_expr<Json> parse(const Json& root, const char_type* p, const char_type* end_expr, const char_type** end_ptr)
    {
        output_stack_.clear();
        operator_stack_.clear();
        state_stack_.clear();

        string_type buffer;

        int depth = 0;
        filter_state state = filter_state::start;
        while (p < end_expr && state != filter_state::done)
        {
            switch (state)
            {
            case filter_state::cr:
                ++line_;
                column_ = 1;
                switch (*p)
                {
                case '\n':
                    state = pop_state();
                    ++p;
                    ++column_;
                    break;
                default:
                    state = pop_state();
                    break;
                }
                break;
            case filter_state::lf:
                ++line_;
                column_ = 1;
                state = pop_state();
                break;
            case filter_state::start:
                switch (*p)
                {
                case '\r':
                    push_state(state);
                    state = filter_state::cr;
                    break;
                case '\n':
                    push_state(state);
                    state = filter_state::lf;
                    break;
                case '(':
                    state = filter_state::expect_path_or_value_or_unary_op;
                    ++depth;
                    add_token(token<Json>(token_type::lparen));
                    break;
                case ')':
                    state = filter_state::expect_path_or_value_or_unary_op;
                    add_token(token<Json>(token_type::rparen));
                    if (--depth == 0)
                    {
                        state = filter_state::done;
                    }
                    break;
                }
                ++p;
                ++column_;
                break;
            case filter_state::function_argument:
                {
                    switch (*p)
                    {
                    case '\r':
                        push_state(state);
                        state = filter_state::cr;
                        break;
                    case '\n':
                        push_state(state);
                        state = filter_state::lf;
                        break;
                    case ' ':case '\t':
                        break;
                    case ')':
                        if (buffer.length() > 0)
                        {
                            if (operator_stack_.back().is_aggregate())
                            {
                                try
                                {
                                    // path, parse against root, get value
                                    jsonpath_evaluator<Json,const Json&,detail::VoidPathConstructor<Json>> evaluator;
                                    evaluator.evaluate(root,buffer.data(),buffer.length());
                                    auto result = evaluator.get_values();
                                    add_token(token<Json>(token_type::operand,std::make_shared<value_term<Json>>(result)));
                                }
                                catch (const parse_error& e)
                                {
                                    throw parse_error(e.code(),line_,column_);
                                }
                            }
                            else
                            {
                                add_token(token<Json>(token_type::operand,std::make_shared<path_term<Json>>(buffer)));
                            }
                            buffer.clear();
                            state = filter_state::expect_oper_or_right_round_bracket;
                        }
                        break;
                    default: 
                        buffer.push_back(*p);
                        break;
                    }
                    ++p;
                    ++column_;
                }
                break;
            case filter_state::oper:
                switch (*p)
                {
                case '~':
                    {
                        buffer.push_back(*p);
                        ++p;
                        ++column_;
                        auto it = binary_operators_.find(buffer);
                        if (it == binary_operators_.end())
                        {
                            throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator, line_, column_);
                        }
                        buffer.clear();
                        add_token(token<Json>(it->second));
                        state = filter_state::expect_regex;
                    }
                    break;
                case '=':
                case '&':
                case '|':
                    {
                        buffer.push_back(*p);
                        ++p;
                        ++column_;
                        auto it = binary_operators_.find(buffer);
                        if (it == binary_operators_.end())
                        {
                            throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator, line_, column_);
                        }
                        buffer.clear();
                        add_token(token<Json>(it->second));
                        state = filter_state::expect_path_or_value_or_unary_op;
                    }
                    break;
                default:
                    {
                        auto it = binary_operators_.find(buffer);
                        if (it == binary_operators_.end())
                        {
                            throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator, line_, column_);
                        }
                        buffer.clear();
                        add_token(token<Json>(it->second));
                        state = filter_state::expect_path_or_value_or_unary_op;
                    }
                    break;
                }
                break;
            case filter_state::unquoted_text: 
                {
                    switch (*p)
                    {
                    case ' ':case '\t':
                        if (buffer.length() > 0)
                        {
                            try
                            {
                                auto val = Json::parse(buffer);
                                add_token(token<Json>(token_type::operand,std::make_shared<value_term<Json>>(val)));
                            }
                            catch (const parse_error& e)
                            {
                                throw parse_error(e.code(),line_,column_);
                            }
                            buffer.clear();
                        }
                        ++p;
                        ++column_;
                        break; 
                    case '(':
                    {
                        auto it = functions_.find(buffer);
                        if (it == functions_.end())
                        {
                            throw parse_error(jsonpath_parser_errc::invalid_filter_unsupported_operator,line_,column_);
                        }
                        add_token(token<Json>(it->second));
                        state = filter_state::function_argument;
                        buffer.clear();
                        ++p;
                        ++column_;
                        break;
                    }
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
                                    add_token(token<Json>(token_type::operand,std::make_shared<value_term<Json>>(val)));
                                }
                                catch (const parse_error& e)
                                {
                                    throw parse_error(e.code(),line_,column_);
                                }
                                buffer.clear();
                            }
                            buffer.push_back(*p);
                            state = filter_state::oper;
                            ++p;
                            ++column_;
                        }
                        break;
                    case ')':
                        if (buffer.length() > 0)
                        {
                            try
                            {
                                auto val = Json::parse(buffer);
                                add_token(token<Json>(token_type::operand,std::make_shared<value_term<Json>>(val)));
                            }
                            catch (const parse_error& e)
                            {
                                throw parse_error(e.code(),line_,column_);
                            }
                            buffer.clear();
                        }
                        add_token(token<Json>(token_type::rparen));
                        if (--depth == 0)
                        {
                            state = filter_state::done;
                        }
                        else
                        {
                            state = filter_state::expect_path_or_value_or_unary_op;
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
            case filter_state::single_quoted_text: 
                {
                    switch (*p)
                    {                   
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
                                add_token(token<Json>(token_type::operand,std::make_shared<value_term<Json>>(val)));
                            }
                            catch (const parse_error& e)
                            {
                                throw parse_error(e.code(),line_,column_);
                            }
                            buffer.clear();
                        }
                        state = filter_state::expect_path_or_value_or_unary_op;
                        break;

                    default: 
                        buffer.push_back(*p);
                        break;
                    }
                }
                ++p;
                ++column_;
                break;
            case filter_state::double_quoted_text: 
                {
                    switch (*p)
                    {                   
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
                                add_token(token<Json>(token_type::operand,std::make_shared<value_term<Json>>(val)));
                            }
                            catch (const parse_error& e)
                            {
                                throw parse_error(e.code(),line_,column_);
                            }
                            buffer.clear();
                        }
                        state = filter_state::expect_path_or_value_or_unary_op;
                        break;

                    default: 
                        buffer.push_back(*p);
                        break;
                    }
                }
                ++p;
                ++column_;
                break;
            case filter_state::expect_path_or_value_or_unary_op: 
                switch (*p)
                {
                case '\r':
                    push_state(state);
                    state = filter_state::cr;
                    ++p;
                    break;
                case '\n':
                    push_state(state);
                    state = filter_state::lf;
                    ++p;
                    break;
                case ' ':case '\t':
                    ++p;
                    ++column_;
                    break;
                case '!':
                {
                    std::function<Json(const term<Json>&)> f = [](const term<Json>& b) {return Json(b.exclaim());};
                    add_token(token<Json>(1, true, f));
                    ++p;
                    ++column_;
                    break;
                }
                case '-':
                {
                    std::function<Json(const term<Json>&)> f = [](const term<Json>& b) {return b.unary_minus();};
                    add_token(token<Json>(1, true, f));
                    ++p;
                    ++column_;
                    break;
                }
                case '@':
                    buffer.push_back(*p);
                    state = filter_state::path;
                    ++p;
                    ++column_;
                    break;
                case '\'':
                    buffer.push_back('\"');
                    state = filter_state::single_quoted_text;
                    ++p;
                    ++column_;
                    break;
                case '\"':
                    buffer.push_back(*p);
                    state = filter_state::double_quoted_text;
                    ++p;
                    ++column_;
                    break;
                case '(':
                    ++depth;
                    add_token(token<Json>(token_type::lparen));
                    ++p;
                    ++column_;
                    break;
                case ')':
                    add_token(token<Json>(token_type::rparen));
                    if (--depth == 0)
                    {
                        state = filter_state::done;
                    }
                    ++p;
                    ++column_;
                    break;
                default: 
                    // don't increment
                    state = filter_state::unquoted_text;
                    break;
                };
                break;
            case filter_state::expect_oper_or_right_round_bracket: 
                switch (*p)
                {
                case '\r':
                    push_state(state);
                    state = filter_state::cr;
                    ++p;
                    break;
                case '\n':
                    push_state(state);
                    state = filter_state::lf;
                    ++p;
                    break;
                case ' ':case '\t':
                    ++p;
                    ++column_;
                    break;
                case ')':
                    add_token(token<Json>(token_type::rparen));
                    if (--depth == 0)
                    {
                        state = filter_state::done;
                        ++p; // fix
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
                        buffer.push_back(*p);
                        state = filter_state::oper;
                        ++p;
                        ++column_;
                    }
                    break;
                default: 
                    throw parse_error(jsonpath_parser_errc::invalid_filter,line_,column_);
                    break;
                };
                break;
            case filter_state::expect_right_round_bracket: 
                switch (*p)
                {
                case '\r':
                    push_state(state);
                    state = filter_state::cr;
                    break;
                case '\n':
                    push_state(state);
                    state = filter_state::lf;
                    break;
                case ' ':case '\t':
                    break;
                case ')':
                    add_token(token<Json>(token_type::rparen));
                    if (--depth == 0)
                    {
                        state = filter_state::done;
                    }
                    else 
                    {
                        state = filter_state::expect_oper_or_right_round_bracket;
                    }
                    break;
                default: 
                    throw parse_error(jsonpath_parser_errc::invalid_filter,line_,column_);
                    break;
                };
                ++p;
                ++column_;
                break;
            case filter_state::path: 
                switch (*p)
                {
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
                            add_token(token<Json>(token_type::operand,std::make_shared<path_term<Json>>(buffer)));
                            buffer.clear();
                        }
                        buffer.push_back(*p);
                        ++p;
                        ++column_;
                        state = filter_state::oper;
                    }
                    break;
                case ')':
                    if (buffer.length() > 0)
                    {
                        add_token(token<Json>(token_type::operand,std::make_shared<path_term<Json>>(buffer)));
                        add_token(token<Json>(token_type::rparen));
                        buffer.clear();
                    }
                    if (--depth == 0)
                    {
                        state = filter_state::done;
                    }
                    else
                    {
                        state = filter_state::expect_path_or_value_or_unary_op;
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
            case filter_state::expect_regex: 
                switch (*p)
                {
                case '\r':
                    push_state(state);
                    state = filter_state::cr;
                    break;
                case '\n':
                    push_state(state);
                    state = filter_state::lf;
                    break;
                case ' ':case '\t':
                    break;
                case '/':
                    state = filter_state::regex;
                    break;
                default: 
                    throw parse_error(jsonpath_parser_errc::invalid_filter_expected_slash,line_,column_);
                    break;
                };
                ++p;
                ++column_;
                break;
            case filter_state::regex: 
                {
                    switch (*p)
                    {                   
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
                            add_token(token<Json>(token_type::operand,std::make_shared<regex_term<Json>>(buffer,flags)));
                            buffer.clear();
                        }
                        state = filter_state::expect_path_or_value_or_unary_op;
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
            throw parse_error(jsonpath_parser_errc::invalid_filter_unbalanced_paren,line_,column_);
        }
        *end_ptr = p;

        return jsonpath_filter_expr<Json>(output_stack_,line_,column_);
    }
};

template <class Json>
const operator_properties<Json> jsonpath_filter_parser<Json>::op_properties_[] =
{
    {2,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.regex_term(b));}},
    {3,false,[](const term<Json>& a, const term<Json>& b) {return a.mult_term(b);}},
    {3,false,[](const term<Json>& a, const term<Json>& b) {return a.div_term(b);}},
    {4,false,[](const term<Json>& a, const term<Json>& b) {return a.plus_term(b);}},
    {4,false,[](const term<Json>& a, const term<Json>& b) {return a.minus_term(b);}},
    {5,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.lt_term(b));}},
    {5,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.lt_term(b) || a.eq_term(b));}},
    {5,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.gt_term(b));}},
    {5,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.gt_term(b) || a.eq_term(b));}},
    {6,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.eq_term(b)); }},
    {6,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.ne_term(b)); }},
    {7,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.ampamp_term(b));}},
    {8,false,[](const term<Json>& a, const term<Json>& b) {return Json(a.pipepipe_term(b));}}
};

}}}

#endif
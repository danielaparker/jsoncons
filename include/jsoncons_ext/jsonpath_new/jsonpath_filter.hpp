// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_FILTER_HPP
#define JSONCONS_JSONPATH_FILTER_HPP
 
#include <string>
#include <map> // std::map
#include <vector>
#include <memory>
#include <regex>
#include <functional>
#include <cmath> 
#include <new> 
#include <limits> // std::numeric_limits
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath_new/jsonpath_error.hpp>
#include <jsoncons_ext/jsonpath_new/jsonpath_expression.hpp>

namespace jsoncons { 
namespace jsonpath_new { 
namespace detail {

    template<class Json>
    struct PathConstructor
    {
        using char_type = typename Json::char_type;
        using string_view_type = typename Json::string_view_type;
        using string_type = std::basic_string<char_type>;

        string_type operator()(const string_type& path, std::size_t index) const
        {
            string_type s;
            s.append(path);
            s.push_back('[');
            jsoncons::detail::from_integer(index,s);
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
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;
        using string_view_type = typename Json::string_view_type;

        string_type operator()(const string_type&, std::size_t) const
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

    enum class filter_path_mode
    {
        path,
        root_path,
        current_path
    };

    enum class filter_state
    {
        start,
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
        expect_arg,
        path_argument,
        unquoted_argument,
        single_quoted_argument,
        double_quoted_argument,
        expect_more_args_or_right_round_bracket,
        done
    };

    enum class filter_token_type 
    {
        value,
        path,
        regex,
        unary_operator,
        binary_operator,
        lparen,
        rparen
    };

    template <class Json>
    Json unary_minus(const Json& lhs)
    {
        Json result = Json::null();
        if (lhs.is_int64())
        {
            result = -lhs.template as<int64_t>();
        }
        else if (lhs.is_double())
        {
            result = -lhs.as_double();
        }
        return result;
    }

    template <class Json>
    class term
    {
    public:
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;

        virtual ~term() noexcept {}

        term() = default;
        term(const term&) = default;
        term(term&&) = default;
        term& operator=(const term&) = default;
        term& operator=(term&&) = default;

        virtual term_type type() const = 0;

        virtual bool accept_single_node() const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
        virtual Json get_single_node() const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
        virtual bool unary_not() const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        virtual Json unary_minus() const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
    };

    template <class Json>
    class value_term final : public term<Json>
    {
        Json value_;
    public:
        value_term() = default;

        value_term(const Json& val)
            : value_(val)
        {
        }
        value_term(Json&& val)
            : value_(std::move(val))
        {
        }

        value_term(const value_term&) = default;
        value_term(value_term&&) = default;
        value_term& operator=(const value_term&) = default;
        value_term& operator=(value_term&&) = default;

        term_type type() const override {return term_type::value;}

        bool accept_single_node() const override
        {
            return value_.as_bool();
        }

        Json get_single_node() const override
        {
            return value_;
        }

        const Json& value() const
        {
            return value_;
        }

        bool unary_not() const override
        {
            return !value_.as_bool();
        }

        Json unary_minus() const override
        {
            return jsoncons::jsonpath_new::detail::unary_minus(value_);
        }
    };

    template <class Json>
    class regex_term final : public term<Json>
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;
        std::basic_regex<char_type> pattern_;
    public:
        regex_term(const string_type& pattern, std::regex::flag_type flags)
            : pattern_(pattern,flags)
        {
        }

        regex_term() = default;
        regex_term(const regex_term&) = default;
        regex_term(regex_term&&) = default;
        regex_term& operator=(const regex_term&) = default;
        regex_term& operator=(regex_term&&) = default;

        term_type type() const override {return term_type::regex;}

        bool evaluate(const string_type& subject) const
        {
            return std::regex_search(subject, pattern_);
        }
    };

    template <class Json>
    class raw_path_term final : public term<Json>
    {
        using path_expression_type = path_expression<Json,const Json&>;
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;

        path_expression_type path_;
        std::size_t line_;
        std::size_t column_;
        Json nodes_;
    public:
        raw_path_term(path_expression_type&& path, std::size_t line, std::size_t column)
            : path_(std::move(path)), line_(line), column_(column)
        {
        }

        raw_path_term(const raw_path_term&) = default;
        raw_path_term(raw_path_term&&) = default;
        raw_path_term& operator=(const raw_path_term&) = default;
        raw_path_term& operator=(raw_path_term&&) = default;

        term_type type() const override {return term_type::path;}

        Json evaluate(dynamic_resources<Json>& resources, const Json& instance) const 
        {
            return path_.evaluate(resources, instance);
        }

        const Json& result() const
        {
            return nodes_;
        }

        bool accept_single_node() const override
        {
            return nodes_.size() != 0;
        }

        Json get_single_node() const override
        {
            return nodes_.size() == 1 ? nodes_[0] : nodes_;
        }

        bool unary_not() const override
        {
            return nodes_.size() == 0;
        }

        Json unary_minus() const override
        {
            return nodes_.size() == 1 ? jsoncons::jsonpath_new::detail::unary_minus(nodes_[0]) : Json::null();
        }
    };

    template <class Json>
    class evaluated_path_term final : public term<Json>
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;

        Json nodes_;
    public:
        evaluated_path_term(Json&& nodes)
            : nodes_(std::move(nodes))
        {
        }

        evaluated_path_term(const evaluated_path_term&) = default;
        evaluated_path_term(evaluated_path_term&&) = default;
        evaluated_path_term& operator=(const evaluated_path_term&) = default;
        evaluated_path_term& operator=(evaluated_path_term&&) = default;

        term_type type() const override {return term_type::path;}


        const Json& result() const
        {
            return nodes_;
        }

        bool accept_single_node() const override
        {
            return nodes_.size() != 0;
        }

        Json get_single_node() const override
        {
            return nodes_.size() == 1 ? nodes_[0] : nodes_;
        }

        bool unary_not() const override
        {
            return nodes_.size() == 0;
        }

        Json unary_minus() const override
        {
            return nodes_.size() == 1 ? jsoncons::jsonpath_new::detail::unary_minus(nodes_[0]) : Json::null();
        }
    };

    template <class Json>
    class token;

    template <class Json>
    class raw_token
    {
        friend class token<Json>;

        filter_token_type type_;

        union
        {
            const unary_operator_properties<Json>* unary_op_properties_;
            const binary_operator_properties<Json>* binary_op_properties_;
            value_term<Json> value_term_;
            raw_path_term<Json> path_term_;
            regex_term<Json> regex_term_;
        };

    public:

        raw_token(lparen_arg_t)
            : type_(filter_token_type::lparen)
        {
        }

        raw_token(rparen_arg_t)
            : type_(filter_token_type::rparen)
        {
        }

        raw_token(value_term<Json>&& term)
            : type_(filter_token_type::value), value_term_(std::move(term))
        {
        }
        raw_token(raw_path_term<Json>&& term)
            : type_(filter_token_type::path), path_term_(std::move(term))
        {
        }
        raw_token(regex_term<Json>&& term)
            : type_(filter_token_type::regex), regex_term_(std::move(term))
        {
        }

        raw_token(const unary_operator_properties<Json>* properties)
            : type_(filter_token_type::unary_operator), 
              unary_op_properties_(properties)
        {
        }

        raw_token(const binary_operator_properties<Json>* properties)
            : type_(filter_token_type::binary_operator), 
              binary_op_properties_(properties)
        {
        }

        raw_token(const raw_token& other) = delete;

        raw_token(raw_token&& other)
        {
            construct(std::forward<raw_token>(other));
        }

        ~raw_token() noexcept
        {
            destroy();
        }
        raw_token& operator=(const raw_token& other) = delete;

        raw_token& operator=(raw_token&& other)
        {
            if (&other != this)
            {
                if (other.type_ == type_)
                {
                    type_ = other.type_;
                    switch(type_)
                    {
                        case filter_token_type::value:
                            value_term_ = std::move(other.value_term_);
                            break;
                        case filter_token_type::path:
                            path_term_ = std::move(other.path_term_);
                            break;
                        case filter_token_type::regex:
                            regex_term_ = std::move(other.regex_term_);
                            break;
                        case filter_token_type::unary_operator:
                            unary_op_properties_ = other.unary_op_properties_;
                            break;
                        case filter_token_type::binary_operator:
                            binary_op_properties_ = other.binary_op_properties_;
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    destroy();
                    construct(std::forward<raw_token>(other));
                }
            }
            return *this;
        }

        filter_token_type type() const
        {
            return type_;
        }

        bool is_operator() const
        {
            return is_unary_operator() || is_binary_operator(); 
        }

        bool is_unary_operator() const
        {
            return type_ == filter_token_type::unary_operator; 
        }

        bool is_binary_operator() const
        {
            return type_ == filter_token_type::binary_operator; 
        }

        bool is_operand() const
        {
            switch(type_)
            {
                case filter_token_type::value:
                case filter_token_type::path:
                case filter_token_type::regex:
                    return true;
                default:
                    return false;
            }
        }

        bool is_lparen() const
        {
            return type_ == filter_token_type::lparen; 
        }

        bool is_rparen() const
        {
            return type_ == filter_token_type::rparen; 
        }

        std::size_t precedence_level() const
        {
            switch(type_)
            {
                case filter_token_type::unary_operator:
                    return unary_op_properties_->precedence_level;
                case filter_token_type::binary_operator:
                    return binary_op_properties_->precedence_level;
                default:
                    return 0;
            }
        }

        bool is_right_associative() const
        {
            switch(type_)
            {
                case filter_token_type::unary_operator:
                    return unary_op_properties_->is_right_associative;
                case filter_token_type::binary_operator:
                    return binary_op_properties_->is_right_associative;
                default:
                    return false;
            }
        }

    private:

        void construct(raw_token&& other)
        {
            type_ = other.type_;
            switch (type_)
            {
            case filter_token_type::value:
                ::new(static_cast<void*>(&this->value_term_))value_term<Json>(std::move(other.value_term_));
                break;
            case filter_token_type::path:
                ::new(static_cast<void*>(&this->path_term_))raw_path_term<Json>(std::move(other.path_term_));
                break;
            case filter_token_type::regex:
                ::new(static_cast<void*>(&this->regex_term_))regex_term<Json>(std::move(other.regex_term_));
                break;
            case filter_token_type::unary_operator:
                this->unary_op_properties_ = other.unary_op_properties_;
                break;
            case filter_token_type::binary_operator:
                this->binary_op_properties_ = other.binary_op_properties_;
                break;
            default:
                break;
            }
        }

        void destroy() noexcept 
        {
            switch(type_)
            {
                case filter_token_type::value:
                    value_term_.~value_term();
                    break;
                case filter_token_type::path:
                    path_term_.~raw_path_term();
                    break;
                case filter_token_type::regex:
                    regex_term_.~regex_term();
                    break;
                default:
                    break;
            }
        }
    };

    template <class Json>
    class token
    {
        filter_token_type type_;

        union
        {
            const unary_operator_properties<Json>* unary_op_properties_;
            const binary_operator_properties<Json>* binary_op_properties_;
            value_term<Json> value_term_;
            evaluated_path_term<Json> path_term_;
            regex_term<Json> regex_term_;
        };

    public:

        token(value_term<Json>&& term)
            : type_(filter_token_type::value), value_term_(std::move(term))
        {
        }

        token(regex_term<Json>&& term)
            : type_(filter_token_type::value), value_term_(std::move(term))
        {
        }

        token(evaluated_path_term<Json>&& term)
            : type_(filter_token_type::value), value_term_(std::move(term))
        {
        }

        token(const raw_token<Json>& other, dynamic_resources<Json>& resources, const Json& instance)
            : type_(other.type_)
        {
            switch (type_)
            {
                case filter_token_type::value:
                    ::new(static_cast<void*>(&this->value_term_))value_term<Json>(other.value_term_);
                    break;
                case filter_token_type::path:
                {
                    ::new(static_cast<void*>(&this->path_term_))evaluated_path_term<Json>(other.path_term_.evaluate(resources, instance));
                    break;
                }
                case filter_token_type::regex:
                    ::new(static_cast<void*>(&this->regex_term_))regex_term<Json>(other.regex_term_);
                    break;
                case filter_token_type::unary_operator:
                    this->unary_op_properties_ = other.unary_op_properties_;
                    break;
                case filter_token_type::binary_operator:
                    this->binary_op_properties_ = other.binary_op_properties_;
                    break;
                default:
                    break;
            }
        }

        token(const token& other)
        {
            construct(other);
        }

        token(token&& other)
        {
            construct(std::forward<token>(other));
        }

        ~token() noexcept
        {
            destroy();
        }

        token& operator=(const token& other)
        {
            if (&other != this)
            {
                if (other.type_ == type_)
                {
                    switch(type_)
                    {
                        case filter_token_type::value:
                            value_term_ = other.value_term_;
                            break;
                        case filter_token_type::path:
                            path_term_ = other.path_term_;
                            break;
                        case filter_token_type::regex:
                            regex_term_ = other.regex_term_;
                            break;
                        case filter_token_type::unary_operator:
                            unary_op_properties_ = other.unary_op_properties_;
                            break;
                        case filter_token_type::binary_operator:
                            binary_op_properties_ = other.binary_op_properties_;
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    destroy();
                    construct(other);
                }
            }
            return *this;
        }

        token& operator=(token&& other)
        {
            if (&other != this)
            {
                if (other.type_ == type_)
                {
                    switch(type_)
                    {
                        case filter_token_type::value:
                            value_term_ = std::move(other.value_term_);
                            break;
                        case filter_token_type::path:
                            path_term_ = std::move(other.path_term_);
                            break;
                        case filter_token_type::regex:
                            regex_term_ = std::move(other.regex_term_);
                            break;
                        case filter_token_type::unary_operator:
                            unary_op_properties_ = other.unary_op_properties_;
                            break;
                        case filter_token_type::binary_operator:
                            binary_op_properties_ = other.binary_op_properties_;
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    destroy();
                    construct(std::forward<token>(other));
                }
            }
            return *this;
        }

        filter_token_type type() const
        {
            return type_;
        }

        Json operator()(const term<Json>& a)
        {
            switch(type_)
            {
                case filter_token_type::unary_operator:
                    return unary_op_properties_->op(a);
                default:
                    JSONCONS_UNREACHABLE();
                    break;
            }
        }

        Json operator()(const term<Json>& a, const term<Json>& b)
        {
            switch(type_)
            {
                case filter_token_type::binary_operator:
                    return binary_op_properties_->op(a,b);
                default:
                    JSONCONS_UNREACHABLE();
                    break;
            }
        }

        bool is_operator() const
        {
            return is_unary_operator() || is_binary_operator(); 
        }

        bool is_unary_operator() const
        {
            return type_ == filter_token_type::unary_operator; 
        }

        bool is_binary_operator() const
        {
            return type_ == filter_token_type::binary_operator; 
        }

        bool is_operand() const
        {
            switch(type_)
            {
                case filter_token_type::value:
                case filter_token_type::path:
                case filter_token_type::regex:
                    return true;
                default:
                    return false;
            }
        }

        bool is_lparen() const
        {
            return type_ == filter_token_type::lparen; 
        }

        bool is_rparen() const
        {
            return type_ == filter_token_type::rparen; 
        }

        std::size_t precedence_level() const
        {
            switch(type_)
            {
                case filter_token_type::unary_operator:
                    return unary_op_properties_->precedence_level;
                case filter_token_type::binary_operator:
                    return binary_op_properties_->precedence_level;
                default:
                    return 0;
            }
        }

        bool is_right_associative() const
        {
            switch(type_)
            {
                case filter_token_type::unary_operator:
                    return unary_op_properties_->is_right_associative;
                case filter_token_type::binary_operator:
                    return binary_op_properties_->is_right_associative;
                default:
                    return false;
            }
        }

        const term<Json>& operand()
        {
            switch(type_)
            {
                case filter_token_type::value:
                    return value_term_;
                case filter_token_type::path:
                    return path_term_;
                case filter_token_type::regex:
                    return regex_term_;
                default:
                    JSONCONS_UNREACHABLE();
            }
        }

    private:

        void construct(const token& other)
        {
            type_ = other.type_;
            switch (type_)
            {
            case filter_token_type::value:
                ::new(static_cast<void*>(&this->value_term_))value_term<Json>(other.value_term_);
                break;
            case filter_token_type::path:
                ::new(static_cast<void*>(&this->path_term_))evaluated_path_term<Json>(other.path_term_);
                break;
            case filter_token_type::regex:
                ::new(static_cast<void*>(&this->regex_term_))regex_term<Json>(other.regex_term_);
                break;
            case filter_token_type::unary_operator:
                this->unary_op_properties_ = other.unary_op_properties_;
                break;
            case filter_token_type::binary_operator:
                this->binary_op_properties_ = other.binary_op_properties_;
                break;
            default:
                break;
            }
        }

        void construct(token&& other)
        {
            type_ = other.type_;
            switch (type_)
            {
            case filter_token_type::value:
                ::new(static_cast<void*>(&this->value_term_))value_term<Json>(std::move(other.value_term_));
                break;
            case filter_token_type::path:
                ::new(static_cast<void*>(&this->path_term_))evaluated_path_term<Json>(std::move(other.path_term_));
                break;
            case filter_token_type::regex:
                ::new(static_cast<void*>(&this->regex_term_))regex_term<Json>(std::move(other.regex_term_));
                break;
            case filter_token_type::unary_operator:
                this->unary_op_properties_ = other.unary_op_properties_;
                break;
            case filter_token_type::binary_operator:
                this->binary_op_properties_ = other.binary_op_properties_;
                break;
            default:
                break;
            }
        }

        void destroy() noexcept 
        {
            switch(type_)
            {
                case filter_token_type::value:
                    value_term_.~value_term();
                    break;
                case filter_token_type::path:
                    path_term_.~evaluated_path_term();
                    break;
                case filter_token_type::regex:
                    regex_term_.~regex_term();
                    break;
                default:
                    break;
            }
        }
    };

    template <class Json>
    token<Json> evaluate(dynamic_resources<Json>& resources, 
                         const Json& instance, 
                         const std::vector<raw_token<Json>>& raw_tokens)
    {
        std::vector<token<Json>> tokens;
        for (const auto& tok : raw_tokens)
        {
            tokens.emplace_back(tok, resources, instance);
        }
        std::vector<token<Json>> stack;
        stack.reserve(tokens.size());
        for (auto& t : tokens)
        {
            if (t.is_operand())
            {
                stack.push_back(t);
            }
            else if (t.is_unary_operator())
            {
                auto rhs = std::move(stack.back());
                stack.pop_back();
                stack.push_back(token<Json>(value_term<Json>(t(rhs.operand()))));
            }
            else if (t.is_binary_operator())
            {
                auto rhs = std::move(stack.back());
                stack.pop_back();
                auto lhs = std::move(stack.back());
                stack.pop_back();
                stack.push_back(token<Json>(value_term<Json>(t(lhs.operand(), rhs.operand()))));
            }
        }
        if (stack.size() != 1)
        {
            JSONCONS_THROW(json_runtime_error<std::runtime_error>("Invalid state"));
        }

        return stack.back();
    }

    template <class Json>
    class jsonpath_filter_expr
    {
    public:
        std::vector<raw_token<Json>> tokens_;
    public:
        jsonpath_filter_expr()
        {
        }

        jsonpath_filter_expr(std::vector<raw_token<Json>>&& tokens)
            : tokens_(std::move(tokens))
        {
        }

        Json eval(dynamic_resources<Json>& resources, const Json& instance) const
        {
            auto t = evaluate(resources, instance, tokens_);
            return t.operand().get_single_node();
        }

        bool exists(dynamic_resources<Json>& resources, const Json& instance) const
        {
            auto t = evaluate(resources, instance,tokens_);
            return t.operand().accept_single_node();
        }
    };

    template <class Json>
    class jsonpath_filter_parser
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;
        using string_view_type = typename Json::string_view_type;

        std::vector<raw_token<Json>> output_stack_;
        std::vector<raw_token<Json>> operator_stack_;

        std::size_t line_;
        std::size_t column_;

    public:
        jsonpath_filter_parser()
            : jsonpath_filter_parser(1,1)
        {
        }
        jsonpath_filter_parser(std::size_t line, std::size_t column)
            : line_(line), column_(column)
        {
        }

        std::size_t line() const
        {
            return line_;
        }

        std::size_t column() const
        {
            return column_;
        }

        void push_token(raw_token<Json>&& raw_token)
        {
            switch (raw_token.type())
            {
                case filter_token_type::value:
                case filter_token_type::path:
                case filter_token_type::regex:
                    output_stack_.push_back(std::move(raw_token));
                    break;
                case filter_token_type::lparen:
                    operator_stack_.push_back(std::move(raw_token));
                    break;
                case filter_token_type::rparen:
                    {
                        auto it = operator_stack_.rbegin();
                        while (it != operator_stack_.rend() && !it->is_lparen())
                        {
                            output_stack_.push_back(std::move(*it));
                            ++it;
                        }
                        if (it == operator_stack_.rend())
                        {
                            JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced parenthesis"));
                        }
                        operator_stack_.erase(it.base(),operator_stack_.end());
                        operator_stack_.pop_back();
                        break;
                    }
                case filter_token_type::unary_operator:
                case filter_token_type::binary_operator:
                {
                    if (operator_stack_.empty() || operator_stack_.back().is_lparen())
                    {
                        operator_stack_.push_back(std::move(raw_token));
                    }
                    else if (raw_token.precedence_level() < operator_stack_.back().precedence_level()
                             || (raw_token.precedence_level() == operator_stack_.back().precedence_level() && raw_token.is_right_associative()))
                    {
                        operator_stack_.push_back(std::move(raw_token));
                    }
                    else
                    {
                        auto it = operator_stack_.rbegin();
                        while (it != operator_stack_.rend() && it->is_operator()
                               && (raw_token.precedence_level() > it->precedence_level()
                             || (raw_token.precedence_level() == it->precedence_level() && raw_token.is_right_associative())))
                        {
                            output_stack_.push_back(std::move(*it));
                            ++it;
                        }

                        operator_stack_.erase(it.base(),operator_stack_.end());
                        operator_stack_.push_back(std::move(raw_token));
                    }
                    break;
                }
                default:
                    break;
            }
        }

        jsonpath_filter_expr<Json> parse(static_resources<Json>& resources, 
                                         const char_type* p, 
                                         const char_type* end_expr, 
                                         const char_type** end_ptr)
        {
            output_stack_.clear();
            operator_stack_.clear();
            std::vector<filter_state> state_stack;
            std::vector<filter_path_mode> path_mode_stack;

            string_type buffer;
            std::size_t buffer_line = 1;
            std::size_t buffer_column = 1;

            int depth = 0;
            filter_state state = filter_state::start;
            while (p < end_expr && state != filter_state::done)
            {
                switch (state)
                {
                    case filter_state::start:
                        switch (*p)
                        {
                            case '\r':
                                if (p+1 < end_expr && *(p+1) == '\n')
                                    ++p;
                                ++line_;
                                column_ = 1;
                                ++p;
                                break;
                            case '\n':
                                ++line_;
                                column_ = 1;
                                ++p;
                                break;
                            case '(':
                                state = filter_state::expect_path_or_value_or_unary_op;
                                ++depth;
                                push_token(raw_token<Json>(lparen_arg));
                                break;
                            case ')':
                                state = filter_state::expect_path_or_value_or_unary_op;
                                push_token(raw_token<Json>(rparen_arg));
                                if (--depth == 0)
                                {
                                    state = filter_state::done;
                                }
                                break;
                    }
                    ++p;
                    ++column_;
                    break;

                    case filter_state::expect_arg:
                    {
                        switch (*p)
                        {
                            case ' ':case '\t':
                                break;
                            case '\r':
                                if (p+1 < end_expr && *(p+1) == '\n')
                                    ++p;
                                ++line_;
                                column_ = 1;
                                ++p;
                                break;
                            case '\n':
                                ++line_;
                                column_ = 1;
                                ++p;
                                break;
                            case '$':
                                buffer.push_back(*p);
                                path_mode_stack.back() = filter_path_mode::root_path;
                                state = filter_state::path_argument;
                                break;
                            case '@':
                                buffer.push_back('$');
                                path_mode_stack.back() = filter_path_mode::current_path;
                                state = filter_state::path_argument;
                                break;
                            // Maybe error from here down
                            case '\'':
                                buffer.push_back('\"');
                                state = filter_state::single_quoted_argument;
                                break;
                            case '\"':
                                buffer.push_back('\"');
                                state = filter_state::double_quoted_argument;
                                break;
                            default: 
                                buffer.push_back(*p);
                                state = filter_state::unquoted_argument;
                                break;
                        }
                        ++p;
                        ++column_;
                        break;
                    }

                    case filter_state::path_argument:
                    {
                        switch (*p)
                        {
                            case '\r':
                                if (p+1 < end_expr && *(p+1) == '\n')
                                    ++p;
                                ++line_;
                                column_ = 1;
                                ++p;
                                break;
                            case '\n':
                                ++line_;
                                column_ = 1;
                                ++p;
                                break;
                            case ' ':case '\t':
                                break;
                            case ',':
                                buffer.push_back(*p);
                                state = filter_state::expect_arg;
                                break;
                            case ')':
                            {
                                buffer.push_back(*p);
                                state = filter_state::path;
                                break;
                            }
                            default: 
                                buffer.push_back(*p);
                                break;
                        }
                        ++p;
                        ++column_;
                        break;
                    }
                    case filter_state::single_quoted_argument:
                    {
                        switch (*p)
                        {
                            case '\'':
                                buffer.push_back('\"');
                                state = filter_state::expect_more_args_or_right_round_bracket;
                                break;
                            default: 
                                buffer.push_back(*p);
                                break;
                        }
                        ++p;
                        ++column_;
                        break;
                    }
                    case filter_state::double_quoted_argument:
                    {
                        switch (*p)
                        {
                            case '\"':
                                buffer.push_back('\"');
                                state = filter_state::expect_more_args_or_right_round_bracket;
                                break;
                            default: 
                                buffer.push_back(*p);
                                break;
                        }
                        ++p;
                        ++column_;
                        break;
                    }
                    case filter_state::unquoted_argument:
                    {
                        switch (*p)
                        {
                            case ',':
                                buffer.push_back(*p);
                                state = filter_state::expect_arg;
                                break;
                            case ')':
                            {
                                buffer.push_back(*p);
                                state = filter_state::path;
                                break;
                            }
                            default:
                                buffer.push_back(*p);
                                break;
                        }
                        ++p;
                        ++column_;
                        break;
                    }
                    case filter_state::expect_more_args_or_right_round_bracket:
                    {
                        switch (*p)
                        {
                            case ' ':
                            case '\t':
                                break;
                            case ',':
                                buffer.push_back(*p);
                                state = filter_state::expect_arg;
                                break;
                            case ')':
                            {
                                buffer.push_back(*p);
                                state = filter_state::path;
                                break;
                            }
                            default:
                                JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator, line_, column_));
                        }
                        ++p;
                        ++column_;
                        break;
                    }

                case filter_state::oper:
                    switch (*p)
                    {
                        case '~':
                        {
                            buffer.push_back(*p);
                            ++p;
                            ++column_;
                            auto properties = resources.get_binary_operator_properties(buffer);
                            if (properties == nullptr)
                            {
                                JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator, line_, column_));
                            }
                            buffer.clear();
                            buffer_line = buffer_column = 1;
                            push_token(raw_token<Json>(properties));
                            state = filter_state::expect_regex;
                            break;
                        }
                        case '=':
                        case '&':
                        case '|':
                        {
                            buffer.push_back(*p);
                            ++p;
                            ++column_;
                            auto properties = resources.get_binary_operator_properties(buffer);
                            if (properties == nullptr)
                            {
                                JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator, line_, column_));
                            }
                            buffer.clear();
                            buffer_line = buffer_column = 1;
                            push_token(raw_token<Json>(properties));
                            state = filter_state::expect_path_or_value_or_unary_op;
                            break;
                        }
                        default:
                        {
                            auto properties = resources.get_binary_operator_properties(buffer);
                            if (properties == nullptr)
                            {
                                JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator, line_, column_));
                            }
                            buffer.clear();
                            buffer_line = buffer_column = 1;
                            push_token(raw_token<Json>(properties));
                            state = filter_state::expect_path_or_value_or_unary_op;
                            break;
                        }
                    }
                    break;
                case filter_state::unquoted_text: 
                    {
                        switch (*p)
                        {
                            case ' ':case '\t':
                                if (buffer.length() > 0)
                                {
                                    JSONCONS_TRY
                                    {
                                        push_token(raw_token<Json>(value_term<Json>(Json::parse(buffer))));
                                    }
                                    JSONCONS_CATCH(const ser_error&)     
                                    {
                                        JSONCONS_THROW(jsonpath_error(jsonpath_errc::parse_error_in_filter,line_,column_));
                                    }
                                    buffer.clear();
                                    buffer_line = buffer_column = 1;
                                }
                                ++p;
                                ++column_;
                                break; 
                            case '(':
                            {
                                buffer.push_back(*p);
                                path_mode_stack.push_back(filter_path_mode::path);
                                state = filter_state::expect_arg;
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
                                        JSONCONS_TRY
                                        {
                                            push_token(raw_token<Json>(value_term<Json>(Json::parse(buffer))));
                                        }
                                        JSONCONS_CATCH(const ser_error&)     
                                        {
                                            JSONCONS_THROW(jsonpath_error(jsonpath_errc::parse_error_in_filter,line_,column_));
                                        }
                                        buffer.clear();
                                        buffer_line = buffer_column = 1;
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
                                    JSONCONS_TRY
                                    {
                                        auto val = Json::parse(buffer);
                                        push_token(raw_token<Json>(value_term<Json>(std::move(val))));
                                    }
                                    JSONCONS_CATCH(const ser_error&)     
                                    {
                                        JSONCONS_THROW(jsonpath_error(jsonpath_errc::parse_error_in_filter,line_,column_));
                                    }
                                    buffer.clear();
                                    buffer_line = buffer_column = 1;
                                }
                                push_token(raw_token<Json>(rparen_arg));
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
                                    JSONCONS_TRY
                                    {
                                        auto val = Json::parse(buffer);
                                        push_token(raw_token<Json>(value_term<Json>(std::move(val))));
                                    }
                                    JSONCONS_CATCH(const ser_error&)     
                                    {
                                        JSONCONS_THROW(jsonpath_error(jsonpath_errc::parse_error_in_filter,line_,column_));
                                    }
                                    buffer.clear();
                                    buffer_line = buffer_column = 1;
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
                                JSONCONS_TRY
                                {
                                    auto val = Json::parse(buffer);
                                    push_token(raw_token<Json>(value_term<Json>(std::move(val))));
                                }
                                JSONCONS_CATCH(const ser_error&)     
                                {
                                    JSONCONS_THROW(jsonpath_error(jsonpath_errc::parse_error_in_filter,line_,column_));
                                }
                                buffer.clear();
                                buffer_line = buffer_column = 1;
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
                            if (p+1 < end_expr && *(p+1) == '\n')
                                ++p;
                            ++line_;
                            column_ = 1;
                            ++p;
                            break;
                        case '\n':
                            ++line_;
                            column_ = 1;
                            ++p;
                            break;
                        case ' ':case '\t':
                            ++p;
                            ++column_;
                            break;
                        case '!':
                        {
                            push_token(raw_token<Json>(resources.get_not_properties()));
                            ++p;
                            ++column_;
                            break;
                        }
                        case '-':
                        {
                            push_token(raw_token<Json>(resources.get_unary_minus_properties()));
                            ++p;
                            ++column_;
                            break;
                        }
                        case '@':
                            buffer_line = line_;
                            buffer_column = column_;
                            buffer.push_back('$');
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
                            push_token(raw_token<Json>(lparen_arg));
                            ++p;
                            ++column_;
                            break;
                        case ')':
                            push_token(raw_token<Json>(rparen_arg));
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
                            if (p+1 < end_expr && *(p+1) == '\n')
                                ++p;
                            ++line_;
                            column_ = 1;
                            ++p;
                            break;
                        case '\n':
                            ++line_;
                            column_ = 1;
                            ++p;
                            break;
                        case ' ':case '\t':
                            ++p;
                            ++column_;
                            break;
                        case ')':
                            push_token(raw_token<Json>(rparen_arg));
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
                            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter,line_,column_));
                            break;
                    };
                    break;
                case filter_state::expect_right_round_bracket: 
                    switch (*p)
                    {
                        case '\r':
                            if (p+1 < end_expr && *(p+1) == '\n')
                                ++p;
                            ++line_;
                            column_ = 1;
                            ++p;
                            break;
                        case '\n':
                            ++line_;
                            column_ = 1;
                            ++p;
                            break;
                        case ' ':case '\t':
                            break;
                        case ')':
                            push_token(raw_token<Json>(rparen_arg));
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
                            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter,line_,column_));
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
                                jsonpath_evaluator<Json,const Json&,detail::VoidPathConstructor<Json>> evaluator(buffer_line,buffer_column);
                                auto expr = evaluator.compile(resources, buffer.data(), buffer.length());
                                if (!path_mode_stack.empty())
                                {
                                    //if (path_mode_stack[0] == filter_path_mode::root_path)
                                    //{
                                    //    auto result = expr.evaluate(resources, root);
                                    //    if (result.size() > 0)
                                    //    {
                                    //        push_token(raw_token<Json>(value_term<Json>(std::move(result.at(0)))));
                                    //    }
                                    //}
                                    //else
                                    //{
                                    //    push_token(raw_token<Json>(raw_path_term<Json>(std::move(expr), buffer_line, buffer_column)));
                                    //}
                                    path_mode_stack.pop_back();
                                }
                                //else
                                {
                                    push_token(raw_token<Json>(raw_path_term<Json>(std::move(expr), buffer_line, buffer_column)));
                                }
                                buffer.clear();
                                buffer_line = buffer_column = 1;
                                buffer.push_back(*p);
                                ++p;
                                ++column_;
                                state = filter_state::oper;
                            }
                            break;
                        case ')':
                        {
                            jsonpath_evaluator<Json,const Json&,detail::VoidPathConstructor<Json>> evaluator(buffer_line,buffer_column);
                            auto expr = evaluator.compile(resources, buffer.data(), buffer.length());
                            if (!path_mode_stack.empty())
                            {
                                if (path_mode_stack[0] == filter_path_mode::root_path)
                                {
                                    //auto result = expr.evaluate(resources, root);
                                    //if (result.size() > 0)
                                    //{
                                    //    push_token(raw_token<Json>(value_term<Json>(std::move(result.at(0)))));
                                    //}
                                    push_token(raw_token<Json>(rparen_arg));
                                }
                                //else
                                {
                                    push_token(raw_token<Json>(raw_path_term<Json>(std::move(expr), buffer_line, buffer_column)));
                                }
                                path_mode_stack.pop_back();
                            }
                            else
                            {
                                push_token(raw_token<Json>(raw_path_term<Json>(std::move(expr), buffer_line, buffer_column)));
                                push_token(raw_token<Json>(rparen_arg));
                            }
                            buffer.clear();
                            buffer_line = buffer_column = 1;
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
                        }
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
                                if (p+1 < end_expr && *(p+1) == '\n')
                                    ++p;
                                ++line_;
                                column_ = 1;
                                ++p;
                                break;
                            case '\n':
                                ++line_;
                                column_ = 1;
                                ++p;
                                break;
                            case ' ':case '\t':
                                break;
                            case '/':
                                state = filter_state::regex;
                                break;
                            default: 
                                JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_expected_slash,line_,column_));
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
                                    push_token(raw_token<Json>(regex_term<Json>(buffer,flags)));
                                    buffer.clear();
                                    buffer_line = buffer_column = 1;
                                }
                                state = filter_state::expect_path_or_value_or_unary_op;
                                break;

                            default: 
                                buffer.push_back(*p);
                                break;
                        }
                        ++p;
                        ++column_;
                        break;
                    }
                    default:
                        ++p;
                        ++column_;
                        break;
                }
            }
            if (depth != 0)
            {
                JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unbalanced_paren,line_,column_));
            }
            *end_ptr = p;

            return jsonpath_filter_expr<Json>(std::move(output_stack_));
        }
    };

} // namespace detail
} // namespace jsonpath_new
} // namespace jsoncons

#endif
// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JMESPATH_JMESPATH_HPP
#define JSONCONS_JMESPATH_JMESPATH_HPP

#include <array> // std::array
#include <string>
#include <vector>
#include <memory>
#include <type_traits> // std::is_const
#include <limits> // std::numeric_limits
#include <utility> // std::move
#include <regex>
#include <set> // std::set
#include <iterator> // std::make_move_iterator
#include <functional> // 
#include <algorithm> // std::sort
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jmespath/jmespath_error.hpp>

namespace jsoncons { 
namespace jmespath {

    enum class token_type 
    {
        source_placeholder,
        lparen,
        rparen,
        begin_multi_select_hash,
        end_multi_select_hash,
        begin_multi_select_list,
        end_multi_select_list,
        begin_filter,
        end_filter,
        pipe,
        separator,
        key,
        literal,
        expression,
        binary_operator,
        unary_operator,
        end_of_expression
    };

    struct literal_arg_t
    {
        explicit literal_arg_t() = default;
    };
    constexpr literal_arg_t literal_arg{};

    struct end_of_expression_arg_t
    {
        explicit end_of_expression_arg_t() = default;
    };
    constexpr end_of_expression_arg_t end_of_expression_arg{};

    struct separator_arg_t
    {
        explicit separator_arg_t() = default;
    };
    constexpr separator_arg_t separator_arg{};

    struct key_arg_t
    {
        explicit key_arg_t() = default;
    };
    constexpr key_arg_t key_arg{};

    struct lparen_arg_t
    {
        explicit lparen_arg_t() = default;
    };
    constexpr lparen_arg_t lparen_arg{};

    struct rparen_arg_t
    {
        explicit rparen_arg_t() = default;
    };
    constexpr rparen_arg_t rparen_arg{};

    struct begin_multi_select_hash_arg_t
    {
        explicit begin_multi_select_hash_arg_t() = default;
    };
    constexpr begin_multi_select_hash_arg_t begin_multi_select_hash_arg{};

    struct end_multi_select_hash_arg_t
    {
        explicit end_multi_select_hash_arg_t() = default;
    };
    constexpr end_multi_select_hash_arg_t end_multi_select_hash_arg{};

    struct begin_multi_select_list_arg_t
    {
        explicit begin_multi_select_list_arg_t() = default;
    };
    constexpr begin_multi_select_list_arg_t begin_multi_select_list_arg{};

    struct end_multi_select_list_arg_t
    {
        explicit end_multi_select_list_arg_t() = default;
    };
    constexpr end_multi_select_list_arg_t end_multi_select_list_arg{};

    struct begin_filter_arg_t
    {
        explicit begin_filter_arg_t() = default;
    };
    constexpr begin_filter_arg_t begin_filter_arg{};

    struct end_filter_arg_t
    {
        explicit end_filter_arg_t() = default;
    };
    constexpr end_filter_arg_t end_filter_arg{};

    struct pipe_arg_t
    {
        explicit pipe_arg_t() = default;
    };
    constexpr pipe_arg_t pipe_arg{};

    struct source_placeholder_arg_t
    {
        explicit source_placeholder_arg_t() = default;
    };
    constexpr source_placeholder_arg_t source_placeholder_arg{};

    JSONCONS_STRING_LITERAL(sort_by,'s','o','r','t','-','b','y')

    struct slice
    {
        optional<int64_t> start_;
        optional<int64_t> stop_;
        int64_t step_;

        slice()
            : start_(), stop_(), step_(1)
        {
        }

        slice(const optional<int64_t>& start, const optional<int64_t>& end, int64_t step) 
            : start_(start), stop_(end), step_(step)
        {
        }

        slice(const slice& other)
            : start_(other.start_), stop_(other.stop_), step_(other.step_)
        {
        }

        slice& operator=(const slice& rhs) 
        {
            if (this != &rhs)
            {
                if (rhs.start_)
                {
                    start_ = rhs.start_;
                }
                else
                {
                    start_.reset();
                }
                if (rhs.stop_)
                {
                    stop_ = rhs.stop_;
                }
                else
                {
                    stop_.reset();
                }
                step_ = rhs.step_;
            }
            return *this;
        }

        int64_t get_start(std::size_t size) const
        {
            if (start_)
            {
                auto len = start_.value() >= 0 ? start_.value() : (static_cast<int64_t>(size) + start_.value());
                return len <= static_cast<int64_t>(size) ? len : static_cast<int64_t>(size);
            }
            else
            {
                if (step_ >= 0)
                {
                    return 0;
                }
                else 
                {
                    return static_cast<int64_t>(size);
                }
            }
        }

        int64_t get_stop(std::size_t size) const
        {
            if (stop_)
            {
                auto len = stop_.value() >= 0 ? stop_.value() : (static_cast<int64_t>(size) + stop_.value());
                return len <= static_cast<int64_t>(size) ? len : static_cast<int64_t>(size);
            }
            else
            {
                return step_ >= 0 ? static_cast<int64_t>(size) : -1;
            }
        }

        int64_t step() const
        {
            return step_; // Allow negative
        }
    };

    namespace detail {
    template <class Json,
        class JsonReference>
        class jmespath_evaluator;
    }

    template<class Json>
    Json search(const Json& root, const typename Json::string_view_type& path)
    {
        jsoncons::jmespath::detail::jmespath_evaluator<Json,const Json&> evaluator;
        return evaluator.evaluate(root, path);
    }

    template<class Json>
    Json search(const Json& root, const typename Json::string_view_type& path, std::error_code& ec)
    {
        jsoncons::jmespath::detail::jmespath_evaluator<Json,const Json&> evaluator;
        return evaluator.evaluate(root, path, ec);
    }

    namespace detail {
     
    enum class expr_state 
    {
        start,
        sub_or_index_expression,
        quoted_string,
        raw_string,
        raw_string_escape_char,
        quoted_string_escape_char,
        escape_u1, 
        escape_u2, 
        escape_u3, 
        escape_u4, 
        escape_expect_surrogate_pair1, 
        escape_expect_surrogate_pair2, 
        escape_u5, 
        escape_u6, 
        escape_u7, 
        escape_u8, 
        literal,
        key_expr,
        val_expr,
        identifier_or_function_expr,
        arg_or_right_paren,
        unquoted_string,
        expression_item,
        key_val_expr,
        number,
        digit,
        id_or_multiselectlist_or_multiselecthash_or_star,
        bracket_specifier9,
        bracket_specifier,
        filter,
        multi_select_list,
        multi_select_hash,
        bracket_specifier2,
        bracket_specifier3,
        bracket_specifier4,
        expect_dot,
        expect_filter_right_bracket,
        expect_right_brace,
        expect_colon,
        expect_multi_select_list,
        cmp_lt_or_lte,
        cmp_eq,
        cmp_gt_or_gte,
        cmp_ne,
        cmp_lt_or_lte_old,
        cmp_eq_old,
        cmp_gt_or_gte_old,
        cmp_ne_old,
        expect_pipe_or_or,
        expect_and
    };

    template<class Json,
             class JsonReference>
    class jmespath_evaluator : public ser_context
    {
    public:
        typedef typename Json::char_type char_type;
        typedef typename Json::char_traits_type char_traits_type;
        typedef std::basic_string<char_type,char_traits_type> string_type;
        typedef typename Json::string_view_type string_view_type;
        typedef JsonReference reference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;
        typedef typename Json::const_pointer const_pointer;

        // jmespath_context
        class jmespath_storage;

        static bool is_false(reference ref)
        {
            return (ref.is_array() && ref.empty()) ||
                   (ref.is_object() && ref.empty()) ||
                   (ref.is_string() && ref.as_string_view().size() == 0) ||
                   (ref.is_bool() && !ref.as_bool()) ||
                   ref.is_null();
        }

        static bool is_true(reference ref)
        {
            return !is_false(ref);
        }

        class unary_operator
        {
        public:
            virtual ~unary_operator() = default;
            virtual std::size_t precedence_level() const = 0;
            virtual bool is_right_associative() const = 0;
            virtual reference evaluate(reference val, jmespath_storage&, std::error_code& ec) = 0;
        };

        class not_expression : public unary_operator
        {
            std::size_t precedence_level() const override
            {
                return 1;
            }
            bool is_right_associative() const
            {
                return true;
            }
            reference evaluate(reference val, jmespath_storage&, std::error_code&) override
            {
                static const Json t(true, semantic_tag::none);
                static const Json f(false, semantic_tag::none);

                return is_false(val) ? t : f;
            }
        };

        class binary_operator
        {
        public:
            virtual ~binary_operator() = default;
            virtual std::size_t precedence_level() const = 0;
            virtual reference evaluate(reference lhs, reference rhs, jmespath_storage&, std::error_code& ec) = 0;

            virtual std::string to_string(size_t indent = 0) const
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("to_string not implemented\n");
                return s;
            }
        };

        // expression_base
        class expression_base
        {
        public:
            expression_base()
            {
            }

            virtual ~expression_base() = default;

            virtual std::size_t precedence_level() const
            {
                return 1;
            }

            virtual reference evaluate(reference val, jmespath_storage&, std::error_code& ec) = 0;

            virtual void add_expression(std::unique_ptr<expression_base>&& expressions) = 0;

            virtual std::string to_string(std::size_t = 0) const
            {
                return std::string("to_string not implemented");
            }

            virtual bool is_projection() const = 0;

            virtual bool is_right_associative() const = 0;
        };  

        // token

        class token
        {
        public:
            token_type type_;

            union
            {
                std::unique_ptr<expression_base> expression_;
                unary_operator* unary_operator_;
                std::unique_ptr<binary_operator> binary_operator_;
                Json value_;
                string_type key_;
            };
        public:

            token(source_placeholder_arg_t)
                : type_(token_type::source_placeholder)
            {
            }

            token(separator_arg_t)
                : type_(token_type::separator)
            {
            }

            token(lparen_arg_t)
                : type_(token_type::lparen)
            {
            }

            token(rparen_arg_t)
                : type_(token_type::rparen)
            {
            }

            token(end_of_expression_arg_t)
                : type_(token_type::end_of_expression)
            {
            }

            token(begin_multi_select_hash_arg_t)
                : type_(token_type::begin_multi_select_hash)
            {
            }

            token(end_multi_select_hash_arg_t)
                : type_(token_type::end_multi_select_hash)
            {
            }

            token(begin_multi_select_list_arg_t)
                : type_(token_type::begin_multi_select_list)
            {
            }

            token(end_multi_select_list_arg_t)
                : type_(token_type::end_multi_select_list)
            {
            }

            token(begin_filter_arg_t)
                : type_(token_type::begin_filter)
            {
            }

            token(end_filter_arg_t)
                : type_(token_type::end_filter)
            {
            }

            token(pipe_arg_t)
                : type_(token_type::pipe)
            {
            }

            token(key_arg_t, const string_type& key)
                : type_(token_type::key)
            {
                new (&key_) string_type(key);
            }

            token(std::unique_ptr<expression_base> expression)
                : type_(token_type::expression)
            {
                new (&expression_) std::unique_ptr<expression_base>(std::move(expression));
            }

            token(unary_operator* expression)
                : type_(token_type::unary_operator),
                  unary_operator_(expression)
            {
            }

            token(std::unique_ptr<binary_operator>&& expression)
                : type_(token_type::binary_operator)
            {
                new (&binary_operator_) std::unique_ptr<binary_operator>(std::move(expression));
            }

            token(literal_arg_t, Json&& value)
                : type_(token_type::literal), value_(std::move(value))
            {
            }

            token(token&& other)
            {
                construct(std::forward<token>(other));
            }

            token& operator=(token&& other)
            {
                if (&other != this)
                {
                    if (type_ == other.type_)
                    {
                        switch (type_)
                        {
                            case token_type::source_placeholder:
                            case token_type::separator:
                            case token_type::lparen:
                            case token_type::rparen:
                            case token_type::begin_multi_select_list:
                            case token_type::end_multi_select_list:
                            case token_type::begin_filter:
                            case token_type::end_filter:
                            case token_type::pipe:
                            case token_type::begin_multi_select_hash:
                            case token_type::end_multi_select_hash:
                            case token_type::end_of_expression:
                                break;
                            case token_type::expression:
                                expression_ = std::move(other.expression_);
                                break;
                            case token_type::key:
                                key_ = std::move(other.key_);
                                break;
                            case token_type::unary_operator:
                                unary_operator_ = other.unary_operator_;
                                break;
                            case token_type::binary_operator:
                                binary_operator_ = std::move(other.binary_operator_);
                                break;
                            case token_type::literal:
                                value_ = std::move(other.value_);
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

            ~token() noexcept
            {
                destroy();
            }

            token_type type() const
            {
                return type_;
            }

            bool is_lparen() const
            {
                return type_ == token_type::lparen; 
            }

            bool is_lbrace() const
            {
                return type_ == token_type::begin_multi_select_hash; 
            }

            bool is_key() const
            {
                return type_ == token_type::key; 
            }

            bool is_rparen() const
            {
                return type_ == token_type::rparen; 
            }

            bool is_source_placeholder() const
            {
                return type_ == token_type::source_placeholder; 
            }

            bool is_projection() const
            {
                return type_ == token_type::expression && expression_->is_projection(); 
            }

            bool is_expression() const
            {
                return type_ == token_type::expression; 
            }

            bool is_operator() const
            {
                return type_ == token_type::unary_operator || 
                       type_ == token_type::binary_operator; 
            }

            std::size_t precedence_level() const
            {
                switch(type_)
                {
                    case token_type::unary_operator:
                        return unary_operator_->precedence_level();
                    case token_type::binary_operator:
                        return binary_operator_->precedence_level();
                    case token_type::expression:
                        return expression_->precedence_level();
                    default:
                        return 0;
                }
            }

            bool is_right_associative() const
            {
                switch(type_)
                {
                    case token_type::unary_operator:
                        return unary_operator_->is_right_associative();
                    case token_type::expression:
                        return expression_->is_right_associative();
                    default:
                        return false;
                }
            }

            void construct(token&& other)
            {
                type_ = other.type_;
                switch (type_)
                {
                    case token_type::source_placeholder:
                    case token_type::separator:
                    case token_type::lparen:
                    case token_type::rparen:
                    case token_type::begin_multi_select_list:
                    case token_type::end_multi_select_list:
                    case token_type::begin_filter:
                    case token_type::end_filter:
                    case token_type::pipe:
                    case token_type::begin_multi_select_hash:
                    case token_type::end_multi_select_hash:
                    case token_type::end_of_expression:
                        break;
                    case token_type::expression:
                        new (&expression_) std::unique_ptr<expression_base>(std::move(other.expression_));
                        break;
                    case token_type::key:
                        new (&key_) string_type(std::move(other.key_));
                        break;
                    case token_type::unary_operator:
                        unary_operator_ = other.unary_operator_;
                        break;
                    case token_type::binary_operator:
                        new (&binary_operator_) std::unique_ptr<binary_operator>(std::move(other.binary_operator_));
                        break;
                    case token_type::literal:
                        new (&value_) Json(std::move(other.value_));
                        break;
                }
            }

            void destroy() noexcept 
            {
                switch(type_)
                {
                    case token_type::expression:
                        expression_.~unique_ptr();
                        break;
                    case token_type::key:
                        key_.~basic_string();
                        break;
                    case token_type::binary_operator:
                        binary_operator_.~unique_ptr();
                        break;
                    case token_type::literal:
                        value_.~Json();
                        break;
                    default:
                        break;
                }
            }

            std::string to_string(std::size_t indent = 0) const
            {
                switch(type_)
                {
                    case token_type::expression:
                        return expression_->to_string(indent);
                        break;
                    case token_type::unary_operator:
                        return std::string("unary_operator");
                        break;
                    case token_type::binary_operator:
                        return binary_operator_->to_string(indent);
                        break;
                    case token_type::source_placeholder:
                        return std::string("source_placeholder");
                        break;
                    case token_type::separator:
                        return std::string("separator");
                        break;
                    case token_type::literal:
                        return std::string("literal");
                        break;
                    case token_type::key:
                        return std::string("key") + key_;
                        break;
                    case token_type::begin_multi_select_hash:
                        return std::string("begin_multi_select_hash");
                        break;
                    case token_type::begin_multi_select_list:
                        return std::string("begin_multi_select_list");
                        break;
                    case token_type::begin_filter:
                        return std::string("begin_filter");
                        break;
                    case token_type::pipe:
                        return std::string("pipe");
                        break;
                    case token_type::lparen:
                        return std::string("lparen");
                        break;
                    default:
                        return std::string("default");
                        break;
                }
            }
        };

        static reference evaluate_tokens(reference root, const std::vector<token>& output_stack, jmespath_storage& storage, std::error_code& ec)
        {
            pointer root_ptr = std::addressof(root);
            std::vector<pointer> stack;
            for (std::size_t i = 0; i < output_stack.size(); ++i)
            {
                auto& t = output_stack[i];
                switch (t.type())
                {
                    case token_type::literal:
                    {
                        stack.push_back(&t.value_);
                        break;
                    }
                    case token_type::pipe:
                    {
                        JSONCONS_ASSERT(!stack.empty());
                        root_ptr = stack.back();
                        break;
                    }
                    case token_type::source_placeholder:
                        stack.push_back(root_ptr);
                        break;
                    case token_type::expression:
                    {
                        JSONCONS_ASSERT(!stack.empty());
                        auto ptr = stack.back();
                        stack.pop_back();
                        auto& ref = t.expression_->evaluate(*ptr, storage, ec);
                        stack.push_back(std::addressof(ref));
                        break;
                    }
                    case token_type::unary_operator:
                    {
                        JSONCONS_ASSERT(stack.size() >= 1);
                        auto ptr = stack.back();
                        stack.pop_back();
                        reference r = t.unary_operator_->evaluate(*ptr, storage, ec);
                        stack.push_back(std::addressof(r));
                        break;
                    }
                    case token_type::binary_operator:
                    {
                        JSONCONS_ASSERT(stack.size() >= 2);
                        auto rhs = stack.back();
                        stack.pop_back();
                        auto lhs = stack.back();
                        stack.pop_back();
                        reference r = t.binary_operator_->evaluate(*lhs,*rhs, storage, ec);
                        stack.push_back(std::addressof(r));
                        break;
                    }
                    default:
                        break;
                }
            }
            JSONCONS_ASSERT(stack.size() == 1);
            return *stack.back();
        }

        // Implementations

        class or_operator : public binary_operator
        {
            std::size_t precedence_level() const 
            {
                return 9;
            }
            reference evaluate(reference lhs, reference rhs, jmespath_storage&, std::error_code&) 
            {
                if (lhs.is_null() && rhs.is_null())
                {
                    return Json::null();
                }
                if (!is_false(lhs))
                {
                    return lhs;
                }
                else
                {
                    return rhs;
                }
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("or_operator\n");
                return s;
            }
        };

        class and_operator : public binary_operator
        {
            std::size_t precedence_level() const 
            {
                return 8;
            }
            reference evaluate(reference lhs, reference rhs, jmespath_storage&, std::error_code&) 
            {
                if (is_true(lhs))
                {
                    return rhs;
                }
                else
                {
                    return lhs;
                }
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("and_operator\n");
                return s;
            }
        };

        class eq_operator : public binary_operator
        {
            std::size_t precedence_level() const 
            {
                return 6;
            }
            reference evaluate(reference lhs, reference rhs, jmespath_storage&, std::error_code&) 
            {
                static const Json t(true, semantic_tag::none);
                static const Json f(false, semantic_tag::none);

                return lhs == rhs ? t : f;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("eq_operator\n");
                return s;
            }
        };

        class ne_operator : public binary_operator
        {
            std::size_t precedence_level() const 
            {
                return 6;
            }
            reference evaluate(reference lhs, reference rhs, jmespath_storage&, std::error_code&) 
            {
                static const Json t(true, semantic_tag::none);
                static const Json f(false, semantic_tag::none);

                return lhs != rhs ? t : f;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("ne_operator\n");
                return s;
            }
        };

        class lt_operator : public binary_operator
        {
            std::size_t precedence_level() const 
            {
                return 5;
            }
            reference evaluate(reference lhs, reference rhs, jmespath_storage&, std::error_code&) 
            {
                static const Json t(true, semantic_tag::none);
                static const Json f(false, semantic_tag::none);

                if (!(lhs.is_number() && rhs.is_number()))
                {
                    return Json::null();
                }
                return lhs < rhs ? t : f;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("lt_operator\n");
                return s;
            }
        };

        class lte_operator : public binary_operator
        {
            std::size_t precedence_level() const 
            {
                return 5;
            }
            reference evaluate(reference lhs, reference rhs, jmespath_storage&, std::error_code&) 
            {
                static const Json t(true, semantic_tag::none);
                static const Json f(false, semantic_tag::none);

                if (!(lhs.is_number() && rhs.is_number()))
                {
                    return Json::null();
                }
                return lhs <= rhs ? t : f;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("lte_operator\n");
                return s;
            }
        };

        class gt_operator : public binary_operator
        {
            std::size_t precedence_level() const 
            {
                return 5;
            }
            reference evaluate(reference lhs, reference rhs, jmespath_storage&, std::error_code&) 
            {
                static const Json t(true, semantic_tag::none);
                static const Json f(false, semantic_tag::none);

                if (!(lhs.is_number() && rhs.is_number()))
                {
                    return Json::null();
                }
                return lhs > rhs ? t : f;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("gt_operator\n");
                return s;
            }
        };

        class gte_operator : public binary_operator
        {
            std::size_t precedence_level() const 
            {
                return 5;
            }
            reference evaluate(reference lhs, reference rhs, jmespath_storage&, std::error_code&) 
            {
                static const Json t(true, semantic_tag::none);
                static const Json f(false, semantic_tag::none);

                if (!(lhs.is_number() && rhs.is_number()))
                {
                    return Json::null();
                }
                return lhs >= rhs ? t : f;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("gte_operator\n");
                return s;
            }
        };

        // selector_base
        class selector_base :  public expression_base
        {
            void add_expression(std::unique_ptr<expression_base>&&) override
            {
            }

            bool is_projection() const override
            {
                return false;
            }

            bool is_right_associative() const override
            {
                return false;
            }
        };

        class identifier_selector final : public selector_base
        {
        private:
            string_type identifier_;
        public:
            identifier_selector(const string_view_type& name)
                : identifier_(name)
            {
            }

            reference evaluate(reference val, jmespath_storage&, std::error_code&) override
            {
                //std::cout << "(identifier_selector " << identifier_  << " ) " << pretty_print(val) << "\n";
                if (val.is_object() && val.contains(identifier_))
                {
                    return val.at(identifier_);
                }
                else 
                {
                    return Json::null();
                }
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("identifier_selector ");
                s.append(identifier_);
                return s;
            }
        };

        class current_node final : public selector_base
        {
        public:
            current_node()
            {
            }

            reference evaluate(reference val, jmespath_storage&, std::error_code&) override
            {
                return val;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("current_node ");
                return s;
            }
        };

        class index_selector final : public selector_base
        {
            int64_t index_;
        public:
            index_selector(int64_t index)
                : index_(index)
            {
            }

            virtual std::size_t precedence_level() const
            {
                return 0;
            }

            reference evaluate(reference val, jmespath_storage&, std::error_code&) override
            {
                if (!val.is_array())
                {
                    return Json::null();
                }
                int64_t slen = static_cast<int64_t>(val.size());
                if (index_ >= 0 && index_ < slen)
                {
                    std::size_t index = static_cast<std::size_t>(index_);
                    return val.at(index);
                }
                else if ((slen + index_) >= 0 && (slen+index_) < slen)
                {
                    std::size_t index = static_cast<std::size_t>(slen + index_);
                    return val.at(index);
                }
                else
                {
                    return Json::null();
                }
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("index_selector ");
                s.append(std::to_string(index_));
                return s;
            }
        };

        // projection_base
        class projection_base : public expression_base
        {
        protected:
            std::vector<std::unique_ptr<expression_base>> expressions_;
        public:

            void add_expression(std::unique_ptr<expression_base>&& expr) override
            {
                if (!expressions_.empty() && expressions_.back()->is_projection() && 
                    (expr->precedence_level() < expressions_.back()->precedence_level() ||
                     (expr->precedence_level() == expressions_.back()->precedence_level() && expr->is_right_associative())))
                {
                    expressions_.back()->add_expression(std::move(expr));
                }
                else
                {
                    expressions_.emplace_back(std::move(expr));
                }
            }

            reference apply_expressions(reference val, jmespath_storage& storage, std::error_code& ec)
            {
                pointer ptr = std::addressof(val);
                for (auto& expression : expressions_)
                {
                    ptr = std::addressof(expression->evaluate(*ptr, storage, ec));
                }
                return *ptr;
            }

            bool is_projection() const override
            {
                return true;
            }

            bool is_right_associative() const override
            {
                return true;
            }
        };

        class object_projection final : public projection_base
        {
        public:
            object_projection()
            {
            }

            virtual std::size_t precedence_level() const
            {
                return 11;
            }

            reference evaluate(reference val, jmespath_storage& storage, std::error_code& ec) override
            {
                if (!val.is_object())
                {
                    return Json::null();
                }

                auto result = storage.new_instance(json_array_arg);
                for (auto item : val.object_range())
                {
                    if (!item.value().is_null())
                    {
                        auto j = this->apply_expressions(item.value(), storage, ec);
                        if (!j.is_null())
                        {
                            result->push_back(j);
                        }
                    }
                }
                return *result;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("object_projection\n");
                for (auto& expr : this->expressions_)
                {
                    std::string sss = expr->to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        class list_projection final : public projection_base
        {
        public:
            list_projection()
            {
            }

            virtual std::size_t precedence_level() const
            {
                return 11;
            }

            reference evaluate(reference val, jmespath_storage& storage, std::error_code& ec) override
            {
                if (!val.is_array())
                {
                    return Json::null();
                }

                auto result = storage.new_instance(json_array_arg);
                for (reference item : val.array_range())
                {
                    if (!item.is_null())
                    {
                        auto j = this->apply_expressions(item, storage, ec);
                        if (!j.is_null())
                        {
                            result->push_back(j);
                        }
                    }
                }
                return *result;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("list_projection\n");
                for (auto& expr : this->expressions_)
                {
                    std::string sss = expr->to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        class slice_projection final : public projection_base
        {
            slice slice_;
        public:
            slice_projection(const slice& s)
                : slice_(s)
            {
            }

            virtual std::size_t precedence_level() const
            {
                return 11;
            }

            reference evaluate(reference val, jmespath_storage& storage, std::error_code& ec) override
            {
                if (!val.is_array())
                {
                    return Json::null();
                }

                auto start = slice_.get_start(val.size());
                auto end = slice_.get_stop(val.size());
                auto step = slice_.step();

                if (step == 0)
                {
                    ec = jmespath_errc::step_cannot_be_zero;
                    return Json::null();
                }

                auto result = storage.new_instance(json_array_arg);
                if (step > 0)
                {
                    if (start < 0)
                    {
                        start = 0;
                    }
                    if (end > static_cast<int64_t>(val.size()))
                    {
                        end = val.size();
                    }
                    for (int64_t i = start; i < end; i += step)
                    {
                        auto j = this->apply_expressions(val.at(static_cast<std::size_t>(i)), storage, ec);
                        if (!j.is_null())
                        {
                            result->push_back(j);
                        }
                    }
                }
                else
                {
                    if (start >= static_cast<int64_t>(val.size()))
                    {
                        start = static_cast<int64_t>(val.size()) - 1;
                    }
                    if (end < -1)
                    {
                        end = -1;
                    }
                    for (int64_t i = start; i > end; i += step)
                    {
                        auto j = this->apply_expressions(val.at(static_cast<std::size_t>(i)), storage, ec);
                        if (!j.is_null())
                        {
                            result->push_back(j);
                        }
                    }
                }

                return *result;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("slice_projection\n");
                for (auto& expr : this->expressions_)
                {
                    std::string sss = expr->to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        class filter_expression final : public projection_base
        {
            std::vector<token> token_list_;
        public:
            filter_expression(std::vector<token>&& token_list)
                : token_list_(std::move(token_list))
            {
            }

            virtual std::size_t precedence_level() const
            {
                return 11;
            }

            reference evaluate(reference val, jmespath_storage& storage, std::error_code& ec) override
            {
                if (!val.is_array())
                {
                    return Json::null();
                }
                auto result = storage.new_instance(json_array_arg);

                for (auto& item : val.array_range())
                {
                    reference j = evaluate_tokens(item, token_list_, storage, ec);
                    if (is_true(j))
                    {
                        auto jj = this->apply_expressions(item, storage, ec);
                        if (!jj.is_null())
                        {
                            result->push_back(jj);
                        }
                    }
                }
                return *result;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("filter_expression\n");
                for (auto& item : token_list_)
                {
                    std::string sss = item.to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        class flatten_projection final : public projection_base
        {
        public:
            flatten_projection()
            {
            }

            virtual std::size_t precedence_level() const
            {
                return 11;
            }

            bool is_right_associative() const override
            {
                return false;
            }

            reference evaluate(reference val, jmespath_storage& storage, std::error_code& ec) override
            {
                if (!val.is_array())
                {
                    return Json::null();
                }

                auto currentp = storage.new_instance(json_array_arg);
                for (reference item : val.array_range())
                {
                    if (item.is_array())
                    {
                        for (reference item_of_item : item.array_range())
                        {
                            currentp->push_back(item_of_item);
                        }
                    }
                    else
                    {
                        currentp->push_back(item);
                    }
                }
                auto result = storage.new_instance(json_array_arg);
                for (reference item : currentp->array_range())
                {
                    if (!item.is_null())
                    {
                        auto j = this->apply_expressions(item, storage, ec);
                        if (!j.is_null())
                        {
                            result->push_back(j);
                        }
                    }
                }
                return *result;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("flatten_projection\n");
                for (auto& expr : this->expressions_)
                {
                    std::string sss = expr->to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        class multi_select_list final : public selector_base
        {
            std::vector<std::vector<token>> token_lists_;
        public:
            multi_select_list(std::vector<std::vector<token>>&& token_lists)
                : token_lists_(std::move(token_lists))
            {
            }

            virtual std::size_t precedence_level() const
            {
                return 1;
            }

            reference evaluate(reference val, jmespath_storage& storage, std::error_code& ec) override
            {
                if (val.is_null())
                {
                    return val;
                }
                auto result = storage.new_instance(json_array_arg);
                result->reserve(token_lists_.size());

                for (auto& list : token_lists_)
                {
                    result->push_back(evaluate_tokens(val, list, storage, ec));
                }
                return *result;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("multi_select_list\n");
                for (auto& list : token_lists_)
                {
                    for (auto& item : list)
                    {
                        std::string sss = item.to_string(indent+2);
                        s.insert(s.end(), sss.begin(), sss.end());
                        s.push_back('\n');
                    }
                    s.append("---\n");
                }
                return s;
            }
        };

        struct key_tokens
        {
            string_type key;
            std::vector<token> tokens;

            key_tokens(string_type&& key, std::vector<token>&& tokens)
                : key(std::move(key)), tokens(std::move(tokens))
            {
            }
        };

        class multi_select_hash final : public selector_base
        {
        public:
            std::vector<key_tokens> key_toks_;

            multi_select_hash(std::vector<key_tokens>&& key_toks)
                : key_toks_(std::move(key_toks))
            {
            }

            reference evaluate(reference val, jmespath_storage& storage, std::error_code& ec) override
            {
                if (val.is_null())
                {
                    return val;
                }
                auto resultp = storage.new_instance(json_object_arg);
                resultp->reserve(key_toks_.size());
                for (auto& item : key_toks_)
                {
                    resultp->try_emplace(item.key, evaluate_tokens(val, item.tokens, storage, ec));
                }

                return *resultp;
            }

            std::string to_string(std::size_t indent = 0) const override
            {
                std::string s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("multi_select_list\n");
                /*for (auto& key_expr : key_toks_)
                {
                    for (std::size_t i = 0; i <= indent+2; ++i)
                    {
                        s.push_back(' ');
                    }
                    s.append(key_expr.key);
                    std::string sss = key_expr.expression->to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }*/
                return s;
            }
        };

        class jmespath_storage
        {
            std::vector<std::unique_ptr<Json>> temp_storage_;
            not_expression not_expr_;
        public:

            jmespath_storage()
                : not_expr_()
            {
            }

            unary_operator* get_not_expression()
            {
                return &not_expr_;
            }

            template <typename... Args>
            Json* new_instance(Args&& ... args)
            {
                auto temp = jsoncons::make_unique<Json>(std::forward<Args>(args)...);
                Json* ptr = temp.get();
                temp_storage_.emplace_back(std::move(temp));
                return ptr;
            }
        };
    private:
        std::size_t line_;
        std::size_t column_;
        const char_type* begin_input_;
        const char_type* end_input_;
        const char_type* p_;

        jmespath_storage storage_;
        std::vector<expr_state> state_stack_;

        std::vector<token> output_stack_;
        std::vector<token> operator_stack_;

    public:
        jmespath_evaluator()
            : line_(1), column_(1),
              begin_input_(nullptr), end_input_(nullptr),
              p_(nullptr)
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

        reference evaluate(reference root, const string_view_type& path)
        {
            std::error_code ec;
            reference j = evaluate(root, path.data(), path.length(), ec);
            if (ec)
            {
                JSONCONS_THROW(jmespath_error(ec, line_, column_));
            }
            return j;
        }

        reference evaluate(reference root, const string_view_type& path, std::error_code& ec)
        {
            JSONCONS_TRY
            {
                return evaluate(root, path.data(), path.length(), ec);
            }
            JSONCONS_CATCH(...)
            {
                ec = jmespath_errc::unknown_error;
            }
        }
     
        reference evaluate(reference root, 
                           const char_type* path, 
                           std::size_t length,
                           std::error_code& ec)
        {
            push_token(source_placeholder_arg);
            state_stack_.emplace_back(expr_state::start);

            string_type buffer;
            uint32_t cp = 0;
            uint32_t cp2 = 0;
     
            begin_input_ = path;
            end_input_ = path + length;
            p_ = begin_input_;

            slice slic{};
            int paren_level = 0;

            while (p_ < end_input_)
            {
                switch (state_stack_.back())
                {
                    case expr_state::start: 
                    {
                        state_stack_.back() = expr_state::sub_or_index_expression;
                        state_stack_.emplace_back(expr_state::expression_item);
                        break;
                    }
                    case expr_state::sub_or_index_expression:
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '.': 
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(expr_state::id_or_multiselectlist_or_multiselecthash_or_star);
                                break;
                            case '|':
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(expr_state::expression_item);
                                state_stack_.emplace_back(expr_state::expect_pipe_or_or);
                                break;
                            case '&':
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(expr_state::expression_item);
                                state_stack_.emplace_back(expr_state::expect_and);
                                break;
                            case '<':
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(expr_state::expression_item);
                                state_stack_.emplace_back(expr_state::cmp_lt_or_lte);
                                break;
                            case '>':
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(expr_state::expression_item);
                                state_stack_.emplace_back(expr_state::cmp_gt_or_gte);
                                break;
                            case '(':
                            {
                                ++p_;
                                ++column_;
                                ++paren_level;
                                push_token(lparen_arg);
                                break;
                            }
                            case '=':
                            {
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(expr_state::expression_item);
                                state_stack_.emplace_back(expr_state::cmp_eq);
                                break;
                            }
                            case '!':
                            {
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(expr_state::expression_item);
                                state_stack_.emplace_back(expr_state::cmp_ne);
                                break;
                            }
                            case ')':
                            {
                                ++p_;
                                ++column_;
                                --paren_level;
                                push_token(rparen_arg);
                                break;
                            }
                            case '[':
                                //state_stack_.emplace_back(expr_state::expression_item);
                                state_stack_.emplace_back(expr_state::bracket_specifier);
                                ++p_;
                                ++column_;
                                break;
                            case '{':
                                state_stack_.emplace_back(expr_state::expression_item);
                                break;
                            default:
                                if (state_stack_.size() > 1)
                                {
                                    state_stack_.pop_back();
                                }
                                else
                                {
                                    ec = jmespath_errc::invalid_expression;
                                    return Json::null();
                                }
                                break;
                        }
                        break;
                    case expr_state::expression_item: 
                    {
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '\"':
                                state_stack_.back() = expr_state::val_expr;
                                state_stack_.emplace_back(expr_state::quoted_string);
                                ++p_;
                                ++column_;
                                break;
                            case '\'':
                                state_stack_.back() = expr_state::raw_string;
                                ++p_;
                                ++column_;
                                break;
                            case '`':
                                state_stack_.back() = expr_state::literal;
                                ++p_;
                                ++column_;
                                break;
                            case '{':
                                push_token(begin_multi_select_hash_arg);
                                state_stack_.back() = expr_state::multi_select_hash;
                                ++p_;
                                ++column_;
                                break;
                            case '*':
                                push_token(token(jsoncons::make_unique<object_projection>()));
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            case '(':
                            {
                                ++p_;
                                ++column_;
                                ++paren_level;
                                push_token(lparen_arg);
                                break;
                            }
                            case ')':
                            {
                                ++p_;
                                ++column_;
                                --paren_level;
                                push_token(rparen_arg);
                                break;
                            }
                            case '!':
                            {
                                ++p_;
                                ++column_;
                                push_token(token(storage_.get_not_expression()));
                                break;
                            }
                            case '@':
                                ++p_;
                                ++column_;
                                push_token(token(jsoncons::make_unique<current_node>()));
                                state_stack_.pop_back();
                                break;
                            case '[': // index
                                state_stack_.back() = expr_state::bracket_specifier;
                                ++p_;
                                ++column_;
                                break;
                            default:
                                if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                                {
                                    state_stack_.back() = expr_state::identifier_or_function_expr;
                                    state_stack_.emplace_back(expr_state::unquoted_string);
                                    buffer.push_back(*p_);
                                    ++p_;
                                    ++column_;
                                }
                                else
                                {
                                    ec = jmespath_errc::expected_identifier;
                                    return Json::null();
                                }
                                break;
                        };
                        break;
                    }
                    case expr_state::id_or_multiselectlist_or_multiselecthash_or_star: 
                    {
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '\"':
                                state_stack_.back() = expr_state::val_expr;
                                state_stack_.emplace_back(expr_state::quoted_string);
                                ++p_;
                                ++column_;
                                break;
                            case '{':
                                push_token(begin_multi_select_hash_arg);
                                state_stack_.back() = expr_state::multi_select_hash;
                                ++p_;
                                ++column_;
                                break;
                            case '*':
                                push_token(token(jsoncons::make_unique<object_projection>()));
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            case '[': 
                                state_stack_.back() = expr_state::expect_multi_select_list;
                                ++p_;
                                ++column_;
                                break;
                            default:
                                if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                                {
                                    state_stack_.back() = expr_state::identifier_or_function_expr;
                                    state_stack_.emplace_back(expr_state::unquoted_string);
                                    buffer.push_back(*p_);
                                    ++p_;
                                    ++column_;
                                }
                                else
                                {
                                    ec = jmespath_errc::expected_identifier;
                                    return Json::null();
                                }
                                break;
                        };
                        break;
                    }
                    case expr_state::key_expr:
                        switch (*p_)
                        {
                            case '\"':
                                ++p_;
                                ++column_;
                                push_token(token(key_arg, buffer));
                                buffer.clear(); 
                                state_stack_.pop_back(); 
                                break;
                            default:
                                push_token(token(key_arg, buffer));
                                buffer.clear(); 
                                state_stack_.pop_back(); 
                                break;
                        }
                        break;
                    case expr_state::val_expr:
                        switch (*p_)
                        {
                            case '\"':
                                ++p_;
                                ++column_;
                                push_token(token(jsoncons::make_unique<identifier_selector>(buffer)));
                                buffer.clear();
                                state_stack_.pop_back(); 
                                break;
                            default:
                                push_token(token(jsoncons::make_unique<identifier_selector>(buffer)));
                                buffer.clear();
                                state_stack_.pop_back(); 
                                break;
                        }
                        break;
                    case expr_state::identifier_or_function_expr:
                        switch(*p_)
                        {
                            case '(':
                            {
                                state_stack_.back() = expr_state::arg_or_right_paren;
                                state_stack_.emplace_back(expr_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                            {
                                push_token(token(jsoncons::make_unique<identifier_selector>(buffer)));
                                buffer.clear();
                                state_stack_.pop_back(); 
                                break;
                            }
                        }
                        break;

                    case expr_state::arg_or_right_paren:
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ',':
                                state_stack_.emplace_back(expr_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            case ')':
                            {
                                push_token(token(jsoncons::make_unique<identifier_selector>(buffer)));
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                break;
                        }
                        break;

                    case expr_state::quoted_string: 
                        switch (*p_)
                        {
                            case '\"':
                                state_stack_.pop_back(); // quoted_string
                                break;
                            case '\\':
                                state_stack_.emplace_back(expr_state::quoted_string_escape_char);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                                break;
                        };
                        break;

                    case expr_state::unquoted_string: 
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                state_stack_.pop_back(); // unquoted_string
                                advance_past_space_character();
                                break;
                            default:
                                if ((*p_ >= '0' && *p_ <= '9') || (*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                                {
                                    buffer.push_back(*p_);
                                    ++p_;
                                    ++column_;
                                }
                                else
                                {
                                    state_stack_.pop_back(); // unquoted_string
                                }
                                break;
                        };
                        break;
                    case expr_state::raw_string_escape_char:
                        switch (*p_)
                        {
                            case '\'':
                                buffer.push_back(*p_);
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            default:
                                buffer.push_back('\\');
                                buffer.push_back(*p_);
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                        }
                        break;
                    case expr_state::quoted_string_escape_char:
                        switch (*p_)
                        {
                            case '\"':
                                buffer.push_back('\"');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case '\\': 
                                buffer.push_back('\\');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case '/':
                                buffer.push_back('/');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case 'b':
                                buffer.push_back('\b');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case 'f':
                                buffer.push_back('\f');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case 'n':
                                buffer.push_back('\n');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case 'r':
                                buffer.push_back('\r');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case 't':
                                buffer.push_back('\t');
                                ++p_;
                                ++column_;
                                state_stack_.pop_back();
                                break;
                            case 'u':
                                ++p_;
                                ++column_;
                                state_stack_.back() = expr_state::escape_u1;
                                break;
                            default:
                                ec = jmespath_errc::illegal_escaped_character;
                                return Json::null();
                        }
                        break;
                    case expr_state::escape_u1:
                        cp = append_to_codepoint(0, *p_, ec);
                        if (ec)
                        {
                            return Json::null();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = expr_state::escape_u2;
                        break;
                    case expr_state::escape_u2:
                        cp = append_to_codepoint(cp, *p_, ec);
                        if (ec)
                        {
                            return Json::null();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = expr_state::escape_u3;
                        break;
                    case expr_state::escape_u3:
                        cp = append_to_codepoint(cp, *p_, ec);
                        if (ec)
                        {
                            return Json::null();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = expr_state::escape_u4;
                        break;
                    case expr_state::escape_u4:
                        cp = append_to_codepoint(cp, *p_, ec);
                        if (ec)
                        {
                            return Json::null();
                        }
                        if (unicons::is_high_surrogate(cp))
                        {
                            ++p_;
                            ++column_;
                            state_stack_.back() = expr_state::escape_expect_surrogate_pair1;
                        }
                        else
                        {
                            unicons::convert(&cp, &cp + 1, std::back_inserter(buffer));
                            ++p_;
                            ++column_;
                            state_stack_.pop_back();
                        }
                        break;
                    case expr_state::escape_expect_surrogate_pair1:
                        switch (*p_)
                        {
                            case '\\': 
                                ++p_;
                                ++column_;
                                state_stack_.back() = expr_state::escape_expect_surrogate_pair2;
                                break;
                            default:
                                ec = jmespath_errc::invalid_codepoint;
                                return Json::null();
                        }
                        break;
                    case expr_state::escape_expect_surrogate_pair2:
                        switch (*p_)
                        {
                            case 'u': 
                                ++p_;
                                ++column_;
                                state_stack_.back() = expr_state::escape_u5;
                                break;
                            default:
                                ec = jmespath_errc::invalid_codepoint;
                                return Json::null();
                        }
                        break;
                    case expr_state::escape_u5:
                        cp2 = append_to_codepoint(0, *p_, ec);
                        if (ec)
                        {
                            return Json::null();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = expr_state::escape_u6;
                        break;
                    case expr_state::escape_u6:
                        cp2 = append_to_codepoint(cp2, *p_, ec);
                        if (ec)
                        {
                            return Json::null();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = expr_state::escape_u7;
                        break;
                    case expr_state::escape_u7:
                        cp2 = append_to_codepoint(cp2, *p_, ec);
                        if (ec)
                        {
                            return Json::null();
                        }
                        ++p_;
                        ++column_;
                        state_stack_.back() = expr_state::escape_u8;
                        break;
                    case expr_state::escape_u8:
                    {
                        cp2 = append_to_codepoint(cp2, *p_, ec);
                        if (ec)
                        {
                            return Json::null();
                        }
                        uint32_t codepoint = 0x10000 + ((cp & 0x3FF) << 10) + (cp2 & 0x3FF);
                        unicons::convert(&codepoint, &codepoint + 1, std::back_inserter(buffer));
                        state_stack_.pop_back();
                        ++p_;
                        ++column_;
                        break;
                    }
                    case expr_state::raw_string: 
                        switch (*p_)
                        {
                            case '\'':
                            {
                                push_token(token(literal_arg, Json(buffer)));
                                buffer.clear();
                                state_stack_.pop_back(); // raw_string
                                ++p_;
                                ++column_;
                                break;
                            }
                            case '\\':
                                state_stack_.emplace_back(expr_state::raw_string_escape_char);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                                break;
                        };
                        break;
                    case expr_state::literal: 
                        switch (*p_)
                        {
                            case '`':
                            {
                                json_decoder<Json> decoder;
                                basic_json_reader<char_type,string_source<char_type>> reader(buffer, decoder);
                                std::error_code parse_ec;
                                reader.read(parse_ec);
                                if (parse_ec)
                                {
                                    ec = jmespath_errc::invalid_literal;
                                    return Json::null();
                                }
                                auto j = decoder.get_result();

                                push_token(token(literal_arg, std::move(j)));
                                buffer.clear();
                                state_stack_.pop_back(); // json_value
                                ++p_;
                                ++column_;
                                break;
                            }
                            case '\\':
                                if (p_+1 < end_input_)
                                {
                                    ++p_;
                                    ++column_;
                                    if (*p_ != '`')
                                    {
                                        buffer.push_back('\\');
                                    }
                                    buffer.push_back(*p_);
                                }
                                else
                                {
                                    ec = jmespath_errc::unexpected_end_of_input;
                                    return Json::null();
                                }
                                ++p_;
                                ++column_;
                                break;
                            default:
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                                break;
                        };
                        break;
                    case expr_state::number:
                        switch(*p_)
                        {
                            case '-':
                                buffer.push_back(*p_);
                                state_stack_.back() = expr_state::digit;
                                ++p_;
                                ++column_;
                                break;
                            default:
                                state_stack_.back() = expr_state::digit;
                                break;
                        }
                        break;
                    case expr_state::digit:
                        switch(*p_)
                        {
                            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                state_stack_.pop_back(); // digit
                                break;
                        }
                        break;

                    case expr_state::bracket_specifier:
                        switch(*p_)
                        {
                            case '*':
                                push_token(token(jsoncons::make_unique<list_projection>()));
                                state_stack_.back() = expr_state::bracket_specifier4;
                                ++p_;
                                ++column_;
                                break;
                            case ']':
                                push_token(token(jsoncons::make_unique<flatten_projection>()));
                                state_stack_.pop_back(); // bracket_specifier
                                ++p_;
                                ++column_;
                                break;
                            case '?':
                                push_token(token(begin_filter_arg));
                                state_stack_.back() = expr_state::filter;
                                state_stack_.emplace_back(expr_state::sub_or_index_expression);
                                state_stack_.emplace_back(expr_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            case ':':
                                state_stack_.back() = expr_state::bracket_specifier2;
                                state_stack_.emplace_back(expr_state::number);
                                ++p_;
                                ++column_;
                                break;
                            case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                state_stack_.back() = expr_state::bracket_specifier9;
                                state_stack_.emplace_back(expr_state::number);
                                break;
                            default:
                                push_token(token(begin_multi_select_list_arg));
                                state_stack_.back() = expr_state::multi_select_list;
                                state_stack_.emplace_back(expr_state::expression_item);
                                break;
                        }
                        break;

                    case expr_state::expect_multi_select_list:
                        switch(*p_)
                        {
                            case ']':
                            case '?':
                            case ':':
                            case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                ec = jmespath_errc::expected_multi_select_list;
                                return Json::null();
                            case '*':
                                push_token(token(jsoncons::make_unique<list_projection>()));
                                state_stack_.back() = expr_state::bracket_specifier4;
                                ++p_;
                                ++column_;
                                break;
                            default:
                                push_token(token(begin_multi_select_list_arg));
                                state_stack_.back() = expr_state::multi_select_list;
                                state_stack_.emplace_back(expr_state::expression_item);
                                break;
                        }
                        break;

                    case expr_state::multi_select_hash:
                        switch(*p_)
                        {
                            case '*':
                            case ']':
                            case '?':
                            case ':':
                            case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                break;
                            default:
                                state_stack_.back() = expr_state::key_val_expr;
                                break;
                        }
                        break;

                    case expr_state::bracket_specifier9:
                        switch(*p_)
                        {
                            case ']':
                            {
                                if (buffer.empty())
                                {
                                    push_token(token(jsoncons::make_unique<flatten_projection>()));
                                }
                                else
                                {
                                    auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                                    if (!r)
                                    {
                                        ec = jmespath_errc::invalid_number;
                                        return Json::null();
                                    }
                                    push_token(token(jsoncons::make_unique<index_selector>(r.value())));

                                    buffer.clear();
                                }
                                state_stack_.pop_back(); // bracket_specifier
                                ++p_;
                                ++column_;
                                break;
                            }
                            case ':':
                            {
                                if (!buffer.empty())
                                {
                                    auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                                    if (!r)
                                    {
                                        ec = jmespath_errc::invalid_number;
                                        return Json::null();
                                    }
                                    slic.start_ = r.value();
                                    buffer.clear();
                                }
                                state_stack_.back() = expr_state::bracket_specifier2;
                                state_stack_.emplace_back(expr_state::number);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return Json::null();
                        }
                        break;
                    case expr_state::bracket_specifier2:
                    {
                        if (!buffer.empty())
                        {
                            auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                            if (!r)
                            {
                                ec = jmespath_errc::invalid_number;
                                return Json::null();
                            }
                            slic.stop_ = optional<int64_t>(r.value());
                            buffer.clear();
                        }
                        switch(*p_)
                        {
                            case ']':
                                push_token(token(jsoncons::make_unique<slice_projection>(slic)));
                                slic = slice{};
                                state_stack_.pop_back(); // bracket_specifier2
                                ++p_;
                                ++column_;
                                break;
                            case ':':
                                state_stack_.back() = expr_state::bracket_specifier3;
                                state_stack_.emplace_back(expr_state::number);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::bracket_specifier3:
                    {
                        if (!buffer.empty())
                        {
                            auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                            if (!r)
                            {
                                ec = jmespath_errc::invalid_number;
                                return Json::null();
                            }
                            if (r.value() == 0)
                            {
                                ec = jmespath_errc::step_cannot_be_zero;
                                return Json::null();
                            }
                            slic.step_ = r.value();
                            buffer.clear();
                        }
                        switch(*p_)
                        {
                            case ']':
                                push_token(token(jsoncons::make_unique<slice_projection>(slic)));
                                buffer.clear();
                                slic = slice{};
                                state_stack_.pop_back(); // bracket_specifier3
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::bracket_specifier4:
                    {
                        switch(*p_)
                        {
                            case ']':
                                state_stack_.pop_back(); // bracket_specifier4
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::key_val_expr: 
                    {
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '\"':
                                state_stack_.back() = expr_state::expect_colon;
                                state_stack_.emplace_back(expr_state::key_expr);
                                state_stack_.emplace_back(expr_state::quoted_string);
                                ++p_;
                                ++column_;
                                break;
                            case '\'':
                                state_stack_.back() = expr_state::expect_colon;
                                state_stack_.emplace_back(expr_state::raw_string);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                                {
                                    state_stack_.back() = expr_state::expect_colon;
                                    state_stack_.emplace_back(expr_state::key_expr);
                                    state_stack_.emplace_back(expr_state::unquoted_string);
                                    buffer.push_back(*p_);
                                    ++p_;
                                    ++column_;
                                }
                                else
                                {
                                    ec = jmespath_errc::expected_key;
                                    return Json::null();
                                }
                                break;
                        };
                        break;
                    }
                    case expr_state::cmp_lt_or_lte:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token(jsoncons::make_unique<lte_operator>()));
                                push_token(token(source_placeholder_arg));
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            default:
                                push_token(token(jsoncons::make_unique<lt_operator>()));
                                push_token(token(source_placeholder_arg));
                                state_stack_.pop_back();
                                break;
                        }
                        break;
                    }
                    case expr_state::cmp_gt_or_gte:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token(jsoncons::make_unique<gte_operator>()));
                                push_token(token(source_placeholder_arg));
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            default:
                                push_token(token(jsoncons::make_unique<gt_operator>()));
                                push_token(token(source_placeholder_arg));
                                state_stack_.pop_back(); 
                                break;
                        }
                        break;
                    }
                    case expr_state::cmp_eq:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token(jsoncons::make_unique<eq_operator>()));
                                push_token(token(source_placeholder_arg));
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_comparator;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::cmp_ne:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token(jsoncons::make_unique<ne_operator>()));
                                push_token(token(source_placeholder_arg));
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_comparator;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::cmp_lt_or_lte_old:
                    {
                        switch(*p_)
                        {
                            case '=':
                                state_stack_.back() = expr_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(expr_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                state_stack_.back() = expr_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(expr_state::expression_item);
                                break;
                        }
                        break;
                    }
                    case expr_state::cmp_eq_old:
                    {
                        switch(*p_)
                        {
                            case '=':
                                state_stack_.back() = expr_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(expr_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_comparator;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::cmp_gt_or_gte_old:
                    {
                        switch(*p_)
                        {
                            case '=':
                                state_stack_.back() = expr_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(expr_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                state_stack_.back() = expr_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(expr_state::expression_item);
                                break;
                        }
                        break;
                    }
                    case expr_state::cmp_ne_old:
                    {
                        switch(*p_)
                        {
                            case '=':
                                state_stack_.back() = expr_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(expr_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_comparator;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::expect_dot:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '.':
                                state_stack_.pop_back(); // expect_dot
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_dot;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::expect_pipe_or_or:
                    {
                        switch(*p_)
                        {
                            case '|':
                                push_token(token(jsoncons::make_unique<or_operator>()));
                                push_token(token(source_placeholder_arg));
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            default:
                                push_token(token(pipe_arg));
                                state_stack_.pop_back(); 
                                break;
                        }
                        break;
                    }
                    case expr_state::expect_and:
                    {
                        switch(*p_)
                        {
                            case '&':
                                push_token(token(jsoncons::make_unique<and_operator>()));
                                push_token(token(source_placeholder_arg));
                                state_stack_.pop_back(); // expect_and
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_and;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::expect_filter_right_bracket:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ']':
                            {
                                state_stack_.pop_back();

                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::multi_select_list:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ',':
                                push_token(token(separator_arg));
                                state_stack_.emplace_back(expr_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                                state_stack_.emplace_back(expr_state::expression_item);
                                break;
                            case '.':
                                state_stack_.emplace_back(expr_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            case '|':
                            {
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(expr_state::expression_item);
                                state_stack_.emplace_back(expr_state::expect_pipe_or_or);
                                break;
                            }
                            case ']':
                            {
                                push_token(token(end_multi_select_list_arg));
                                state_stack_.pop_back();

                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::filter:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ']':
                            {
                                push_token(token(end_filter_arg));
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::expect_right_brace:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ',':
                                push_token(token(separator_arg));
                                state_stack_.back() = expr_state::key_val_expr; 
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                            case '{':
                                state_stack_.back() = expr_state::expect_right_brace;
                                state_stack_.emplace_back(expr_state::expression_item);
                                break;
                            case '.':
                                state_stack_.back() = expr_state::expect_right_brace;
                                state_stack_.emplace_back(expr_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            case '}':
                            {
                                state_stack_.pop_back();
                                push_token(end_multi_select_hash_arg);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jmespath_errc::expected_right_brace;
                                return Json::null();
                        }
                        break;
                    }
                    case expr_state::expect_colon:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ':':
                                state_stack_.back() = expr_state::expect_right_brace;
                                state_stack_.emplace_back(expr_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_colon;
                                return Json::null();
                        }
                        break;
                    }
                }
                
            }

            if (state_stack_.size() >= 3 && state_stack_.back() == expr_state::unquoted_string)
            {
                push_token(token(jsoncons::make_unique<identifier_selector>(buffer)));
                state_stack_.pop_back(); // unquoted_string
                if (state_stack_.back() == expr_state::val_expr || state_stack_.back() == expr_state::identifier_or_function_expr)
                {
                    buffer.clear();
                    state_stack_.pop_back(); // val_expr
                }
            }
            if (state_stack_.size() >= 3 && state_stack_.back() == expr_state::expect_dot)
            {
                state_stack_.pop_back(); // expr_state::expect_dot
                if (state_stack_.back() == expr_state::expression_item)
                {
                    state_stack_.pop_back(); // expression_item
                }
            }

            JSONCONS_ASSERT(state_stack_.size() == 1);
            JSONCONS_ASSERT(state_stack_.back() == expr_state::expression_item ||
                            state_stack_.back() == expr_state::sub_or_index_expression);
            state_stack_.pop_back();

            push_token(end_of_expression_arg);

            //for (auto& t : output_stack_)
            //{
            //    std::cout << t.to_string() << "\n";
            //}

            if (paren_level != 0)
            {
                ec = jmespath_errc::unbalanced_parentheses;
                return Json::null();
            }

            reference r = evaluate_tokens(root, output_stack_, storage_, ec);

            return r;
        }

        void advance_past_space_character()
        {
            switch (*p_)
            {
                case ' ':case '\t':
                    ++p_;
                    ++column_;
                    break;
                case '\r':
                    if (p_+1 < end_input_ && *(p_+1) == '\n')
                        ++p_;
                    ++line_;
                    column_ = 1;
                    ++p_;
                    break;
                case '\n':
                    ++line_;
                    column_ = 1;
                    ++p_;
                    break;
                default:
                    break;
            }
        }

        void unwind_rparen()
        {
            auto it = operator_stack_.rbegin();
            while (it != operator_stack_.rend() && !it->is_lparen())
            {
                output_stack_.emplace_back(std::move(*it));
                ++it;
            }
            if (it == operator_stack_.rend())
            {
                JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced parenthesis"));
            }
            ++it;
            operator_stack_.erase(it.base(),operator_stack_.end());
        }

        void push_token(token&& tok)
        {
            switch (tok.type())
            {
                case token_type::end_filter:
                {
                    unwind_rparen();
                    std::vector<token> toks;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_type::begin_filter)
                    {
                        toks.insert(toks.begin(), std::move(*it));
                        ++it;
                    }
                    if (it == output_stack_.rend())
                    {
                        JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced braces"));
                    }
                    ++it;
                    output_stack_.erase(it.base(),output_stack_.end());

                    if (toks.front().type() != token_type::literal)
                    {
                        toks.emplace(toks.begin(), source_placeholder_arg);
                    }
                    if (!output_stack_.empty() && output_stack_.back().is_projection() && 
                        (tok.precedence_level() < output_stack_.back().precedence_level() ||
                        (tok.precedence_level() == output_stack_.back().precedence_level() && tok.is_right_associative())))
                    {
                        output_stack_.back().expression_->add_expression(jsoncons::make_unique<filter_expression>(std::move(toks)));
                    }
                    else
                    {
                        output_stack_.emplace_back(token(jsoncons::make_unique<filter_expression>(std::move(toks))));
                    }
                    break;
                }
                case token_type::end_multi_select_list:
                {
                    unwind_rparen();
                    std::vector<std::vector<token>> vals;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_type::begin_multi_select_list)
                    {
                        std::vector<token> toks;
                        do
                        {
                            toks.insert(toks.begin(), std::move(*it));
                            ++it;
                        } while (it != output_stack_.rend() && it->type() != token_type::begin_multi_select_list && it->type() != token_type::separator);
                        if (it->type() == token_type::separator)
                        {
                            ++it;
                        }
                        if (toks.front().type() != token_type::literal)
                        {
                            toks.emplace(toks.begin(), source_placeholder_arg);
                        }
                        vals.insert(vals.begin(), std::move(toks));
                    }
                    if (it == output_stack_.rend())
                    {
                        JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced braces"));
                    }
                    ++it;
                    output_stack_.erase(it.base(),output_stack_.end());

                    if (!output_stack_.empty() && output_stack_.back().is_projection() && 
                        (tok.precedence_level() < output_stack_.back().precedence_level() ||
                        (tok.precedence_level() == output_stack_.back().precedence_level() && tok.is_right_associative())))
                    {
                        output_stack_.back().expression_->add_expression(jsoncons::make_unique<multi_select_list>(std::move(vals)));
                    }
                    else
                    {
                        output_stack_.emplace_back(token(jsoncons::make_unique<multi_select_list>(std::move(vals))));
                    }
                    break;
                }
                case token_type::end_multi_select_hash:
                {
                    unwind_rparen();
                    std::vector<key_tokens> key_toks;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_type::begin_multi_select_hash)
                    {
                        std::vector<token> toks;
                        do
                        {
                            toks.emplace(toks.begin(), std::move(*it));
                            ++it;
                        } while (it->type() != token_type::key);
                        JSONCONS_ASSERT(it->is_key());
                        auto key = std::move(it->key_);
                        ++it;
                        if (it->type() == token_type::separator)
                        {
                            ++it;
                        }
                        if (toks.front().type() != token_type::literal)
                        {
                            toks.emplace(toks.begin(), source_placeholder_arg);
                        }
                        key_toks.emplace(key_toks.begin(), std::move(key), std::move(toks));
                    }
                    if (it == output_stack_.rend())
                    {
                        JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced braces"));
                    }
                    ++it;
                    output_stack_.erase(it.base(),output_stack_.end());

                    if (!output_stack_.empty() && output_stack_.back().is_projection() && 
                        (tok.precedence_level() < output_stack_.back().precedence_level() ||
                        (tok.precedence_level() == output_stack_.back().precedence_level() && tok.is_right_associative())))
                    {
                        output_stack_.back().expression_->add_expression(jsoncons::make_unique<multi_select_hash>(std::move(key_toks)));
                    }
                    else
                    {
                        output_stack_.emplace_back(token(jsoncons::make_unique<multi_select_hash>(std::move(key_toks))));
                    }
                    break;
                }
                case token_type::literal:
                    if (!output_stack_.empty() && output_stack_.back().type() == token_type::source_placeholder)
                    {
                        output_stack_.back() = std::move(tok);
                    }
                    else
                    {
                        output_stack_.emplace_back(std::move(tok));
                    }
                    break;
                case token_type::expression:
                    if (!output_stack_.empty() && output_stack_.back().is_projection() && 
                        (tok.precedence_level() < output_stack_.back().precedence_level() ||
                        (tok.precedence_level() == output_stack_.back().precedence_level() && tok.is_right_associative())))
                    {
                        output_stack_.back().expression_->add_expression(std::move(tok.expression_));
                    }
                    else
                    {
                        output_stack_.emplace_back(std::move(tok));
                    }
                    break;
                case token_type::rparen:
                    {
                        unwind_rparen();
                        break;
                    }
                case token_type::end_of_expression:
                    {
                        auto it = operator_stack_.rbegin();
                        while (it != operator_stack_.rend())
                        {
                            output_stack_.emplace_back(std::move(*it));
                            ++it;
                        }
                        operator_stack_.clear();
                        break;
                    }
                case token_type::unary_operator:
                case token_type::binary_operator:
                {
                    if (operator_stack_.empty() || operator_stack_.back().is_lparen())
                    {
                        operator_stack_.emplace_back(std::move(tok));
                    }
                    else if (tok.precedence_level() < operator_stack_.back().precedence_level()
                             || (tok.precedence_level() == operator_stack_.back().precedence_level() && tok.is_right_associative()))
                    {
                        operator_stack_.emplace_back(std::move(tok));
                    }
                    else
                    {
                        auto it = operator_stack_.rbegin();
                        while (it != operator_stack_.rend() && it->is_operator()
                               && (tok.precedence_level() > it->precedence_level()
                             || (tok.precedence_level() == it->precedence_level() && tok.is_right_associative())))
                        {
                            output_stack_.emplace_back(std::move(*it));
                            ++it;
                        }

                        operator_stack_.erase(it.base(),operator_stack_.end());
                        operator_stack_.emplace_back(std::move(tok));
                    }
                    break;
                }
                case token_type::separator:
                {
                    unwind_rparen();
                    output_stack_.emplace_back(std::move(tok));
                    operator_stack_.emplace_back(token(lparen_arg));
                    break;
                }
                case token_type::begin_filter:
                    output_stack_.emplace_back(std::move(tok));
                    operator_stack_.emplace_back(token(lparen_arg));
                    break;
                case token_type::begin_multi_select_list:
                    output_stack_.emplace_back(std::move(tok));
                    operator_stack_.emplace_back(token(lparen_arg));
                    break;
                case token_type::begin_multi_select_hash:
                    output_stack_.emplace_back(std::move(tok));
                    operator_stack_.emplace_back(token(lparen_arg));
                    break;
                case token_type::source_placeholder:
                case token_type::key:
                case token_type::pipe:
                    output_stack_.emplace_back(std::move(tok));
                    break;
                case token_type::lparen:
                    operator_stack_.emplace_back(std::move(tok));
                    break;
                default:
                    break;
            }
        }

        uint32_t append_to_codepoint(uint32_t cp, int c, std::error_code& ec)
        {
            cp *= 16;
            if (c >= '0'  &&  c <= '9')
            {
                cp += c - '0';
            }
            else if (c >= 'a'  &&  c <= 'f')
            {
                cp += c - 'a' + 10;
            }
            else if (c >= 'A'  &&  c <= 'F')
            {
                cp += c - 'A' + 10;
            }
            else
            {
                ec = jmespath_errc::invalid_codepoint;
            }
            return cp;
        }
    };

    }

} // namespace jmespath
} // namespace jsoncons

#endif

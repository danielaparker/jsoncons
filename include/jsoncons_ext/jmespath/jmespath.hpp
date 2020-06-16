// Copyright 2020 Daniel 
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
        value,
        expression,
        projection,
        binary_expression,
        expression_begin,
        unary_expression,
        lparen,
        rparen
    };

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

    struct expression_begin_arg_t
    {
        explicit expression_begin_arg_t() = default;
    };
    constexpr expression_begin_arg_t expression_begin_arg{};

    JSONCONS_STRING_LITERAL(sort_by,'s','o','r','t','-','b','y')

    struct slice
    {
        int64_t start_;
        optional<int64_t> end_;
        int64_t step_;

        slice()
            : start_(0), end_(), step_(1)
        {
        }

        slice(int64_t start, const optional<int64_t>& end, int64_t step) 
            : start_(start), end_(end), step_(step)
        {
        }

        slice(const slice& other)
            : start_(other.start_), end_(other.end_), step_(other.step_)
        {
        }

        slice& operator=(const slice& rhs) 
        {
            if (this != &rhs)
            {
                start_ = rhs.start_;
                if (rhs.end_)
                {
                    end_ = rhs.end_;
                }
                else
                {
                    end_.reset();
                }
                step_ = rhs.step_;
            }
            return *this;
        }

        int64_t get_start(std::size_t size) const
        {
            return start_ >= 0 ? start_ : (static_cast<int64_t>(size) + start_);
        }

        int64_t get_end(std::size_t size) const
        {
            if (end_)
            {
                auto len = end_.value() >= 0 ? end_.value() : (static_cast<int64_t>(size) + end_.value());
                return len <= static_cast<int64_t>(size) ? len : static_cast<int64_t>(size);
            }
            else
            {
                return static_cast<int64_t>(size);
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
     
    enum class path_state 
    {
        start,
        sub_or_index_expression,
        quoted_string,
        raw_string,
        json_value,
        key_expr,
        val_expr,
        identifier_or_function_expr,
        arg_or_right_paren,
        unquoted_string,
        expression_item,
        key_val_expr,
        number,
        digit,
        bracket_specifier9,
        bracket_specifier,
        multi_select_hash,
        bracket_specifier2,
        bracket_specifier3,
        bracket_specifier4,
        expect_dot,
        expect_filter_right_bracket,
        expect_right_bracket4,
        expect_right_brace,
        expect_colon,
        comparator,
        cmp_lt_or_lte,
        cmp_eq,
        cmp_gt_or_gte,
        cmp_ne,
        comparator_old,
        cmp_lt_or_lte_old,
        cmp_eq_old,
        cmp_gt_or_gte_old,
        cmp_ne_old,
        expect_or,
        expect_and
    };

    template<class Json,
             class JsonReference>
    class jmespath_evaluator : public ser_context
    {
        typedef typename Json::char_type char_type;
        typedef typename Json::char_traits_type char_traits_type;
        typedef std::basic_string<char_type,char_traits_type> string_type;
        typedef typename Json::string_view_type string_view_type;
        typedef JsonReference reference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;
        typedef typename Json::const_pointer const_pointer;

        // jmespath_context

        class jmespath_storage
        {
            std::vector<std::unique_ptr<Json>> temp_storage_;
        public:

            template <typename... Args>
            Json* new_instance(Args&& ... args)
            {
                auto temp = jsoncons::make_unique<Json>(std::forward<Args>(args)...);
                Json* ptr = temp.get();
                temp_storage_.emplace_back(std::move(temp));
                return ptr;
            }
        };

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

        class unary_expression
        {
        public:
            virtual std::size_t precedence_level() const = 0;
            virtual bool is_right_associative() const = 0;
            virtual reference evaluate(reference val, jmespath_storage&, std::error_code& ec) = 0;
        };

        class not_expression : public unary_expression
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

        class binary_expression
        {
        public:
            virtual std::size_t precedence_level() const = 0;
            virtual reference evaluate(reference lhs, reference rhs, jmespath_storage&, std::error_code& ec) = 0;

            virtual string_type to_string() const
            {
                return string_type("to_string not implemented");
            }
        };

        class or_expression : public binary_expression
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

            string_type to_string() const override
            {
                return string_type("or_expression");
            }
        };

        class and_expression : public binary_expression
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

            string_type to_string() const override
            {
                return string_type("and_expression");
            }
        };

        class eq_expression : public binary_expression
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

            string_type to_string() const override
            {
                return string_type("eq_expression");
            }
        };

        class ne_expression : public binary_expression
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

            string_type to_string() const override
            {
                return string_type("ne_expression");
            }
        };

        class lt_expression : public binary_expression
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

            string_type to_string() const override
            {
                return string_type("lt_expression");
            }
        };

        class lte_expression : public binary_expression
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

            string_type to_string() const override
            {
                return string_type("lte_expression");
            }
        };

        class gt_expression : public binary_expression
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

            string_type to_string() const override
            {
                return string_type("gt_expression");
            }
        };

        class gte_expression : public binary_expression
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

            string_type to_string() const override
            {
                return string_type("gte_expression");
            }
        };

        // expression_base
        class expression_base
        {
        public:
            expression_base()
            {
            }

            virtual ~expression_base()
            {
            }

            virtual reference evaluate(reference val, jmespath_storage&, std::error_code& ec) = 0;

            virtual string_type to_string() const
            {
                return string_type("to_string not implemented");
            }
        };

        class identifier_expression final : public expression_base
        {
        private:
            string_type identifier_;
        public:
            identifier_expression(const string_view_type& name)
                : identifier_(name)
            {
            }

            reference evaluate(reference val, jmespath_storage&, std::error_code&) override
            {
                //std::cout << "(identifier_expression " << identifier_  << " ) " << pretty_print(val) << "\n";
                if (val.is_object() && val.contains(identifier_))
                {
                    return val.at(identifier_);
                }
                else 
                {
                    return Json::null();
                }
            }

            string_type to_string() const override
            {
                return string_type("identifier_expression ") + identifier_ + "\n";
            }
        };


        // projection_base
        class projection_base
        {
        public:
            projection_base()
            {
            }

            virtual ~projection_base()
            {
            }

            virtual reference evaluate(reference val, std::vector<std::unique_ptr<expression_base>>& selectors, jmespath_storage&, std::error_code& ec) = 0;

            virtual string_type to_string() const
            {
                return string_type("to_string not implemented");
            }
        };

        class object_projection final : public projection_base
        {
        public:
            object_projection()
            {
            }

            reference evaluate(reference val, std::vector<std::unique_ptr<expression_base>>& selectors, jmespath_storage& storage, std::error_code& ec) override
            {
                if (!val.is_object())
                {
                    return Json::null();
                }

                auto result = storage.new_instance(json_array_arg);
                for (auto& item : val.object_range())
                {
                    if (!item.value().is_null())
                    {
                        pointer ptr = std::addressof(item.value());
                        for (auto& selector : selectors)
                        {
                            ptr = std::addressof(selector->evaluate(*ptr, storage, ec)        );
                        }
                        if (!ptr->is_null())
                        {
                            result->push_back(*ptr);
                        }
                    }
                }
                return *result;
            }

            string_type to_string() const override
            {
                return string_type("object_projection\n");
            }
        };


        // token

        class token
        {
        public:
            token_type type_;

            union
            {
                std::unique_ptr<expression_base> expression_;
                std::unique_ptr<projection_base> projection_;
                std::unique_ptr<unary_expression> unary_expression_;
                std::unique_ptr<binary_expression> binary_expression_;
            };
        public:

            token(expression_begin_arg_t)
                : type_(token_type::expression_begin)
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

            token(std::unique_ptr<expression_base>&& expression)
                : type_(token_type::expression)
            {
                new (&expression_) std::unique_ptr<expression_base>(std::move(expression));
            }

            token(std::unique_ptr<projection_base>&& projection)
                : type_(token_type::projection)
            {
                new (&projection_) std::unique_ptr<projection_base>(std::move(projection));
            }

            token(std::unique_ptr<unary_expression>&& expression)
                : type_(token_type::unary_expression)
            {
                new (&unary_expression_) std::unique_ptr<unary_expression>(std::move(expression));
            }

            token(std::unique_ptr<binary_expression>&& expression)
                : type_(token_type::binary_expression)
            {
                new (&binary_expression_) std::unique_ptr<binary_expression>(std::move(expression));
            }

            token(token&& other)
                : type_(token_type::lparen)
            {
                swap(other);
            }

            void swap(token& other) noexcept
            {
                if (type_ == other.type_)
                {
                    switch (type_)
                    {
                        case token_type::expression:
                            expression_.swap(other.expression_);
                            break;
                        case token_type::projection:
                            projection_.swap(other.projection_);
                            break;
                        case token_type::unary_expression:
                            unary_expression_.swap(other.unary_expression_);
                            break;
                        case token_type::binary_expression:
                            binary_expression_.swap(other.binary_expression_);
                            break;
                    }
                }
                else
                {
                    switch (type_)
                    {
                        case token_type::value:
                            break;
                        case token_type::expression:
                            new (&other.expression_) std::unique_ptr<expression_base>(std::move(expression_));
                            break;
                        case token_type::projection:
                            new (&other.projection_) std::unique_ptr<projection_base>(std::move(projection_));
                            break;
                        case token_type::unary_expression:
                            new (&other.unary_expression_) std::unique_ptr<unary_expression>(std::move(unary_expression_));
                            break;
                        case token_type::binary_expression:
                            new (&other.binary_expression_) std::unique_ptr<binary_expression>(std::move(binary_expression_));
                            break;
                        default:
                            break;
                    }
                    switch (other.type_)
                    {
                        case token_type::value:
                            break;
                        case token_type::expression:
                            new (&expression_) std::unique_ptr<expression_base>(std::move(other.expression_));
                            break;
                        case token_type::projection:
                            new (&projection_) std::unique_ptr<projection_base>(std::move(other.projection_));
                            break;
                        case token_type::unary_expression:
                            new (&unary_expression_) std::unique_ptr<unary_expression>(std::move(other.unary_expression_));
                            break;
                        case token_type::binary_expression:
                            new (&binary_expression_) std::unique_ptr<binary_expression>(std::move(other.binary_expression_));
                            break;
                        default:
                            break;
                    }
                    std::swap(type_,other.type_);
                }

            }

            token& operator=(token&& other)
            {
                if (&other != this)
                {
                    swap(other);
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

            bool is_rparen() const
            {
                return type_ == token_type::rparen; 
            }

            bool is_expression_begin() const
            {
                return type_ == token_type::expression_begin; 
            }

            bool is_expression() const
            {
                return type_ == token_type::expression; 
            }

            bool is_projection() const
            {
                return type_ == token_type::projection; 
            }

            bool is_operator() const
            {
                return type_ == token_type::unary_expression || 
                       type_ == token_type::binary_expression; 
            }

            std::size_t precedence_level() const
            {
                switch(type_)
                {
                    case token_type::unary_expression:
                        return unary_expression_->precedence_level();
                    case token_type::binary_expression:
                        return binary_expression_->precedence_level();
                    default:
                        return 0;
                }
            }

            bool is_right_associative() const
            {
                switch(type_)
                {
                    case token_type::unary_expression:
                        return unary_expression_->is_right_associative();
                    default:
                        return false;
                }
            }

            void destroy() noexcept 
            {
                switch(type_)
                {
                    case token_type::expression:
                        expression_.~unique_ptr();
                        break;
                    case token_type::projection:
                        expression_.~unique_ptr();
                        break;
                    case token_type::unary_expression:
                        unary_expression_.~unique_ptr();
                        break;
                    case token_type::binary_expression:
                        binary_expression_.~unique_ptr();
                        break;
                    default:
                        break;
                }
            }

            string_type to_string() const
            {
                switch(type_)
                {
                    case token_type::expression:
                        return expression_->to_string();
                        break;
                    case token_type::unary_expression:
                        return string_type("unary_expression");
                        break;
                    case token_type::binary_expression:
                        return binary_expression_->to_string();
                        break;
                    case token_type::expression_begin:
                        return string_type("expression_begin");
                        break;
                    default:
                        return string_type("default");
                        break;
                }
            }
        };

        std::size_t line_;
        std::size_t column_;
        const char_type* begin_input_;
        const char_type* end_input_;
        const char_type* p_;

        std::vector<path_state> state_stack_;
        jmespath_storage storage_;

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
            push_token(lparen_arg);
            push_token(expression_begin_arg);
            state_stack_.emplace_back(path_state::start);

            string_type buffer;
     
            begin_input_ = path;
            end_input_ = path + length;
            p_ = begin_input_;

            slice a_slice{};

            while (p_ < end_input_)
            {
                switch (state_stack_.back())
                {
                    case path_state::start: 
                    {
                        state_stack_.back() = path_state::sub_or_index_expression;
                        state_stack_.emplace_back(path_state::expression_item);
                        break;
                    }
                    case path_state::sub_or_index_expression:
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '.': 
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::expression_item);
                                break;
                            case '|':
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::expression_item);
                                state_stack_.emplace_back(path_state::expect_or);
                                break;
                            case '&':
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::expression_item);
                                state_stack_.emplace_back(path_state::expect_and);
                                break;

                            case '<':
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::expression_item);
                                state_stack_.emplace_back(path_state::cmp_lt_or_lte);
                                break;
                            case '>':
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::expression_item);
                                state_stack_.emplace_back(path_state::cmp_gt_or_gte);
                                break;
                            case '(':
                            {
                                ++p_;
                                ++column_;
                                push_token(lparen_arg);
                                break;
                            }
                            case '=':
                            {
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::expression_item);
                                state_stack_.emplace_back(path_state::cmp_eq);
                                break;
                            }
                            case '!':
                            {
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::expression_item);
                                state_stack_.emplace_back(path_state::cmp_ne);
                                break;
                            }
                            case ')':
                            {
                                ++p_;
                                ++column_;
                                push_token(rparen_arg);
                                break;
                            }
                            case '[':
                            case '{':
                                state_stack_.emplace_back(path_state::expression_item);
                                break;
                            default:
                                ec = jmespath_errc::expected_index;
                                return Json::null();
                        }
                        break;
                    case path_state::expression_item: 
                    {
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '\"':
                                state_stack_.back() = path_state::val_expr;
                                state_stack_.emplace_back(path_state::quoted_string);
                                ++p_;
                                ++column_;
                                break;
                            case '\'':
                                state_stack_.back() = path_state::raw_string;
                                ++p_;
                                ++column_;
                                break;
                            case '`':
                                state_stack_.back() = path_state::json_value;
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                                state_stack_.back() = path_state::bracket_specifier;
                                ++p_;
                                ++column_;
                                break;
                            case '{':
                                state_stack_.back() = path_state::multi_select_hash;
                                ++p_;
                                ++column_;
                                break;
                            case '*':
                                push_token(token(jsoncons::make_unique<object_projection>()));
                                state_stack_.emplace_back(path_state::expect_dot);
                                ++p_;
                                ++column_;
                                break;
                            case '(':
                            {
                                ++p_;
                                ++column_;
                                push_token(lparen_arg);
                                break;
                            }
                            case ')':
                            {
                                ++p_;
                                ++column_;
                                push_token(rparen_arg);
                                break;
                            }
                            case '!':
                            {
                                ++p_;
                                ++column_;
                                push_token(token(jsoncons::make_unique<not_expression>()));
                                break;
                            }
                            default:
                                if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                                {
                                    state_stack_.back() = path_state::identifier_or_function_expr;
                                    state_stack_.emplace_back(path_state::unquoted_string);
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
                    case path_state::key_expr:
                        buffer.clear(); 
                        state_stack_.pop_back(); 
                        break;
                    case path_state::val_expr:
                        switch(*p_)
                        {
                            case '\"':
                                push_token(token(jsoncons::make_unique<identifier_expression>(buffer)));
                                ++p_;
                                ++column_;
                                buffer.clear();
                                state_stack_.pop_back(); 
                                break;
                            default:
                                push_token(token(jsoncons::make_unique<identifier_expression>(buffer)));
                                buffer.clear();
                                state_stack_.pop_back(); 
                                break;
                        }
                        break;
                    case path_state::identifier_or_function_expr:
                        switch(*p_)
                        {
                            case '(':
                            {
                                state_stack_.back() = path_state::arg_or_right_paren;
                                state_stack_.emplace_back(path_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                            {
                                push_token(token(jsoncons::make_unique<identifier_expression>(buffer)));
                                buffer.clear();
                                state_stack_.pop_back(); 
                                break;
                            }
                        }
                        break;

                    case path_state::arg_or_right_paren:
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ',':
                                state_stack_.emplace_back(path_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            case ')':
                            {
                                push_token(token(jsoncons::make_unique<identifier_expression>(buffer)));
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                break;
                        }
                        break;

                    case path_state::quoted_string: 
                        switch (*p_)
                        {
                            case '\"':
                                state_stack_.pop_back(); // quoted_string
                                break;
                            case '\\':
                                if (p_+1 < end_input_)
                                {
                                    ++p_;
                                    ++column_;
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

                    case path_state::unquoted_string: 
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
                    case path_state::raw_string: 
                        switch (*p_)
                        {
                            case '\'':
                            {
                                buffer.clear();
                                state_stack_.pop_back(); // raw_string
                                ++p_;
                                ++column_;
                                break;
                            }
                            case '\\':
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
                    case path_state::json_value: 
                        switch (*p_)
                        {
                            case '`':
                            {
                                auto j = Json::parse(buffer);
                                buffer.clear();
                                state_stack_.pop_back(); // json_value
                                ++p_;
                                ++column_;
                                break;
                            }
                            case '\\':
                                break;
                            default:
                                buffer.push_back(*p_);
                                ++p_;
                                ++column_;
                                break;
                        };
                        break;
                    case path_state::number:
                        switch(*p_)
                        {
                            case '-':
                                buffer.push_back(*p_);
                                state_stack_.back() = path_state::digit;
                                ++p_;
                                ++column_;
                                break;
                            default:
                                state_stack_.back() = path_state::digit;
                                break;
                        }
                        break;
                    case path_state::digit:
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

                    case path_state::bracket_specifier:
                        switch(*p_)
                        {
                            case '*':
                                state_stack_.back() = path_state::bracket_specifier4;
                                ++p_;
                                ++column_;
                                break;
                            case ']':
                                state_stack_.pop_back(); // bracket_specifier
                                ++p_;
                                ++column_;
                                break;
                            case '?':
                                state_stack_.back() = path_state::comparator_old;
                                state_stack_.emplace_back(path_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            case ':':
                                state_stack_.back() = path_state::bracket_specifier2;
                                state_stack_.emplace_back(path_state::number);
                                ++p_;
                                ++column_;
                                break;
                            case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                state_stack_.back() = path_state::bracket_specifier9;
                                state_stack_.emplace_back(path_state::number);
                                break;
                            default:
                                state_stack_.back() = path_state::expect_right_bracket4;
                                state_stack_.emplace_back(path_state::expression_item);
                                break;
                        }
                        break;

                    case path_state::multi_select_hash:
                        switch(*p_)
                        {
                            case '*':
                            case ']':
                            case '?':
                            case ':':
                            case '-':case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                break;
                            default:
                                state_stack_.back() = path_state::key_val_expr;
                                break;
                        }
                        break;

                    case path_state::bracket_specifier9:
                        switch(*p_)
                        {
                            case ']':
                            {
                                if (buffer.empty())
                                {
                                }
                                else
                                {
                                    auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                                    if (!r)
                                    {
                                        ec = jmespath_errc::invalid_number;
                                        return Json::null();
                                    }

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
                                    a_slice.start_ = r.value();
                                    buffer.clear();
                                }
                                state_stack_.back() = path_state::bracket_specifier2;
                                state_stack_.emplace_back(path_state::number);
                                ++p_;
                                ++column_;
                                break;
                            }
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return Json::null();
                        }
                        break;
                    case path_state::bracket_specifier2:
                    {
                        if (!buffer.empty())
                        {
                            auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                            if (!r)
                            {
                                ec = jmespath_errc::invalid_number;
                                return Json::null();
                            }
                            a_slice.end_ = optional<int64_t>(r.value());
                            buffer.clear();
                        }
                        switch(*p_)
                        {
                            case ']':
                                a_slice = slice{};
                                state_stack_.pop_back(); // bracket_specifier2
                                ++p_;
                                ++column_;
                                break;
                            case ':':
                                state_stack_.back() = path_state::bracket_specifier3;
                                state_stack_.emplace_back(path_state::number);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_right_bracket;
                                return Json::null();
                        }
                        break;
                    }
                    case path_state::bracket_specifier3:
                    {
                        if (!buffer.empty())
                        {
                            auto r = jsoncons::detail::to_integer<int64_t>(buffer.data(), buffer.size());
                            if (!r)
                            {
                                ec = jmespath_errc::invalid_number;
                                return Json::null();
                            }
                            a_slice.step_ = r.value();
                            buffer.clear();
                        }
                        switch(*p_)
                        {
                            case ']':
                                buffer.clear();
                                a_slice = slice{};
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
                    case path_state::bracket_specifier4:
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
                    case path_state::key_val_expr: 
                    {
                        switch (*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case '\"':
                                state_stack_.back() = path_state::expect_colon;
                                state_stack_.emplace_back(path_state::key_expr);
                                state_stack_.emplace_back(path_state::quoted_string);
                                ++p_;
                                ++column_;
                                break;
                            case '\'':
                                state_stack_.back() = path_state::expect_colon;
                                state_stack_.emplace_back(path_state::raw_string);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                if ((*p_ >= 'A' && *p_ <= 'Z') || (*p_ >= 'a' && *p_ <= 'z') || (*p_ == '_'))
                                {
                                    state_stack_.back() = path_state::expect_colon;
                                    state_stack_.emplace_back(path_state::key_expr);
                                    state_stack_.emplace_back(path_state::unquoted_string);
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
                    case path_state::comparator_old:
                    {
                        switch(*p_)
                        {
                            case '.': 
                                ++p_;
                                ++column_;
                                state_stack_.emplace_back(path_state::expression_item);
                                break;
                            case '<':
                                state_stack_.back() = path_state::cmp_lt_or_lte_old;
                                ++p_;
                                ++column_;
                                break;
                            case '=':
                                state_stack_.back() = path_state::cmp_eq_old;
                                ++p_;
                                ++column_;
                                break;
                            case '>':
                                state_stack_.back() = path_state::cmp_gt_or_gte_old;
                                ++p_;
                                ++column_;
                                break;
                            case '!':
                                state_stack_.back() = path_state::cmp_ne_old;
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_comparator;
                                return Json::null();
                        }
                        break;
                    }
                    case path_state::cmp_lt_or_lte:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token(jsoncons::make_unique<lte_expression>()));
                                push_token(token(expression_begin_arg));
                                state_stack_.pop_back();
                                ++p_;
                                ++column_;
                                break;
                            default:
                                push_token(token(jsoncons::make_unique<lt_expression>()));
                                push_token(token(expression_begin_arg));
                                state_stack_.pop_back();
                                break;
                        }
                        break;
                    }
                    case path_state::cmp_gt_or_gte:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token(jsoncons::make_unique<gte_expression>()));
                                push_token(token(expression_begin_arg));
                                state_stack_.pop_back(); 
                                ++p_;
                                ++column_;
                                break;
                            default:
                                push_token(token(jsoncons::make_unique<gt_expression>()));
                                push_token(token(expression_begin_arg));
                                state_stack_.pop_back(); 
                                break;
                        }
                        break;
                    }
                    case path_state::cmp_eq:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token(jsoncons::make_unique<eq_expression>()));
                                push_token(token(expression_begin_arg));
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
                    case path_state::cmp_ne:
                    {
                        switch(*p_)
                        {
                            case '=':
                                push_token(token(jsoncons::make_unique<ne_expression>()));
                                push_token(token(expression_begin_arg));
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
                    case path_state::cmp_lt_or_lte_old:
                    {
                        switch(*p_)
                        {
                            case '=':
                                state_stack_.back() = path_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(path_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                state_stack_.back() = path_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(path_state::expression_item);
                                break;
                        }
                        break;
                    }
                    case path_state::cmp_eq_old:
                    {
                        switch(*p_)
                        {
                            case '=':
                                state_stack_.back() = path_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(path_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_comparator;
                                return Json::null();
                        }
                        break;
                    }
                    case path_state::cmp_gt_or_gte_old:
                    {
                        switch(*p_)
                        {
                            case '=':
                                state_stack_.back() = path_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(path_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                state_stack_.back() = path_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(path_state::expression_item);
                                break;
                        }
                        break;
                    }
                    case path_state::cmp_ne_old:
                    {
                        switch(*p_)
                        {
                            case '=':
                                state_stack_.back() = path_state::expect_filter_right_bracket;
                                state_stack_.emplace_back(path_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_comparator;
                                return Json::null();
                        }
                        break;
                    }
                    case path_state::expect_dot:
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
                    case path_state::expect_or:
                    {
                        switch(*p_)
                        {
                            case '|':
                                push_token(token(jsoncons::make_unique<or_expression>()));
                                push_token(token(expression_begin_arg));
                                state_stack_.pop_back(); // expect_or
                                ++p_;
                                ++column_;
                                break;
                            default:
                                ec = jmespath_errc::expected_or;
                                return Json::null();
                        }
                        break;
                    }
                    case path_state::expect_and:
                    {
                        switch(*p_)
                        {
                            case '&':
                                push_token(token(jsoncons::make_unique<and_expression>()));
                                push_token(token(expression_begin_arg));
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
                    case path_state::expect_filter_right_bracket:
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
                    case path_state::expect_right_bracket4:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ',':
                                state_stack_.back() = path_state::expect_right_bracket4;
                                state_stack_.emplace_back(path_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                                state_stack_.back() = path_state::expect_right_bracket4;
                                state_stack_.emplace_back(path_state::expression_item);
                                break;
                            case '.':
                                state_stack_.back() = path_state::expect_right_bracket4;
                                state_stack_.emplace_back(path_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            case '|':
                            {
                                ++p_;
                                ++column_;
                                state_stack_.back() = path_state::expect_right_bracket4;
                                state_stack_.emplace_back(path_state::expression_item);
                                break;
                            }
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
                    case path_state::expect_right_brace:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ',':
                                state_stack_.back() = path_state::key_val_expr; 
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                            case '{':
                                state_stack_.back() = path_state::expect_right_brace;
                                state_stack_.emplace_back(path_state::expression_item);
                                break;
                            case '.':
                                state_stack_.back() = path_state::expect_right_brace;
                                state_stack_.emplace_back(path_state::expression_item);
                                ++p_;
                                ++column_;
                                break;
                            case '}':
                            {
                                state_stack_.pop_back();

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
                    case path_state::expect_colon:
                    {
                        switch(*p_)
                        {
                            case ' ':case '\t':case '\r':case '\n':
                                advance_past_space_character();
                                break;
                            case ':':
                                state_stack_.back() = path_state::expect_right_brace;
                                state_stack_.emplace_back(path_state::expression_item);
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

            if (state_stack_.size() >= 3 && state_stack_.back() == path_state::unquoted_string)
            {
                push_token(token(jsoncons::make_unique<identifier_expression>(buffer)));
                state_stack_.pop_back(); // unquoted_string
                if (state_stack_.back() == path_state::val_expr || state_stack_.back() == path_state::identifier_or_function_expr)
                {
                    buffer.clear();
                    state_stack_.pop_back(); // val_expr
                }
            }

            JSONCONS_ASSERT(state_stack_.size() == 1);
            JSONCONS_ASSERT(state_stack_.back() == path_state::expression_item ||
                            state_stack_.back() == path_state::sub_or_index_expression);
            state_stack_.pop_back();

            push_token(rparen_arg);

            for (auto& t : output_stack_)
            {
                std::cout << t.to_string() << "\n";
            }

            reference r = evaluate(root, ec);

            return r;
        }

        reference evaluate(reference root, std::error_code& ec)
        {
            std::vector<pointer> stack;
            for (std::size_t i = 0; i < output_stack_.size(); ++i)
            {
                auto& t = output_stack_[i];
                switch (t.type())
                {
                    case token_type::expression_begin:
                        stack.push_back(std::addressof(root));
                        break;
                    case token_type::expression:
                    {
                        JSONCONS_ASSERT(!stack.empty());
                        auto ptr = stack.back();
                        stack.pop_back();
                        auto& ref = t.expression_->evaluate(*ptr, storage_, ec);
                        stack.push_back(std::addressof(ref));
                        break;
                    }
                    case token_type::projection:
                    {
                        JSONCONS_ASSERT(!stack.empty());
                        auto ptr = stack.back();
                        stack.pop_back();
                        auto p = std::move(t.projection_);
                        std::vector<std::unique_ptr<expression_base>> selectors;

                        while (i+1 < output_stack_.size() && output_stack_[i+1].is_expression())
                        {
                            ++i;
                            selectors.push_back(std::move(output_stack_[i].expression_));
                        }
                        auto& ref = p->evaluate(*ptr, selectors, storage_, ec);
                        stack.push_back(std::addressof(ref));
                        break;
                    }
                    case token_type::unary_expression:
                    {
                        JSONCONS_ASSERT(stack.size() >= 1);
                        auto ptr = stack.back();
                        stack.pop_back();
                        reference r = t.unary_expression_->evaluate(*ptr, storage_, ec);
                        stack.push_back(std::addressof(r));
                        break;
                    }
                    case token_type::binary_expression:
                    {
                        JSONCONS_ASSERT(stack.size() >= 2);
                        auto rhs = stack.back();
                        stack.pop_back();
                        auto lhs = stack.back();
                        stack.pop_back();
                        reference r = t.binary_expression_->evaluate(*lhs,*rhs, storage_, ec);
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

        void push_token(token&& token)
        {
            switch (token.type())
            {
                case token_type::value:
                case token_type::expression:
                case token_type::projection:
                case token_type::expression_begin:
                    output_stack_.emplace_back(std::move(token));
                    break;
                case token_type::lparen:
                    operator_stack_.emplace_back(std::move(token));
                    break;
                case token_type::rparen:
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
                        operator_stack_.erase(it.base(),operator_stack_.end());
                        operator_stack_.pop_back();
                        break;
                    }
                case token_type::unary_expression:
                case token_type::binary_expression:
                {
                    if (operator_stack_.empty() || operator_stack_.back().is_lparen())
                    {
                        operator_stack_.emplace_back(std::move(token));
                    }
                    else if (token.precedence_level() < operator_stack_.back().precedence_level()
                             || (token.precedence_level() == operator_stack_.back().precedence_level() && token.is_right_associative()))
                    {
                        operator_stack_.emplace_back(std::move(token));
                    }
                    else
                    {
                        auto it = operator_stack_.rbegin();
                        while (it != operator_stack_.rend() && it->is_operator()
                               && (token.precedence_level() > it->precedence_level()
                             || (token.precedence_level() == it->precedence_level() && token.is_right_associative())))
                        {
                            output_stack_.emplace_back(std::move(*it));
                            ++it;
                        }

                        operator_stack_.erase(it.base(),operator_stack_.end());
                        operator_stack_.emplace_back(std::move(token));
                    }
                    break;
                }
                default:
                    break;
            }
        }
    };

    }

} // namespace jmespath
} // namespace jsoncons

#endif

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
        key,
        literal,
        expression,
        binary_operator,
        unary_operator
    };

    struct literal_arg_t
    {
        explicit literal_arg_t() = default;
    };
    constexpr literal_arg_t literal_arg{};

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
     
    enum class path_state 
    {
        start,
        sub_or_index_expression,
        quoted_string,
        raw_string,
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
        cmp_lt_or_lte,
        cmp_eq,
        cmp_gt_or_gte,
        cmp_ne,
        comparator,
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
    public:
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

            virtual string_type to_string() const
            {
                string_type s("to_string not implemented");
                return s;
            }
        };

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

            string_type to_string() const override
            {
                return string_type("or_operator");
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

            string_type to_string() const override
            {
                return string_type("and_operator");
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

            string_type to_string() const override
            {
                return string_type("eq_operator");
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

            string_type to_string() const override
            {
                return string_type("ne_operator");
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

            string_type to_string() const override
            {
                return string_type("lt_operator");
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

            string_type to_string() const override
            {
                return string_type("lte_operator");
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

            string_type to_string() const override
            {
                return string_type("gt_operator");
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

            string_type to_string() const override
            {
                return string_type("gte_operator");
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

            virtual string_type to_string(std::size_t = 0) const
            {
                return string_type("to_string not implemented");
            }

            virtual bool is_projection() const = 0;
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
        };

        class compound_expression final : public selector_base
        {
        public:
            std::vector<std::unique_ptr<expression_base>> expressions_;

            compound_expression(std::vector<std::unique_ptr<expression_base>>&& expressions)
                : expressions_(std::move(expressions))
            {
            }

            reference evaluate(reference val, jmespath_storage& storage, std::error_code& ec) override
            {
                pointer ptr = std::addressof(val);
                for (auto& expression : expressions_)
                {
                    ptr = std::addressof(expression->evaluate(*ptr, storage, ec));
                }
                return *ptr;
            }

            string_type to_string(std::size_t = 0) const override
            {
                string_type s("compound_expression\n");
                for (auto& item : expressions_)
                {
                    s.push_back(' ');
                    s.push_back(' ');
                    string_type ss = item->to_string();
                    s.insert(s.end(), ss.begin(), ss.end());
                }
                return s;
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

            string_type to_string(std::size_t indent = 0) const override
            {
                string_type s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("identifier_selector ");
                s.append(identifier_);
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
                else if (index_ < 0 && (slen+index_) < slen)
                {
                    std::size_t index = static_cast<std::size_t>(slen + index_);
                    return val.at(index);
                }
                else
                {
                    return Json::null();
                }
            }

            string_type to_string(std::size_t indent = 0) const override
            {
                string_type s;
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

            void add_expression(std::unique_ptr<expression_base>&& expression) override
            {
                if (!expressions_.empty() && expressions_.back()->is_projection())
                {
                    expressions_.back()->add_expression(std::move(expression));
                }
                else
                {
                    expressions_.emplace_back(std::move(expression));
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
        };

        class object_projection final : public projection_base
        {
        public:
            object_projection()
            {
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

            string_type to_string(std::size_t indent = 0) const override
            {
                string_type s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("object_projection\n");
                for (auto& expr : this->expressions_)
                {
                    string_type sss = expr->to_string(indent+2);
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

            string_type to_string(std::size_t indent = 0) const override
            {
                string_type s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("list_projection\n");
                for (auto& expr : this->expressions_)
                {
                    string_type sss = expr->to_string(indent+2);
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

            string_type to_string(std::size_t indent = 0) const override
            {
                string_type s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("slice_projection\n");
                for (auto& expr : this->expressions_)
                {
                    string_type sss = expr->to_string(indent+2);
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
                auto currentp = storage.new_instance(json_array_arg);
                for (reference item : result->array_range())
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
                return *currentp;
            }

            string_type to_string(std::size_t indent = 0) const override
            {
                string_type s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("slice_projection\n");
                for (auto& expr : this->expressions_)
                {
                    string_type sss = expr->to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        class multi_select_list final : public selector_base
        {
            std::vector<std::unique_ptr<expression_base>> expressions_;
        public:
            multi_select_list(std::vector<std::unique_ptr<expression_base>>&& expressions)
                : expressions_(std::move(expressions))
            {
            }

            virtual std::size_t precedence_level() const
            {
                return 1;
            }

            reference evaluate(reference val, jmespath_storage& storage, std::error_code& ec) override
            {
                if (!val.is_object())
                {
                    return Json::null();
                }

                auto result = storage.new_instance(json_array_arg);
                result->reserve(expressions_.size());
                for (auto& expr : expressions_)
                {
                    result->push_back(expr->evaluate(val, storage, ec));
                }
                return *result;
            }

            string_type to_string(std::size_t indent = 0) const override
            {
                string_type s;
                for (std::size_t i = 0; i <= indent; ++i)
                {
                    s.push_back(' ');
                }
                s.append("multi_select_list\n");
                for (auto& expr : expressions_)
                {
                    string_type sss = expr->to_string(indent+2);
                    s.insert(s.end(), sss.begin(), sss.end());
                    s.push_back('\n');
                }
                return s;
            }
        };

        struct key_expression
        {
            string_type key;
            std::unique_ptr<expression_base> expression;

            key_expression(string_type&& key, std::unique_ptr<expression_base>&& expression)
                : key(std::move(key)), expression(std::move(expression))
            {
            }
        };

        class multi_select_hash final : public selector_base
        {
        public:
            std::vector<key_expression> keyvals_;

            multi_select_hash(std::vector<key_expression>&& keyvals)
                : keyvals_(std::move(keyvals))
            {
            }

            reference evaluate(reference val, jmespath_storage& storage, std::error_code& ec) override
            {
                if (!val.is_object())
                {
                    return Json::null();
                }

                auto resultp = storage.new_instance(json_object_arg);
                resultp->reserve(keyvals_.size());
                for (auto& key_expression : keyvals_)
                {
                    resultp->try_emplace(key_expression.key,key_expression.expression->evaluate(val, storage, ec));
                }

                return *resultp;
            }

            string_type to_string(std::size_t = 0) const override
            {
                return string_type("multi_select_hash\n");
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
                std::unique_ptr<unary_operator> unary_operator_;
                std::unique_ptr<binary_operator> binary_operator_;
                Json value_;
                string_type key_;
            };
        public:

            token(source_placeholder_arg_t)
                : type_(token_type::source_placeholder)
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

            token(std::unique_ptr<unary_operator>&& expression)
                : type_(token_type::unary_operator)
            {
                new (&unary_operator_) std::unique_ptr<unary_operator>(std::move(expression));
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
                            case token_type::lparen:
                            case token_type::rparen:
                            case token_type::begin_multi_select_hash:
                            case token_type::end_multi_select_hash:
                                break;
                            case token_type::expression:
                                expression_ = std::move(other.expression_);
                                break;
                            case token_type::key:
                                key_ = std::move(other.key_);
                                break;
                            case token_type::unary_operator:
                                unary_operator_ = std::move(other.unary_operator_);
                                break;
                            case token_type::binary_operator:
                                binary_operator_ = std::move(binary_operator_);
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
                    case token_type::lparen:
                    case token_type::rparen:
                    case token_type::begin_multi_select_hash:
                    case token_type::end_multi_select_hash:
                        break;
                    case token_type::expression:
                        new (&expression_) std::unique_ptr<expression_base>(std::move(other.expression_));
                        break;
                    case token_type::key:
                        new (&key_) string_type(std::move(other.key_));
                        break;
                    case token_type::unary_operator:
                        new (&unary_operator_) std::unique_ptr<unary_operator>(std::move(other.unary_operator_));
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
                    case token_type::unary_operator:
                        unary_operator_.~unique_ptr();
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

            string_type to_string() const
            {
                switch(type_)
                {
                    case token_type::expression:
                        return expression_->to_string();
                        break;
                    case token_type::unary_operator:
                        return string_type("unary_operator");
                        break;
                    case token_type::binary_operator:
                        return binary_operator_->to_string();
                        break;
                    case token_type::source_placeholder:
                        return string_type("source_placeholder");
                        break;
                    case token_type::literal:
                        return string_type("literal");
                        break;
                    case token_type::key:
                        return string_type("key") + key_;
                        break;
                    case token_type::begin_multi_select_hash:
                        return string_type("lbrace");
                        break;
                    default:
                        return string_type("default");
                        break;
                }
            }
        };
    private:
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
            push_token(source_placeholder_arg);
            state_stack_.emplace_back(path_state::start);

            string_type buffer;
     
            begin_input_ = path;
            end_input_ = path + length;
            p_ = begin_input_;

            slice slic{};

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
                                state_stack_.back() = path_state::literal;
                                ++p_;
                                ++column_;
                                break;
                            case '[':
                                state_stack_.back() = path_state::bracket_specifier;
                                ++p_;
                                ++column_;
                                break;
                            case '{':
                                push_token(begin_multi_select_hash_arg);
                                state_stack_.back() = path_state::multi_select_hash;
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
                        push_token(token(key_arg, buffer));
                        buffer.clear(); 
                        state_stack_.pop_back(); 
                        break;
                    case path_state::val_expr:
                        push_token(token(jsoncons::make_unique<identifier_selector>(buffer)));
                        buffer.clear();
                        state_stack_.pop_back(); 
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
                                push_token(token(jsoncons::make_unique<identifier_selector>(buffer)));
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

                    case path_state::quoted_string: 
                        switch (*p_)
                        {
                            case '\"':
                                ++p_;
                                ++column_;
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
                                push_token(token(literal_arg, Json(buffer)));
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
                    case path_state::literal: 
                        switch (*p_)
                        {
                            case '`':
                            {
                                std::cout << "json_value: " << buffer << "\n";
                                auto j = Json::parse(buffer);
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
                                push_token(token(jsoncons::make_unique<list_projection>()));
                                state_stack_.back() = path_state::bracket_specifier4;
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
                                state_stack_.back() = path_state::comparator;
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
                                push_token(token(begin_multi_select_list_arg));
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
                    case path_state::comparator:
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
                    case path_state::cmp_gt_or_gte:
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
                    case path_state::cmp_eq:
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
                    case path_state::cmp_ne:
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
                                push_token(token(jsoncons::make_unique<or_operator>()));
                                push_token(token(source_placeholder_arg));
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
                push_token(token(jsoncons::make_unique<identifier_selector>(buffer)));
                state_stack_.pop_back(); // unquoted_string
                if (state_stack_.back() == path_state::val_expr || state_stack_.back() == path_state::identifier_or_function_expr)
                {
                    buffer.clear();
                    state_stack_.pop_back(); // val_expr
                }
            }
            if (state_stack_.size() >= 3 && state_stack_.back() == path_state::expect_dot)
            {
                state_stack_.pop_back(); // path_state::expect_dot
                if (state_stack_.back() == path_state::expression_item)
                {
                    state_stack_.pop_back(); // expression_item
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
                    case token_type::literal:
                    {
                        stack.push_back(&t.value_);
                        break;
                    }
                    case token_type::source_placeholder:
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
                    case token_type::unary_operator:
                    {
                        JSONCONS_ASSERT(stack.size() >= 1);
                        auto ptr = stack.back();
                        stack.pop_back();
                        reference r = t.unary_operator_->evaluate(*ptr, storage_, ec);
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
                        reference r = t.binary_operator_->evaluate(*lhs,*rhs, storage_, ec);
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

        void push_token(token&& tok)
        {
            switch (tok.type())
            {
                case token_type::end_multi_select_list:
                {
                    std::vector<std::unique_ptr<expression_base>> expressions;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_type::begin_multi_select_list)
                    {
                        std::cout << "type: " << (int)it->type() << std::endl;
                        JSONCONS_ASSERT(it->is_expression());
                        do
                        {
                            expressions.insert(expressions.begin(), std::move(it->expression_));
                            ++it;
                        } while (it->is_expression());
                    }
                    if (it == output_stack_.rend())
                    {
                        JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced braces"));
                    }
                    output_stack_.erase(it.base(),output_stack_.end());
                    output_stack_.pop_back();

                    if (!output_stack_.empty() && output_stack_.back().is_projection() && output_stack_.back().precedence_level() >= tok.precedence_level())
                    {
                        output_stack_.back().expression_->add_expression(jsoncons::make_unique<multi_select_list>(std::move(expressions)));
                    }
                    else
                    {
                        output_stack_.emplace_back(token(jsoncons::make_unique<multi_select_list>(std::move(expressions))));
                    }
                    break;
                }
                case token_type::end_multi_select_hash:
                {
                    std::vector<key_expression> keyvals;
                    auto it = output_stack_.rbegin();
                    while (it != output_stack_.rend() && it->type() != token_type::begin_multi_select_hash)
                    {
                        std::vector<std::unique_ptr<expression_base>> expressions;
                        JSONCONS_ASSERT(it->is_expression());
                        do
                        {
                            expressions.insert(expressions.begin(), std::move(it->expression_));
                            ++it;
                        } while (it->is_expression());
                        JSONCONS_ASSERT(it->is_key());
                        auto key = std::move(it->key_);
                        ++it;
                        if (expressions.size() == 1)
                        {
                            keyvals.emplace_back(std::move(key),std::move(expressions.back()));
                        }
                        else
                        {
                            keyvals.emplace_back(std::move(key),make_unique<compound_expression>(std::move(expressions)));
                        }
                    }
                    if (it == output_stack_.rend())
                    {
                        JSONCONS_THROW(json_runtime_error<std::runtime_error>("Unbalanced braces"));
                    }
                    output_stack_.erase(it.base(),output_stack_.end());
                    output_stack_.pop_back();

                    if (!output_stack_.empty() && output_stack_.back().is_projection() && output_stack_.back().precedence_level() >= tok.precedence_level())
                    {
                        output_stack_.back().expression_->add_expression(jsoncons::make_unique<multi_select_hash>(std::move(keyvals)));
                    }
                    else
                    {
                        output_stack_.emplace_back(token(jsoncons::make_unique<multi_select_hash>(std::move(keyvals))));
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
                    if (!output_stack_.empty() && output_stack_.back().is_projection() && output_stack_.back().precedence_level() >= tok.precedence_level())
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
                case token_type::source_placeholder:
                case token_type::begin_multi_select_hash:
                case token_type::begin_multi_select_list:
                case token_type::key:
                    output_stack_.emplace_back(std::move(tok));
                    break;
                case token_type::lparen:
                    operator_stack_.emplace_back(std::move(tok));
                    break;
                default:
                    break;
            }
        }
    };

    }

} // namespace jmespath
} // namespace jsoncons

#endif

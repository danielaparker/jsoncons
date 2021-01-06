// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSONPATH_EXPRESSION_HPP
#define JSONCONS_JSONPATH_JSONPATH_EXPRESSION_HPP

#include <jsoncons_ext/jsonpath_new/jsonpath_error.hpp>
#include <jsoncons_ext/jsonpath_new/jsonpath_function.hpp>

namespace jsoncons { 
namespace jsonpath_new {
namespace detail {

    template <class Json>
    class term;
    template <class Json>
    class value_term;
    template <class Json>
    class evaluated_path_term;
    template <class Json>
    class regex_term;

    enum class term_type {value,regex,path};

    template <class Json>
    struct cmp_plus
    {
        Json plus(const Json& lhs, const Json& rhs) const
        {
            Json result = Json(jsoncons::null_type());
            if (lhs.is_int64() && rhs.is_int64())
            {
                result = Json(((lhs.template as<int64_t>() + rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                result = Json((lhs.template as<uint64_t>() + rhs.template as<uint64_t>()));
            }
            else if ((lhs.is_number() && rhs.is_number()))
            {
                result = Json((lhs.as_double() + rhs.as_double()));
            }
            return result;
        }

        Json operator()(const value_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            return plus(lhs.value(), rhs.value());
        }

        Json operator()(const value_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(lhs.value(),rhs.result()[0]);
        }

        Json operator()(const evaluated_path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(rhs, lhs);
        }

        Json operator()(const evaluated_path_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (lhs.result().empty() || rhs.result().empty())
            {
                return Json::null();
            }
            return (*this)(lhs.result()[0],rhs.result()[0]);
        }

        bool operator()(const value_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        bool operator()(const evaluated_path_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
    };

    template <class Json>
    struct cmp_mult
    {
        Json mult(const Json& lhs, const Json& rhs) const
        {
            Json result = Json(jsoncons::null_type());
            if (lhs.is_int64() && rhs.is_int64())
            {
                result = Json(((lhs.template as<int64_t>() * rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                result = Json((lhs.template as<uint64_t>() * rhs.template as<uint64_t>()));
            }
            else if ((lhs.is_number() && rhs.is_number()))
            {
                result = Json((lhs.as_double() * rhs.as_double()));
            }
            return result;
        }

        Json operator()(const value_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            return mult(lhs.value(), rhs.value());
        }

        Json operator()(const value_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return mult(lhs.value(),rhs.result()[0]);
        }

        Json operator()(const evaluated_path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(rhs, lhs);
        }

        Json operator()(const evaluated_path_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (lhs.result().empty() || rhs.result().empty())
            {
                return Json::null();
            }
            return mult(lhs.result()[0],rhs.result()[0]);
        }

        bool operator()(const value_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        bool operator()(const evaluated_path_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
    };

    template <class Json>
    struct cmp_div
    {
        Json div(const Json& lhs, const Json& rhs) const
        {
            Json result = Json(jsoncons::null_type());
            if (lhs.is_int64() && rhs.is_int64())
            {
                result = Json((double)(lhs.template as<int64_t>() / (double)rhs.template as<int64_t>()));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                result = Json((double)(lhs.template as<uint64_t>() / (double)rhs.template as<uint64_t>()));
            }
            else if ((lhs.is_number() && rhs.is_number()))
            {
                result = Json((lhs.as_double() / rhs.as_double()));
            }
            return result;
        }

        Json operator()(const value_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            return div(lhs.value(), rhs.value());
        }

        Json operator()(const value_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return div(lhs.value(),rhs.result()[0]);
        }

        Json operator()(const evaluated_path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return div(lhs.result()[0],rhs.value());
        }

        Json operator()(const evaluated_path_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (lhs.result().empty() || rhs.result().empty())
            {
                return Json::null();
            }
            return div(lhs.result()[0],rhs.result()[0]);
        }

        bool operator()(const value_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        bool operator()(const evaluated_path_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
    };

    template <class Json>
    struct cmp_eq
    {
        bool operator()(const value_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            return lhs.value() == rhs.value();
        }

        bool operator()(const value_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return lhs.value() == rhs.result()[0];
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(rhs, lhs);
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (lhs.result().empty())
            {
                return !rhs.result().empty();
            }
            if (rhs.result().empty())
            {
                return false;
            }

            if (lhs.result().size() != rhs.result().size())
            {
                return false;
            }
            for (std::size_t i = 0; i < lhs.result().size(); ++i)
            {
                if (lhs.result()[i] != rhs.result()[i])
                {
                    return false;
                }
            }
            return true;
        }

        bool operator()(const value_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        bool operator()(const evaluated_path_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
    };

    template <class Json>
    struct cmp_ne
    {
        cmp_eq<Json> eq;
        constexpr cmp_ne()
            : eq{}
        {
        }

        bool operator()(const value_term<Json>& lhs, const value_term<Json>& rhs) const 
        {
            return !eq(lhs, rhs);
        }

        bool operator()(const value_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const 
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return !eq(lhs, rhs);
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return !eq(lhs, rhs);
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const 
        {
            if (lhs.result().empty())
            {
                return !rhs.result().empty();
            }
            if (rhs.result().empty())
            {
                return false;
            }
            return !eq(lhs, rhs);
        }

        bool operator()(const value_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        bool operator()(const evaluated_path_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
    };

    template <class Json>
    struct cmp_pipepipe
    {
        bool operator()(const value_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            return lhs.value().as_bool() || rhs.value().as_bool();
        }

        bool operator()(const value_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(lhs.value(),rhs.result()[0]);
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(rhs, lhs);
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (lhs.result().empty())
            {
                return !rhs.result().empty();
            }
            if (rhs.result().empty())
            {
                return false;
            }
            if (lhs.result().size() != rhs.result().size())
            {
                return false;
            }
            for (std::size_t i = 0; i < lhs.result().size(); ++i)
            {
                if (!(*this)(lhs.result()[i], rhs.result()[i]))
                {
                    return false;
                }
            }
            return true;
        }

        bool operator()(const value_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        bool operator()(const evaluated_path_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
    };

    template <class Json>
    struct cmp_ampamp
    {
        bool operator()(const value_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            return lhs.value().as_bool() && rhs.value().as_bool();
        }

        bool operator()(const value_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(lhs.value(),rhs.result()[0]);
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(rhs, lhs);
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (lhs.result().empty())
            {
                return !rhs.result().empty();
            }
            if (rhs.result().empty())
            {
                return false;
            }
            if (lhs.result().size() != rhs.result().size())
            {
                return false;
            }
            for (std::size_t i = 0; i < lhs.result().size(); ++i)
            {
                if (!(*this)(lhs.result()[i], rhs.result()[i]))
                {
                    return false;
                }
            }
            return true;
        }

        bool operator()(const value_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        bool operator()(const evaluated_path_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
    };

    template <class Json>
    struct cmp_lt
    {
        cmp_eq<Json> eq;

        constexpr cmp_lt()
            : eq{}
        {
        }

        bool lt(const Json& lhs, const Json& rhs) const
        {
            bool result = false;
            if (lhs.template is<int64_t>() && rhs.template is<int64_t>())
            {
                result = lhs.template as<int64_t>() < rhs.template as<int64_t>();
            }
            else if (lhs.template is<uint64_t>() && rhs.template is<uint64_t>())
            {
                result = lhs.template as<uint64_t>() < rhs.template as<uint64_t>();
            }
            else if (lhs.is_number() && rhs.is_number())
            {
                result = lhs.as_double() < rhs.as_double();
            }
            else if (lhs.is_string() && rhs.is_string())
            {
                result = lhs.as_string_view() < rhs.as_string_view();
            }
            return result;
        }

        bool operator()(const value_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            return lhs.value() < rhs.value();
        }

        bool operator()(const value_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return !((*this)(rhs, lhs) || eq(rhs,lhs));
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
             {
                return false;
            }
            bool result = lt(lhs.result()[0], rhs.value());
            return result;
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (lhs.result().empty())
            {
                return !rhs.result().empty();
            }
            if (rhs.result().empty())
            {
                return false;
            }
            bool result = true;
            std::size_t min_len = lhs.result().size() < rhs.result().size() ? lhs.result().size() : rhs.result().size();
            for (std::size_t i = 0; result && i < min_len; ++i)
            {
                result = (*this)(lhs.result()[i],rhs.result()[i]);
            }
            return result ? min_len == lhs.result().size() : false;
        }

        bool operator()(const value_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        bool operator()(const evaluated_path_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
    };

    template <class Json>
    struct cmp_lte
    {
        cmp_lt<Json> lt;

        constexpr cmp_lte()
            : lt{}
        {
        }

        bool operator()(const value_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            return lhs.value() <= rhs.value();
        }

        bool operator()(const value_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return !lt(rhs, lhs);
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
             {
                return false;
            }
            bool result = (*this)(lhs.result()[0], rhs.value());
            return result;
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (lhs.result().empty())
            {
                return true;
            }
            if (rhs.result().empty())
            {
                return false;
            }
            bool result = true;
            std::size_t min_len = lhs.result().size() < rhs.result().size() ? lhs.result().size() : rhs.result().size();
            for (std::size_t i = 0; result && i < min_len; ++i)
            {
                result = (*this)(lhs.result()[i],rhs.result()[i]);
            }
            return result ? min_len == lhs.result().size() : false;
        }

        bool operator()(const value_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        bool operator()(const evaluated_path_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
    };

    template <class Json>
    struct cmp_minus
    {
        cmp_lt<Json> lt;

        constexpr cmp_minus()
            : lt{}
        {
        }

        Json minus(const Json& lhs, const Json& rhs) const
        {
            Json result = Json::null();
            if (lhs.is_int64() && rhs.is_int64())
            {
                result = ((lhs.template as<int64_t>() - rhs.template as<int64_t>()));
            }
            else if (lhs.is_uint64() && rhs.is_uint64() && lt(rhs,lhs))
            {
                result = (lhs.template as<uint64_t>() - rhs.template as<uint64_t>());
            }
            else if ((lhs.is_number() && rhs.is_number()))
            {
                result = (lhs.as_double() - rhs.as_double());
            }
            return result;
        }

        Json operator()(const value_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            return minus(lhs.value(), rhs.value());
        }

        Json operator()(const value_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return minus(lhs.value(),rhs.result()[0]);
        }

        Json operator()(const evaluated_path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return minus(lhs.result()[0],rhs.value());
        }

        Json operator()(const evaluated_path_term<Json>& lhs, const evaluated_path_term<Json>& rhs) const
        {
            if (lhs.result().empty() || rhs.result().empty())
            {
                return Json::null();
            }
            return minus(lhs.result()[0],rhs.result()[0]);
        }

        bool operator()(const value_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        bool operator()(const evaluated_path_term<Json>&, const regex_term<Json>&) const
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
    };

    template <class Json>
    struct cmp_regex
    {
        bool operator()(const value_term<Json>&, const value_term<Json>&) const 
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
        bool operator()(const evaluated_path_term<Json>&, const value_term<Json>&) const 
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
        bool operator()(const evaluated_path_term<Json>&, const evaluated_path_term<Json>&) const 
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
        bool operator()(const value_term<Json>& lhs, const regex_term<Json>& rhs) const
        {
            return rhs.evaluate(lhs.value().as_string()); 
        }
        bool operator()(const value_term<Json>&, const evaluated_path_term<Json>&) const 
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        bool operator()(const evaluated_path_term<Json>& lhs, const regex_term<Json>& rhs) const
        {
            if (lhs.result().empty())
                return false;
            for (const auto& val : lhs.result().array_range())
            {
                if (!rhs.evaluate(val.as_string()))
                {
                    return false;
                }
            }
            return true;
        }
    };

    template <class Json>
    struct dynamic_resources
    {
        std::vector<std::unique_ptr<Json>> temp_json_values_;

        Json& true_value() const
        {
            static Json value(true, semantic_tag::none);
            return value;
        }

        Json& false_value() const
        {
            static Json value(false, semantic_tag::none);
            return value;
        }

        Json& null_value() const
        {
            static Json value(null_type(), semantic_tag::none);
            return value;
        }

        template <typename... Args>
        Json* create_json(Args&& ... args)
        {
            auto temp = jsoncons::make_unique<Json>(std::forward<Args>(args)...);
            Json* ptr = temp.get();
            temp_json_values_.emplace_back(std::move(temp));
            return ptr;
        }
    };

    template <class Json>
    struct unary_operator
    {
        typedef std::function<Json(const term<Json>&)> operator_type;

        std::size_t precedence_level_;
        bool is_right_associative_;
        operator_type op;

        unary_operator(std::size_t precedence_level,
                       bool is_right_associative)
            : precedence_level_(precedence_level),
              is_right_associative_(is_right_associative)
        {
        }

        unary_operator(std::size_t precedence_level,
                       bool is_right_associative,
                       operator_type op)
            : precedence_level_(precedence_level),
              is_right_associative_(is_right_associative),
              op(std::move(op))
        {
        }

        std::size_t precedence_level() const 
        {
            return precedence_level_;
        }
        bool is_right_associative() const
        {
            return is_right_associative_;
        }

        virtual const Json& evaluate(dynamic_resources<Json>&,
                                     const Json&, 
                                     std::error_code&) const 
        {
            return Json::null();
        }
    };

    template <class Json>
    bool is_false(const Json& val)
    {
        return ((val.is_array() && val.empty()) ||
                 (val.is_object() && val.empty()) ||
                 (val.is_string() && val.as_string_view().empty()) ||
                 (val.is_bool() && !val.as_bool()) ||
                 val.is_null());
    }

    template <class Json>
    bool is_true(const Json& val)
    {
        return !is_false(val);
    }

    template <class Json>
    class not_expression final : public unary_operator<Json>
    {
    public:
        not_expression()
            : unary_operator<Json>(1, true)
        {}

        const Json& evaluate(dynamic_resources<Json>& resources,
                             const Json& val, 
                             std::error_code&) const override
        {
            return is_false(val) ? resources.true_value() : resources.false_value();
        }
    };

    template <class Json>
    struct binary_operator
    {
        typedef std::function<Json(const term<Json>&, const term<Json>&)> operator_type;

        std::size_t precedence_level_;
        bool is_right_associative_;
        operator_type op;

        binary_operator(std::size_t precedence_level,
                        bool is_right_associative = false)
            : precedence_level_(precedence_level),
              is_right_associative_(is_right_associative)
        {
        }

        binary_operator(std::size_t precedence_level,
                        bool is_right_associative,
                        operator_type op)
            : precedence_level_(precedence_level),
              is_right_associative_(is_right_associative),
              op(std::move(op))
        {
        }

        std::size_t precedence_level() const 
        {
            return precedence_level_;
        }
        bool is_right_associative() const
        {
            return is_right_associative_;
        }

        virtual const Json& evaluate(dynamic_resources<Json>&,
                             const Json&, 
                             const Json&, 
                             std::error_code&) const
        {
            return Json::null();
        }
    };

    // Implementations

    template <class Json>
    class or_operator final : public binary_operator<Json>
    {
    public:
        or_operator()
            : binary_operator<Json>(9)
        {
        }

        const Json& evaluate(dynamic_resources<Json>& resources, const Json& lhs, const Json& rhs, 
                             std::error_code&) const override
        {
            if (lhs.is_null() && rhs.is_null())
            {
                return resources.null_value();
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
    };

    template <class Json>
    class and_operator final : public binary_operator<Json>
    {
    public:
        and_operator()
            : binary_operator<Json>(8)
        {
        }

        const Json& evaluate(dynamic_resources<Json>&, const Json& lhs, const Json& rhs, std::error_code&) const override
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
    };

    template <class Json>
    class eq_operator final : public binary_operator<Json>
    {
    public:
        eq_operator()
            : binary_operator<Json>(6)
        {
        }

        const Json& evaluate(dynamic_resources<Json>& resources, const Json& lhs, const Json& rhs, std::error_code&) const override 
        {
            return lhs == rhs ? resources.true_value() : resources.false_value();
        }
    };

    template <class Json>
    class ne_operator final : public binary_operator<Json>
    {
    public:
        ne_operator()
            : binary_operator<Json>(6)
        {
        }

        const Json& evaluate(dynamic_resources<Json>& resources, const Json& lhs, const Json& rhs, std::error_code&) const override 
        {
            return lhs != rhs ? resources.true_value() : resources.false_value();
        }
    };

    template <class Json>
    class lt_operator final : public binary_operator<Json>
    {
    public:
        lt_operator()
            : binary_operator<Json>(5)
        {
        }

        const Json& evaluate(dynamic_resources<Json>& resources, const Json& lhs, const Json& rhs, std::error_code&) const override 
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return resources.null_value();
            }
            return lhs < rhs ? resources.true_value() : resources.false_value();
        }
    };

    template <class Json>
    class lte_operator final : public binary_operator<Json>
    {
    public:
        lte_operator()
            : binary_operator<Json>(5)
        {
        }

        const Json& evaluate(dynamic_resources<Json>& resources, const Json& lhs, const Json& rhs, std::error_code&) const override 
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return resources.null_value();
            }
            return lhs <= rhs ? resources.true_value() : resources.false_value();
        }
    };

    template <class Json>
    class gt_operator final : public binary_operator<Json>
    {
    public:
        gt_operator()
            : binary_operator<Json>(5)
        {
        }

        const Json& evaluate(dynamic_resources<Json>& resources, const Json& lhs, const Json& rhs, std::error_code&) const override
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return resources.null_value();
            }
            return lhs > rhs ? resources.true_value() : resources.false_value();
        }
    };

    template <class Json>
    class gte_operator final : public binary_operator<Json>
    {
    public:
        gte_operator()
            : binary_operator<Json>(5)
        {
        }

        const Json& evaluate(dynamic_resources<Json>& resources, const Json& lhs, const Json& rhs, std::error_code&) const override
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return resources.null_value();
            }
            return lhs >= rhs ? resources.true_value() : resources.false_value();
        }
    };

    template <class Json>
    class plus_operator final : public binary_operator<Json>
    {
    public:
        plus_operator()
            : binary_operator<Json>(4)
        {
        }

        const Json& evaluate(dynamic_resources<Json>& resources, const Json& lhs, const Json& rhs, std::error_code&) const override
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return resources.null_value();
            }
            else if (lhs.is_int64() && rhs.is_int64())
            {
                return *resources.create_json(((lhs.template as<int64_t>() + rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                return *resources.create_json((lhs.template as<uint64_t>() + rhs.template as<uint64_t>()));
            }
            else
            {
                return *resources.create_json((lhs.as_double() + rhs.as_double()));
            }
        }
    };

    template <class Json>
    class minus_operator final : public binary_operator<Json>
    {
    public:
        minus_operator()
            : binary_operator<Json>(4)
        {
        }

        const Json& evaluate(dynamic_resources<Json>& resources, const Json& lhs, const Json& rhs, std::error_code&) const override
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return resources.null_value();
            }
            else if (lhs.is_int64() && rhs.is_int64())
            {
                return *resources.create_json(((lhs.template as<int64_t>() - rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                return *resources.create_json((lhs.template as<uint64_t>() - rhs.template as<uint64_t>()));
            }
            else
            {
                return *resources.create_json((lhs.as_double() - rhs.as_double()));
            }
        }
    };

    template <class Json>
    class mult_operator final : public binary_operator<Json>
    {
    public:
        mult_operator()
            : binary_operator<Json>(3)
        {
        }

        const Json& evaluate(dynamic_resources<Json>& resources, const Json& lhs, const Json& rhs, std::error_code&) const override
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return resources.null_value();
            }
            else if (lhs.is_int64() && rhs.is_int64())
            {
                return *resources.create_json(((lhs.template as<int64_t>() * rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                return *resources.create_json((lhs.template as<uint64_t>() * rhs.template as<uint64_t>()));
            }
            else
            {
                return *resources.create_json((lhs.as_double() * rhs.as_double()));
            }
        }
    };

    template <class Json>
    class div_operator final : public binary_operator<Json>
    {
    public:
        div_operator()
            : binary_operator<Json>(3)
        {
        }

        const Json& evaluate(dynamic_resources<Json>& resources, const Json& lhs, const Json& rhs, std::error_code&) const override
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return resources.null_value();
            }
            else if (lhs.is_int64() && rhs.is_int64())
            {
                return *resources.create_json(((lhs.template as<int64_t>() / rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                return *resources.create_json((lhs.template as<uint64_t>() / rhs.template as<uint64_t>()));
            }
            else
            {
                return *resources.create_json((lhs.as_double() / rhs.as_double()));
            }
        }
    };

    template <typename Visitor,typename Json>
    Json visit(Visitor vis, const term<Json>& v, const term<Json>& w)
    {
        switch(v.type())
        {
            case term_type::value:
            {
                const auto& t1 = static_cast<const value_term<Json>&>(v);
                switch(w.type())
                {
                    case term_type::value:
                    {
                        const auto& t2 = static_cast<const value_term<Json>&>(w);
                        return vis(t1, t2);
                    }
                    case term_type::path:
                    {
                        const auto& t2 = static_cast<const evaluated_path_term<Json>&>(w);
                        return vis(t1, t2);
                    }
                    case term_type::regex:
                    {
                        const auto& t2 = static_cast<const regex_term<Json>&>(w);
                        return vis(t1, t2);
                    }
                }
                break;
            }
            case term_type::path:
            {
                const auto& t1 = static_cast<const evaluated_path_term<Json>&>(v);
                switch(w.type())
                {
                    case term_type::value:
                    {
                        const auto& t2 = static_cast<const value_term<Json>&>(w);
                        return vis(t1, t2);
                    }
                    case term_type::path:
                    {
                        const auto& t2 = static_cast<const evaluated_path_term<Json>&>(w);
                        return vis(t1, t2);
                    }
                    case term_type::regex:
                    {
                        const auto& t2 = static_cast<const regex_term<Json>&>(w);
                        return vis(t1, t2);
                    }
                }
                break;
            }
            case term_type::regex:
            {
                JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
            }
        }

        return false;
    }

    // function_base
    template <class Json,class JsonReference>
    class function_base
    {
        jsoncons::optional<std::size_t> arg_count_;
    public:
        using reference = JsonReference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;

        function_base(jsoncons::optional<std::size_t> arg_count)
            : arg_count_(arg_count)
        {
        }

        jsoncons::optional<std::size_t> arg_count() const
        {
            return arg_count_;
        }

        virtual ~function_base() = default;

        virtual reference evaluate(dynamic_resources<Json>& resources,
                                   std::vector<pointer>& args, 
                                   std::error_code& ec) const = 0;

    };  

    template <class Json,class JsonReference>
    class sum_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        sum_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json>& resources,
                           std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            JSONCONS_ASSERT(args.size() == *this->arg_count());

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_array())
            {
                //std::cout << "arg: " << *arg0_ptr << "\n";
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }
            double sum = 0;
            for (auto& j : arg0_ptr->array_range())
            {
                if (!j.is_number())
                {
                    ec = jsonpath_errc::invalid_type;
                    return Json::null();
                }
                sum += j.template as<double>();
            }

            return *resources.create_json(sum);
        }
    };

    template <class Json,class JsonReference>
    class avg_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        avg_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json>& resources,
                           std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            JSONCONS_ASSERT(args.size() == *this->arg_count());

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_array())
            {
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }
            if (arg0_ptr->empty())
            {
                return resources.null_value();
            }
            double sum = 0;
            for (auto& j : arg0_ptr->array_range())
            {
                if (!j.is_number())
                {
                    ec = jsonpath_errc::invalid_type;
                    return Json::null();
                }
                sum += j.template as<double>();
            }

            return *resources.create_json(sum / static_cast<double>(arg0_ptr->size()));
        }
    };

    template <class Json,class JsonReference>
    class min_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        min_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json>& resources,
                           std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            JSONCONS_ASSERT(args.size() == *this->arg_count());

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_array())
            {
                //std::cout << "arg: " << *arg0_ptr << "\n";
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }
            if (arg0_ptr->empty())
            {
                return resources.null_value();
            }
            bool is_number = arg0_ptr->at(0).is_number();
            bool is_string = arg0_ptr->at(0).is_string();
            if (!is_number && !is_string)
            {
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }

            std::size_t index = 0;
            for (std::size_t i = 1; i < arg0_ptr->size(); ++i)
            {
                if (!(arg0_ptr->at(i).is_number() == is_number && arg0_ptr->at(i).is_string() == is_string))
                {
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
                }
                if (arg0_ptr->at(i) < arg0_ptr->at(index))
                {
                    index = i;
                }
            }

            return arg0_ptr->at(index);
        }
    };

    template <class Json,class JsonReference>
    class max_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        max_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json>& resources,
                           std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            JSONCONS_ASSERT(args.size() == *this->arg_count());

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_array())
            {
                //std::cout << "arg: " << *arg0_ptr << "\n";
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }
            if (arg0_ptr->empty())
            {
                return resources.null_value();
            }

            bool is_number = arg0_ptr->at(0).is_number();
            bool is_string = arg0_ptr->at(0).is_string();
            if (!is_number && !is_string)
            {
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }

            std::size_t index = 0;
            for (std::size_t i = 1; i < arg0_ptr->size(); ++i)
            {
                if (!(arg0_ptr->at(i).is_number() == is_number && arg0_ptr->at(i).is_string() == is_string))
                {
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
                }
                if (arg0_ptr->at(i) > arg0_ptr->at(index))
                {
                    index = i;
                }
            }

            return arg0_ptr->at(index);
        }
    };

    template <class Json,class JsonReference>
    class length_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;
        using string_view_type = typename Json::string_view_type;

        length_function()
            : function_base<Json,JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json>& resources,
                           std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            JSONCONS_ASSERT(args.size() == *this->arg_count());

            pointer arg0_ptr = args[0];

            switch (arg0_ptr->type())
            {
                case json_type::object_value:
                case json_type::array_value:
                    return *resources.create_json(arg0_ptr->size());
                case json_type::string_value:
                {
                    auto sv0 = arg0_ptr->template as<string_view_type>();
                    auto length = unicons::u32_length(sv0.begin(), sv0.end());
                    return *resources.create_json(length);
                }
                default:
                {
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
                }
            }
        }
    };

    template <class Json,class JsonReference>
    class keys_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;
        using string_view_type = typename Json::string_view_type;

        keys_function()
            : function_base<Json,JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json>& resources,
                           std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            JSONCONS_ASSERT(args.size() == *this->arg_count());

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_object())
            {
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }

            auto result = resources.create_json(json_array_arg);
            result->reserve(args.size());

            for (auto& item : arg0_ptr->object_range())
            {
                result->emplace_back(item.key());
            }
            return *result;
        }
    };

    template <class Json, class JsonReference>
    struct static_resources
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;
        using reference = JsonReference;
        using function_base_type = function_base<Json,JsonReference>;

        std::vector<std::unique_ptr<Json>> temp_json_values_;

        static_resources()
        {
        }

        function_base_type* get_function(const string_type& name, std::error_code& ec) const
        {
            static sum_function<Json,JsonReference> sum_func;
            static avg_function<Json,JsonReference> avg_func;
            static min_function<Json,JsonReference> min_func;
            static max_function<Json,JsonReference> max_func;
            static length_function<Json,JsonReference> length_func;
            static keys_function<Json,JsonReference> keys_func;

            static std::unordered_map<string_type,function_base_type*> functions =
            {
                {string_type{'s','u','m'}, &sum_func},
                {string_type{'a','v','g'}, &avg_func},
                {string_type{'m','i','n'}, &min_func},
                {string_type{'m','a','x'}, &max_func},
                {string_type{'l','e','n','g','t','h'}, &length_func},
                {string_type{'k','e','y','s'}, &keys_func}
            };

            auto it = functions.find(name);
            if (it == functions.end())
            {
                ec = jsonpath_errc::unknown_function;
                return nullptr;
            }
            return it->second;
        }

        const unary_operator<Json>* get_not_operator() const
        {
            static not_expression<Json> not_oper;
            return &not_oper;

            //static unary_operator<Json> not_properties{ 1,true, unary_not_op };
            //return &not_properties;
        }

        binary_operator<Json>* get_or_operator() const
        {
            static or_operator<Json> or_oper;

            return &or_oper;
        }

        binary_operator<Json>* get_and_operator() const
        {
            static and_operator<Json> and_oper;

            return &and_oper;
        }

        binary_operator<Json>* get_eq_operator() const
        {
            static eq_operator<Json> eq_oper;
            return &eq_oper;
        }

        binary_operator<Json>* get_ne_operator() const
        {
            static ne_operator<Json> ne_oper;
            return &ne_oper;
        }

        binary_operator<Json>* get_lt_operator() const
        {
            static lt_operator<Json> lt_oper;
            return &lt_oper;
        }

        binary_operator<Json>* get_lte_operator() const
        {
            static lte_operator<Json> lte_oper;
            return &lte_oper;
        }

        binary_operator<Json>* get_gt_operator() const
        {
            static gt_operator<Json> gt_oper;
            return &gt_oper;
        }

        binary_operator<Json>* get_gte_operator() const
        {
            static gte_operator<Json> oper;
            return &oper;
        }

        binary_operator<Json>* get_plus_operator() const
        {
            static plus_operator<Json> oper;
            return &oper;
        }

        binary_operator<Json>* get_minus_operator() const
        {
            static minus_operator<Json> oper;
            return &oper;
        }

        binary_operator<Json>* get_mult_operator() const
        {
            static mult_operator<Json> oper;
            return &oper;
        }

        binary_operator<Json>* get_div_operator() const
        {
            static div_operator<Json> oper;
            return &oper;
        }

        const unary_operator<Json>* get_unary_minus_operator() const
        {
            static unary_operator<Json> unary_minus_properties { 1,true, unary_minus_op };
            return &unary_minus_properties;
        }

        const binary_operator<Json>* get_binary_operator_operator(const string_type& id) const
        {
            static const binary_operator<Json> lt_properties{5,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_lt<Json>(),a,b); }};
            static const binary_operator<Json> gt_properties{5,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_lt<Json>(),b,a); }};
            static const binary_operator<Json> mult_properties{3,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_mult<Json>(),a,b); }};
            static const binary_operator<Json> div_properties{3,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_div<Json>(),a,b); }};
            static const binary_operator<Json> plus_properties{4,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_plus<Json>(),a,b); }};
            static const binary_operator<Json> minus_properties{4,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_minus<Json>(),a,b); }};
            static const binary_operator<Json> lte_properties{5,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_lte<Json>(),a,b); }};
            static const binary_operator<Json> gte_properties{5,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_lte<Json>(),b,a); }};
            static const binary_operator<Json> ne_properties{6,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_ne<Json>(),a,b); }};
            static const binary_operator<Json> eq_properties{6,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_eq<Json>(),a,b); }};
            static const binary_operator<Json> eqtilde_properties{2,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_regex<Json>(),a,b); }};
            static const binary_operator<Json> ampamp_properties{7,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_ampamp<Json>(),a,b); }};
            static const binary_operator<Json> pipepipe_properties{8,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_pipepipe<Json>(),a,b); }};

            switch(id.size())
            {
                case 1:
                {
                    char_type c1 = id[0];
                    switch (c1)
                    {
                        case '<':
                            return &lt_properties;
                        case '>':
                            return &gt_properties;
                        case '+':
                            return &plus_properties;
                        case '-':
                            return &minus_properties;
                        case '*':
                            return &mult_properties;
                        case '/':
                            return &div_properties;
                        default:
                            return nullptr;
                    }
                    break;
                }
                case 2:
                {
                    char_type c1 = id[0];
                    char_type c2 = id[1];
                    switch (c1)
                    {
                        case '<':
                            return c2 == '=' ? &lte_properties : nullptr;
                        case '>':
                            return c2 == '=' ? &gte_properties : nullptr;
                        case '!':
                            return c2 == '=' ? &ne_properties : nullptr;
                        case '=':
                            switch(c2)
                            {
                                case '=':
                                    return &eq_properties;
                                case '~':
                                    return &eqtilde_properties;
                                default:
                                    return nullptr;
                            }
                        case '&':
                            return c2 == '&' ? &ampamp_properties : nullptr;
                        case '|':
                            return c2 == '|' ? &pipepipe_properties : nullptr;
                        default:
                            return nullptr;
                    }
                    break;
                }
                default:
                    return nullptr;
            }
        }

        template <typename... Args>
        Json* create_json(Args&& ... args)
        {
            auto temp = jsoncons::make_unique<Json>(std::forward<Args>(args)...);
            Json* ptr = temp.get();
            temp_json_values_.emplace_back(std::move(temp));
            return ptr;
        }

    private:
        static Json unary_not_op(const term<Json>& a)
        {
            return a.unary_not();
        }

        static Json unary_minus_op(const term<Json>& a)
        {
            return a.unary_minus();
        }
    };

    enum class token_kind
    {
        root_node,
        current_node,
        recursive_descent,
        lparen,
        rparen,
        begin_union,
        end_union,
        begin_filter,
        end_filter,
        separator,
        literal,
        selector,
        function,
        begin_function,
        end_function,
        argument,
        begin_expression_type,
        end_expression_type,
        end_of_expression,
        unary_operator,
        binary_operator
    };

    struct literal_arg_t
    {
        explicit literal_arg_t() = default;
    };
    constexpr literal_arg_t literal_arg{};

    struct recursive_descent_arg_t
    {
        explicit recursive_descent_arg_t() = default;
    };
    constexpr recursive_descent_arg_t recursive_descent_arg{};

    struct begin_expression_type_arg_t
    {
        explicit begin_expression_type_arg_t() = default;
    };
    constexpr begin_expression_type_arg_t begin_expression_type_arg{};

    struct end_expression_type_arg_t
    {
        explicit end_expression_type_arg_t() = default;
    };
    constexpr end_expression_type_arg_t end_expression_type_arg{};

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

    struct begin_union_arg_t
    {
        explicit begin_union_arg_t() = default;
    };
    constexpr begin_union_arg_t begin_union_arg{};

    struct end_union_arg_t
    {
        explicit end_union_arg_t() = default;
    };
    constexpr end_union_arg_t end_union_arg{};

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

    struct current_node_arg_t
    {
        explicit current_node_arg_t() = default;
    };
    constexpr current_node_arg_t current_node_arg{};

    struct root_node_arg_t
    {
        explicit root_node_arg_t() = default;
    };
    constexpr root_node_arg_t root_node_arg{};

    struct end_function_arg_t
    {
        explicit end_function_arg_t() = default;
    };
    constexpr end_function_arg_t end_function_arg{};

    struct begin_function_arg_t
    {
        explicit begin_function_arg_t() = default;
    };
    constexpr begin_function_arg_t begin_function_arg{};

    struct argument_arg_t
    {
        explicit argument_arg_t() = default;
    };
    constexpr argument_arg_t argument_arg{};

    template <class Json,class JsonReference>
    struct path_node
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>>;
        using reference = JsonReference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;

        string_type path;
        pointer val_ptr;

        path_node() = default;
        path_node(const string_type& p, const pointer& valp)
            : path(p),val_ptr(valp)
        {
        }

        path_node(string_type&& p, pointer&& valp) noexcept
            : path(std::move(p)),val_ptr(valp)
        {
        }
        path_node(const path_node&) = default;

        path_node(path_node&& other) noexcept
            : path(std::move(other.path)), val_ptr(other.val_ptr)
        {

        }
        path_node& operator=(const path_node&) = default;

        path_node& operator=(path_node&& other) noexcept
        {
            path.swap(other.path);
            val_ptr = other.val_ptr;
            return *this;
        }
    };

    template <class Json,class JsonReference>
    struct node_less
    {
        bool operator()(const path_node<Json,JsonReference>& a, const path_node<Json,JsonReference>& b) const
        {
            return *(a.val_ptr) < *(b.val_ptr);
        }
    };

    template <class Json,class JsonReference>
    class selector_base
    {
        bool is_projection_;
        bool is_filter_;
        std::size_t precedence_level_;
    public:
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>>;
        using reference = JsonReference;
        using path_node_type = path_node<Json,JsonReference>;

        selector_base()
            : is_projection_(false), 
              is_filter_(false),
              precedence_level_(0)
        {
        }

        selector_base(bool is_projection,
                      bool is_filter,
                      std::size_t precedence_level = 0)
            : is_projection_(is_projection), 
              is_filter_(is_filter),
              precedence_level_(precedence_level)
        {
        }

        virtual ~selector_base() noexcept = default;

        bool is_projection() const 
        {
            return is_projection_;
        }

        bool is_filter() const
        {
            return is_filter_;
        }

        std::size_t precedence_level() const
        {
            return precedence_level_;
        }

        bool is_right_associative() const
        {
            return true;
        }

        virtual void select(dynamic_resources<Json>& resources,
                            const string_type& path, 
                            reference root,
                            reference val, 
                            std::vector<path_node_type>& nodes) const = 0;

        virtual void add_selector(std::unique_ptr<selector_base>&&) 
        {
        }

        virtual std::string to_string(int = 0) const
        {
            return std::string();
        }
    };

    template <class Json,class JsonReference>
    class token
    {
    public:
        using selector_base_type = selector_base<Json,JsonReference>;

        token_kind type_;

        union
        {
            std::unique_ptr<selector_base_type> selector_;
            const unary_operator<Json>* unary_operator_;
            const binary_operator<Json>* binary_operator_;
            function_base<Json,JsonReference>* function_;
            Json value_;
        };
    public:

        token(const unary_operator<Json>* expression) noexcept
            : type_(token_kind::unary_operator),
              unary_operator_(expression)
        {
        }

        token(const binary_operator<Json>* expression) noexcept
            : type_(token_kind::binary_operator),
              binary_operator_(expression)
        {
        }

        token(current_node_arg_t) noexcept
            : type_(token_kind::current_node)
        {
        }

        token(root_node_arg_t) noexcept
            : type_(token_kind::root_node)
        {
        }

        token(begin_function_arg_t) noexcept
            : type_(token_kind::begin_function)
        {
        }

        token(end_function_arg_t) noexcept
            : type_(token_kind::end_function)
        {
        }

        token(separator_arg_t) noexcept
            : type_(token_kind::separator)
        {
        }

        token(lparen_arg_t) noexcept
            : type_(token_kind::lparen)
        {
        }

        token(rparen_arg_t) noexcept
            : type_(token_kind::rparen)
        {
        }

        token(end_of_expression_arg_t) noexcept
            : type_(token_kind::end_of_expression)
        {
        }

        token(begin_union_arg_t) noexcept
            : type_(token_kind::begin_union)
        {
        }

        token(end_union_arg_t) noexcept
            : type_(token_kind::end_union)
        {
        }

        token(begin_filter_arg_t) noexcept
            : type_(token_kind::begin_filter)
        {
        }

        token(end_filter_arg_t) noexcept
            : type_(token_kind::end_filter)
        {
        }

        token(std::unique_ptr<selector_base_type>&& expression)
            : type_(token_kind::selector)
        {
            new (&selector_) std::unique_ptr<selector_base_type>(std::move(expression));
        }

        token(function_base<Json,JsonReference>* function) noexcept
            : type_(token_kind::function),
              function_(function)
        {
        }

        token(argument_arg_t) noexcept
            : type_(token_kind::argument)
        {
        }

        token(begin_expression_type_arg_t) noexcept
            : type_(token_kind::begin_expression_type)
        {
        }

        token(end_expression_type_arg_t) noexcept
            : type_(token_kind::end_expression_type)
        {
        }

        token(recursive_descent_arg_t) noexcept
            : type_(token_kind::recursive_descent)
        {
        }

        token(literal_arg_t, Json&& value) noexcept
            : type_(token_kind::literal), value_(std::move(value))
        {
        }

        token(token&& other) noexcept
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
                        case token_kind::selector:
                            selector_ = std::move(other.selector_);
                            break;
                        case token_kind::unary_operator:
                            unary_operator_ = other.unary_operator_;
                            break;
                        case token_kind::binary_operator:
                            binary_operator_ = other.binary_operator_;
                            break;
                        case token_kind::function:
                            function_ = other.function_;
                            break;
                        case token_kind::literal:
                            value_ = std::move(other.value_);
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

        ~token() noexcept
        {
            destroy();
        }

        token_kind type() const
        {
            return type_;
        }

        bool is_lparen() const
        {
            return type_ == token_kind::lparen; 
        }

        bool is_rparen() const
        {
            return type_ == token_kind::rparen; 
        }

        bool is_current_node() const
        {
            return type_ == token_kind::current_node; 
        }

        bool is_projection() const
        {
            return type_ == token_kind::selector && selector_->is_projection(); 
        }

        bool is_expression() const
        {
            return type_ == token_kind::selector; 
        }

        bool is_operator() const
        {
            return type_ == token_kind::unary_operator || 
                   type_ == token_kind::binary_operator; 
        }

        bool is_recursive_descent() const
        {
            return type_ == token_kind::recursive_descent; 
        }

        std::size_t precedence_level() const
        {
            switch(type_)
            {
                case token_kind::selector:
                    return selector_->precedence_level();
                case token_kind::unary_operator:
                    return unary_operator_->precedence_level();
                case token_kind::binary_operator:
                    return binary_operator_->precedence_level();
                default:
                    return 0;
            }
        }

        bool is_right_associative() const
        {
            switch(type_)
            {
                case token_kind::selector:
                    return selector_->is_right_associative();
                case token_kind::unary_operator:
                    return unary_operator_->is_right_associative();
                case token_kind::binary_operator:
                    return binary_operator_->is_right_associative();
                default:
                    return false;
            }
        }

        void construct(token&& other)
        {
            type_ = other.type_;
            switch (type_)
            {
                case token_kind::selector:
                    new (&selector_) std::unique_ptr<selector_base_type>(std::move(other.selector_));
                    break;
                case token_kind::unary_operator:
                    unary_operator_ = other.unary_operator_;
                    break;
                case token_kind::binary_operator:
                    binary_operator_ = other.binary_operator_;
                    break;
                case token_kind::function:
                    function_ = other.function_;
                    break;
                case token_kind::literal:
                    new (&value_) Json(std::move(other.value_));
                    break;
                default:
                    break;
            }
        }

        void destroy() noexcept 
        {
            switch(type_)
            {
                case token_kind::selector:
                    selector_.~unique_ptr();
                    break;
                case token_kind::literal:
                    value_.~Json();
                    break;
                default:
                    break;
            }
        }

        std::string to_string(int level = 0) const
        {
            std::string s("Token type: ");
            s.append(std::to_string((int)type_));
            switch (type_)
            {
                case token_kind::selector:
                    s.append(selector_->to_string(level+1));
                    break;
                default:
                    break;
            }
            return s;
        }
    };

    template <class Json,class JsonReference>
    class path_expression
    {
    public:
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>>;
        using string_view_type = typename Json::string_view_type;
        using path_node_type = path_node<Json,JsonReference>;
        using reference = typename path_node_type::reference;
        using pointer = typename path_node_type::pointer;
        using token_type = token<Json,JsonReference>;
    private:
        std::vector<token_type> token_list_;
    public:
        path_expression()
        {
        }

        path_expression(path_expression&& expr)
            : token_list_(std::move(expr.token_list_))
        {
        }

        path_expression(std::vector<token_type>&& token_stack)
            : token_list_(std::move(token_stack))
        {
        }

        path_expression& operator=(path_expression&& expr) = default;

        Json evaluate(dynamic_resources<Json>& resources, 
                      reference root,
                      reference instance) const
        {
            Json result(json_array_arg);

            std::vector<path_node_type> output_stack;
            auto callback = [&output_stack](path_node_type& node)
            {
                output_stack.push_back(node);
            };
            evaluate(resources, root, instance, callback);
            if (!output_stack.empty())
            {
                result.reserve(output_stack.size());
                for (const auto& p : output_stack)
                {
                    result.push_back(*(p.val_ptr));
                }
            }

            return result;
        }

        enum class node_set_tag {none,single,multi};

        struct node_set
        {
            node_set_tag tag;
            union
            {
                path_node_type node;
                std::vector<path_node_type> nodes;
            };

            node_set() noexcept
                : tag(node_set_tag::none), node()
            {
            }
            node_set(path_node_type&& node) noexcept
                : tag(node_set_tag::single),
                node(std::move(node))
            {
            }
            node_set(std::vector<path_node_type>&& nds) noexcept
            {
                if (nds.empty())
                {
                    tag = node_set_tag::none;
                }
                else if (nds.size() == 1)
                {
                    tag = node_set_tag::single;
                    new (&node)path_node_type(nds.back());
                }
                else
                {
                    tag = node_set_tag::multi;
                    new (&nodes) std::vector<path_node_type>(nds);
                }
            }

            node_set(node_set&& other) noexcept
            {
                construct(std::forward<node_set>(other));
            }

            ~node_set() noexcept
            {
                destroy();
            }

            node_set& operator=(node_set&& other)
            {
                if (&other != this)
                {
                    if (tag == other.tag)
                    {
                        switch (tag)
                        {
                            case node_set_tag::single:
                                node = std::move(other.node);
                                break;
                            case node_set_tag::multi:
                                nodes = std::move(other.nodes);
                                break;
                            default:
                                break;
                        }
                    }
                    else
                    {
                        destroy();
                        construct(std::forward<node_set>(other));
                    }
                }
                return *this;
            }

            pointer to_pointer(dynamic_resources<Json>& resources) const
            {
                switch (tag)
                {
                    case node_set_tag::single:
                        return node.val_ptr;
                    case node_set_tag::multi:
                    {
                        auto j = resources.create_json(json_array_arg);
                        j->reserve(nodes.size());
                        for (auto& item : nodes)
                        {
                            j->emplace_back(*item.val_ptr);
                        }
                        return j;
                    }
                    default:
                        return &resources.false_value();
                }
            }

            void construct(node_set&& other) noexcept
            {
                tag = other.tag;
                switch (tag)
                {
                    case node_set_tag::single:
                        new (&node) path_node_type(std::move(other.node));
                        break;
                    case node_set_tag::multi:
                        new (&nodes) std::vector<path_node_type>(std::move(other.nodes));
                        break;
                    default:
                        break;
                }
            }

            void destroy() noexcept 
            {
                switch(tag)
                {
                    case node_set_tag::multi:
                        nodes.~vector<path_node_type>();
                        break;
                    default:
                        break;
                }
            }
        };

        template <class Callback>
        typename std::enable_if<jsoncons::detail::is_function_object<Callback,path_node_type&>::value,void>::type
        evaluate(dynamic_resources<Json>& resources, 
                 reference root,
                 reference current, 
                 Callback callback) const
        {
            std::error_code ec;

            std::vector<node_set> stack;
            std::vector<path_node_type> recursive_in_stack;
            std::vector<path_node_type> recursive_out_stack;
            std::vector<path_node_type> collected;
            std::vector<pointer> arg_stack;
            string_type path;
            path.push_back('$');
            Json result(json_array_arg);
            bool is_recursive_descent = false;

            //std::cout << "EVALUATE BEGIN\n";
            //for (auto& tok : token_list_)
            //{
            //    std::cout << tok.to_string() << "\n";
            //}

            if (!token_list_.empty())
            {
                for (std::size_t i = 0; 
                     i < token_list_.size();
                     )
                {
                    auto& tok = token_list_[i];
                    //std::cout << "Token: " << tok.to_string() << "\n";
                    switch (tok.type())
                    { 
                        case token_kind::literal:
                        {
                            stack.emplace_back(path_node_type(string_type(), &tok.value_));
                            break;
                        }
                        case token_kind::unary_operator:
                        {
                            JSONCONS_ASSERT(stack.size() >= 1);
                            pointer ptr = stack.back().to_pointer(resources);
                            stack.pop_back();

                            reference r = tok.unary_operator_->evaluate(resources, *ptr, ec);
                            stack.emplace_back(path_node_type("",std::addressof(r)));
                            break;
                        }
                        case token_kind::binary_operator:
                        {
                            JSONCONS_ASSERT(stack.size() >= 2);
                            pointer rhs = stack.back().to_pointer(resources);
                            stack.pop_back();
                            pointer lhs = stack.back().to_pointer(resources);
                            stack.pop_back();

                            reference r = tok.binary_operator_->evaluate(resources, *lhs, *rhs, ec);
                            stack.emplace_back(path_node_type("",std::addressof(r)));
                            break;
                        }
                        case token_kind::root_node:
                            //std::cout << "root: " << root << "\n";
                            stack.emplace_back(path_node_type(string_type(),std::addressof(root)));
                            break;
                        case token_kind::current_node:
                            stack.emplace_back(path_node_type(string_type(),std::addressof(current)));
                            break;
                        case token_kind::argument:
                            JSONCONS_ASSERT(!stack.empty());
                            //std::cout << "argument stack items " << stack.size() << "\n";
                            //for (auto& item : stack)
                            //{
                            //    std::cout << *item.to_pointer(resources) << "\n";
                            //}
                            //std::cout << "\n";
                            arg_stack.push_back(stack.back().to_pointer(resources));
                            //for (auto& item : arg_stack)
                            //{
                            //    std::cout << *item << "\n";
                            //}
                            //std::cout << "\n";
                            stack.pop_back();
                            break;
                        case token_kind::function:
                        {
                            if (tok.function_->arg_count() && *(tok.function_->arg_count()) != arg_stack.size())
                            {
                                //ec = jmespath_errc::invalid_arity;
                                return;
                            }
                            //std::cout << "function arg stack:\n";
                            //for (auto& item : arg_stack)
                            //{
                            //    std::cout << *item << "\n";
                            //}
                            //std::cout << "\n";

                            reference r = tok.function_->evaluate(resources, arg_stack, ec);
                            if (ec)
                            {
                                return;
                            }
                            //std::cout << "function result: " << r << "\n";
                            arg_stack.clear();
                            stack.emplace_back(path_node_type(string_type(),std::addressof(r)));
                            break;
                        }
                        case token_kind::selector:
                        {
                            if (is_recursive_descent)
                            {
                                for (auto& item : recursive_in_stack)
                                {
                                    tok.selector_->select(resources, path, root, *(item.val_ptr), recursive_out_stack);
                                }
                            }
                            else
                            {
                                JSONCONS_ASSERT(!stack.empty());
                                pointer ptr = nullptr;
                                //for (auto& item : stack)
                                //{
                                    //std::cout << "selector stack input:\n";
                                    //switch (item.tag)
                                    //{
                                    //    case node_set_tag::single:
                                    //        std::cout << "single: " << *(item.node.val_ptr) << "\n";
                                    //        break;
                                    //    case node_set_tag::multi:
                                    //        for (auto& node : stack.back().nodes)
                                    //        {
                                    //            std::cout << "multi: " << *node.val_ptr << "\n";
                                    //        }
                                    //        break;
                                    //    default:
                                    //        break;
                                    //}
                                    //std::cout << "\n";
                                //}
                                switch (stack.back().tag)
                                {
                                    case node_set_tag::single:
                                        ptr = stack.back().node.val_ptr;
                                        break;
                                    case node_set_tag::multi:
                                        if (!stack.back().nodes.empty())
                                        {
                                            ptr = stack.back().nodes.back().val_ptr;
                                        }
                                        ptr = stack.back().to_pointer(resources);
                                        break;
                                    default:
                                        break;
                                }
                                if (ptr)
                                {
                                    //std::cout << "selector item: " << *ptr << "\n";
                                    stack.pop_back();
                                    std::vector<path_node_type> temp;
                                    tok.selector_->select(resources, path, root, *ptr, temp);
                                    stack.emplace_back(std::move(temp));

                                    //std::cout << "selector output\n";
                                    //for (auto& item : temp)
                                    //{
                                    //    std::cout << *item.val_ptr << "\n";
                                    //}
                                    //std::cout << "\n";

                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }

                    if (is_recursive_descent && recursive_in_stack.empty()) // recursive_out_stack is empty too
                    {
                        stack.emplace_back(std::move(collected));
                        collected.clear();
                        is_recursive_descent = false;
                        ++i;
                    }
                    else if (is_recursive_descent)
                    {
                        for (auto& item : recursive_out_stack)
                        {
                            collected.emplace_back(item);
                        }
                        recursive_out_stack.clear();
                        for (auto& item : recursive_in_stack)
                        {
                            if (item.val_ptr->is_object())
                            {
                                for (auto& val : item.val_ptr->object_range())
                                {
                                    recursive_out_stack.emplace_back(val.key(),std::addressof(val.value()));
                                }
                            }
                            else if (item.val_ptr->is_array())
                            {
                                for (auto& val : item.val_ptr->array_range())
                                {
                                    recursive_out_stack.emplace_back("",std::addressof(val));
                                }
                            }
                        }
                        recursive_in_stack = std::move(recursive_out_stack);
                        recursive_out_stack.clear();
                    }
                    else if (tok.is_recursive_descent())
                    {
                        JSONCONS_ASSERT(!stack.empty());
                        switch (stack.back().tag)
                        {
                            case node_set_tag::single:
                                recursive_in_stack.emplace_back(stack.back().node);
                                stack.pop_back();
                                is_recursive_descent = true;
                                break;
                            case node_set_tag::multi:
                                if (!stack.back().nodes.empty())
                                {
                                    recursive_in_stack.emplace_back(stack.back().nodes.back());
                                    stack.pop_back();
                                    is_recursive_descent = true;
                                }
                                break;
                            default:
                                break;
                        }
                        ++i;
                    }
                    else 
                    {
                        ++i;
                    }
                }
            }
            if (!stack.empty())
            {
                for (auto& item : stack)
                {
                    switch (item.tag)
                    {
                        case node_set_tag::single:
                            callback(item.node);
                            break;
                        case node_set_tag::multi:
                            for (auto& node : item.nodes)
                            {
                                callback(node);
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            //std::cout << "EVALUATE END\n";
        }
    };

} // namespace detail
} // namespace jsonpath_new
} // namespace jsoncons

#endif // JSONCONS_JSONPATH_JSONPATH_EXPRESSION_HPP

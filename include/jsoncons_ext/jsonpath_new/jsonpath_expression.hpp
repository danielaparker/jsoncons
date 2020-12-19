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
    class path_term;
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

        Json operator()(const value_term<Json>& lhs, const path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(lhs.value(),rhs.result()[0]);
        }

        Json operator()(const path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(rhs, lhs);
        }

        Json operator()(const path_term<Json>& lhs, const path_term<Json>& rhs) const
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

        bool operator()(const path_term<Json>&, const regex_term<Json>&) const
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

        Json operator()(const value_term<Json>& lhs, const path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return mult(lhs.value(),rhs.result()[0]);
        }

        Json operator()(const path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(rhs, lhs);
        }

        Json operator()(const path_term<Json>& lhs, const path_term<Json>& rhs) const
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

        bool operator()(const path_term<Json>&, const regex_term<Json>&) const
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

        Json operator()(const value_term<Json>& lhs, const path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return div(lhs.value(),rhs.result()[0]);
        }

        Json operator()(const path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return div(lhs.result()[0],rhs.value());
        }

        Json operator()(const path_term<Json>& lhs, const path_term<Json>& rhs) const
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

        bool operator()(const path_term<Json>&, const regex_term<Json>&) const
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

        bool operator()(const value_term<Json>& lhs, const path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return lhs.value() == rhs.result()[0];
        }

        bool operator()(const path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(rhs, lhs);
        }

        bool operator()(const path_term<Json>& lhs, const path_term<Json>& rhs) const
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

        bool operator()(const path_term<Json>&, const regex_term<Json>&) const
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

        bool operator()(const value_term<Json>& lhs, const path_term<Json>& rhs) const 
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return !eq(lhs, rhs);
        }

        bool operator()(const path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return !eq(lhs, rhs);
        }

        bool operator()(const path_term<Json>& lhs, const path_term<Json>& rhs) const 
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

        bool operator()(const path_term<Json>&, const regex_term<Json>&) const
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

        bool operator()(const value_term<Json>& lhs, const path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(lhs.value(),rhs.result()[0]);
        }

        bool operator()(const path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(rhs, lhs);
        }

        bool operator()(const path_term<Json>& lhs, const path_term<Json>& rhs) const
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

        bool operator()(const path_term<Json>&, const regex_term<Json>&) const
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

        bool operator()(const value_term<Json>& lhs, const path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(lhs.value(),rhs.result()[0]);
        }

        bool operator()(const path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return (*this)(rhs, lhs);
        }

        bool operator()(const path_term<Json>& lhs, const path_term<Json>& rhs) const
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

        bool operator()(const path_term<Json>&, const regex_term<Json>&) const
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

        bool operator()(const value_term<Json>& lhs, const path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return !((*this)(rhs, lhs) || eq(rhs,lhs));
        }

        bool operator()(const path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
             {
                return false;
            }
            bool result = lt(lhs.result()[0], rhs.value());
            return result;
        }

        bool operator()(const path_term<Json>& lhs, const path_term<Json>& rhs) const
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

        bool operator()(const path_term<Json>&, const regex_term<Json>&) const
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

        bool operator()(const value_term<Json>& lhs, const path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return !lt(rhs, lhs);
        }

        bool operator()(const path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
             {
                return false;
            }
            bool result = (*this)(lhs.result()[0], rhs.value());
            return result;
        }

        bool operator()(const path_term<Json>& lhs, const path_term<Json>& rhs) const
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

        bool operator()(const path_term<Json>&, const regex_term<Json>&) const
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

        Json operator()(const value_term<Json>& lhs, const path_term<Json>& rhs) const
        {
            if (rhs.result().size() != 1)
            {
                return false;
            }
            return minus(lhs.value(),rhs.result()[0]);
        }

        Json operator()(const path_term<Json>& lhs, const value_term<Json>& rhs) const
        {
            if (lhs.result().size() != 1)
            {
                return false;
            }
            return minus(lhs.result()[0],rhs.value());
        }

        Json operator()(const path_term<Json>& lhs, const path_term<Json>& rhs) const
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

        bool operator()(const path_term<Json>&, const regex_term<Json>&) const
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
        bool operator()(const path_term<Json>&, const value_term<Json>&) const 
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
        bool operator()(const path_term<Json>&, const path_term<Json>&) const 
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }
        bool operator()(const value_term<Json>& lhs, const regex_term<Json>& rhs) const
        {
            return rhs.evaluate(lhs.value().as_string()); 
        }
        bool operator()(const value_term<Json>&, const path_term<Json>&) const 
        {
            JSONCONS_THROW(jsonpath_error(jsonpath_errc::invalid_filter_unsupported_operator));
        }

        bool operator()(const path_term<Json>& lhs, const regex_term<Json>& rhs) const
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
    struct unary_operator_properties
    {
        typedef std::function<Json(const term<Json>&)> operator_type;

        std::size_t precedence_level;
        bool is_right_associative;
        operator_type op;
    };

    template <class Json>
    struct binary_operator_properties
    {
        typedef std::function<Json(const term<Json>&, const term<Json>&)> operator_type;

        std::size_t precedence_level;
        bool is_right_associative;
        operator_type op;
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
                        const auto& t2 = static_cast<const path_term<Json>&>(w);
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
                const auto& t1 = static_cast<const path_term<Json>&>(v);
                switch(w.type())
                {
                    case term_type::value:
                    {
                        const auto& t2 = static_cast<const value_term<Json>&>(w);
                        return vis(t1, t2);
                    }
                    case term_type::path:
                    {
                        const auto& t2 = static_cast<const path_term<Json>&>(w);
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

    template <class Json>
    struct jsonpath_resources
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;

        std::vector<std::unique_ptr<Json>> temp_json_values_;

        jsonpath_resources()
        {
        }

        const unary_operator_properties<Json>* get_not_properties() const
        {
            static unary_operator_properties<Json> not_properties{ 1,true, unary_not_op };
            return &not_properties;
        }

        const unary_operator_properties<Json>* get_unary_minus_properties() const
        {
            static unary_operator_properties<Json> unary_minus_properties { 1,true, unary_minus_op };
            return &unary_minus_properties;
        }

        const binary_operator_properties<Json>* get_binary_operator_properties(const string_type& id) const
        {
            static const binary_operator_properties<Json> lt_properties{5,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_lt<Json>(),a,b); }};
            static const binary_operator_properties<Json> gt_properties{5,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_lt<Json>(),b,a); }};
            static const binary_operator_properties<Json> mult_properties{3,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_mult<Json>(),a,b); }};
            static const binary_operator_properties<Json> div_properties{3,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_div<Json>(),a,b); }};
            static const binary_operator_properties<Json> plus_properties{4,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_plus<Json>(),a,b); }};
            static const binary_operator_properties<Json> minus_properties{4,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_minus<Json>(),a,b); }};
            static const binary_operator_properties<Json> lte_properties{5,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_lte<Json>(),a,b); }};
            static const binary_operator_properties<Json> gte_properties{5,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_lte<Json>(),b,a); }};
            static const binary_operator_properties<Json> ne_properties{6,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_ne<Json>(),a,b); }};
            static const binary_operator_properties<Json> eq_properties{6,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_eq<Json>(),a,b); }};
            static const binary_operator_properties<Json> eqtilde_properties{2,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_regex<Json>(),a,b); }};
            static const binary_operator_properties<Json> ampamp_properties{7,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_ampamp<Json>(),a,b); }};
            static const binary_operator_properties<Json> pipepipe_properties{8,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_pipepipe<Json>(),a,b); }};

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
        Json* create_temp(Args&& ... args)
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

    enum class path_token_type
    {
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
        end_of_expression
    };

} // namespace detail
} // namespace jsonpath_new
} // namespace jsoncons

#endif // JSONCONS_JSONPATH_JSONPATH_EXPRESSION_HPP

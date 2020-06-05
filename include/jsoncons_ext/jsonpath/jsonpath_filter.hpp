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
#include <jsoncons_ext/jsonpath/jsonpath_error.hpp>

namespace jsoncons { namespace jsonpath { namespace detail {

template <class Json>
class term;
template <class Json>
class value_term;
template <class Json>
class path_term;
template <class Json>
class regex_term;

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
        for (size_t i = 0; i < lhs.result().size(); ++i)
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
        for (size_t i = 0; i < lhs.result().size(); ++i)
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
        for (size_t i = 0; i < lhs.result().size(); ++i)
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

    unary_operator_properties<Json> not_properties;
    unary_operator_properties<Json> unary_minus_properties;

    binary_operator_properties<Json> lt_properties;
    binary_operator_properties<Json> gt_properties;
    binary_operator_properties<Json> mult_properties;
    binary_operator_properties<Json> div_properties;
    binary_operator_properties<Json> plus_properties;
    binary_operator_properties<Json> minus_properties;
    binary_operator_properties<Json> lte_properties;
    binary_operator_properties<Json> gte_properties;
    binary_operator_properties<Json> ne_properties;
    binary_operator_properties<Json> eq_properties;
    binary_operator_properties<Json> eqtilde_properties;
    binary_operator_properties<Json> ampamp_properties;
    binary_operator_properties<Json> pipepipe_properties;

    jsonpath_resources()
        : not_properties{ 1,true, unary_not_op },
          unary_minus_properties{ 1,true, unary_minus_op },
          lt_properties{5,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_lt<Json>(),a,b); }},
          gt_properties{5,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_lt<Json>(),b,a); }},
          mult_properties{3,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_mult<Json>(),a,b); }},
          div_properties{3,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_div<Json>(),a,b); }},
          plus_properties{4,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_plus<Json>(),a,b); }},
          minus_properties{4,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_minus<Json>(),a,b); }},
          lte_properties{5,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_lte<Json>(),a,b); }},
          gte_properties{5,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_lte<Json>(),b,a); }},
          ne_properties{6,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_ne<Json>(),a,b); }},
          eq_properties{6,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_eq<Json>(),a,b); }},
          eqtilde_properties{2,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_regex<Json>(),a,b); }},
          ampamp_properties{7,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_ampamp<Json>(),a,b); }},
          pipepipe_properties{8,false,[](const term<Json>& a, const term<Json>& b) -> Json {return visit(cmp_pipepipe<Json>(),a,b); }}
    {
    }

    const binary_operator_properties<Json>* get_binary_operator_properties(const string_type& id) const
    {
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
        jsoncons::detail::write_integer(index,s);
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

enum class token_type 
{
    value,
    path,
    regex,
    unary_operator,
    binary_operator,
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

    virtual void initialize(jsonpath_resources<Json>& resources, const Json&) = 0;

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

    void initialize(jsonpath_resources<Json>&, const Json&) override
    {
    }

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
        return jsoncons::jsonpath::detail::unary_minus(value_);
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

    void initialize(jsonpath_resources<Json>&, const Json&) override
    {
    }

    term_type type() const override {return term_type::regex;}

    bool evaluate(const string_type& subject) const
    {
        return std::regex_search(subject, pattern_);
    }
};

template <class Json>
class path_term final : public term<Json>
{
    using char_type = typename Json::char_type;
    using string_type = std::basic_string<char_type>;

    string_type path_;
    std::size_t line_;
    std::size_t column_;
    Json nodes_;
public:
    path_term(const string_type& path, std::size_t line, std::size_t column)
        : path_(path), line_(line), column_(column)
    {
    }

    path_term(const path_term&) = default;
    path_term(path_term&&) = default;
    path_term& operator=(const path_term&) = default;
    path_term& operator=(path_term&&) = default;

    void initialize(jsonpath_resources<Json>& resources, const Json& current_node) override
    {
        jsonpath_evaluator<Json,const Json&,VoidPathConstructor<Json>> evaluator(line_,column_);
        evaluator.evaluate(resources, current_node, path_);
        nodes_ = evaluator.get_values();
    }

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
        return nodes_.size() == 1 ? jsoncons::jsonpath::detail::unary_minus(nodes_[0]) : Json::null();
    }
};

template <class Json>
class token
{
private:
    token_type type_;

    union
    {
        const unary_operator_properties<Json>* unary_op_properties_;
        const binary_operator_properties<Json>* binary_op_properties_;
        value_term<Json> value_term_;
        path_term<Json> path_term_;
        regex_term<Json> regex_term_;
    };

public:

    token(lparen_arg_t)
        : type_(token_type::lparen)
    {
    }

    token(rparen_arg_t)
        : type_(token_type::rparen)
    {
    }

    token(value_term<Json>&& term)
        : type_(token_type::value), value_term_(std::move(term))
    {
    }
    token(path_term<Json>&& term)
        : type_(token_type::path), path_term_(std::move(term))
    {
    }
    token(regex_term<Json>&& term)
        : type_(token_type::regex), regex_term_(std::move(term))
    {
    }

    token(const unary_operator_properties<Json>* properties)
        : type_(token_type::unary_operator), 
          unary_op_properties_(properties)
    {
    }

    token(const binary_operator_properties<Json>* properties)
        : type_(token_type::binary_operator), 
          binary_op_properties_(properties)
    {
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
                type_ = other.type_;
                switch(type_)
                {
                    case token_type::value:
                        value_term_ = other.value_term_;
                        break;
                    case token_type::path:
                        path_term_ = other.path_term_;
                        break;
                    case token_type::regex:
                        regex_term_ = other.regex_term_;
                        break;
                    case token_type::unary_operator:
                        unary_op_properties_ = other.unary_op_properties_;
                        break;
                    case token_type::binary_operator:
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
                type_ = other.type_;
                switch(type_)
                {
                    case token_type::value:
                        value_term_ = std::move(other.value_term_);
                        break;
                    case token_type::path:
                        path_term_ = std::move(other.path_term_);
                        break;
                    case token_type::regex:
                        regex_term_ = std::move(other.regex_term_);
                        break;
                    case token_type::unary_operator:
                        unary_op_properties_ = other.unary_op_properties_;
                        break;
                    case token_type::binary_operator:
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

    token_type type() const
    {
        return type_;
    }

    Json operator()(const term<Json>& a)
    {
        switch(type_)
        {
            case token_type::unary_operator:
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
            case token_type::binary_operator:
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
        return type_ == token_type::unary_operator; 
    }

    bool is_binary_operator() const
    {
        return type_ == token_type::binary_operator; 
    }

    bool is_operand() const
    {
        switch(type_)
        {
            case token_type::value:
            case token_type::path:
            case token_type::regex:
                return true;
            default:
                return false;
        }
    }

    bool is_lparen() const
    {
        return type_ == token_type::lparen; 
    }

    bool is_rparen() const
    {
        return type_ == token_type::rparen; 
    }

    std::size_t precedence_level() const
    {
        switch(type_)
        {
            case token_type::unary_operator:
                return unary_op_properties_->precedence_level;
            case token_type::binary_operator:
                return binary_op_properties_->precedence_level;
            default:
                return 0;
        }
    }

    bool is_right_associative() const
    {
        switch(type_)
        {
            case token_type::unary_operator:
                return unary_op_properties_->is_right_associative;
            case token_type::binary_operator:
                return binary_op_properties_->is_right_associative;
            default:
                return false;
        }
    }

    const term<Json>& operand()
    {
        switch(type_)
        {
            case token_type::value:
                return value_term_;
            case token_type::path:
                return path_term_;
            case token_type::regex:
                return regex_term_;
            default:
                JSONCONS_UNREACHABLE();
        }
    }

    void initialize(jsonpath_resources<Json>& resources, const Json& current_node)
    {
        switch(type_)
        {
            case token_type::value:
                value_term_.initialize(resources, current_node);
                break;
            case token_type::path:
                path_term_.initialize(resources, current_node);
                break;
            case token_type::regex:
                regex_term_.initialize(resources, current_node);
                break;
            default:
                break;
        }
    }

private:

    void construct(const token& other)
    {
        type_ = other.type_;
        switch (type_)
        {
        case token_type::value:
            ::new(static_cast<void*>(&this->value_term_))value_term<Json>(other.value_term_);
            break;
        case token_type::path:
            ::new(static_cast<void*>(&this->path_term_))path_term<Json>(other.path_term_);
            break;
        case token_type::regex:
            ::new(static_cast<void*>(&this->regex_term_))regex_term<Json>(other.regex_term_);
            break;
        case token_type::unary_operator:
            this->unary_op_properties_ = other.unary_op_properties_;
            break;
        case token_type::binary_operator:
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
        case token_type::value:
            ::new(static_cast<void*>(&this->value_term_))value_term<Json>(std::move(other.value_term_));
            break;
        case token_type::path:
            ::new(static_cast<void*>(&this->path_term_))path_term<Json>(std::move(other.path_term_));
            break;
        case token_type::regex:
            ::new(static_cast<void*>(&this->regex_term_))regex_term<Json>(std::move(other.regex_term_));
            break;
        case token_type::unary_operator:
            this->unary_op_properties_ = other.unary_op_properties_;
            break;
        case token_type::binary_operator:
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
            case token_type::value:
                value_term_.~value_term();
                break;
            case token_type::path:
                path_term_.~path_term();
                break;
            case token_type::regex:
                regex_term_.~regex_term();
                break;
            default:
                break;
        }
    }
};

template <class Json>
token<Json> evaluate(jsonpath_resources<Json>& resources, const Json& context, std::vector<token<Json>>& tokens)
{
    for (auto it = tokens.begin(); it != tokens.end(); ++it)
    {
        it->initialize(resources, context);
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
    std::vector<token<Json>> tokens_;
public:
    jsonpath_filter_expr()
    {
    }

    jsonpath_filter_expr(std::vector<token<Json>>&& tokens)
        : tokens_(std::move(tokens))
    {
    }

    Json eval(jsonpath_resources<Json>& resources, const Json& current_node)
    {
        auto t = evaluate(resources, current_node, tokens_);
        return t.operand().get_single_node();
    }

    bool exists(jsonpath_resources<Json>& resources, const Json& current_node)
    {
        auto t = evaluate(resources, current_node,tokens_);
        return t.operand().accept_single_node();
    }
};

template <class Json>
class jsonpath_filter_parser
{
    using char_type = typename Json::char_type;
    using string_type = std::basic_string<char_type>;
    using string_view_type = typename Json::string_view_type;

    std::vector<token<Json>> output_stack_;
    std::vector<token<Json>> operator_stack_;

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

    void push_token(token<Json>&& token)
    {
        switch (token.type())
        {
            case token_type::value:
            case token_type::path:
            case token_type::regex:
                output_stack_.push_back(std::move(token));
                break;
            case token_type::lparen:
                operator_stack_.push_back(std::move(token));
                break;
            case token_type::rparen:
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
            case token_type::unary_operator:
            case token_type::binary_operator:
            {
                if (operator_stack_.empty() || operator_stack_.back().is_lparen())
                {
                    operator_stack_.push_back(std::move(token));
                }
                else if (token.precedence_level() < operator_stack_.back().precedence_level()
                         || (token.precedence_level() == operator_stack_.back().precedence_level() && token.is_right_associative()))
                {
                    operator_stack_.push_back(std::move(token));
                }
                else
                {
                    auto it = operator_stack_.rbegin();
                    while (it != operator_stack_.rend() && it->is_operator()
                           && (token.precedence_level() > it->precedence_level()
                         || (token.precedence_level() == it->precedence_level() && token.is_right_associative())))
                    {
                        output_stack_.push_back(std::move(*it));
                        ++it;
                    }

                    operator_stack_.erase(it.base(),operator_stack_.end());
                    operator_stack_.push_back(std::move(token));
                }
                break;
            }
            default:
                break;
        }
    }

    jsonpath_filter_expr<Json> parse(jsonpath_resources<Json>& resources, 
                                     const Json& root, 
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
                            push_token(token<Json>(lparen_arg));
                            break;
                        case ')':
                            state = filter_state::expect_path_or_value_or_unary_op;
                            push_token(token<Json>(rparen_arg));
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
                        push_token(token<Json>(properties));
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
                        push_token(token<Json>(properties));
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
                        push_token(token<Json>(properties));
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
                                    push_token(token<Json>(value_term<Json>(Json::parse(buffer))));
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
                                        push_token(token<Json>(value_term<Json>(Json::parse(buffer))));
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
                                    push_token(token<Json>(value_term<Json>(std::move(val))));
                                }
                                JSONCONS_CATCH(const ser_error&)     
                                {
                                    JSONCONS_THROW(jsonpath_error(jsonpath_errc::parse_error_in_filter,line_,column_));
                                }
                                buffer.clear();
                                buffer_line = buffer_column = 1;
                            }
                            push_token(token<Json>(rparen_arg));
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
                                    push_token(token<Json>(value_term<Json>(std::move(val))));
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
                                push_token(token<Json>(value_term<Json>(std::move(val))));
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
                        push_token(token<Json>(&(resources.not_properties)));
                        ++p;
                        ++column_;
                        break;
                    }
                    case '-':
                    {
                        push_token(token<Json>(&(resources.unary_minus_properties)));
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
                        push_token(token<Json>(lparen_arg));
                        ++p;
                        ++column_;
                        break;
                    case ')':
                        push_token(token<Json>(rparen_arg));
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
                        push_token(token<Json>(rparen_arg));
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
                        push_token(token<Json>(rparen_arg));
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
                            if (!path_mode_stack.empty())
                            {
                                if (path_mode_stack[0] == filter_path_mode::root_path)
                                {
                                    jsonpath_evaluator<Json,const Json&,detail::VoidPathConstructor<Json>> evaluator(buffer_line,buffer_column);
                                    evaluator.evaluate(resources, root, buffer);
                                    auto result = evaluator.get_values();
                                    if (result.size() > 0)
                                    {
                                        push_token(token<Json>(value_term<Json>(std::move(result[0]))));
                                    }
                                }
                                else
                                {
                                    push_token(token<Json>(path_term<Json>(buffer, buffer_line, buffer_column)));
                                }
                                path_mode_stack.pop_back();
                            }
                            else
                            {
                                push_token(token<Json>(path_term<Json>(buffer, buffer_line, buffer_column)));
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
                        if (!path_mode_stack.empty())
                        {
                            if (path_mode_stack[0] == filter_path_mode::root_path)
                            {
                                jsonpath_evaluator<Json,const Json&,detail::VoidPathConstructor<Json>> evaluator(buffer_line,buffer_column);
                                evaluator.evaluate(resources, root, buffer);
                                auto result = evaluator.get_values();
                                if (result.size() > 0)
                                {
                                    push_token(token<Json>(value_term<Json>(std::move(result[0]))));
                                }
                                push_token(token<Json>(rparen_arg));
                            }
                            else
                            {
                                push_token(token<Json>(path_term<Json>(buffer, buffer_line, buffer_column)));
                            }
                            path_mode_stack.pop_back();
                        }
                        else
                        {
                            push_token(token<Json>(path_term<Json>(buffer, buffer_line, buffer_column)));
                            push_token(token<Json>(rparen_arg));
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
                                push_token(token<Json>(regex_term<Json>(buffer,flags)));
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

}}}

#endif
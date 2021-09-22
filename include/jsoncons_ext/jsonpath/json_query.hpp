// Copyright 2021 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSON_QUERY_HPP
#define JSONCONS_JSONPATH_JSON_QUERY_HPP

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_expression.hpp>

namespace jsoncons { 
namespace jsonpath {
     
    template<class Json>
    Json json_query(const Json& instance,
                    const typename Json::string_view_type& path, 
                    result_options options = result_options(),
                    const custom_functions<Json>& functions = custom_functions<Json>())
    {
        auto expr = make_expression<Json>(path, functions);
        return expr.evaluate(instance, options);
    }

    template<class Json,class Callback>
    typename std::enable_if<type_traits::is_binary_function_object<Callback,const std::basic_string<typename Json::char_type>&,const Json&>::value,void>::type
    json_query(const Json& instance, 
               const typename Json::string_view_type& path, 
               Callback callback,
               result_options options = result_options(),
               const custom_functions<Json>& functions = custom_functions<Json>())
    {
        auto expr = make_expression<Json>(path, functions);
        expr.evaluate(instance, callback, options);
    }

    template<class Json, class T>
    typename std::enable_if<is_json_type_traits_specialized<Json,T>::value,void>::type
        json_replace(Json& instance, const typename Json::string_view_type& path, T&& new_value,
                     result_options options = result_options::nodups,
                     const custom_functions<Json>& funcs = custom_functions<Json>())
    {
        using evaluator_t = typename jsoncons::jsonpath::detail::jsonpath_evaluator<Json, Json&>;
        //using string_type = typename evaluator_t::string_type;
        using value_type = typename evaluator_t::value_type;
        using reference = typename evaluator_t::reference;
        using json_selector_t = typename evaluator_t::path_expression_type;
        using json_location_type = typename evaluator_t::json_location_type;

        jsoncons::jsonpath::detail::static_resources<value_type,reference> static_resources(funcs);
        evaluator_t e;
        json_selector_t expr = e.compile(static_resources, path);

        jsoncons::jsonpath::detail::dynamic_resources<Json,reference> resources;
        auto callback = [&new_value](const json_location_type&, reference v)
        {
            v = std::forward<T>(new_value);
        };
        expr.evaluate(resources, instance, resources.root_path_node(), instance, callback, options);
    }

    template<class Json, class UnaryCallback>
    typename std::enable_if<type_traits::is_unary_function_object<UnaryCallback,Json>::value,void>::type
    json_replace(Json& instance, const typename Json::string_view_type& path , UnaryCallback callback)
    {
        using evaluator_t = typename jsoncons::jsonpath::detail::jsonpath_evaluator<Json, Json&>;
        //using string_type = typename evaluator_t::string_type;
        using value_type = typename evaluator_t::value_type;
        using reference = typename evaluator_t::reference;
        using json_selector_t = typename evaluator_t::path_expression_type;
        using json_location_type = typename evaluator_t::json_location_type;

        jsoncons::jsonpath::detail::static_resources<value_type,reference> static_resources;
        evaluator_t e;
        json_selector_t expr = e.compile(static_resources, path);

        jsoncons::jsonpath::detail::dynamic_resources<Json,reference> resources;
        auto f = [callback](const json_location_type&, reference v)
        {
            v = callback(v);
        };
        expr.evaluate(resources, instance, resources.root_path_node(), instance, f, result_options::nodups);
    }

    template<class Json, class BinaryCallback>
    typename std::enable_if<type_traits::is_binary_function_object<BinaryCallback,const std::basic_string<typename Json::char_type>&,Json&>::value,void>::type
    json_replace(Json& instance, const typename Json::string_view_type& path , BinaryCallback callback, 
                 result_options options = result_options::nodups,
                 const custom_functions<Json>& funcs = custom_functions<Json>())
    {
        using evaluator_t = typename jsoncons::jsonpath::detail::jsonpath_evaluator<Json, Json&>;
        //using string_type = typename evaluator_t::string_type;
        using value_type = typename evaluator_t::value_type;
        using reference = typename evaluator_t::reference;
        using json_selector_t = typename evaluator_t::path_expression_type;
        using json_location_type = typename evaluator_t::json_location_type;

        jsoncons::jsonpath::detail::static_resources<value_type,reference> static_resources(funcs);
        evaluator_t e;
        json_selector_t expr = e.compile(static_resources, path);

        jsoncons::jsonpath::detail::dynamic_resources<Json,reference> resources;

        auto f = [&callback](const json_location_type& path, reference val)
        {
            callback(path.to_string(), val);
        };
        expr.evaluate(resources, instance, resources.root_path_node(), instance, f, options);
    }

} // namespace jsonpath
} // namespace jsoncons

#endif

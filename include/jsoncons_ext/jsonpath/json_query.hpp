// Copyright 2013-2023 Daniel Parker
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
    typename std::enable_if<extension_traits::is_binary_function_object<Callback,const typename Json::string_type&,const Json&>::value,void>::type
    json_query(const Json& instance, 
               const typename Json::string_view_type& path, 
               Callback callback,
               result_options options = result_options(),
               const custom_functions<Json>& functions = custom_functions<Json>())
    {
        auto expr = make_expression<Json>(path, functions);
        expr.evaluate(instance, callback, options);
    }

    template<class Json, class TempAllocator>
    Json json_query(const allocator_set<typename Json::allocator_type,TempAllocator>& alloc_set, 
        const Json& instance, const typename Json::string_view_type& path, 
        result_options options = result_options(),
        const custom_functions<Json>& functions = custom_functions<Json>())
    {
        auto expr = make_expression<Json>(alloc_set, path, functions);
        return expr.evaluate(instance, options);
    }

    template<class Json,class Callback,class TempAllocator>
    typename std::enable_if<extension_traits::is_binary_function_object<Callback,const typename Json::string_type&,const Json&>::value,void>::type
    json_query(const allocator_set<typename Json::allocator_type,TempAllocator>& alloc_set, 
        const Json& instance, const typename Json::string_view_type& path, 
        Callback callback,
        result_options options = result_options(),
        const custom_functions<Json>& functions = custom_functions<Json>())
    {
        auto expr = make_expression<Json>(alloc_set, path, functions);
        expr.evaluate(instance, callback, options);
    }

    template<class Json, class T>
    typename std::enable_if<is_json_type_traits_specialized<Json,T>::value,void>::type
        json_replace(Json& instance, const typename Json::string_view_type& path, T&& new_value,
                     result_options options = result_options::nodups,
                     const custom_functions<Json>& funcs = custom_functions<Json>())
    {
        using jsonpath_traits_type = jsoncons::jsonpath::detail::jsonpath_traits<Json, Json&>;

        using value_type = typename jsonpath_traits_type::value_type;
        using reference = typename jsonpath_traits_type::reference;
        using evaluator_type = typename jsonpath_traits_type::evaluator_type;
        using path_expression_type = typename jsonpath_traits_type::path_expression_type;
        using json_location_type = typename jsonpath_traits_type::json_location_type;

        auto static_resources = jsoncons::make_unique<jsoncons::jsonpath::detail::static_resources<value_type,reference>>(funcs);
        evaluator_type evaluator;
        path_expression_type expr = evaluator.compile(*static_resources, path);

        jsoncons::jsonpath::detail::dynamic_resources<Json,reference> resources;
        auto callback = [&new_value](const json_location_type&, reference v)
        {
            v = std::forward<T>(new_value);
        };
        expr.evaluate(resources, instance, resources.root_path_node(), instance, callback, options);
    }

    template<class Json, class T, class TempAllocator>
    typename std::enable_if<is_json_type_traits_specialized<Json,T>::value,void>::type
        json_replace(const allocator_set<typename Json::allocator_type,TempAllocator>& alloc_set, 
            Json& instance, const typename Json::string_view_type& path, T&& new_value,
            result_options options = result_options::nodups,
            const custom_functions<Json>& funcs = custom_functions<Json>())
    {
        using jsonpath_traits_type = jsoncons::jsonpath::detail::jsonpath_traits<Json, Json&>;

        using value_type = typename jsonpath_traits_type::value_type;
        using reference = typename jsonpath_traits_type::reference;
        using evaluator_type = typename jsonpath_traits_type::evaluator_type;
        using path_expression_type = typename jsonpath_traits_type::path_expression_type;
        using json_location_type = typename jsonpath_traits_type::json_location_type;

        auto static_resources = jsoncons::make_unique<jsoncons::jsonpath::detail::static_resources<value_type,reference>>(funcs, alloc_set.get_allocator());
        evaluator_type evaluator{alloc_set.get_allocator()};
        path_expression_type expr = evaluator.compile(*static_resources, path);

        jsoncons::jsonpath::detail::dynamic_resources<Json,reference> resources{alloc_set.get_allocator()};
        auto callback = [&new_value](const json_location_type&, reference v)
        {
            v = Json(std::forward<T>(new_value), semantic_tag::none);
        };
        expr.evaluate(resources, instance, resources.root_path_node(), instance, callback, options);
    }

    template<class Json, class BinaryCallback>
    typename std::enable_if<extension_traits::is_binary_function_object<BinaryCallback,const typename Json::string_type&,Json&>::value,void>::type
    json_replace(Json& instance, const typename Json::string_view_type& path , BinaryCallback callback, 
                 result_options options = result_options::nodups,
                 const custom_functions<Json>& funcs = custom_functions<Json>())
    {
        using jsonpath_traits_type = jsoncons::jsonpath::detail::jsonpath_traits<Json, Json&>;

        using value_type = typename jsonpath_traits_type::value_type;
        using reference = typename jsonpath_traits_type::reference;
        using evaluator_type = typename jsonpath_traits_type::evaluator_type;
        using path_expression_type = typename jsonpath_traits_type::path_expression_type;
        using json_location_type = typename jsonpath_traits_type::json_location_type;

        auto static_resources = jsoncons::make_unique<jsoncons::jsonpath::detail::static_resources<value_type,reference>>(funcs);
        evaluator_type evaluator;
        path_expression_type expr = evaluator.compile(*static_resources, path);

        jsoncons::jsonpath::detail::dynamic_resources<Json,reference> resources;

        auto f = [&callback](const json_location_type& path, reference val)
        {
            callback(path.to_string(), val);
        };
        expr.evaluate(resources, instance, resources.root_path_node(), instance, f, options);
    }

    template<class Json, class BinaryCallback, class TempAllocator>
    typename std::enable_if<extension_traits::is_binary_function_object<BinaryCallback,const typename Json::string_type&,Json&>::value,void>::type
    json_replace(const allocator_set<typename Json::allocator_type,TempAllocator>& alloc_set, 
        Json& instance, const typename Json::string_view_type& path , BinaryCallback callback, 
        result_options options = result_options::nodups,
        const custom_functions<Json>& funcs = custom_functions<Json>())
    {
        using jsonpath_traits_type = jsoncons::jsonpath::detail::jsonpath_traits<Json, Json&>;

        using value_type = typename jsonpath_traits_type::value_type;
        using reference = typename jsonpath_traits_type::reference;
        using evaluator_type = typename jsonpath_traits_type::evaluator_type;
        using path_expression_type = typename jsonpath_traits_type::path_expression_type;
        using json_location_type = typename jsonpath_traits_type::json_location_type;

        auto static_resources = jsoncons::make_unique<jsoncons::jsonpath::detail::static_resources<value_type,reference>>(funcs, alloc_set.get_allocator());
        evaluator_type evaluator{alloc_set.get_allocator()};
        path_expression_type expr = evaluator.compile(*static_resources, path);

        jsoncons::jsonpath::detail::dynamic_resources<Json,reference> resources{alloc_set.get_allocator()};

        auto f = [&callback](const json_location_type& path, reference val)
        {
            callback(path.to_string(), val);
        };
        expr.evaluate(resources, instance, resources.root_path_node(), instance, f, options);
    }

    // Legacy replace function
    template<class Json, class UnaryCallback>
    typename std::enable_if<extension_traits::is_unary_function_object<UnaryCallback,Json>::value,void>::type
    json_replace(Json& instance, const typename Json::string_view_type& path , UnaryCallback callback)
    {
        using jsonpath_traits_type = jsoncons::jsonpath::detail::jsonpath_traits<Json, Json&>;

        using value_type = typename jsonpath_traits_type::value_type;
        using reference = typename jsonpath_traits_type::reference;
        using evaluator_type = typename jsonpath_traits_type::evaluator_type;
        using path_expression_type = typename jsonpath_traits_type::path_expression_type;
        using json_location_type = typename jsonpath_traits_type::json_location_type;

        auto static_resources = jsoncons::make_unique<jsoncons::jsonpath::detail::static_resources<value_type,reference>>();
        evaluator_type evaluator;
        path_expression_type expr = evaluator.compile(*static_resources, path);

        jsoncons::jsonpath::detail::dynamic_resources<Json,reference> resources;
        auto f = [callback](const json_location_type&, reference v)
        {
            v = callback(v);
        };
        expr.evaluate(resources, instance, resources.root_path_node(), instance, f, result_options::nodups);
    }

} // namespace jsonpath
} // namespace jsoncons

#endif

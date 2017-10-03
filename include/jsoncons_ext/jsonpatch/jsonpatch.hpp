// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPOINTER_JSONPATCH_HPP
#define JSONCONS_JSONPOINTER_JSONPATCH_HPP

#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <cstdlib>
#include <memory>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonpatch/jsonpatch_error_category.hpp>

namespace jsoncons { namespace jsonpatch {

namespace detail {

    JSONCONS_DEFINE_LITERAL(test_literal,"test");
    JSONCONS_DEFINE_LITERAL(add_literal,"add");
    JSONCONS_DEFINE_LITERAL(remove_literal,"remove");
    JSONCONS_DEFINE_LITERAL(replace_literal,"replace");
    JSONCONS_DEFINE_LITERAL(move_literal,"move");
    JSONCONS_DEFINE_LITERAL(copy_literal,"copy");
    JSONCONS_DEFINE_LITERAL(op_literal,"op");
    JSONCONS_DEFINE_LITERAL(path_literal,"path");
    JSONCONS_DEFINE_LITERAL(from_literal,"from");
    JSONCONS_DEFINE_LITERAL(value_literal,"value");

    enum class op_type {add,remove,replace};
    enum class state_type {begin,abort,commit};

    template <class Json>
    struct operation_unwinder
    {
        typedef typename Json::string_type string_type;
        typedef typename Json::string_view_type string_view_type;

        struct entry
        {
            op_type op;
            string_type path;
            Json value;
        };

        Json& target;
        state_type state;
        std::vector<entry> stack;

        operation_unwinder(Json& j)
            : target(j), state(state_type::begin)
        {
        }

        ~operation_unwinder()
        {
            //std::cout << "state: " << std::boolalpha << (state == state_type::commit) << ", stack size: " << stack.size() << std::endl;
            if (state != state_type::commit)
            {
                for (auto it = stack.rbegin(); it != stack.rend(); ++it)
                {
                    if (it->op == op_type::add)
                    {
                        if (jsonpointer::insert_or_assign(target,it->path,it->value) != jsonpointer::jsonpointer_errc())
                        {
                            //std::cout << "add: " << it->path << std::endl;
                            break;
                        }
                    }
                    else if (it->op == op_type::remove)
                    {
                        if (jsonpointer::remove(target,it->path) != jsonpointer::jsonpointer_errc())
                        {
                            //std::cout << "remove: " << it->path << std::endl;
                            break;
                        }
                    }
                    else if (it->op == op_type::replace)
                    {
                        if (jsonpointer::replace(target,it->path,it->value) != jsonpointer::jsonpointer_errc())
                        {
                            //std::cout << "replace: " << it->path << std::endl;
                            break;
                        }
                    }
                }
            }
        }
    };

    template <class Json>
    Json diff(const Json& source, const Json& target, const typename Json::string_type& path)
    {
        typedef typename Json::char_type char_type;
        typedef typename Json::string_type string_type;
        typedef typename Json::string_view_type string_view_type;

        Json result = typename Json::array();

        if (source == target)
        {
            return result;
        }

        if (source.is_array() && target.is_array())
        {
            size_t common = (std::min)(source.size(),target.size());
            for (size_t i = 0; i < common; ++i)
            {
                std::basic_ostringstream<char_type> ss; 
                ss << path << '/' << i;
                auto temp_diff = diff(source[i],target[i],ss.str());
                result.insert(result.array_range().end(),temp_diff.array_range().begin(),temp_diff.array_range().end());
            }
            // Element in source, not in target - remove
            for (size_t i = target.size(); i < source.size(); ++i)
            {
                std::basic_ostringstream<char_type> ss; 
                ss << path << '/' << i;
                Json val = typename Json::object();
                val.insert_or_assign(op_literal<char_type>(), remove_literal<char_type>());
                val.insert_or_assign(path_literal<char_type>(), ss.str());
                result.push_back(std::move(val));
            }
            // Element in target, not in source - add, 
            // Fix contributed by Alexander rog13
            for (size_t i = source.size(); i < target.size(); ++i)
            {
                const auto& a = target[i];
                std::basic_ostringstream<char_type> ss; 
                ss << path << '/' << i;
                Json val = typename Json::object();
                val.insert_or_assign(op_literal<char_type>(), add_literal<char_type>());
                val.insert_or_assign(path_literal<char_type>(), ss.str());
                val.insert_or_assign(value_literal<char_type>(), a);
                result.push_back(std::move(val));
            }
        }
        else if (source.is_object() && target.is_object())
        {
            for (const auto& a : source.object_range())
            {
                std::basic_ostringstream<char_type> ss; 
                ss << path << '/';
                jsonpointer::escape(a.key(),ss);
                auto it = target.find(a.key());
                if (it != target.object_range().end())
                {
                    auto temp_diff = diff(a.value(),it->value(),ss.str());
                    result.insert(result.array_range().end(),temp_diff.array_range().begin(),temp_diff.array_range().end());
                }
                else
                {
                    Json val = typename Json::object();
                    val.insert_or_assign(op_literal<char_type>(), remove_literal<char_type>());
                    val.insert_or_assign(path_literal<char_type>(), ss.str());
                    result.push_back(std::move(val));
                }
            }
            for (const auto& a : target.object_range())
            {
                auto it = source.find(a.key());
                if (it == source.object_range().end())
                {
                    std::basic_ostringstream<char_type> ss; 
                    ss << path << '/';
                    jsonpointer::escape(a.key(),ss);
                    Json val = typename Json::object();
                    val.insert_or_assign(op_literal<char_type>(), add_literal<char_type>());
                    val.insert_or_assign(path_literal<char_type>(), ss.str());
                    val.insert_or_assign(value_literal<char_type>(), a.value());
                    result.push_back(std::move(val));
                }
            }
        }
        else
        {
            Json val = typename Json::object();
            val.insert_or_assign(op_literal<char_type>(), replace_literal<char_type>());
            val.insert_or_assign(path_literal<char_type>(), path);
            val.insert_or_assign(value_literal<char_type>(), target);
            result.push_back(std::move(val));
        }

        return result;
    }
}

template <class Json>
std::tuple<jsonpatch_errc,typename Json::string_type> patch(Json& target, const Json& patch)
{
    typedef typename Json::char_type char_type;
    typedef typename Json::string_type string_type;
    typedef typename Json::string_view_type string_view_type;

    detail::operation_unwinder<Json> unwinder(target);

    // Validate
    
    jsonpatch_errc patch_ec = jsonpatch_errc();
    string_type bad_path;
    for (const auto& operation : patch.array_range())
    {
        unwinder.state = detail::state_type::begin;

        if (operation.count(detail::op_literal<char_type>()) != 1 || operation.count(detail::path_literal<char_type>()) != 1)
        {
            patch_ec = jsonpatch_errc::invalid_patch;
            unwinder.state = detail::state_type::abort;
        }
        else
        {
            const string_view_type op = operation.at(detail::op_literal<char_type>()).as_string_view();
            const string_view_type path = operation.at(detail::path_literal<char_type>()).as_string_view();

            if (op == detail::test_literal<char_type>())
            {
                Json val;
                jsonpointer::jsonpointer_errc ec;
                std::tie(val,ec) = jsonpointer::get(target,path);
                if (ec != jsonpointer::jsonpointer_errc())
                {
                    patch_ec = jsonpatch_errc::test_failed;
                    unwinder.state = detail::state_type::abort;
                }
                else if (operation.count(detail::value_literal<char_type>()) != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state = detail::state_type::abort;
                }
                else if (val != operation.at(detail::value_literal<char_type>()))
                {
                    patch_ec = jsonpatch_errc::test_failed;
                    unwinder.state = detail::state_type::abort;
                }
            }
            else if (op == detail::add_literal<char_type>())
            {
                if (operation.count(detail::value_literal<char_type>()) != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state = detail::state_type::abort;
                }
                else
                {
                    Json val = operation.at(detail::value_literal<char_type>());
                    auto npath = jsonpointer::normalized_path(target,path);
                    auto insert_ec = jsonpointer::insert(target,npath,val); // try insert without replace
                    if (insert_ec == jsonpointer::jsonpointer_errc::key_already_exists) // try a replace
                    {
                        Json orig_val;
                        jsonpointer::jsonpointer_errc select_ec;
                        std::tie(orig_val,select_ec) = jsonpointer::get(target,npath);
                        if (select_ec != jsonpointer::jsonpointer_errc()) // shouldn't happen
                        {
                            patch_ec = jsonpatch_errc::add_failed;
                            unwinder.state = detail::state_type::abort;
                        }
                        else if (jsonpointer::replace(target,npath,val) != jsonpointer::jsonpointer_errc())
                        {
                            patch_ec = jsonpatch_errc::add_failed;
                            unwinder.state = detail::state_type::abort;
                        }
                        else
                        {
                            unwinder.stack.push_back({detail::op_type::replace,npath,orig_val});
                        }
                    }
                    else if (insert_ec == jsonpointer::jsonpointer_errc())
                    {
                        unwinder.stack.push_back({detail::op_type::remove,npath,Json::null()});
                    }
                    else
                    {
                        patch_ec = jsonpatch_errc::add_failed;
                        unwinder.state = detail::state_type::abort;
                    }
                }
            }
            else if (op == detail::remove_literal<char_type>())
            {
                Json val;
                jsonpointer::jsonpointer_errc ec;
                std::tie(val,ec) = jsonpointer::get(target,path);
                if (ec != jsonpointer::jsonpointer_errc())
                {
                    patch_ec = jsonpatch_errc::remove_failed;
                    unwinder.state = detail::state_type::abort;
                }
                else
                {
                    ec = jsonpointer::remove(target,path);
                    if (ec != jsonpointer::jsonpointer_errc())
                    {
                        patch_ec = jsonpatch_errc::remove_failed;
                        unwinder.state = detail::state_type::abort;
                    }
                    else
                    {
                        unwinder.stack.push_back({detail::op_type::add,path,val});
                    }
                }
            }
            else if (op == detail::replace_literal<char_type>())
            {
                Json val;
                jsonpointer::jsonpointer_errc ec;
                std::tie(val,ec) = jsonpointer::get(target,path);
                if (ec != jsonpointer::jsonpointer_errc())
                {
                    patch_ec = jsonpatch_errc::replace_failed;
                    unwinder.state = detail::state_type::abort;
                }
                else if (operation.count(detail::value_literal<char_type>()) != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state = detail::state_type::abort;
                }
                else if (jsonpointer::replace(target,path,operation.at(detail::value_literal<char_type>())) != jsonpointer::jsonpointer_errc())
                {
                    patch_ec = jsonpatch_errc::replace_failed;
                    unwinder.state = detail::state_type::abort;
                }
                else
                {
                    unwinder.stack.push_back({detail::op_type::replace,path,val});
                }
            }
            else if (op == detail::move_literal<char_type>())
            {
                if (operation.count(detail::from_literal<char_type>()) != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state = detail::state_type::abort;
                }
                else
                {
                    Json val;
                    jsonpointer::jsonpointer_errc ec;
                    string_view_type from = operation.at(detail::from_literal<char_type>()).as_string_view();
                    std::tie(val,ec) = jsonpointer::get(target,from);
                    if (ec != jsonpointer::jsonpointer_errc())
                    {
                        patch_ec = jsonpatch_errc::move_failed;
                        unwinder.state = detail::state_type::abort;
                    }
                    else if (jsonpointer::remove(target,from) != jsonpointer::jsonpointer_errc())
                    {
                        patch_ec = jsonpatch_errc::move_failed;
                        unwinder.state = detail::state_type::abort;
                    }
                    else
                    {
                        unwinder.stack.push_back({detail::op_type::add,from,val});
                        // add
                        auto npath = jsonpointer::normalized_path(target,path);
                        auto insert_ec = jsonpointer::insert(target,npath,val); // try insert without replace
                        if (insert_ec == jsonpointer::jsonpointer_errc::key_already_exists) // try a replace
                        {
                            Json orig_val;
                            jsonpointer::jsonpointer_errc select_ec;
                            std::tie(orig_val,select_ec) = jsonpointer::get(target,npath);
                            if (select_ec != jsonpointer::jsonpointer_errc()) // shouldn't happen
                            {
                                patch_ec = jsonpatch_errc::copy_failed;
                                unwinder.state = detail::state_type::abort;
                            }
                            else if (jsonpointer::replace(target,npath,val) != jsonpointer::jsonpointer_errc())
                            {
                                patch_ec = jsonpatch_errc::copy_failed;
                                unwinder.state = detail::state_type::abort;
                            }
                            else
                            {
                                unwinder.stack.push_back({detail::op_type::replace,npath,orig_val});
                            }
                        }
                        else if (insert_ec == jsonpointer::jsonpointer_errc())
                        {
                            unwinder.stack.push_back({detail::op_type::remove,npath,Json::null()});
                        }
                        else
                        {
                            patch_ec = jsonpatch_errc::copy_failed;
                            unwinder.state = detail::state_type::abort;
                        }
                    }           
                }
            }
            else if (op == detail::copy_literal<char_type>())
            {
                if (operation.count(detail::from_literal<char_type>()) != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state = detail::state_type::abort;
                }
                else
                {
                    Json val;
                    jsonpointer::jsonpointer_errc ec;
                    string_view_type from = operation.at(detail::from_literal<char_type>()).as_string_view();
                    std::tie(val,ec) = jsonpointer::get(target,from);
                    if (ec != jsonpointer::jsonpointer_errc())
                    {
                        patch_ec = jsonpatch_errc::copy_failed;
                        unwinder.state = detail::state_type::abort;
                    }
                    else
                    {
                        // add
                        auto npath = jsonpointer::normalized_path(target,path);
                        auto insert_ec = jsonpointer::insert(target,npath,val); // try insert without replace
                        if (insert_ec == jsonpointer::jsonpointer_errc::key_already_exists) // try a replace
                        {
                            Json orig_val;
                            jsonpointer::jsonpointer_errc select_ec;
                            std::tie(orig_val,select_ec) = jsonpointer::get(target,npath);
                            if (select_ec != jsonpointer::jsonpointer_errc()) // shouldn't happen
                            {
                                patch_ec = jsonpatch_errc::copy_failed;
                                unwinder.state = detail::state_type::abort;
                            }
                            else if (jsonpointer::replace(target,npath,val) != jsonpointer::jsonpointer_errc())
                            {
                                patch_ec = jsonpatch_errc::copy_failed;
                                unwinder.state = detail::state_type::abort;
                            }
                            else
                            {
                                unwinder.stack.push_back({detail::op_type::replace,npath,orig_val});
                            }
                        }
                        else if (insert_ec == jsonpointer::jsonpointer_errc())
                        {
                            unwinder.stack.push_back({detail::op_type::remove,npath,Json::null()});
                        }
                        else
                        {
                            patch_ec = jsonpatch_errc::copy_failed;
                            unwinder.state = detail::state_type::abort;
                        }
                    }
                }
            }
            if (unwinder.state != detail::state_type::begin)
            {
                bad_path = path;
            }
        }
        if (unwinder.state != detail::state_type::begin)
        {
            break;
        }
    }
    if (unwinder.state == detail::state_type::begin)
    {
        unwinder.state = detail::state_type::commit;
    }
    return std::make_tuple(patch_ec,bad_path);
}


template <class Json>
Json diff(const Json& source, const Json& target)
{
    typename Json::string_type path;
    return detail::diff(source, target, path);
}

}}

#endif

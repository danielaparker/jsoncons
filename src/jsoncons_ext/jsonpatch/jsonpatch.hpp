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
            if (state != state_type::commit)
            {
                for (const auto& entry : stack)
                {
                    if (entry.op == op_type::add)
                    {
                        if (jsonpointer::try_add(target,entry.path,entry.value) != jsonpointer::jsonpointer_errc())
                        {
                            break;
                        }
                    }
                    else if (entry.op == op_type::remove)
                    {
                        if (jsonpointer::try_remove(target,entry.path) != jsonpointer::jsonpointer_errc())
                        {
                            break;
                        }
                    }
                    else if (entry.op == op_type::replace)
                    {
                        if (jsonpointer::try_replace(target,entry.path,entry.value) != jsonpointer::jsonpointer_errc())
                        {
                            break;
                        }
                    }
                }
            }
        }
    };
}

template <class Json>
std::tuple<jsonpatch_errc,typename Json::string_type,typename Json::string_type> patch(Json& target, const Json& patch)
{
    typedef typename Json::string_type string_type;
    typedef typename Json::string_view_type string_view_type;

    const string_type test_op = string_type({ 't','e','s','t' });
    const string_type add_op = string_type({ 'a','d','d' });
    const string_type remove_op = string_type({ 'r','e','m','o','v','e' });
    const string_type replace_op = string_type({ 'r','e','p','l','a','c','e' });
    const string_type move_op = string_type({ 'm','o','v','e' });
    const string_type copy_op = string_type({ 'c','o','p','y' });

    detail::operation_unwinder<Json> unwinder(target);

    // Validate
    
    jsonpatch_errc patch_ec = jsonpatch_errc();
    string_type bad_op;
    string_type bad_path;
    for (const auto& operation : patch.array_range())
    {
        unwinder.state = detail::state_type::begin;

        if (operation.count("op") != 1 || operation.count("path") != 1)
        {
            patch_ec = jsonpatch_errc::invalid_patch;
            unwinder.state = detail::state_type::abort;
        }
        else
        {
            const string_view_type op = operation.at("op").as_string_view();
            const string_view_type path = operation.at("path").as_string_view();

            if (op == test_op)
            {
                Json val;
                jsonpointer::jsonpointer_errc ec;
                std::tie(val,ec) = jsonpointer::try_select(target,path);
                if (ec != jsonpointer::jsonpointer_errc())
                {
                    patch_ec = jsonpatch_errc::test_failed;
                    unwinder.state = detail::state_type::abort;
                }
                else if (operation.count("value") != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state = detail::state_type::abort;
                }
                else if (val != operation.at("value"))
                {
                    patch_ec = jsonpatch_errc::test_failed;
                    unwinder.state = detail::state_type::abort;
                }
            }
            else if (op == add_op)
            {
                if (operation.count("value") != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state = detail::state_type::abort;
                }
                else if (jsonpointer::try_add(target,path,operation.at("value")) != jsonpointer::jsonpointer_errc())
                {
                    patch_ec = jsonpatch_errc::add_failed;
                    unwinder.state = detail::state_type::abort;
                }
                else
                {
                    unwinder.stack.push_back({detail::op_type::remove,path,Json::null()});
                }
            }
            else if (op == remove_op)
            {
                Json val;
                jsonpointer::jsonpointer_errc ec;
                std::tie(val,ec) = jsonpointer::try_select(target,path);
                if (ec != jsonpointer::jsonpointer_errc())
                {
                    patch_ec = jsonpatch_errc::remove_failed;
                    unwinder.state = detail::state_type::abort;
                }
                else
                {
                    ec = jsonpointer::try_remove(target,path);
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
            else if (op == replace_op)
            {
                Json val;
                jsonpointer::jsonpointer_errc ec;
                std::tie(val,ec) = jsonpointer::try_select(target,path);
                if (ec != jsonpointer::jsonpointer_errc())
                {
                    patch_ec = jsonpatch_errc::replace_failed;
                    unwinder.state = detail::state_type::abort;
                }
                else if (operation.count("value") != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state = detail::state_type::abort;
                }
                else if (jsonpointer::try_replace(target,path,operation.at("value")) != jsonpointer::jsonpointer_errc())
                {
                    patch_ec = jsonpatch_errc::replace_failed;
                    unwinder.state = detail::state_type::abort;
                }
                else
                {
                    unwinder.stack.push_back({detail::op_type::replace,path,val});
                }
            }
            else if (op == move_op)
            {
                if (operation.count("from") != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state = detail::state_type::abort;
                }
                else
                {
                    Json val;
                    jsonpointer::jsonpointer_errc ec;
                    string_view_type from = operation.at("from").as_string_view();
                    std::tie(val,ec) = jsonpointer::try_select(target,from);
                    if (ec != jsonpointer::jsonpointer_errc())
                    {
                        patch_ec = jsonpatch_errc::move_failed;
                        unwinder.state = detail::state_type::abort;
                    }
                    else if (jsonpointer::try_remove(target,from) != jsonpointer::jsonpointer_errc())
                    {
                        patch_ec = jsonpatch_errc::move_failed;
                        unwinder.state = detail::state_type::abort;
                    }
                    else
                    {
                        unwinder.stack.push_back({detail::op_type::add,from,val});
                        if (jsonpointer::try_add(target,path,val) != jsonpointer::jsonpointer_errc())
                        {
                            patch_ec = jsonpatch_errc::move_failed;
                            unwinder.state = detail::state_type::abort;
                        }
                        else
                        {
                            unwinder.stack.push_back({detail::op_type::remove,path,Json::null()});
                        }
                    }           
                }
            }
            else if (op == copy_op)
            {
                if (operation.count("from") != 1)
                {
                    patch_ec = jsonpatch_errc::invalid_patch;
                    unwinder.state = detail::state_type::abort;
                }
                else
                {
                    Json val;
                    jsonpointer::jsonpointer_errc ec;
                    string_view_type from = operation.at("from").as_string_view();
                    std::tie(val,ec) = jsonpointer::try_select(target,from);
                    if (ec != jsonpointer::jsonpointer_errc())
                    {
                        patch_ec = jsonpatch_errc::copy_failed;
                        unwinder.state = detail::state_type::abort;
                    }
                    else
                    {
                        unwinder.stack.push_back({detail::op_type::add,from,val});
                        if (jsonpointer::try_add(target,path,val) != jsonpointer::jsonpointer_errc())
                        {
                            patch_ec = jsonpatch_errc::copy_failed;
                            unwinder.state = detail::state_type::abort;
                        }
                        else
                        {
                            unwinder.stack.push_back({detail::op_type::remove,path,Json::null()});
                        }
                    }
                }
            }
            if (unwinder.state != detail::state_type::begin)
            {
                bad_op = op;
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
    return std::make_tuple(patch_ec,bad_op,bad_path);
}

}}

#endif

// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSON_QUERY_HPP
#define JSONCONS_JSONPATH_JSON_QUERY_HPP

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
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpath/json_query.hpp>

namespace jsoncons { namespace jsonpath {

    template <class String,class Result>
    typename std::enable_if<std::is_convertible<typename String::value_type,typename Result::value_type>::value>::type
    escape(const String& s, Result& result)
    {
        for (auto c : s)
        {
            if (c == '\'')
            {
                // error
            }
            else
            {
                result.push_back(c);
            }
        }
    }

    template<class Json>
    void flatten_(const std::basic_string<typename Json::char_type>& parent_key,
                  const Json& parent_value,
                  Json& result)
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;

        switch (parent_value.type())
        {
            case json_type::array_value:
            {
                if (parent_value.empty())
                {
                    result.try_emplace(parent_key, parent_value);
                }
                else
                {
                    for (std::size_t i = 0; i < parent_value.size(); ++i)
                    {
                        string_type key(parent_key);
                        key.push_back('[');
                        jsoncons::detail::write_integer(i,key);
                        key.push_back(']');
                        flatten_(key, parent_value.at(i), result);
                    }
                }
                break;
            }

            case json_type::object_value:
            {
                if (parent_value.empty())
                {
                    result.try_emplace(parent_key, Json());
                }
                else
                {
                    for (const auto& item : parent_value.object_range())
                    {
                        string_type key(parent_key);
                        key.push_back('[');
                        key.push_back('\'');
                        escape(jsoncons::basic_string_view<char_type>(item.key().data(),item.key().size()), key);
                        key.push_back('\'');
                        key.push_back(']');
                        flatten_(key, item.value(), result);
                    }
                }
                break;
            }

            default:
            {
                result[parent_key] = parent_value;
                break;
            }
        }
    }

    template<class Json>
    Json flatten(const Json& value)
    {
        Json result;
        std::basic_string<typename Json::char_type> parent_key = {'$'};
        flatten_(parent_key, value, result);
        return result;
    }

    enum class unflatten_state 
    {
        start,
        expect_left_bracket,
        left_bracket,
        name_state,
        index_state,
        expect_right_bracket
    };

    template<class Json>
    Json unflatten(const Json& value)
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;

        if (JSONCONS_UNLIKELY(!value.is_object()))
        {
            //JSONCONS_THROW
        }

        Json result;

        for (const auto& item : value.object_range())
        {
            Json* part = &result;
            string_type buffer;
            unflatten_state state = unflatten_state::start;

            auto it = item.key().begin();
            auto last = item.key().end();

            for (; it != last; ++it)
            {
                switch (state)
                {
                    case unflatten_state::start:
                    {
                        switch (*it)
                        {
                            case '$':
                                state = unflatten_state::expect_left_bracket;
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case unflatten_state::expect_left_bracket:
                    {
                        switch (*it)
                        {
                            case '[':
                                state = unflatten_state::left_bracket;
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case unflatten_state::left_bracket:
                    {
                        switch (*it)
                        {
                            case '\'':
                                state = unflatten_state::name_state;
                                break;
                            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                buffer.push_back(*it);
                                state = unflatten_state::index_state;
                            default:
                                break;
                        }
                        break;
                    }
                    case unflatten_state::name_state:
                    {
                        switch (*it)
                        {
                            case '\'':
                                if (it != last-2)
                                {
                                    part->try_emplace(buffer,Json());
                                }
                                else
                                {
                                    part->try_emplace(buffer,item.value());
                                }
                                part = &part->at(buffer);
                                buffer.clear();
                                state = unflatten_state::expect_right_bracket;
                                break;
                            default:
                                buffer.push_back(*it);
                                break;
                        }
                        break;
                    }
                    case unflatten_state::index_state:
                    {
                        switch (*it)
                        {
                            case ']':
                            {
                                auto r = jsoncons::detail::to_integer<size_t>(buffer.data(), buffer.size());
                                if (r)
                                {
                                    if (!part->is_array())
                                    {
                                        *part = Json(json_array_arg);
                                    }
                                    if (it != last-1)
                                    {
                                        if (r.value()+1 > part->size())
                                        {
                                            part->push_back(Json());
                                        }
                                    }
                                    else
                                    {
                                        part->push_back(item.value());
                                    }
                                    part = &part->at(r.value());
                                }
                                buffer.clear();
                                state = unflatten_state::expect_left_bracket;
                                break;
                            }
                            case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
                                buffer.push_back(*it);
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case unflatten_state::expect_right_bracket:
                    {
                        switch (*it)
                        {
                            case ']':
                                state = unflatten_state::expect_left_bracket;
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        return result;
    }
}}

#endif

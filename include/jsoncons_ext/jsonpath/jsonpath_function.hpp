// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSONPATH_FUNCTION_HPP
#define JSONCONS_JSONPATH_JSONPATH_FUNCTION_HPP

#include <string> // std::basic_string
#include <vector> // std::vector
#include <unordered_map> // std::unordered_map
#include <map>
#include <limits> // std::numeric_limits
#include <utility> // std::move
#include <jsoncons_ext/jsonpath/jsonpath_error.hpp>

namespace jsoncons { namespace jsonpath {

JSONCONS_STRING_LITERAL(keys_literal,'k','e','y','s')
JSONCONS_STRING_LITERAL(avg_literal,'a','v','g')
JSONCONS_STRING_LITERAL(max_literal,'m','a','x')
JSONCONS_STRING_LITERAL(min_literal,'m','i','n')
JSONCONS_STRING_LITERAL(sum_literal,'s','u','m')
JSONCONS_STRING_LITERAL(prod_literal,'p','r','o','d')
JSONCONS_STRING_LITERAL(count_literal,'c','o','u','n','t')
JSONCONS_STRING_LITERAL(tokenize_literal,'t','o','k','e','n','i','z','e')

template <class Json, class JsonPointer>
class function_table
{
public:
    using char_type = typename Json::char_type;
    using char_traits_type = typename Json::char_traits_type;
    using string_type = std::basic_string<char_type,char_traits_type>;
    using string_view_type = typename Json::string_view_type;
    using pointer = JsonPointer;
    using argument_type = std::vector<pointer>;
    typedef std::function<Json(const std::vector<argument_type>&, std::error_code&)> function_type;
    using function_dictionary = std::map<basic_string_view<char_type>,function_type>;
private:
    const function_dictionary functions_ =
    {
        {
            keys_literal<char_type>(),[](const std::vector<argument_type>& args, std::error_code& ec) -> Json
                {
                    Json j = typename Json::array();
                    if (args.size() != 1)
                    {
                        ec = jsonpath_errc::invalid_argument;
                        return j; 
                    }
                    if (args[0].size() != 1 && !args[0][0]->is_object())
                    {
                        return j; 
                    }
                    pointer arg = args[0][0];
                    for (const auto& kv : arg->object_range())
                    {
                        j.emplace_back(kv.key());
                    }

                    return j;
                }
        },
        {
            max_literal<char_type>(),[](const std::vector<argument_type>& args, std::error_code& ec) -> Json
                {
                   if (args.size() != 1)
                   {
                       ec = jsonpath_errc::invalid_argument;
                       return Json(); 
                   }
                    const auto& arg = args[0];
                    double v = std::numeric_limits<double>::lowest();
                    for (auto& node : arg)
                    {
                        double x = node->template as<double>();
                        if (x > v)
                        {
                            v = x;
                        }
                    }
                    return Json(v);
                }
        },
        {
            min_literal<char_type>(),[](const std::vector<argument_type>& args, std::error_code& ec) -> Json
                {
                    if (args.size() != 1)
                    {
                        ec = jsonpath_errc::invalid_argument;
                        return Json();
                    }
                    const auto& arg = args[0];
                    double v = (std::numeric_limits<double>::max)(); 
                    for (const auto& node : arg)
                    {
                        double x = node->template as<double>();
                        if (x < v)
                        {
                            v = x;
                        }
                    }
                    return Json(v);
                }
        },
        {
            avg_literal<char_type>(),[](const std::vector<argument_type>& args, std::error_code& ec) -> Json
                {
                    if (args.size() != 1)
                    {
                        ec = jsonpath_errc::invalid_argument;
                        return Json();
                    }
                    const auto& arg = args[0];
                    double v = 0.0;
                    for (const auto& node : arg)
                    {
                        v += node->template as<double>();
                    }
                    return arg.size() > 0 ? Json(v/arg.size()) : Json(null_type());
                }
        },
        {
            sum_literal<char_type>(),[](const std::vector<argument_type>& args, std::error_code& ec) -> Json
                {
                    if (args.size() != 1)
                    {
                        ec = jsonpath_errc::invalid_argument;
                        return Json();
                    }
                    const auto& arg = args[0];
                    double v = 0.0;
                    for (const auto& node : arg)
                    {
                        v += node->template as<double>();
                    }
                    return Json(v);
                }
        },
        {
            count_literal<char_type>(),[](const std::vector<argument_type>& args, std::error_code& ec) -> Json
                {
                    if (args.size() != 1)
                    {
                        ec = jsonpath_errc::invalid_argument;
                        return Json();
                    }
                    const auto& arg = args[0];
                    std::size_t count = 0;
                    while (count < arg.size())
                    {
                        ++count;
                    }
                    return Json(count);
                }
        },
        {
            prod_literal<char_type>(),[](const std::vector<argument_type>& args, std::error_code& ec) -> Json
                {
                    if (args.size() != 1)
                    {
                        ec = jsonpath_errc::invalid_argument;
                        return Json();
                    }
                    const auto& arg = args[0];
                    double v = 0.0;
                    for (const auto& node : arg)
                    {
                        double x = node->template as<double>();
                        v == 0.0 && x != 0.0
                        ? (v = x)
                        : (v *= x);

                    }
                    return Json(v);
                }
        }
#if !(defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 9))
// GCC 4.8 has broken regex support: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631
        ,{
            tokenize_literal<char_type>(),[](const std::vector<argument_type>& args, std::error_code& ec) -> Json
                {
                    if (args.size() != 2)
                    {
                        ec = jsonpath_errc::invalid_argument;
                        return Json();
                    }
                    string_type arg1 = args[0][0]->as_string();
                    string_type arg2 = args[1][0]->as_string();

                    std::regex::flag_type flags = std::regex_constants::ECMAScript; 
                    std::basic_regex<char_type> pieces_regex(arg2, flags);

                    std::regex_token_iterator<typename string_type::const_iterator> rit ( arg1.begin(), arg1.end(), pieces_regex, -1);
                    std::regex_token_iterator<typename string_type::const_iterator> rend;

                    Json j = typename Json::array();
                    while (rit!=rend) 
                    {
                        j.push_back(rit->str());
                        ++rit;
                    }
                    return j;
                }
        }
#endif
    };
public:
    function_type get(const string_type& name, std::error_code& ec) const
    {
        auto it = functions_.find(name);
        if (it == functions_.end())
        {
            //std::cout << "Function name " << name << " not found\n";
            ec = jsonpath_errc::function_name_not_found;
            return nullptr;
        }
        return it->second;
    }
};

}}

#endif

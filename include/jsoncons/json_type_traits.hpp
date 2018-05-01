// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TYPE_TRAITS_HPP
#define JSONCONS_JSON_TYPE_TRAITS_HPP

#include <array>
#include <string>
#include <vector>
#include <valarray>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <algorithm>
#include <fstream>
#include <limits>
#include <type_traits>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/detail/json_type_traits.hpp>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

namespace jsoncons {

// json_type_traits

template <class Json, class T, class Enable=void>
struct json_type_traits
{
    static bool is(const Json& j)
    {
        return detail::json_type_traits<Json,T>::is(j);
    }
    static T as(const Json& j)
    {
        return detail::json_type_traits<Json,T>::as(j);
    }
    template <class ... Args>
    static Json to_json(Args&&... args)
    {
        return detail::json_type_traits<Json, T>::to_json(std::forward<Args>(args)...);
    }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif

// Copyright 2019 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_CONFIG_JSONCONS_CONFIG_HPP
#define JSONCONS_CONFIG_JSONCONS_CONFIG_HPP

#include <jsoncons/config/compiler_support.hpp>

#if !defined(JSONCONS_HAS_STD_STRING_VIEW)
#include <jsoncons/detail/string_view.hpp>
namespace jsoncons {
using jsoncons::detail::basic_string_view;
using string_view = basic_string_view<char, std::char_traits<char>>;
using wstring_view = basic_string_view<wchar_t, std::char_traits<wchar_t>>;
}
#else 
#include <string_view>
namespace jsoncons {
using std::basic_string_view;
using std::string_view;
using std::wstring_view;
}
#endif

#if !defined(JSONCONS_HAS_SPAN)
#include <jsoncons/detail/span.hpp>
namespace jsoncons {
using jsoncons::detail::span;
}
#else 
#include <span>
namespace jsoncons {
using std::span;
}
#endif

#if !defined(JSONCONS_HAS_OPTIONAL)
#include <jsoncons/detail/optional.hpp>
namespace jsoncons {
using jsoncons::detail::optional;
}
#else 
#include <optional>
namespace jsoncons {
using std::optional;
}
#endif // !defined(JSONCONS_HAS_OPTIONAL)

#if !defined(JSONCONS_HAS_MAKE_UNIQUE)

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace jsoncons {

    template<class T> 
    struct unique_if 
    {
        typedef std::unique_ptr<T> value_is_not_array;
    };

    template<class T> 
    struct unique_if<T[]> 
    {
        typedef std::unique_ptr<T[]> value_is_array_of_unknown_bound;
    };

    template<class T, size_t N> 
    struct unique_if<T[N]> {
        typedef void value_is_array_of_known_bound;
    };

    template<class T, class... Args>
    typename unique_if<T>::value_is_not_array
    make_unique(Args&&... args) 
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template<class T>
    typename unique_if<T>::value_is_array_of_unknown_bound
    make_unique(size_t n) 
    {
        typedef typename std::remove_extent<T>::type U;
        return std::unique_ptr<T>(new U[n]());
    }

    template<class T, class... Args>
    typename unique_if<T>::value_is_array_of_known_bound
    make_unique(Args&&...) = delete;
}

#else

#include <memory>
namespace jsoncons 
{
    using std::make_unique;
}

#endif // !defined(JSONCONS_HAS_MAKE_UNIQUE)

#endif // JSONCONS_CONFIG_JSONCONS_CONFIG_HPP

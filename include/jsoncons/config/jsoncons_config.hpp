// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONCONS_CONFIG_HPP
#define JSONCONS_JSONCONS_CONFIG_HPP

#include <jsoncons/config/compiler_support.hpp>

#if !defined(JSONCONS_HAS_STRING_VIEW)
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
#endif

#endif

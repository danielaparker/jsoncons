// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_LITERALS_HPP
#define JSONCONS_JSON_LITERALS_HPP

#include <string> // std::string

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>

namespace jsoncons { 

template <typename CharT>
struct json_literals
{
    static constexpr jsoncons::basic_string_view<CharT> true_literal = jsoncons::basic_string_view<CharT>{JSONCONS_CSTRING_CONSTANT(CharT, "true"),4};
    static constexpr jsoncons::basic_string_view<CharT> false_literal = jsoncons::basic_string_view<CharT>{JSONCONS_CSTRING_CONSTANT(CharT, "false"),5};
    static constexpr jsoncons::basic_string_view<CharT> null_literal = jsoncons::basic_string_view<CharT>{JSONCONS_CSTRING_CONSTANT(CharT, "null"),4};
};
#if __cplusplus >= 201703L
// not needed for C++17
#else
    template <typename CharT> constexpr jsoncons::basic_string_view<CharT> json_literals<CharT>::true_literal;
    template <typename CharT> constexpr jsoncons::basic_string_view<CharT> json_literals<CharT>::false_literal;
    template <typename CharT> constexpr jsoncons::basic_string_view<CharT> json_literals<CharT>::null_literal;
#endif

} // namespace jsoncons

#endif // JSONCONS_JSON_LITERALS_HPP

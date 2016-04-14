/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSONPATH_ERROR_CATEGORY_HPP
#define JSONCONS_JSONPATH_JSONPATH_ERROR_CATEGORY_HPP

#include "jsoncons/jsoncons.hpp"
#include <system_error>

namespace jsoncons { namespace jsonpath {

namespace jsonpath_parser_errc 
{
    const int expected_root = 1;
    const int expected_right_bracket = 2;
    const int expected_name = 3;
    const int expected_separator = 4;
    const int invalid_filter = 5;
    const int invalid_filter_expected_slash = 6;
    const int invalid_filter_unbalanced_paren = 7;
    const int invalid_filter_unsupported_operator = 8;
    const int invalid_filter_expected_right_brace = 9;
    const int invalid_filter_expected_primary = 10;
}

class jsonpath_error_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const JSONCONS_NOEXCEPT
    {
        return "jsonpath";
    }
    virtual std::string message(int ev) const
    {
        switch (ev)
        {
        case jsonpath_parser_errc::expected_root:
            return "Expected $";
        case jsonpath_parser_errc::expected_right_bracket:
            return "Expected ]";
        case jsonpath_parser_errc::expected_name:
            return "Expected a name following a dot";
        case jsonpath_parser_errc::expected_separator:
            return "Expected dot or left bracket separator";
        case jsonpath_parser_errc::invalid_filter:
            return "Invalid path filter";
        case jsonpath_parser_errc::invalid_filter_expected_slash:
            return "Invalid path filter, expected '/'";
        case jsonpath_parser_errc::invalid_filter_unbalanced_paren:
            return "Invalid path filter, unbalanced parenthesis";
        case jsonpath_parser_errc::invalid_filter_unsupported_operator:
            return "Unsupported operator";
        case jsonpath_parser_errc::invalid_filter_expected_right_brace:
            return "Invalid path filter, expected right brace }";
        case jsonpath_parser_errc::invalid_filter_expected_primary:
            return "Invalid path filter, expected primary expression.";
        default:
            return "Unknown jsonpath parser error";
        }
    }
};

inline
const std::error_category& jsonpath_error_category()
{
  static jsonpath_error_category_impl instance;
  return instance;
}

}}
#endif

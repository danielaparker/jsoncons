/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSONPATH_ERROR_CATEGORY_HPP
#define JSONCONS_JSONPATH_JSONPATH_ERROR_CATEGORY_HPP

#include <jsoncons/json_exception.hpp>
#include <system_error>

namespace jsoncons { namespace jsonpath {

enum class jsonpath_parser_errc 
{
    ok = 0,
    expected_root = 1,
    expected_right_bracket = 2,
    expected_name = 3,
    expected_separator = 4,
    invalid_filter = 5,
    invalid_filter_expected_slash = 6,
    invalid_filter_unbalanced_paren = 7,
    invalid_filter_unsupported_operator = 8,
    invalid_filter_expected_right_brace = 9,
    invalid_filter_expected_primary = 10,
    expected_index = 11,
    expected_left_bracket_token = 12,
    unexpected_operator = 13
};

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
        switch (static_cast<jsonpath_parser_errc>(ev))
        {
        case jsonpath_parser_errc::expected_root:
            return "Expected $";
        case jsonpath_parser_errc::expected_right_bracket:
            return "Expected ]";
        case jsonpath_parser_errc::expected_name:
            return "Expected a name following a dot";
        case jsonpath_parser_errc::expected_index:
            return "Expected index";
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
        case jsonpath_parser_errc::expected_left_bracket_token:
            return "Expected ?,',\",0-9,*";
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

inline 
std::error_code make_error_code(jsonpath_parser_errc result)
{
    return std::error_code(static_cast<int>(result),jsonpath_error_category());
}

}}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::jsonpath::jsonpath_parser_errc> : public true_type
    {
    };
}

#endif

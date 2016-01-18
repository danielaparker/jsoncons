/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://sourceforge.net/projects/jsoncons/files/ for latest version
// See https://sourceforge.net/p/jsoncons/wiki/Home/ for documentation.

#ifndef JSONCONS_JSONPATH_JSONPATH_ERROR_CATEGORY_HPP
#define JSONCONS_JSONPATH_JSONPATH_ERROR_CATEGORY_HPP

#include "jsoncons/jsoncons.hpp"
#include <system_error>

namespace jsoncons { namespace jsonpath {

namespace jsonpath_parser_errc 
{
    const int expected_root = 0;
    const int expected_right_bracket = 1;
    const int expected_name = 2;
    const int expected_separator = 3;
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
        default:
            return "Unknown JSON parser error";
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

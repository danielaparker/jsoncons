/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPOINTER_JSONPOINTER_ERROR_CATEGORY_HPP
#define JSONCONS_JSONPOINTER_JSONPOINTER_ERROR_CATEGORY_HPP

#include <jsoncons/json_exception.hpp>
#include <system_error>

namespace jsoncons { namespace jsonpointer {

enum class jsonpointer_errc 
{
    ok = 0,
    expected_slash = 1,
    expected_digit_or_dash,
    unexpected_leading_zero,
    index_exceeds_array_size,
    expected_0_or_1,
    name_not_found,
    key_already_exists,
    expected_object_or_array,
    end_of_input
};

class jsonpointer_error_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const JSONCONS_NOEXCEPT
    {
        return "jsoncons.jsonpointer";
    }
    virtual std::string message(int ev) const
    {
        switch (static_cast<jsonpointer_errc>(ev))
        {
        case jsonpointer_errc::expected_slash:
            return "Expected /";
        case jsonpointer_errc::expected_digit_or_dash:
            return "Expected digit or '-'";
        case jsonpointer_errc::unexpected_leading_zero:
            return "Unexpected leading zero";
        case jsonpointer_errc::index_exceeds_array_size:
            return "Index exceeds array size";
        case jsonpointer_errc::expected_0_or_1:
            return "Expected '0' or '1' after escape character '~'";
        case jsonpointer_errc::name_not_found:
            return "Name not found";
        case jsonpointer_errc::key_already_exists:
            return "Key already exists";
        case jsonpointer_errc::expected_object_or_array:
            return "Expected object or array";
        case jsonpointer_errc::end_of_input:
            return "Unexpected end of input";
        default:
            return "Unknown jsonpointer error";
        }
    }
};

inline
const std::error_category& jsonpointer_error_category()
{
  static jsonpointer_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(jsonpointer_errc result)
{
    return std::error_code(static_cast<int>(result),jsonpointer_error_category());
}

}}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::jsonpointer::jsonpointer_errc> : public true_type
    {
    };
}

#endif

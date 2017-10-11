/// Copyright 2017 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATCH_JSONPATCH_ERROR_CATEGORY_HPP
#define JSONCONS_JSONPATCH_JSONPATCH_ERROR_CATEGORY_HPP

#include <jsoncons/json_exception.hpp>
#include <system_error>

namespace jsoncons { namespace jsonpatch {

enum class jsonpatch_errc 
{
    ok = 0,
    invalid_patch = 1,
    test_failed,
    add_failed,
    remove_failed,
    replace_failed,
    move_failed,
    copy_failed

};

class jsonpatch_error_category_impl
   : public std::error_category
{
public:
    virtual const char* name() const JSONCONS_NOEXCEPT
    {
        return "jsonpatch";
    }
    virtual std::string message(int ev) const
    {
        switch (static_cast<jsonpatch_errc>(ev))
        {
        case jsonpatch_errc::invalid_patch:
            return "Invalid JSON Patch document";
        case jsonpatch_errc::test_failed:
            return "JSON Patch test operation failed";
        case jsonpatch_errc::add_failed:
            return "JSON Patch add operation failed";
        case jsonpatch_errc::remove_failed:
            return "JSON Patch remove operation failed";
        case jsonpatch_errc::replace_failed:
            return "JSON Patch replace operation failed";
        case jsonpatch_errc::move_failed:
            return "JSON Patch move operation failed";
        case jsonpatch_errc::copy_failed:
            return "JSON Patch copy operation failed";
        default:
            return "Unknown JSON Patch error";
        }
    }
};

inline
const std::error_category& jsonpatch_error_category()
{
  static jsonpatch_error_category_impl instance;
  return instance;
}

inline 
std::error_code make_error_code(jsonpatch_errc result)
{
    return std::error_code(static_cast<int>(result),jsonpatch_error_category());
}

}}

namespace std {
    template<>
    struct is_error_code_enum<jsoncons::jsonpatch::jsonpatch_errc> : public true_type
    {
    };
}

#endif

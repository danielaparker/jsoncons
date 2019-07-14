/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_PARSE_ERROR_HANDLER_HPP
#define JSONCONS_PARSE_ERROR_HANDLER_HPP

#include <jsoncons/json_exception.hpp>
#include <jsoncons/json_error.hpp>
#include <jsoncons/ser_context.hpp>

namespace jsoncons {

struct default_parse_error_handler
{
    bool operator()(std::error_code code, const ser_context&) noexcept 
    {
        static const std::error_code illegal_comment = make_error_code(json_errc::illegal_comment);

        if (code == illegal_comment)
        {
            return true; // Recover, allow comments
        }
        else
        {
            return false;
        }
    }
};

struct strict_parse_error_handler
{
    bool operator()(std::error_code, const ser_context&) noexcept
    {
        return false;
    }
};

}
#endif

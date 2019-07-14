/// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_PARSE_ERROR_HANDLER_HPP
#define JSONCONS_PARSE_ERROR_HANDLER_HPP

#include <jsoncons/json_parser.hpp>

namespace jsoncons {

struct default_json_parsing
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

struct strict_json_parsing
{
    bool operator()(std::error_code, const ser_context&) noexcept
    {
        return false;
    }
};

#if !defined(JSONCONS_NO_DEPRECATED)
    JSONCONS_DEPRECATED("instead, use default_json_parsing") typedef default_json_parsing strict_parse_error_handler;
    JSONCONS_DEPRECATED("instead, use strict_json_parsing") typedef strict_json_parsing default_parse_error_handler;
#endif

}
#endif

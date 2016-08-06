// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_TYPE_EXTENSIONS_HPP
#define JSONCONS_EXT_TYPE_EXTENSIONS_HPP

#include "jsoncons/json.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

namespace jsoncons 
{
    template <class Json>
    struct json_type_traits<Json,boost::gregorian::date>
    {
        static const bool is_assignable = true;

        static bool is(const Json& val) JSONCONS_NOEXCEPT
        {
            if (!val.is_string())
            {
                return false;
            }
            std::string s = val.template as<std::string>();
            try
            {
                boost::gregorian::date_from_iso_string(s);
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        static boost::gregorian::date as(const Json& val)
        {
            std::string s = val.template as<std::string>();
            return boost::gregorian::from_simple_string(s);
        }

        static void assign(Json& lhs, boost::gregorian::date val)
        {
            lhs.assign_string(to_iso_extended_string(val));
        }
    };
}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif

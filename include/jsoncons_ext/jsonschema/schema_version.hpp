// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_SCHEMA_VERSION_HPP
#define JSONCONS_JSONSCHEMA_SCHEMA_VERSION_HPP

#include <jsoncons/json.hpp>

namespace jsoncons {
namespace jsonschema {

    class schema_version
    {
    public:
        static bool contains(const string_view& url)
        {
            if (url.find("json-schema.org/draft-07/schema") != string_view::npos)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    };


} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_VERSION_HPP

// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_SCHEMA_PARSER_HPP
#define JSONCONS_JSONSCHEMA_SCHEMA_PARSER_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>
#include <jsoncons_ext/jsonschema/schema_location.hpp>
#include <jsoncons_ext/jsonschema/keyword_validator.hpp>

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    class schema_parser
    {
    public:

        virtual void parse(const Json& sch) = 0;

        virtual void parse(const Json& sch, const std::string& retrieval_uri) = 0;
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_HPP

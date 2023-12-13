// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMMON_SCHEMA_PARSER_HPP
#define JSONCONS_JSONSCHEMA_COMMON_SCHEMA_PARSER_HPP

#include <memory>
#include <jsoncons_ext/jsonschema/json_schema.hpp>

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    class schema_parser
    {
    public:
        virtual ~schema_parser() = default;

        virtual void parse(const Json& sch) = 0;

        virtual void parse(const Json& sch, const std::string& retrieval_uri) = 0;

        virtual std::shared_ptr<json_schema<Json>> get_schema() = 0;
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_COMMON_SCHEMA_HPP

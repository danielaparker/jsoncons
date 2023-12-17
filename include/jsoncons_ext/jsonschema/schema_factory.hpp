// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_SCHEMA_FACTORY_HPP
#define JSONCONS_JSONSCHEMA_SCHEMA_FACTORY_HPP

#include <jsoncons_ext/jsonschema/draft7/schema_parser_impl.hpp>
#include <jsoncons_ext/jsonschema/draft201909/schema_parser_impl.hpp>

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& schema, const std::string& retrieval_uri, schema_parser<Json>& parser)
    {
        parser.parse(schema, retrieval_uri);
        return parser.get_schema();
    }

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& schema, const std::string& retrieval_uri, const URIResolver& resolver)
    {
        jsoncons::jsonschema::draft7::schema_parser_impl<Json> parser{ resolver };
        return make_schema(schema, retrieval_uri, parser);
    }

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& schema, const URIResolver& resolver)
    {
        jsoncons::jsonschema::draft7::schema_parser_impl<Json> parser{ resolver };
        return make_schema(schema, "#", parser);
    }

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& schema, const std::string& retrieval_uri)
    {
        jsoncons::jsonschema::draft7::schema_parser_impl<Json> parser{};
        return make_schema(schema, retrieval_uri, parser);
    }

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& schema)
    {
        jsoncons::jsonschema::draft7::schema_parser_impl<Json> parser{};
        return make_schema(schema, "#", parser);
    }

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_HPP

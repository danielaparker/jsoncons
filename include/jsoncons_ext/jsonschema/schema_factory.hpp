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

    template <class Json,class... Args>
    std::unique_ptr<schema_parser<Json>> make_schema_parser(const Json& schema, Args&&... args)
    {
        std::unique_ptr<schema_parser<Json>> parser_ptr;

        if (schema.is_object())
        {
            auto it = schema.find("$schema");
            if (it != schema.object_range().end())
            { 
                if (it->value() == "https://json-schema.org/draft/2019-09/schema")
                {
                    parser_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft201909::schema_parser_impl<Json>>(std::forward<Args>(args)...);
                }
                else
                {
                    parser_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_parser_impl<Json>>(std::forward<Args>(args)...);
                }
            }
            else
            {
                parser_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_parser_impl<Json>>(std::forward<Args>(args)...);
            }
        }
        else
        {
            parser_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_parser_impl<Json>>(std::forward<Args>(args)...);
        }
        return parser_ptr;
    }

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& schema, const std::string& retrieval_uri, const URIResolver& resolver)
    {
        auto parser_ptr = make_schema_parser(schema, resolver);
        parser_ptr->parse(schema, retrieval_uri);
        return parser_ptr->get_schema();
    }

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& schema, const URIResolver& resolver)
    {
        auto parser_ptr = make_schema_parser(schema, resolver);
        parser_ptr->parse(schema, "#");
        return parser_ptr->get_schema();
    }

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& schema, const std::string& retrieval_uri)
    {
        auto parser_ptr = make_schema_parser(schema);
        parser_ptr->parse(schema, retrieval_uri);
        return parser_ptr->get_schema();
    }

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& schema)
    {
        auto parser_ptr = make_schema_parser(schema);
        parser_ptr->parse(schema, "#");
        return parser_ptr->get_schema();
    }

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_HPP

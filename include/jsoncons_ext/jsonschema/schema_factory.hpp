// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_SCHEMA_FACTORY_HPP
#define JSONCONS_JSONSCHEMA_SCHEMA_FACTORY_HPP

#include <jsoncons_ext/jsonschema/common/schema_builder_data.hpp>
#include <jsoncons_ext/jsonschema/draft7/schema_builder_impl.hpp>
#include <jsoncons_ext/jsonschema/draft201909/schema_builder_impl.hpp>

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    struct default_uri_resolver
    {
        Json operator()(const jsoncons::uri& uri)
        {
            if (uri.string() == "https://json-schema.org/draft/2019-09/schema") 
            {
                return jsoncons::jsonschema::draft201909::schema_draft201909<Json>::get_schema();
            }
            else if (uri.string() == "http://json-schema.org/draft-07/schema#") 
            {
                return jsoncons::jsonschema::draft7::schema_draft7<Json>::get_schema();
            }

            JSONCONS_THROW(jsonschema::schema_error("Don't know how to load JSON Schema " + uri.string()));
        }
    };

    template <class Json,class... Args>
    std::unique_ptr<schema_builder<Json>> make_schema_builder(const Json& sch, Args&&... args)
    {
        std::unique_ptr<schema_builder<Json>> parser_ptr;

        if (sch.is_object())
        {
            auto it = sch.find("$schema");
            if (it != sch.object_range().end())
            { 
                if (it->value() == "https://json-schema.org/draft/2019-09/schema")
                {
                    parser_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft201909::schema_builder_impl<Json>>(std::forward<Args>(args)...);
                }
                else if (it->value() == "http://json-schema.org/draft-07/schema#")
                {
                    parser_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_builder_impl<Json>>(std::forward<Args>(args)...);
                }
                else
                {
                    std::string message("Unsupported schema version ");
                    message.append(it->value().template as<std::string>());
                    JSONCONS_THROW(schema_error(message));
                }
            }
            else 
            {
                parser_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_builder_impl<Json>>(std::forward<Args>(args)...);
            }
        }
        else
        {
            parser_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_builder_impl<Json>>(std::forward<Args>(args)...);
        }
        return parser_ptr;
    }

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& sch, const std::string& retrieval_uri, const URIResolver& resolver)
    {
        auto data_ptr = jsoncons::make_unique<schema_builder_data<Json>>();
        auto parser_ptr = make_schema_builder(sch, data_ptr.get(), resolver);
        parser_ptr->parse(sch, retrieval_uri);
        return parser_ptr->get_schema();
    }

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& sch, const std::string& retrieval_uri)
    {
        auto data_ptr = jsoncons::make_unique<schema_builder_data<Json>>();
        auto parser_ptr = make_schema_builder(sch, data_ptr.get(), default_uri_resolver<Json>{});
        parser_ptr->parse(sch, retrieval_uri);
        return parser_ptr->get_schema();
    }

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& sch, const URIResolver& resolver)
    {
        auto data_ptr = jsoncons::make_unique<schema_builder_data<Json>>();
        auto parser_ptr = make_schema_builder(sch, data_ptr.get(), resolver);
        parser_ptr->parse(sch, "#");
        return parser_ptr->get_schema();
    }

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& sch)
    {
        auto data_ptr = jsoncons::make_unique<schema_builder_data<Json>>();
        auto parser_ptr = make_schema_builder(sch, data_ptr.get(), default_uri_resolver<Json>{});
        parser_ptr->parse(sch, "#");
        return parser_ptr->get_schema();
    }

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_HPP

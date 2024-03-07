// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_SCHEMA_FACTORY_HPP
#define JSONCONS_JSONSCHEMA_SCHEMA_FACTORY_HPP

#include <jsoncons_ext/jsonschema/draft7/schema_builder_7.hpp>
#include <jsoncons_ext/jsonschema/draft201909/schema_builder_201909.hpp>
#include <jsoncons_ext/jsonschema/draft202012/schema_builder_202012.hpp>

namespace jsoncons {
namespace jsonschema {

    template <class Json, class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver, Json, std::string>::value, std::unique_ptr<schema_builder<Json>>>::type
    make_schema_validator_factory(const Json& sch,
        const URIResolver& resolver)
    {
        std::unique_ptr<schema_builder<Json>> validator_factory_ptr;

        if (sch.is_object())
        {
            auto it = sch.find("$schema");
            if (it != sch.object_range().end())
            { 
                if (it->value() == "https://json-schema.org/draft/2020-12/schema")
                {
                    validator_factory_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft202012::schema_builder_202012<Json>>(resolver);
                }
                else if (it->value() == "https://json-schema.org/draft/2019-09/schema")
                {
                    validator_factory_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft201909::schema_builder_201909<Json>>(resolver);
                }
                else if (it->value() == "http://json-schema.org/draft-07/schema#")
                {
                    validator_factory_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_builder_7<Json>>(resolver);
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
                validator_factory_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_builder_7<Json>>(resolver);
            }
        }
        else if (sch.is_bool())
        {
            validator_factory_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_builder_7<Json>>(resolver);
        }
        else
        {
             JSONCONS_THROW(schema_error("Schema must be object or boolean"));
        }
        return validator_factory_ptr;
    }  

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

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& sch, const std::string& retrieval_uri, const URIResolver& resolver)
    {
        auto schema_builder = make_schema_validator_factory(sch, resolver);

        schema_builder->build_schema(sch, retrieval_uri);
        return schema_builder->get_schema();
    }

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& sch, const std::string& retrieval_uri)
    {
        auto schema_builder = make_schema_validator_factory(sch, default_uri_resolver<Json>{});

        schema_builder->build_schema(sch, retrieval_uri);
        return schema_builder->get_schema();
    }

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& sch, const URIResolver& resolver)
    {
        auto schema_builder = make_schema_validator_factory(sch, resolver);

        schema_builder->build_schema(sch);
        return schema_builder->get_schema();
    }

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& sch)
    {
        auto schema_builder = make_schema_validator_factory(sch, default_uri_resolver<Json>{});

        schema_builder->build_schema(sch);
        return schema_builder->get_schema();
    }

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_HPP

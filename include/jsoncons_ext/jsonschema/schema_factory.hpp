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

    template <class Json>
    struct default_uri_resolver
    {
        Json operator()(const jsoncons::uri& uri)
        {
            if (uri.string() == "https://json-schema.org/draft/2020-12/schema")
            {
                return jsoncons::jsonschema::draft202012::schema_draft202012<Json>::get_schema();
            }
            else if (uri.string() == "https://json-schema.org/draft/2019-09/schema")
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

    template <class Json>
    struct schema_builder_factory
    {
        std::unique_ptr<schema_builder<Json>> operator()(const Json& sch, const uri_resolver<Json>& resolver)
        {
            std::unique_ptr<schema_builder<Json>> builder_factory;

            if (sch.is_object())
            {
                auto it = sch.find("$schema");
                if (it != sch.object_range().end())
                { 
                    if (it->value() == "https://json-schema.org/draft/2020-12/schema")
                    {
                        builder_factory = jsoncons::make_unique<jsoncons::jsonschema::draft202012::schema_builder_202012<Json>>(*this, resolver);
                    }
                    else if (it->value() == "https://json-schema.org/draft/2019-09/schema")
                    {
                        builder_factory = jsoncons::make_unique<jsoncons::jsonschema::draft201909::schema_builder_201909<Json>>(*this, resolver);
                    }
                    else if (it->value() == "http://json-schema.org/draft-07/schema#")
                    {
                        builder_factory = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_builder_7<Json>>(*this, resolver);
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
                    builder_factory = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_builder_7<Json>>(*this, resolver);
                }
            }
            else if (sch.is_bool())
            {
                builder_factory = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_builder_7<Json>>(*this, resolver);
            }
            else
            {
                 JSONCONS_THROW(schema_error("Schema must be object or boolean"));
            }
            return builder_factory;
        }
    };

    template <class Json>
    std::unique_ptr<document_schema_validator<Json>> make_document_schema_validator(const compilation_context& context,
        const Json& sch, jsoncons::span<const std::string> keys, const uri_resolver<Json>& resolver)
    {
        schema_builder_factory<Json> builder_factory;
        auto schema_builder = builder_factory(sch, resolver);
        std::unordered_map<std::string,uri_wrapper> anchor_dict;
        schema_builder->make_schema_validator(context, sch, keys, anchor_dict);
        return schema_builder->get_schema();
    }

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& sch, const std::string& retrieval_uri, const URIResolver& resolver)
    {
        schema_builder_factory<Json> builder_factory;
        auto schema_builder = builder_factory(sch, resolver);

        schema_builder->build_schema(sch, retrieval_uri);
        return std::make_shared<json_schema<Json>>(schema_builder->get_schema());
    }

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& sch, const std::string& retrieval_uri)
    {
        schema_builder_factory<Json> builder_factory;
        auto schema_builder = builder_factory(sch, default_uri_resolver<Json>{});

        schema_builder->build_schema(sch, retrieval_uri);
        return std::make_shared<json_schema<Json>>(schema_builder->get_schema());
    }

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& sch, const URIResolver& resolver)
    {
        schema_builder_factory<Json> builder_factory;
        auto schema_builder = builder_factory(sch, uri_resolver<Json>{ resolver });

        schema_builder->build_schema(sch);
        return std::make_shared<json_schema<Json>>(schema_builder->get_schema());
    }

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& sch)
    {
        schema_builder_factory<Json> builder_factory;
        auto schema_builder = builder_factory(sch, default_uri_resolver<Json>{});

        schema_builder->build_schema(sch);
        return std::make_shared<json_schema<Json>>(schema_builder->get_schema());
    }

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_HPP

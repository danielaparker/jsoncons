// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_SCHEMA_FACTORY_HPP
#define JSONCONS_JSONSCHEMA_SCHEMA_FACTORY_HPP

#include <jsoncons_ext/jsonschema/common/schema_builder.hpp>
#include <jsoncons_ext/jsonschema/draft7/schema_validator_factory_impl.hpp>
#include <jsoncons_ext/jsonschema/draft201909/schema_validator_factory_impl.hpp>

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    class schema_builder_impl : public schema_builder<Json>
    {
    public:
        schema_builder_impl(const uri_resolver<Json>& resolver)
            : schema_builder<Json>(resolver)
        {
        }

        std::unique_ptr<schema_validator_factory<Json>> make_schema_validator_factory(const Json& sch)
        {
            std::unique_ptr<schema_validator_factory<Json>> validator_factory_ptr;

            if (sch.is_object())
            {
                auto it = sch.find("$schema");
                if (it != sch.object_range().end())
                { 
                    if (it->value() == "https://json-schema.org/draft/2019-09/schema")
                    {
                        validator_factory_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft201909::schema_validator_factory_impl<Json>>(this);
                    }
                    else if (it->value() == "http://json-schema.org/draft-07/schema#")
                    {
                        validator_factory_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_validator_factory_impl<Json>>(this);
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
                    validator_factory_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_validator_factory_impl<Json>>(this);
                }
            }
            else if (sch.is_bool())
            {
                validator_factory_ptr = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_validator_factory_impl<Json>>(this);
            }
            else
            {
                 JSONCONS_THROW(schema_error("Schema must be object or boolean"));
            }
            return validator_factory_ptr;
        }
    };

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
        schema_builder_impl<Json> schema_builder_factory(resolver);

        schema_builder_factory.build_schema(sch, retrieval_uri);
        return schema_builder_factory.get_schema();
    }

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& sch, const std::string& retrieval_uri)
    {
        schema_builder_impl<Json> schema_builder_factory(default_uri_resolver<Json>{});

        schema_builder_factory.build_schema(sch, retrieval_uri);
        return schema_builder_factory.get_schema();
    }

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& sch, const URIResolver& resolver)
    {
        schema_builder_impl<Json> schema_builder_factory(resolver);

        schema_builder_factory.build_schema(sch, "#");
        return schema_builder_factory.get_schema();
    }

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& sch)
    {
        schema_builder_impl<Json> schema_builder_factory(default_uri_resolver<Json>{});

        schema_builder_factory.build_schema(sch, "#");
        return schema_builder_factory.get_schema();
    }

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_HPP

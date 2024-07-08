// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_JSON_SCHEMA_FACTORY_HPP
#define JSONCONS_JSONSCHEMA_JSON_SCHEMA_FACTORY_HPP

#include <jsoncons_ext/jsonschema/draft4/schema_builder_4.hpp>
#include <jsoncons_ext/jsonschema/draft6/schema_builder_6.hpp>
#include <jsoncons_ext/jsonschema/draft7/schema_builder_7.hpp>
#include <jsoncons_ext/jsonschema/draft201909/schema_builder_201909.hpp>
#include <jsoncons_ext/jsonschema/draft202012/schema_builder_202012.hpp>

namespace jsoncons {
namespace jsonschema {

    template <typename Json>
    class schema_builder_factory
    {
    public:
        using schema_store_type = std::map<jsoncons::uri, schema_validator<Json>*>;

        schema_builder_factory()
        {
        }
            
        std::unique_ptr<schema_builder<Json>> operator()(Json sch, 
            const evaluation_options& options, schema_store_type* schema_store_ptr,
            const std::vector<schema_resolver<Json>>& resolvers,
            const std::unordered_map<std::string,bool>& vocabulary) const
        {
            std::unique_ptr<schema_builder<Json>> builder;

            if (sch.is_object())
            {
                auto it = sch.find("$schema");
                if (it != sch.object_range().end())
                { 
                    builder = get_builder(std::move(sch), it->value().as_string_view(), options, schema_store_ptr, resolvers, vocabulary);
                    if (!builder)
                    {
                        std::string message("Unsupported schema version ");
                        message.append(it->value().template as<std::string>());
                        JSONCONS_THROW(schema_error(message));
                    }
                }
                else 
                {
                    builder = get_default_schema_builder(std::move(sch), options, schema_store_ptr, resolvers, vocabulary);
                }
            }
            else if (sch.is_bool())
            {
                builder = get_default_schema_builder(std::move(sch), options, schema_store_ptr, resolvers, vocabulary);
            }
            else
            {
                 JSONCONS_THROW(schema_error("Schema must be object or boolean"));
            }
            return builder;
        }

        std::unique_ptr<schema_builder<Json>> get_default_schema_builder(Json&& sch,
            const evaluation_options& options, 
            schema_store_type* schema_store_ptr,
            const std::vector<schema_resolver<Json>>& resolvers,
            const std::unordered_map<std::string,bool>& vocabulary) const
        {
            if (options.default_version() == schema_version::draft202012())
            {
                return jsoncons::make_unique<jsoncons::jsonschema::draft202012::schema_builder_202012<Json>>(std::move(sch), *this, 
                    options, schema_store_ptr, resolvers, vocabulary);
            }
            else if (options.default_version() == schema_version::draft201909())
            {
                return jsoncons::make_unique<jsoncons::jsonschema::draft201909::schema_builder_201909<Json>>(std::move(sch), *this, 
                    options, schema_store_ptr, resolvers, vocabulary);
            }
            else if (options.default_version() == schema_version::draft7())
            {
                return jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_builder_7<Json>>(std::move(sch), *this, 
                    options, schema_store_ptr, resolvers);
            }
            else if (options.default_version() == schema_version::draft6())
            {
                return jsoncons::make_unique<jsoncons::jsonschema::draft6::schema_builder_6<Json>>(std::move(sch), *this, 
                    options, schema_store_ptr, resolvers);
            }
            else if (options.default_version() == schema_version::draft4())
            {
                return jsoncons::make_unique<jsoncons::jsonschema::draft4::schema_builder_4<Json>>(std::move(sch), *this, 
                    options, schema_store_ptr, resolvers);
            }
            else
            {
                JSONCONS_THROW(schema_error("Unsupported schema version " + options.default_version()));
            }
        }
        
        std::unique_ptr<schema_builder<Json>> get_builder(Json&& sch, const jsoncons::string_view& schema_id,
            const evaluation_options& options, schema_store_type* schema_store_ptr,
            const std::vector<schema_resolver<Json>>& resolvers,
            const std::unordered_map<std::string,bool>& vocabulary) const
        {
            std::unique_ptr<schema_builder<Json>> builder;

            if (schema_id == schema_version::draft202012())
            {
                builder = jsoncons::make_unique<jsoncons::jsonschema::draft202012::schema_builder_202012<Json>>(std::move(sch), *this, 
                    options, schema_store_ptr, resolvers, vocabulary);
            }
            else if (schema_id == schema_version::draft201909())
            {
                builder = jsoncons::make_unique<jsoncons::jsonschema::draft201909::schema_builder_201909<Json>>(std::move(sch), *this, 
                    options, schema_store_ptr, resolvers, vocabulary);
            }
            else if (schema_id == schema_version::draft7())
            {
                builder = jsoncons::make_unique<jsoncons::jsonschema::draft7::schema_builder_7<Json>>(std::move(sch), *this, 
                    options, schema_store_ptr, resolvers);
            }
            else if (schema_id == schema_version::draft6())
            {
                builder = jsoncons::make_unique<jsoncons::jsonschema::draft6::schema_builder_6<Json>>(std::move(sch), *this, 
                    options, schema_store_ptr, resolvers);
            }
            else if (schema_id == schema_version::draft4())
            {
                builder = jsoncons::make_unique<jsoncons::jsonschema::draft4::schema_builder_4<Json>>(std::move(sch), *this, 
                    options, schema_store_ptr, resolvers);
            }
            else
            { 
                builder = get_builder_from_meta_schema(std::move(sch), schema_id, options, schema_store_ptr, resolvers);
            }
            return builder;
        }
        
        std::unique_ptr<schema_builder<Json>> get_builder_from_meta_schema(Json&& sch, const jsoncons::string_view& schema_id,
            const evaluation_options& options, schema_store_type* schema_store_ptr,
            const std::vector<schema_resolver<Json>>& resolvers) const
        {
            std::unique_ptr<schema_builder<Json>> builder;

            bool found = false;
            jsoncons::uri uri{ std::string(schema_id) };
            for (auto it = resolvers.begin(); it != resolvers.end() && !found; ++it)
            {
                Json meta_sch = (*it)(uri.base());
                if (meta_sch.is_object())
                {
                    std::unordered_map<std::string,bool> vocabulary;
                    auto vocab_it = meta_sch.find("$vocabulary");
                    if (vocab_it != meta_sch.object_range().end())
                    {
                        const auto& vocab = vocab_it->value();
                        if (vocab.is_object())
                        {
                            for (const auto& member : vocab.object_range())
                            {
                                vocabulary.emplace(member.key(), member.value().as_bool());
                            }
                        }
                    }
                    auto schema_it = meta_sch.find("$schema");
                    if (schema_it != meta_sch.object_range().end())
                    {
                        builder = get_builder(std::move(sch), schema_it->value().as_string_view(), options, schema_store_ptr, resolvers, vocabulary);
                        found = true;
                    }
                }
            }
            
            return builder;
        }
    };

    template <typename Json>
    Json meta_resolver(const jsoncons::uri& uri)
    {
        if (uri.base() == schema_version::draft202012())
        {
            return jsoncons::jsonschema::draft202012::schema_draft202012<Json>::get_schema();
        }
        else if (uri.base() == schema_version::draft201909())
        {
            return jsoncons::jsonschema::draft201909::schema_draft201909<Json>::get_schema();
        }
        else if (uri.base() == schema_version::draft7())
        {
            return jsoncons::jsonschema::draft7::schema_draft7<Json>::get_schema();
        }
        else if (uri.base() == schema_version::draft6())
        {
            return jsoncons::jsonschema::draft6::schema_draft6<Json>::get_schema();
        }
        else if (uri.base() == schema_version::draft4())
        {
            return jsoncons::jsonschema::draft4::schema_draft4<Json>::get_schema();
        }
        else
        {
            return Json::null();
        }
    }

    template <typename Json,typename URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,json_schema<Json>>::type
    make_json_schema(Json sch, const std::string& retrieval_uri, const URIResolver& resolver, 
        evaluation_options options = evaluation_options{})
    {
        using schema_store_type = std::map<jsoncons::uri, schema_validator<Json>*>;
        schema_store_type schema_store; 
        schema_builder_factory<Json> builder_factory{};
        
        std::unordered_map<std::string,bool> vocabulary{};
        std::vector<schema_resolver<Json>> resolvers = {{meta_resolver<Json>, resolver}};
        auto schema_builder = builder_factory(std::move(sch), options, &schema_store, resolvers, vocabulary);

        schema_builder->build_schema(retrieval_uri);
        return json_schema<Json>(schema_builder->get_schema_validator());
    }

    template <typename Json>
    json_schema<Json> make_json_schema(Json sch, const std::string& retrieval_uri, 
        evaluation_options options = evaluation_options{})
    {
        using schema_store_type = std::map<jsoncons::uri, schema_validator<Json>*>;
        schema_store_type schema_store; 
        schema_builder_factory<Json> builder_factory{};

        std::unordered_map<std::string,bool> vocabulary{};
        std::vector<schema_resolver<Json>> resolvers = {{meta_resolver<Json>}};
        auto schema_builder = builder_factory(std::move(sch), options, &schema_store, resolvers, vocabulary);

        schema_builder->build_schema(retrieval_uri);
        return json_schema<Json>(schema_builder->get_schema_validator());
    }

    template <typename Json,typename URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,json_schema<Json>>::type
    make_json_schema(Json sch, const URIResolver& resolver, 
        evaluation_options options = evaluation_options{})
    {
        using schema_store_type = std::map<jsoncons::uri, schema_validator<Json>*>;
        schema_store_type schema_store; 
        schema_builder_factory<Json> builder_factory{};

        std::unordered_map<std::string,bool> vocabulary{};
        std::vector<schema_resolver<Json>> resolvers = {{meta_resolver<Json>, resolver}};
        auto schema_builder = builder_factory(std::move(sch), options, &schema_store, resolvers, vocabulary);

        schema_builder->build_schema();
        return json_schema<Json>(schema_builder->get_schema_validator());
    }

    template <typename Json>
    json_schema<Json> make_json_schema(Json sch, 
        evaluation_options options = evaluation_options{})
    {
        using schema_store_type = std::map<jsoncons::uri, schema_validator<Json>*>;
        schema_store_type schema_store; 
        schema_builder_factory<Json> builder_factory{};

        std::unordered_map<std::string,bool> vocabulary{};
        std::vector<schema_resolver<Json>> resolvers = {{meta_resolver<Json>}};
        auto schema_builder = builder_factory(std::move(sch), options, &schema_store, resolvers, vocabulary);

        schema_builder->build_schema();
        return json_schema<Json>(schema_builder->get_schema_validator());
    }

#if !defined(JSONCONS_NO_DEPRECATED)
    
    // Legacy
    template <typename Json,typename URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(Json sch, const std::string& retrieval_uri, const URIResolver& resolver)
    {
        using schema_store_type = std::map<jsoncons::uri, schema_validator<Json>*>;
        schema_store_type schema_store; 
        schema_builder_factory<Json> builder_factory{};

        std::unordered_map<std::string,bool> vocabulary{};
        std::vector<schema_resolver<Json>> resolvers = {{meta_resolver<Json>, resolver}};
        auto schema_builder = builder_factory(std::move(sch), 
            jsonschema::evaluation_options{}.default_version(jsonschema::schema_version::draft7()), 
            &schema_store, resolvers, vocabulary);

        schema_builder->build_schema(retrieval_uri);
        return std::make_shared<json_schema<Json>>(schema_builder->get_schema_validator());
    }

    template <typename Json>
    std::shared_ptr<json_schema<Json>> make_schema(Json sch, const std::string& retrieval_uri)
    {
        using schema_store_type = std::map<jsoncons::uri, schema_validator<Json>*>;
        schema_store_type schema_store; 
        schema_builder_factory<Json> builder_factory{};

        std::unordered_map<std::string,bool> vocabulary{};
        std::vector<schema_resolver<Json>> resolvers = {{meta_resolver<Json>}};
        auto schema_builder = builder_factory(std::move(sch), 
            jsonschema::evaluation_options{}.default_version(jsonschema::schema_version::draft7()), 
            &schema_store, resolvers, vocabulary);

        schema_builder->build_schema(retrieval_uri);
        return std::make_shared<json_schema<Json>>(schema_builder->get_schema_validator());
    }

    template <typename Json,typename URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(Json sch, const URIResolver& resolver)
    {
        using schema_store_type = std::map<jsoncons::uri, schema_validator<Json>*>;
        schema_store_type schema_store; 
        schema_builder_factory<Json> builder_factory{};

        std::unordered_map<std::string,bool> vocabulary{};
        std::vector<schema_resolver<Json>> resolvers = {{meta_resolver<Json>, resolver}};
        auto schema_builder = builder_factory(std::move(sch), 
            jsonschema::evaluation_options{}.default_version(jsonschema::schema_version::draft7()), 
            &schema_store, resolvers, vocabulary);

        schema_builder->build_schema();
        return std::make_shared<json_schema<Json>>(schema_builder->get_schema_validator());
    }

    template <typename Json>
    std::shared_ptr<json_schema<Json>> make_schema(Json sch)
    {
        using schema_store_type = std::map<jsoncons::uri, schema_validator<Json>*>;
        schema_store_type schema_store; 
        schema_builder_factory<Json> builder_factory{};

        std::unordered_map<std::string,bool> vocabulary{};
        std::vector<schema_resolver<Json>> resolvers = {{meta_resolver<Json>}};
        auto schema_builder = builder_factory(std::move(sch), 
            jsonschema::evaluation_options{}.default_version(jsonschema::schema_version::draft7()), 
            &schema_store, resolvers, vocabulary);

        schema_builder->build_schema();
        return std::make_shared<json_schema<Json>>(schema_builder->get_schema_validator());
    }
#endif    

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_HPP

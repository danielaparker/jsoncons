// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_SCHEMA_BUILDER_DATA_HPP
#define JSONCONS_JSONSCHEMA_SCHEMA_BUILDER_DATA_HPP

#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/common/schema_identifier.hpp>
#include <jsoncons_ext/jsonschema/common/schema_validator.hpp>
#include <jsoncons_ext/jsonschema/common/schema_builder.hpp>
#include <cassert>
#include <set>
#include <sstream>
#include <iostream>
#include <cassert>

namespace jsoncons {
namespace jsonschema {
    
    template <class Json>
    struct subschema_registry
    {
        using schema_validator_pointer = schema_validator<Json>*;
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;
        using ref_validator_type = ref_validator<Json>;

        std::map<std::string, schema_validator_pointer> schemas; // schemas
        std::vector<std::pair<std::string, ref_validator_type*>> unresolved; // unresolved references
        std::map<std::string, Json> unknown_keywords;
    };

    template <class Json>
    class schema_builder_data 
    {
    public:
        using keyword_validator_type = typename std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_pointer = schema_validator<Json>*;
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;
        using ref_validator_type = ref_validator<Json>;
        using recursive_ref_validator_type = recursive_ref_validator<Json>;

        uri_resolver<Json> resolver_;
        schema_validator_type root_;

        // Owns all subschemas
        std::vector<schema_validator_type> subschemas_;

        // Map location to subschema_registry
        std::map<std::string, subschema_registry<Json>> subschema_registries_;

    public:
        schema_builder_data(const uri_resolver<Json>& resolver) noexcept
            : resolver_(resolver)
        {
        }

        schema_builder_data(const schema_builder_data&) = delete;
        schema_builder_data& operator=(const schema_builder_data&) = delete;
        schema_builder_data(schema_builder_data&&) = default;
        schema_builder_data& operator=(schema_builder_data&&) = default;

        virtual std::unique_ptr<schema_builder<Json>> make_schema_builder(const Json& sch) = 0;

        void parse(const Json& sch) 
        {
            parse(sch, "#");
        }

        void parse(const Json& sch, const std::string& retrieval_uri) 
        {
            auto parser_ptr = make_schema_builder(sch);
            root_ = parser_ptr->make_schema_validator(compilation_context(schema_identifier(retrieval_uri)), sch, {});
        }

        std::shared_ptr<json_schema<Json>> get_schema()
        {                        
            // load all external schemas that have not already been loaded
            std::size_t loaded_count = 0;
            do
            {
                loaded_count = 0;

                std::vector<std::string> locations;
                for (const auto& item : subschema_registries_)
                    locations.push_back(item.first);

                for (const auto& loc : locations)
                {
                    if (subschema_registries_[loc].schemas.empty()) // registry for this file is empty
                    {
                        if (resolver_)
                        {
                            Json external_sch = resolver_(loc);
                            auto builder = make_schema_builder(external_sch);
                            subschemas_.emplace_back(builder->make_schema_validator(compilation_context(schema_identifier(loc)), external_sch, {}));
                            ++loaded_count;
                        }
                        else
                        {
                            JSONCONS_THROW(schema_error("External schema reference '" + loc + "' needs to be loaded, but no resolver provided"));
                        }
                    }
                }
            } while (loaded_count > 0);

            resolve_references();

            return std::make_shared<json_schema<Json>>(std::move(subschemas_), std::move(root_));
        }

        void insert_schema(const schema_identifier& uri, schema_validator<Json>* s)
        {
            auto& file = get_or_create_file(uri.base().string());
            auto schemas_it = file.schemas.find(std::string(uri.fragment()));
            if (schemas_it != file.schemas.end()) 
            {
                //JSONCONS_THROW(schema_error("schema with " + uri.string() + " already inserted"));
                return;
            }

            file.schemas.insert({std::string(uri.fragment()), s});
        }

        void resolve_references()
        {
            for (auto& doc : subschema_registries_)
            {
                for (auto& ref : doc.second.unresolved)
                {
                    auto it = doc.second.schemas.find(ref.first);
                    if (it == doc.second.schemas.end())
                    {
                        JSONCONS_THROW(schema_error(doc.first + " has undefined reference " + ref.first + "."));
                    }
                    ref.second->set_referred_schema(it->second);
                }
            }
        }

        subschema_registry<Json>& get_or_create_file(const std::string& loc)
        {
            auto file = subschema_registries_.find(loc);
            if (file != subschema_registries_.end())
            {
                return file->second;
            }
            else
            {
                return subschema_registries_.insert(file, {loc, {}})->second;
            }
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_DRAFT7_KEYWORD_FACTORY_HPP

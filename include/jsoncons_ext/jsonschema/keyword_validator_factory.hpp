// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_KEYWORD_VALIDATOR_FACTORY_HPP
#define JSONCONS_JSONSCHEMA_KEYWORD_VALIDATOR_FACTORY_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/subschema.hpp>
#include <jsoncons_ext/jsonschema/keyword_validator.hpp>
#include <jsoncons_ext/jsonschema/schema_draft7.hpp>
#include <jsoncons_ext/jsonschema/schema_version.hpp>
#include <cassert>
#include <set>
#include <sstream>
#include <iostream>
#include <cassert>
#if defined(JSONCONS_HAS_STD_REGEX)
#include <regex>
#endif

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    using uri_resolver = std::function<Json(const jsoncons::uri & /*id*/)>;

    template <class Json>
    class reference_schema : public keyword_validator<Json>
    {
        using validator_type = typename std::unique_ptr<keyword_validator<Json>>;
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        validator_pointer referred_schema_;

    public:
        reference_schema(const std::string& id)
            : keyword_validator<Json>(id), referred_schema_(nullptr) {}

        void set_referred_schema(validator_pointer target) { referred_schema_ = target; }

    private:

        void do_validate(const Json& instance, 
                         const jsonpointer::json_pointer& instance_location, 
                         error_reporter& reporter, 
                         Json& patch) const override
        {
            if (!referred_schema_)
            {
                reporter.error(validation_output("", 
                                                 this->schema_path(), 
                                                 instance_location.to_uri_fragment(), 
                                                 "Unresolved schema reference " + this->schema_path()));
                return;
            }

            referred_schema_->validate(instance, instance_location, reporter, patch);
        }

        jsoncons::optional<Json> get_default_value(const jsonpointer::json_pointer& instance_location, 
                                                   const Json& instance, 
                                                   error_reporter& reporter) const override
        {
            if (!referred_schema_)
            {
                reporter.error(validation_output("", 
                                                 this->schema_path(), 
                                                 instance_location.to_uri_fragment(), 
                                                 "Unresolved schema reference " + this->schema_path()));
                return jsoncons::optional<Json>();
            }

            return referred_schema_->get_default_value(instance_location, instance, reporter);
        }
    };

    template <class Json>
    class keyword_validator_factory;

    template <class Json>
    class json_schema
    {
        using validator_type = typename keyword_validator<Json>::validator_type;
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        friend class keyword_validator_factory<Json>;

        std::vector<validator_type> subschemas_;
        validator_type root_;
    public:
        json_schema(std::vector<validator_type>&& subschemas, validator_type&& root)
            : subschemas_(std::move(subschemas)), root_(std::move(root))
        {
            if (root_ == nullptr)
                JSONCONS_THROW(schema_error("There is no root schema to validate an instance against"));
        }

        json_schema(const json_schema&) = delete;
        json_schema(json_schema&&) = default;
        json_schema& operator=(const json_schema&) = delete;
        json_schema& operator=(json_schema&&) = default;

        void validate(const Json& instance, 
                      const jsonpointer::json_pointer& instance_location, 
                      error_reporter& reporter, 
                      Json& patch) const 
        {
            JSONCONS_ASSERT(root_ != nullptr);
            root_->validate(instance, instance_location, reporter, patch);
        }
    };

    template <class Json>
    struct default_uri_resolver
    {
        Json operator()(const jsoncons::uri& uri)
        {
            if (uri.path() == "/draft-07/schema") 
            {
                return jsoncons::jsonschema::schema_draft7<Json>::get_schema();
            }

            JSONCONS_THROW(jsonschema::schema_error("Don't know how to load JSON Schema " + std::string(uri.base())));
        }
    };

    template <class Json>
    class keyword_validator_factory : public subschema_validator_factory<Json>
    {
        using reference_validator_type = reference_validator<Json>;
        using validator_type = typename std::unique_ptr<keyword_validator<Json>>;
        using validator_pointer = typename keyword_validator<Json>::self_pointer;

        struct subschema_registry
        {
            std::map<std::string, validator_pointer> schemas; // schemas
            std::map<std::string, reference_schema<Json>*> unresolved; // unresolved references
            std::map<std::string, Json> unprocessed_keywords;
        };

        uri_resolver<Json> resolver_;
        validator_type root_;

        // Owns all subschemas
        std::vector<std::unique_ptr<keyword_validator<Json>>> subschemas_;

        // Map location to subschema_registry
        std::map<std::string, subschema_registry> subschema_registries_;

    public:
        keyword_validator_factory(uri_resolver<Json>&& resolver) noexcept

            : resolver_(std::move(resolver))
        {
        }

        keyword_validator_factory(const keyword_validator_factory&) = delete;
        keyword_validator_factory& operator=(const keyword_validator_factory&) = delete;
        keyword_validator_factory(keyword_validator_factory&&) = default;
        keyword_validator_factory& operator=(keyword_validator_factory&&) = default;

        std::shared_ptr<json_schema<Json>> get_schema()
        {
            return std::make_shared<json_schema<Json>>(std::move(subschemas_), std::move(root_));
        }

        validator_type make_subschema_validator(const Json& schema,
            const compilation_context& context,
            const std::vector<std::string>& keys) override
        {
            auto new_context = context.update_uris(schema, keys);

            validator_pointer sch = nullptr;

            switch (schema.type())
            {
                case json_type::bool_value:
                    if (schema.template as<bool>())
                    {
                        auto ref =  true_validator<Json>::compile(new_context);
                        sch = ref.get();
                        subschemas_.emplace_back(std::move(ref));
                    }
                    else
                    {
                        auto ref = false_validator<Json>::compile(new_context);
                        sch = ref.get();
                        subschemas_.emplace_back(std::move(ref));
                    }
                    break;
                case json_type::object_value:
                {
                    auto it = schema.find("definitions");
                    if (it != schema.object_range().end()) 
                    {
                        for (const auto& def : it->value().object_range())
                            make_subschema_validator(def.value(), new_context, {"definitions", def.key()});
                    }

                    it = schema.find("$ref");
                    if (it != schema.object_range().end()) // this schema is a reference
                    { 
                        schema_location relative(it->value().template as<std::string>()); 

                        auto id = new_context.resolve_back(relative);
                        //sch = get_or_create_reference(id);
                        auto ref =  get_or_create_reference(id);
                        sch = ref.get();
                        subschemas_.emplace_back(std::move(ref));
                    } 
                    else 
                    {
                        auto ref = jsoncons::make_unique<type_validator<Json>>(this, schema, new_context);
                        sch = ref.get();
                        subschemas_.emplace_back(std::move(ref));
                    }
                    break;
                }
                default:
                    JSONCONS_THROW(schema_error("invalid JSON-type for a schema for " + new_context.uris()[0].string() + ", expected: boolean or object"));
                    break;
            }

            for (const auto& uri : new_context.uris()) 
            { 
                insert_schema(uri, sch);

                if (schema.type() == json_type::object_value)
                {
                    for (const auto& item : schema.object_range())
                        insert_unknown_keyword(uri, item.key(), item.value()); // save unknown keywords for later reference
                }
            }
            return jsoncons::make_unique<reference_validator_type>(sch);
        }

        void load_root(const Json& sch)
        {
            if (sch.is_object())
            {
                auto it = sch.find("$schema");
                if (it != sch.object_range().end())
                {
                    auto sv = it->value().as_string_view();
                    if (!schema_version::contains(sv))
                    {
                        std::string message("Unsupported schema version ");
                        message.append(sv.data(), sv.size());
                        JSONCONS_THROW(schema_error(message));
                    }
                }
            }
            load(sch);
        }

        void load(const Json& sch)
        {
            subschema_registries_.clear();
            root_ = make_subschema_validator(sch, compilation_context(schema_location("#")), {});

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
                            Json external_schema = resolver_(loc);
                            make_subschema_validator(external_schema, compilation_context(schema_location(loc)), {});
                            ++loaded_count;
                        } 
                        else 
                        {
                            JSONCONS_THROW(schema_error("External schema reference '" + loc + "' needs to be loaded, but no resolver provided"));
                        }
                    }
                }
            } 
            while (loaded_count > 0);

            for (const auto &file : subschema_registries_)
            {
                if (!file.second.unresolved.empty())
                {
                    JSONCONS_THROW(schema_error("After all files have been parsed, '" +
                                                (file.first == "" ? "<root>" : file.first) +
                                                "' has still undefined references."));
                }
            }
        }

    private:

        void insert_schema(const schema_location& uri, validator_pointer s)
        {
            auto& file = get_or_create_file(std::string(uri.base()));
            auto schemas_it = file.schemas.find(std::string(uri.fragment()));
            if (schemas_it != file.schemas.end()) 
            {
                JSONCONS_THROW(schema_error("schema with " + uri.string() + " already inserted"));
                return;
            }

            file.schemas.insert({std::string(uri.fragment()), s});

            // is there an unresolved reference to this newly inserted schema?
            auto unresolved_it = file.unresolved.find(std::string(uri.fragment()));
            if (unresolved_it != file.unresolved.end()) 
            {
                unresolved_it->second->set_referred_schema(s);
                file.unresolved.erase(unresolved_it);
            }
        }

        void insert_unknown_keyword(const schema_location& uri, 
                                    const std::string& key, 
                                    const Json& value)
        {
            auto &file = get_or_create_file(std::string(uri.base()));
            auto new_u = uri.append(key);
            schema_location new_uri(new_u);

            if (new_uri.has_fragment() && !new_uri.has_identifier()) 
            {
                auto fragment = std::string(new_uri.fragment());
                // is there a reference looking for this unknown-keyword, which is thus no longer a unknown keyword but a schema
                auto unresolved = file.unresolved.find(fragment);
                if (unresolved != file.unresolved.end())
                    make_subschema_validator(value, compilation_context(new_uri), {});
                else // no, nothing ref'd it, keep for later
                    file.unprocessed_keywords[fragment] = value;

                // recursively add possible subschemas of unknown keywords
                if (value.type() == json_type::object_value)
                    for (const auto& subsch : value.object_range())
                    {
                        insert_unknown_keyword(new_uri, subsch.key(), subsch.value());
                    }
            }
        }

        validator_type get_or_create_reference(const schema_location& uri)
        {
            auto &file = get_or_create_file(std::string(uri.base()));

            // a schema already exists
            auto sch = file.schemas.find(std::string(uri.fragment()));
            if (sch != file.schemas.end())
                return jsoncons::make_unique<reference_validator_type>(sch->second);

            // referencing an unknown keyword, turn it into schema
            //
            // an unknown keyword can only be referenced by a JSONPointer,
            // not by a plain name identifier
            if (uri.has_fragment() && !uri.has_identifier()) 
            {
                std::string fragment = std::string(uri.fragment());
                auto unprocessed_keywords_it = file.unprocessed_keywords.find(fragment);
                if (unprocessed_keywords_it != file.unprocessed_keywords.end()) 
                {
                    auto &subsch = unprocessed_keywords_it->second; 
                    auto s = make_subschema_validator(subsch, compilation_context(uri), {});       //  A JSON Schema MUST be an object or a boolean.
                    file.unprocessed_keywords.erase(unprocessed_keywords_it);
                    return std::move(s);
                }
            }

            // get or create a reference_schema
            auto ref = file.unresolved.find(std::string(uri.fragment()));
            if (ref != file.unresolved.end()) 
            {
                //return ref->second; // unresolved, use existing reference
                return jsoncons::make_unique<reference_validator_type>(ref->second);
            }
            else 
            {
                auto orig = jsoncons::make_unique<reference_schema<Json>>(uri.string());
                auto p = file.unresolved.insert(ref,
                                              {std::string(uri.fragment()), orig.get()})
                    ->second; // unresolved, create new reference
                
                subschemas_.emplace_back(std::move(orig));
                return jsoncons::make_unique<reference_validator_type>(p);
            }
        }

        subschema_registry& get_or_create_file(const std::string& loc)
        {
            auto file = subschema_registries_.find(loc);
            if (file != subschema_registries_.end())
                return file->second;
            else
                return subschema_registries_.insert(file, {loc, {}})->second;
        }

    };

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& schema)
    {
        keyword_validator_factory<Json> loader{default_uri_resolver<Json>()};
        loader.load_root(schema);

        return loader.get_schema();
    }

    template <class Json,class URIResolver>
    typename std::enable_if<traits_extension::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& schema, const URIResolver& resolver)
    {
        keyword_validator_factory<Json> loader(resolver);
        loader.load_root(schema);

        return loader.get_schema();
    }

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_LOADER_HPP

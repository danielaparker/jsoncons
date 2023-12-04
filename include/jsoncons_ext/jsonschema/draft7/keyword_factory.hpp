// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_DRAFT7_KEYWORD_FACTORY_HPP
#define JSONCONS_JSONSCHEMA_DRAFT7_KEYWORD_FACTORY_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/schema.hpp>
#include <jsoncons_ext/jsonschema/keywords.hpp>
#include <jsoncons_ext/jsonschema/draft7/schema_draft7.hpp>
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
namespace draft7 {

    template <class Json>
    struct default_uri_resolver
    {
        Json operator()(const jsoncons::uri& uri)
        {
            if (uri.path() == "/draft-07/schema") 
            {
                return schema_draft7<Json>::get_schema();
            }

            JSONCONS_THROW(jsonschema::schema_error("Don't know how to load JSON Schema " + std::string(uri.base())));
        }
    };

    template <class Json>
    class keyword_factory : public subschema_validator_factory<Json>
    {
    public:
        using reference_validator_type = reference_validator<Json>;
        using validator_type = typename std::unique_ptr<keyword_validator<Json>>;
        using validator_pointer = typename keyword_validator<Json>::self_pointer;
    private:
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
        keyword_factory(uri_resolver<Json>&& resolver) noexcept

            : resolver_(std::move(resolver))
        {
        }

        keyword_factory(const keyword_factory&) = delete;
        keyword_factory& operator=(const keyword_factory&) = delete;
        keyword_factory(keyword_factory&&) = default;
        keyword_factory& operator=(keyword_factory&&) = default;

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

            bool is_ref = false;
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
                    auto it = schema.find("$ref");
                    if (it != schema.object_range().end()) // this schema is a reference
                    {
                        schema_location relative(it->value().template as<std::string>()); 

                        auto id = context.resolve_back(relative);
                        //sch = get_or_create_reference(id);
                        auto ref =  get_or_create_reference(id);
                        sch = ref.get();
                        subschemas_.emplace_back(std::move(ref));
                    } 
                    else 
                    {
                        it = schema.find("definitions");
                        if (it != schema.object_range().end()) 
                        {
                            for (const auto& def : it->value().object_range())
                                make_subschema_validator(def.value(), new_context, {"definitions", def.key()});
                        }
                        auto ref = compile_type_validator(schema, new_context);
                        sch = ref.get();
                        subschemas_.emplace_back(std::move(ref));
                    }
                    break;
                }
                default:
                    JSONCONS_THROW(schema_error("invalid JSON-type for a schema for " + new_context.uris()[0].string() + ", expected: boolean or object"));
                    break;
            }

            if (!is_ref)
            {
                for (const auto& uri : new_context.uris()) 
                { 
                    if (schema.type() == json_type::object_value)
                    {
                            insert_schema(uri, sch);
                            for (const auto& item : schema.object_range())
                                insert_unknown_keyword(uri, item.key(), item.value()); // save unknown keywords for later reference
                    }
                    else
                    {
                        insert_schema(uri, sch);
                    }
                }
            }
            
            return jsoncons::make_unique<reference_validator_type>(sch);
        }

        std::vector<validator_type> make_type_mapping(const std::string& type,
            const Json& schema,
            const compilation_context& context,
            std::set<std::string>& keywords)
        {
            std::vector<validator_type> type_mapping{(uint8_t)(json_type::object_value)+1};

            if (type == "null")
            {
                type_mapping[(uint8_t)json_type::null_value] = null_validator<Json>::compile(context);
            }
            else if (type == "object")
            {
                type_mapping[(uint8_t)json_type::object_value] = jsoncons::make_unique<object_validator<Json>>(this, schema, context);
            }
            else if (type == "array")
            {
                type_mapping[(uint8_t)json_type::array_value] = compile_array_schema(schema, context, this);
            }
            else if (type == "string")
            {
                type_mapping[(uint8_t)json_type::string_value] = compile_string_schema(schema, context);
                // For binary types
                type_mapping[(uint8_t) json_type::byte_string_value] = compile_string_schema(schema, context);
            }
            else if (type == "boolean")
            {
                type_mapping[(uint8_t)json_type::bool_value] = boolean_validator<Json>::compile(context);
            }
            else if (type == "integer")
            {
                type_mapping[(uint8_t)json_type::int64_value] = integer_validator<Json>::compile(schema, context, keywords);
                type_mapping[(uint8_t)json_type::uint64_value] = integer_validator<Json>::compile(schema, context, keywords);
                type_mapping[(uint8_t)json_type::double_value] = integer_validator<Json>::compile(schema, context, keywords);
            }
            else if (type == "number")
            {
                type_mapping[(uint8_t)json_type::double_value] = number_validator<Json>::compile(schema, context, keywords);
                type_mapping[(uint8_t)json_type::int64_value] =  number_validator<Json>::compile(schema, context, keywords);
                type_mapping[(uint8_t)json_type::uint64_value] =  number_validator<Json>::compile(schema, context, keywords);
            }
            else if (type.empty())
            {
                type_mapping[(uint8_t)json_type::null_value] = null_validator<Json>::compile(context);
                type_mapping[(uint8_t)json_type::object_value] = jsoncons::make_unique<object_validator<Json>>(this, schema, context);
                type_mapping[(uint8_t)json_type::array_value] = compile_array_schema(schema, context, this);
                type_mapping[(uint8_t)json_type::string_value] = compile_string_schema(schema, context);
                // For binary types
                type_mapping[(uint8_t) json_type::byte_string_value] = compile_string_schema(schema, context);
                type_mapping[(uint8_t)json_type::bool_value] = boolean_validator<Json>::compile(context);
                type_mapping[(uint8_t)json_type::int64_value] = integer_validator<Json>::compile(schema, context, keywords);
                type_mapping[(uint8_t)json_type::uint64_value] = integer_validator<Json>::compile(schema, context, keywords);
                type_mapping[(uint8_t)json_type::double_value] = integer_validator<Json>::compile(schema, context, keywords);
                type_mapping[(uint8_t)json_type::double_value] = number_validator<Json>::compile(schema, context, keywords);
                type_mapping[(uint8_t)json_type::int64_value] = number_validator<Json>::compile(schema, context, keywords);
                type_mapping[(uint8_t)json_type::uint64_value] = number_validator<Json>::compile(schema, context, keywords);
            }
            return type_mapping;
        }

        std::unique_ptr<type_validator<Json>> compile_type_validator(const Json& schema,
            const compilation_context& context)
        {
            std::string schema_path = context.get_schema_path();
            Json default_value{jsoncons::null_type()};
            jsoncons::optional<enum_validator<Json>> enumvalidator{};
            jsoncons::optional<const_keyword<Json>> const_validator{};
            std::vector<validator_type> combined_validators;
            jsoncons::optional<conditional_validator<Json>> conditionalvalidator;
            std::vector<std::string> expected_types;

            std::vector<validator_type> type_mapping{(uint8_t)(json_type::object_value)+1};
            std::set<std::string> known_keywords;

            auto it = schema.find("type");
            if (it == schema.object_range().end()) 
            {
                type_mapping = make_type_mapping("", schema, context, known_keywords);
            }
            else 
            {
                switch (it->value().type()) 
                { 
                    case json_type::string_value: 
                    {
                        auto type = it->value().template as<std::string>();
                        type_mapping = make_type_mapping(type, schema, context, known_keywords);
                        expected_types.emplace_back(std::move(type));
                        break;
                    } 

                    case json_type::array_value: // "type": ["type1", "type2"]
                    {
                        for (const auto& item : it->value().array_range())
                        {
                            auto type = item.template as<std::string>();
                            type_mapping = make_type_mapping(type, schema, context, known_keywords);
                            expected_types.emplace_back(std::move(type));
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            const auto default_it = schema.find("default");
            if (default_it != schema.object_range().end()) 
            {
                default_value = default_it->value();
            }

            it = schema.find("enum");
            if (it != schema.object_range().end()) 
            {
                enumvalidator = enum_validator<Json>(it->value(), context);
            }

            it = schema.find("const");
            if (it != schema.object_range().end()) 
            {
                const_validator = const_keyword<Json>(it->value(), context);
            }

            it = schema.find("not");
            if (it != schema.object_range().end()) 
            {
                combined_validators.emplace_back(not_validator<Json>::compile(this, it->value(), context));
            }

            it = schema.find("allOf");
            if (it != schema.object_range().end()) 
            {
                combined_validators.emplace_back(jsoncons::make_unique<combining_validator<Json,all_of_criterion<Json>>>(this, it->value(), context));
            }

            it = schema.find("anyOf");
            if (it != schema.object_range().end()) 
            {
                combined_validators.emplace_back(jsoncons::make_unique<combining_validator<Json,any_of_criterion<Json>>>(this, it->value(), context));
            }

            it = schema.find("oneOf");
            if (it != schema.object_range().end()) 
            {
                combined_validators.emplace_back(jsoncons::make_unique<combining_validator<Json,one_of_criterion<Json>>>(this, it->value(), context));
            }

            it = schema.find("if");
            if (it != schema.object_range().end()) 
            {
                conditionalvalidator = conditional_validator<Json>(this, it->value(), schema, context);
                // schema["if"] is object and has id, can be looked up
            }

            return jsoncons::make_unique<type_validator<Json>>(std::move(schema_path), 
                std::move(type_mapping),
                std::move(default_value), 
                std::move(std::move(enumvalidator)),
                std::move(std::move(const_validator)),
                std::move(std::move(combined_validators)),
                std::move(std::move(conditionalvalidator)),
                std::move(std::move(expected_types))
                );
        }

        std::unique_ptr<content_encoding_validator<Json>> compile_content_encoding_schema(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("contentEncoding");
            if (!schema.is_string())
            {
                std::string message("contentEncoding must be a string");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<std::string>();
            return jsoncons::make_unique<content_encoding_validator<Json>>(schema_path, value);
        }

        std::unique_ptr<string_validator<Json>> compile_string_schema(const Json& schema,
            const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("string");
            auto new_context = context.update_uris(schema, schema_path);

            std::vector<validator_type> validators;
            auto it = schema.find("maxLength");
            if (it != schema.object_range().end())
            {
                validators.emplace_back(compile_max_length_schema(it->value(), new_context));
            }

            it = schema.find("minLength");
            if (it != schema.object_range().end())
            {
                validators.emplace_back(min_length_validator<Json>::compile(it->value(), new_context));
            }

            it = schema.find("contentEncoding");
            if (it != schema.object_range().end())
            {
                validators.emplace_back(compile_content_encoding_schema(it->value(), new_context));
                // If "contentEncoding" is set to "binary", a Json value
                // of type json_type::byte_string_value is accepted.
            }

            it = schema.find("contentMediaType");
            if (it != schema.object_range().end())
            {
                validators.emplace_back(compile_content_media_type_schema(it->value(), new_context));
            }

#if defined(JSONCONS_HAS_STD_REGEX)
            it = schema.find("pattern");
            if (it != schema.object_range().end())
            {
                validators.emplace_back(compile_pattern_schema(it->value(), new_context));
            }
#endif

            it = schema.find("format");
            if (it != schema.object_range().end())
            {
                validators.emplace_back(compile_format_schema(it->value(), new_context));
            }

            return jsoncons::make_unique<string_validator<Json>>(schema_path, std::move(validators));
        }

        std::unique_ptr<content_media_type_validator<Json>> compile_content_media_type_schema(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("contentMediaType");
            if (!schema.is_string())
            {
                std::string message("contentMediaType must be a string");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<std::string>();
            return jsoncons::make_unique<content_media_type_validator<Json>>(schema_path, value);
        }

        std::unique_ptr<format_validator<Json>> compile_format_schema(const Json& schema, const compilation_context& context)
        {
            auto schema_path = context.make_schema_path_with("format");

            std::string format = schema.template as<std::string>();

            format_checker format_check;
            if (format == "date-time")
            {
                format_check = rfc3339_date_time_check;
            }
            else if (format == "date") 
            {
                format_check = rfc3339_date_check;
            } 
            else if (format == "time") 
            {
                format_check = rfc3339_time_check;
            } 
            else if (format == "email") 
            {
                format_check = email_check;
            } 
            else if (format == "hostname") 
            {
                format_check = hostname_check;
            } 
            else if (format == "ipv4") 
            {
                format_check = ipv4_check;
            } 
            else if (format == "ipv6") 
            {
                format_check = ipv6_check;
            } 
            else if (format == "regex") 
            {
                format_check = regex_check;
            } 
            else
            {
                // Not supported - ignore
                format_check = nullptr;
            }       

            return jsoncons::make_unique<format_validator<Json>>(schema_path, 
                format_check);
        }

        std::unique_ptr<pattern_validator<Json>> compile_pattern_schema(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("pattern");
            auto pattern_string = schema.template as<std::string>();
            auto regex = std::regex(pattern_string, std::regex::ECMAScript);
            return jsoncons::make_unique<pattern_validator<Json>>(schema_path, 
                pattern_string, regex);
        }

        std::unique_ptr<max_length_validator<Json>> compile_max_length_schema(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("maxLength");
            if (!schema.is_number())
            {
                std::string message("maxLength must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<std::size_t>();
            return jsoncons::make_unique<max_length_validator<Json>>(schema_path, value);
        }

        std::unique_ptr<max_items_validator<Json>> compile_max_items_schema(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("maxItems");
            if (!schema.is_number())
            {
                std::string message("maxItems must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<std::size_t>();
            return jsoncons::make_unique<max_items_validator<Json>>(schema_path, value);
        }

        std::unique_ptr<min_items_validator<Json>> compile_min_items_schema(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("minItems");
            if (!schema.is_number())
            {
                std::string message("minItems must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<std::size_t>();
            return jsoncons::make_unique<min_items_validator<Json>>(schema_path, value);
        }

        std::unique_ptr<array_validator<Json>> compile_array_schema(const Json& schema,
            const compilation_context& context, subschema_validator_factory<Json>* builder)
        {
            std::string schema_path = context.make_schema_path_with("array");
            auto new_context = context.update_uris(schema, schema_path);

            std::vector<validator_type> validators;

            auto it = schema.find("maxItems");
            if (it != schema.object_range().end()) 
            {
                validators.emplace_back(compile_max_items_schema(it->value(), context));
            }

            it = schema.find("minItems");
            if (it != schema.object_range().end()) 
            {
                validators.emplace_back(compile_min_items_schema(it->value(), context));
            }

            it = schema.find("uniqueItems");
            if (it != schema.object_range().end()) 
            {
                validators.emplace_back(unique_items_validator<Json>::compile(it->value(), context));
            }

            it = schema.find("items");
            if (it != schema.object_range().end()) 
            {

                if (it->value().type() == json_type::array_value) 
                {
                    validators.emplace_back(items_array_validator<Json>::compile(schema, it->value(), 
                        context, builder));
                } 
                else if (it->value().type() == json_type::object_value ||
                           it->value().type() == json_type::bool_value)
                {
                    validators.emplace_back(items_object_validator<Json>::compile(schema, it->value(), 
                        context, builder));
                }
            }

            it = schema.find("contains");
            if (it != schema.object_range().end()) 
            {
                validators.emplace_back(contains_validator<Json>::compile(schema, it->value(), 
                    context, builder));
            }

            return jsoncons::make_unique<array_validator<Json>>(schema_path, std::move(validators));
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
                    return s;
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
            {
                return file->second;
            }
            else
            {
                return subschema_registries_.insert(file, {loc, {}})->second;
            }
        }

    };

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& schema)
    {
        keyword_factory<Json> kwFactory{default_uri_resolver<Json>()};
        kwFactory.load_root(schema);

        return kwFactory.get_schema();
    }

    template <class Json,class URIResolver>
    typename std::enable_if<extension_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& schema, const URIResolver& resolver)
    {
        keyword_factory<Json> kwFactory(resolver);
        kwFactory.load_root(schema);

        return kwFactory.get_schema();
    }

} // namespace draft7
} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_DRAFT7_KEYWORD_FACTORY_HPP

// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_DRAFT201909_SCHEMA_PARSER_IMPL_HPP
#define JSONCONS_JSONSCHEMA_DRAFT201909_SCHEMA_PARSER_IMPL_HPP

#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/common/compilation_context.hpp>
#include <jsoncons_ext/jsonschema/json_schema.hpp>
#include <jsoncons_ext/jsonschema/common/keywords.hpp>
#include <jsoncons_ext/jsonschema/common/schema_parser.hpp>
#include <jsoncons_ext/jsonschema/draft7/schema_draft7.hpp>
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
namespace draft201909 {

    template <class Json>
    struct default_uri_resolver
    {
        Json operator()(const jsoncons::uri& uri)
        {
            if (uri.string() == "https://json-schema.org/draft/2019-09/schema") 
            {
                JSONCONS_THROW(jsonschema::schema_error(std::string("Don't currently support") + "https://json-schema.org/draft/2019-09/schema"));
                //return jsoncons::jsonschema::draft7::schema_draft7<Json>::get_schema();
            }

            JSONCONS_THROW(jsonschema::schema_error("Don't know how to load JSON Schema " + uri.base().string()));
        }
    };

    template <class Json>
    class schema_parser_impl : public schema_parser<Json> 
    {
    public:
        using validator_type = std::unique_ptr<validator_base<Json>>;
        using keyword_validator_type = typename std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_pointer = schema_validator<Json>*;
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;
        using ref_validator_type = ref_validator<Json>;
        using recursive_ref_validator_type = recursive_ref_validator<Json>;
    private:
        struct subschema_registry
        {
            std::map<std::string, schema_validator_pointer> schemas; // schemas
            std::vector<std::pair<std::string, ref_validator_type*>> unresolved; // unresolved references
            std::map<std::string, Json> unknown_keywords;
        };

        uri_resolver<Json> resolver_;
        schema_validator_type root_;

        // Owns all subschemas
        std::vector<validator_type> subschemas_;

        // Map location to subschema_registry
        std::map<std::string, subschema_registry> subschema_registries_;

        using keyword_factory_type = std::function<keyword_validator_type(const compilation_context& context, const Json& sch)>;

        std::unordered_map<std::string,keyword_factory_type> keyword_factory_map_;

    public:
        schema_parser_impl(const uri_resolver<Json>& resolver = default_uri_resolver<Json>()) noexcept

            : resolver_(resolver)
        {
            init();
        }

        schema_parser_impl(uri_resolver<Json>&& resolver) noexcept

            : resolver_(std::move(resolver))
        {
            init();
        }

        schema_parser_impl(const schema_parser_impl&) = delete;
        schema_parser_impl& operator=(const schema_parser_impl&) = delete;
        schema_parser_impl(schema_parser_impl&&) = default;
        schema_parser_impl& operator=(schema_parser_impl&&) = default;

        void init()
        {
            keyword_factory_map_.emplace("type", 
                [&](const compilation_context& context, const Json& sch){return make_type_validator(context, sch);});
            keyword_factory_map_.emplace("contentEncoding", 
                [&](const compilation_context& context, const Json& sch){return make_content_encoding_validator(context, sch);});
            keyword_factory_map_.emplace("contentMediaType", 
                [&](const compilation_context& context, const Json& sch){return make_content_media_type_validator(context, sch);});
            keyword_factory_map_.emplace("format", 
                [&](const compilation_context& context, const Json& sch){return make_format_validator(context, sch);});
#if defined(JSONCONS_HAS_STD_REGEX)
            keyword_factory_map_.emplace("pattern", 
                [&](const compilation_context& context, const Json& sch){return make_pattern_validator(context, sch);});
#endif
            keyword_factory_map_.emplace("maxLength", 
                [&](const compilation_context& context, const Json& sch){return make_max_length_validator(context, sch);});
            keyword_factory_map_.emplace("maxItems", 
                [&](const compilation_context& context, const Json& sch){return make_max_items_validator(context, sch);});
            keyword_factory_map_.emplace("minItems", 
                [&](const compilation_context& context, const Json& sch){return make_min_items_validator(context, sch);});
            keyword_factory_map_.emplace("contains", 
                [&](const compilation_context& context, const Json& sch){return make_contains_validator(context, sch);});
            keyword_factory_map_.emplace("uniqueItems", 
                [&](const compilation_context& context, const Json& sch){return make_unique_items_validator(context, sch);});
            keyword_factory_map_.emplace("minLength", 
                [&](const compilation_context& context, const Json& sch){return make_min_length_validator(context, sch);});
            keyword_factory_map_.emplace("not", 
                [&](const compilation_context& context, const Json& sch){return make_not_validator(context, sch);});
            keyword_factory_map_.emplace("maximum", 
                [&](const compilation_context& context, const Json& sch){return make_maximum_validator(context, sch);});
            keyword_factory_map_.emplace("exclusiveMaximum", 
                [&](const compilation_context& context, const Json& sch){return make_exclusive_maximum_validator(context, sch);});
            keyword_factory_map_.emplace("minimum", 
                [&](const compilation_context& context, const Json& sch){return make_minimum_validator(context, sch);});
            keyword_factory_map_.emplace("exclusiveMinimum", 
                [&](const compilation_context& context, const Json& sch){return make_exclusive_minimum_validator(context, sch);});
            keyword_factory_map_.emplace("multipleOf", 
                [&](const compilation_context& context, const Json& sch){return make_multiple_of_validator(context, sch);});
            keyword_factory_map_.emplace("const", 
                [&](const compilation_context& context, const Json& sch){return make_const_validator(context, sch);});
            keyword_factory_map_.emplace("enum", 
                [&](const compilation_context& context, const Json& sch){return make_enum_validator(context, sch);});
            keyword_factory_map_.emplace("allOf", 
                [&](const compilation_context& context, const Json& sch){return make_all_of_validator(context, sch);});
            keyword_factory_map_.emplace("anyOf", 
                [&](const compilation_context& context, const Json& sch){return make_any_of_validator(context, sch);});
            keyword_factory_map_.emplace("oneOf", 
                [&](const compilation_context& context, const Json& sch){return make_one_of_validator(context, sch);});
            keyword_factory_map_.emplace("dependencies", 
                [&](const compilation_context& context, const Json& sch){return make_dependencies_validator(context, sch);});
            keyword_factory_map_.emplace("propertyNames", 
                [&](const compilation_context& context, const Json& sch){return make_property_names_validator(context, sch);});
            keyword_factory_map_.emplace("required", 
                [&](const compilation_context& context, const Json& sch){return make_required_validator(context, sch);});
            keyword_factory_map_.emplace("dependentRequired", 
                [&](const compilation_context& context, const Json& sch){return make_dependent_required_validator(context, sch);});
            keyword_factory_map_.emplace("dependentSchemas", 
                [&](const compilation_context& context, const Json& sch){return make_dependent_schemas_validator(context, sch);});
            keyword_factory_map_.emplace("unevaluatedProperties", 
                [&](const compilation_context& context, const Json& sch){return make_unevaluated_properties_validator(context, sch);});
        }

        std::shared_ptr<json_schema<Json>> get_schema() override
        {
            return std::make_shared<json_schema<Json>>(std::move(subschemas_), std::move(root_));
        }

        schema_validator_type make_schema_validator( 
            const compilation_context& context, const Json& sch, jsoncons::span<const std::string> keys) //override
        {
            auto new_context = make_compilation_context(context, sch, keys);
            //std::cout << "make_schema_validator " << context.get_absolute_uri().string() << ", " << new_context.get_absolute_uri().string() << "\n\n";

            schema_validator_type schema_validator_ptr;

            switch (sch.type())
            {
                case json_type::bool_value:
                {
                    uri schema_path = new_context.get_absolute_uri();
                    if (sch.template as<bool>())
                    {
                        schema_validator_ptr = jsoncons::make_unique<boolean_schema_validator<Json>>( 
                            schema_path, sch.template as<bool>());
                    }
                    else
                    {
                        schema_validator_ptr = jsoncons::make_unique<boolean_schema_validator<Json>>( 
                          schema_path, false);
                    }
                    schema_validator<Json>* p = schema_validator_ptr.get();
                    for (const auto& uri : new_context.uris()) 
                    { 
                        insert_schema(uri, p);
                    }          
                    break;
                }
                case json_type::object_value:
                {
                    std::set<std::string> known_keywords;

                    auto it = sch.find("definitions");
                    if (it != sch.object_range().end()) 
                    {
                        for (const auto& def : it->value().object_range())
                        {
                            std::string sub_keys[] = { "definitions", def.key() };
                            subschemas_.emplace_back(make_schema_validator(
                                new_context, def.value(), sub_keys));
                        }
                        known_keywords.insert("definitions");
                    }
                    it = sch.find("$defs");
                    if (it != sch.object_range().end()) 
                    {
                        for (const auto& def : it->value().object_range())
                        {
                            std::string sub_keys[] = { "$defs", def.key() };
                            subschemas_.emplace_back(make_schema_validator(
                                new_context, def.value(), sub_keys));
                        }
                        known_keywords.insert("definitions");
                    }
                    schema_validator_ptr = make_object_schema_validator(new_context, sch);
                    schema_validator<Json>* p = schema_validator_ptr.get();
                    for (const auto& uri : new_context.uris()) 
                    { 
                        insert_schema(uri, p);
                        for (const auto& item : sch.object_range())
                        {
                            if (known_keywords.find(item.key()) == known_keywords.end())
                            {
                                insert_unknown_keyword(uri, item.key(), item.value()); // save unknown keywords for later reference
                            }
                        }
                    }          
                    break;
                }
                default:
                    JSONCONS_THROW(schema_error("invalid JSON-type for a schema for " + new_context.get_absolute_uri().string() + ", expected: boolean or object"));
                    break;
            }
            
            return schema_validator_ptr;
        }

        schema_validator_type make_object_schema_validator( 
            const compilation_context& context, const Json& sch)
        {
            Json default_value{ jsoncons::null_type() };
            std::vector<keyword_validator_type> validators;
            std::set<std::string> known_keywords;
            bool is_recursive_anchor = false;

            auto it = sch.find("$recursiveAnchor"); 
            if (it != sch.object_range().end()) 
            {
                is_recursive_anchor = it->value().template as<bool>();
            }

            it = sch.find("default");
            if (it != sch.object_range().end()) 
            {
                default_value = it->value();
                known_keywords.insert("default");
            }

            it = sch.find("$ref");
            if (it != sch.object_range().end()) // this schema has a reference
            {
                schema_location relative(it->value().template as<std::string>()); 
                auto id = relative.resolve(context.get_base_uri()); 
                validators.push_back(get_or_create_reference(id));
            }

            it = sch.find("$recursiveRef");
            if (it != sch.object_range().end()) // this schema has a reference
            {
                schema_location relative(it->value().template as<std::string>()); 
                auto base_uri = context.get_base_uri();
                auto id = relative.resolve(base_uri); // REVISIT
                validators.push_back(jsoncons::make_unique<recursive_ref_validator_type>(id.uri()));
            }

            for (const auto& key_value : sch.object_range())
            {
                auto factory_it = keyword_factory_map_.find(key_value.key());
                if (factory_it != keyword_factory_map_.end())
                {
                    auto validator = factory_it->second(context, key_value.value());
                    if (validator)
                    {   
                        validators.emplace_back(std::move(validator));
                    }
                }
            }

            it = sch.find("if");
            if (it != sch.object_range().end()) 
            {
                validators.push_back(make_conditional_validator(context, it->value(), sch));
                known_keywords.insert("if");
                // sch["if"] is object and has id, can be looked up
            }
            else
            {
                auto then_it = sch.find("then");
                if (then_it != sch.object_range().end()) 
                {
                    std::string sub_keys[] = { "then" };
                    subschemas_.emplace_back(make_schema_validator(context, then_it->value(), sub_keys));
                    known_keywords.insert("then");
                }

                auto else_it = sch.find("else");
                if (else_it != sch.object_range().end()) 
                {
                    std::string sub_keys[] = { "else" };
                    subschemas_.emplace_back(make_schema_validator(context, else_it->value(), sub_keys));
                    known_keywords.insert("else");
                }
            }

            // Object validators

            std::unique_ptr<properties_validator<Json>> properties;
            it = sch.find("properties");
            if (it != sch.object_range().end()) 
            {
                properties = make_properties_validator(context, it->value());
            }
            std::unique_ptr<pattern_properties_validator<Json>> pattern_properties;

    #if defined(JSONCONS_HAS_STD_REGEX)
            it = sch.find("patternProperties");
            if (it != sch.object_range().end())
            {
                pattern_properties = make_pattern_properties_validator(context, it->value());
            }
    #endif

            it = sch.find("additionalProperties");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_additional_properties_validator(context, it->value(), 
                    std::move(properties), std::move(pattern_properties)));
            }
            else
            {
                validators.emplace_back(make_additional_properties_validator(context, Json(true), 
                    std::move(properties), std::move(pattern_properties)));
            }

            it = sch.find("items");
            if (it != sch.object_range().end()) 
            {

                if (it->value().type() == json_type::array_value) 
                {
                    validators.emplace_back(make_items_array_validator(context, sch, it->value()));
                } 
                else if (it->value().type() == json_type::object_value ||
                           it->value().type() == json_type::bool_value)
                {
                    validators.emplace_back(make_items_object_validator(context, sch, it->value()));
                }
            }
            
            return jsoncons::make_unique<object_schema_validator<Json>>(
                context.get_absolute_uri(),
                std::move(validators), std::move(default_value), is_recursive_anchor);
        }

        std::unique_ptr<type_validator<Json>> make_type_validator(const compilation_context& context,
            const Json& sch)
        {
            std::string schema_path = context.get_absolute_uri().string();
            std::vector<json_schema_type> expected_types;

            switch (sch.type()) 
            { 
                case json_type::string_value: 
                {
                    auto type = sch.template as<std::string>();
                    if (type == "null")
                    {
                        expected_types.push_back(json_schema_type::null);
                    }
                    else if (type == "object")
                    {
                        expected_types.push_back(json_schema_type::object);
                    }
                    else if (type == "array")
                    {
                        expected_types.push_back(json_schema_type::array);
                    }
                    else if (type == "string")
                    {
                        expected_types.push_back(json_schema_type::string);
                    }
                    else if (type == "boolean")
                    {
                        expected_types.push_back(json_schema_type::boolean);
                    }
                    else if (type == "integer")
                    {
                        expected_types.push_back(json_schema_type::integer);
                    }
                    else if (type == "number")
                    {
                        expected_types.push_back(json_schema_type::number);
                    }
                    break;
                } 

                case json_type::array_value: // "type": ["type1", "type2"]
                {
                    for (const auto& item : sch.array_range())
                    {
                        auto type = item.template as<std::string>();
                        if (type == "null")
                        {
                            expected_types.push_back(json_schema_type::null);
                        }
                        else if (type == "object")
                        {
                            expected_types.push_back(json_schema_type::object);
                        }
                        else if (type == "array")
                        {
                            expected_types.push_back(json_schema_type::array);
                        }
                        else if (type == "string")
                        {
                            expected_types.push_back(json_schema_type::string);
                        }
                        else if (type == "boolean")
                        {
                            expected_types.push_back(json_schema_type::boolean);
                        }
                        else if (type == "integer")
                        {
                            expected_types.push_back(json_schema_type::integer);
                        }
                        else if (type == "number")
                        {
                            expected_types.push_back(json_schema_type::number);
                        }
                    }
                    break;
                }
                default:
                    break;
            }

            return jsoncons::make_unique<type_validator<Json>>( std::move(schema_path), 
                std::move(expected_types)
         );
        }

        std::unique_ptr<content_encoding_validator<Json>> make_content_encoding_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("contentEncoding");
            if (!sch.is_string())
            {
                std::string message("contentEncoding must be a string");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::string>();
            return jsoncons::make_unique<content_encoding_validator<Json>>( schema_path, value);
        }

        std::unique_ptr<content_media_type_validator<Json>> make_content_media_type_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("contentMediaType");
            if (!sch.is_string())
            {
                std::string message("contentMediaType must be a string");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::string>();
            return jsoncons::make_unique<content_media_type_validator<Json>>( schema_path, value);
        }

        std::unique_ptr<format_validator<Json>> make_format_validator(const compilation_context& context, const Json& sch)
        {
            auto schema_path = context.make_schema_path_with("format");

            std::string format = sch.template as<std::string>();

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

            return jsoncons::make_unique<format_validator<Json>>( schema_path, 
                format_check);
        }

        std::unique_ptr<pattern_validator<Json>> make_pattern_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("pattern");
            auto pattern_string = sch.template as<std::string>();
            auto regex = std::regex(pattern_string, std::regex::ECMAScript);
            return jsoncons::make_unique<pattern_validator<Json>>( schema_path, 
                pattern_string, regex);
        }

        std::unique_ptr<max_length_validator<Json>> make_max_length_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("maxLength");
            if (!sch.is_number())
            {
                std::string message("maxLength must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<max_length_validator<Json>>( schema_path, value);
        }

        std::unique_ptr<max_items_validator<Json>> make_max_items_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("maxItems");
            if (!sch.is_number())
            {
                std::string message("maxItems must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<max_items_validator<Json>>( schema_path, value);
        }

        std::unique_ptr<min_items_validator<Json>> make_min_items_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("minItems");
            if (!sch.is_number())
            {
                std::string message("minItems must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<min_items_validator<Json>>( schema_path, value);
        }

        std::unique_ptr<contains_validator<Json>> make_contains_validator(const compilation_context& context,
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("contains");

            std::string sub_keys[] = { "contains" };

            return jsoncons::make_unique<contains_validator<Json>>( schema_path, 
                make_schema_validator(context, sch, sub_keys));
        }

        std::unique_ptr<items_array_validator<Json>> make_items_array_validator(const compilation_context& context, 
            const Json& parent, const Json& sch)
        {
            std::vector<schema_validator_type> item_validators;
            schema_validator_type additional_items_validator = nullptr;

            uri schema_path = context.make_schema_path_with("items");

            if (sch.type() == json_type::array_value) 
            {
                size_t c = 0;
                for (const auto& subsch : sch.array_range())
                {
                    std::string sub_keys[] = {"items", std::to_string(c++)};

                    item_validators.emplace_back(make_schema_validator(context, subsch, sub_keys));
                }

                auto it = parent.find("additionalItems");
                if (it != parent.object_range().end()) 
                {
                    std::string sub_keys[] = {"additionalItems"};
                    additional_items_validator = make_schema_validator(context, it->value(), sub_keys);
                }
            }

            return jsoncons::make_unique<items_array_validator<Json>>( schema_path, 
                std::move(item_validators), std::move(additional_items_validator));
        }

        std::unique_ptr<items_object_validator<Json>> make_items_object_validator(const compilation_context& context, 
            const Json& /* parent */, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("items");

            std::string sub_keys[] = {"items"};

            return jsoncons::make_unique<items_object_validator<Json>>( schema_path, 
                make_schema_validator(context, sch, sub_keys));
        }

        std::unique_ptr<unique_items_validator<Json>> make_unique_items_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("uniqueItems");
            bool are_unique = sch.template as<bool>();
            return jsoncons::make_unique<unique_items_validator<Json>>( schema_path, are_unique);
        }

        std::unique_ptr<min_length_validator<Json>> make_min_length_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("minLength");
            if (!sch.is_number())
            {
                std::string message("minLength must be an integer value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<min_length_validator<Json>>( schema_path, value);
        }

        std::unique_ptr<not_validator<Json>> make_not_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("not");
            std::string not_key[] = { "not" };
            return jsoncons::make_unique<not_validator<Json>>( schema_path, 
                make_schema_validator(context, sch, not_key));
        }

        std::unique_ptr<maximum_validator<Json>> make_maximum_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("maximum");
            if (!sch.is_number())
            {
                std::string message("maximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            return jsoncons::make_unique<maximum_validator<Json>>( schema_path, sch);
        }

        std::unique_ptr<exclusive_maximum_validator<Json>> make_exclusive_maximum_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("exclusiveMaximum");
            if (!sch.is_number())
            {
                std::string message("exclusiveMaximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            return jsoncons::make_unique<exclusive_maximum_validator<Json>>( schema_path, sch);
        }

        std::unique_ptr<minimum_validator<Json>> make_minimum_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("minimum");

            //std::cout << "make_minimum_validator uri: " << context.get_absolute_uri().string() << ", " << schema_path.string() << "\n";
                
            if (!sch.is_number())
            {
                std::string message("minimum must be an integer");
                JSONCONS_THROW(schema_error(message));
            }
            return jsoncons::make_unique<minimum_validator<Json>>( schema_path, sch);
        }

        std::unique_ptr<exclusive_minimum_validator<Json>> make_exclusive_minimum_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("exclusiveMinimum");
            if (!sch.is_number())
            {
                std::string message("exclusiveMinimum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            return jsoncons::make_unique<exclusive_minimum_validator<Json>>( schema_path, sch);
        }

        std::unique_ptr<multiple_of_validator<Json>> make_multiple_of_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("multipleOf");
            if (!sch.is_number())
            {
                std::string message("multipleOf must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<double>();
            return jsoncons::make_unique<multiple_of_validator<Json>>( schema_path, value);
        }

        std::unique_ptr<const_validator<Json>> make_const_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("const");
            return jsoncons::make_unique<const_validator<Json>>( schema_path, sch);
        }

        std::unique_ptr<enum_validator<Json>> make_enum_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("enum");
            return jsoncons::make_unique<enum_validator<Json>>( schema_path, sch);
        }

        std::unique_ptr<required_validator<Json>> make_required_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("required");
            return jsoncons::make_unique<required_validator<Json>>( schema_path, sch.template as<std::vector<std::string>>());
        }

        std::unique_ptr<conditional_validator<Json>> make_conditional_validator(const compilation_context& context,
            const Json& sch_if, const Json& sch)
        {
            std::string schema_path = context.get_absolute_uri().string();
            schema_validator_type if_validator(nullptr);
            schema_validator_type then_validator(nullptr);
            schema_validator_type else_validator(nullptr);

            std::string if_key[] = { "if" };
            if_validator = make_schema_validator(context, sch_if, if_key);

            auto then_it = sch.find("then");
            if (then_it != sch.object_range().end()) 
            {
                std::string then_key[] = { "then" };
                then_validator = make_schema_validator(context, then_it->value(), then_key);
            }

            auto else_it = sch.find("else");
            if (else_it != sch.object_range().end()) 
            {
                std::string else_key[] = { "else" };
                else_validator = make_schema_validator(context, else_it->value(), else_key);
            }

            return jsoncons::make_unique<conditional_validator<Json>>( std::move(schema_path),
                std::move(if_validator), std::move(then_validator), std::move(else_validator));
        }

        std::unique_ptr<combining_validator<Json,all_of_criterion<Json>>> make_all_of_validator(const compilation_context& context,
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("allOf");
            std::vector<schema_validator_type> subschemas;

            size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                std::string sub_keys[] = { all_of_criterion<Json>::key(), std::to_string(c++) };
                subschemas.emplace_back(make_schema_validator(context, subsch, sub_keys));
            }
            return jsoncons::make_unique<combining_validator<Json,all_of_criterion<Json>>>(std::move(schema_path), std::move(subschemas));
        }

        std::unique_ptr<any_of_validator<Json>> make_any_of_validator(const compilation_context& context,
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("anyOf");
            std::vector<schema_validator_type> subschemas;

            size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                std::string sub_keys[] = { any_of_criterion<Json>::key(), std::to_string(c++) };
                subschemas.emplace_back(make_schema_validator(context, subsch, sub_keys));
            }
            return jsoncons::make_unique<any_of_validator<Json>>(std::move(schema_path), std::move(subschemas));
        }

        std::unique_ptr<combining_validator<Json,one_of_criterion<Json>>> make_one_of_validator(const compilation_context& context,
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("oneOf");
            std::vector<schema_validator_type> subschemas;

            size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                std::string sub_keys[] = { one_of_criterion<Json>::key(), std::to_string(c++) };
                subschemas.emplace_back(make_schema_validator(context, subsch, sub_keys));
            }
            return jsoncons::make_unique<combining_validator<Json,one_of_criterion<Json>>>(std::move(schema_path), std::move(subschemas));
        }
                
        std::unique_ptr<properties_validator<Json>> make_properties_validator(
                                                                              const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.get_absolute_uri();
            std::map<std::string, schema_validator_type> properties;

            for (const auto& prop : sch.object_range())
            {
                std::string sub_keys[] =
                {"properties", prop.key()};
                properties.emplace(std::make_pair(prop.key(), 
                    make_schema_validator(context, prop.value(), sub_keys)));
            }

            return jsoncons::make_unique<properties_validator<Json>>(
                                                                     std::move(schema_path), std::move(properties));
        }

#if defined(JSONCONS_HAS_STD_REGEX)
                
        std::unique_ptr<pattern_properties_validator<Json>> make_pattern_properties_validator( const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.get_absolute_uri();
            std::vector<std::pair<std::regex, schema_validator_type>> pattern_properties;
            
            for (const auto& prop : sch.object_range())
            {
                std::string sub_keys[] = {prop.key()};
                pattern_properties.emplace_back(
                    std::make_pair(
                        std::regex(prop.key(), std::regex::ECMAScript),
                        make_schema_validator(context, prop.value(), sub_keys)));
            }

            return jsoncons::make_unique<pattern_properties_validator<Json>>( std::move(schema_path),
                std::move(pattern_properties));
        }
#endif
       

        std::unique_ptr<additional_properties_validator<Json>> make_additional_properties_validator( 
            const compilation_context& context, const Json& sch, 
            std::unique_ptr<properties_validator<Json>>&& properties, std::unique_ptr<pattern_properties_validator<Json>>&& pattern_properties)
        {
            uri schema_path = context.get_absolute_uri();
            std::vector<keyword_validator_type> validators;
            schema_validator_type additional_properties;

            std::string sub_keys[] = {"additionalProperties"};
            additional_properties = make_schema_validator(context, sch, sub_keys);

            return jsoncons::make_unique<additional_properties_validator<Json>>( std::move(schema_path),
                std::move(properties), std::move(pattern_properties),
                std::move(additional_properties));
        }
                

        std::unique_ptr<dependencies_validator<Json>> make_dependencies_validator( const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.get_absolute_uri();
            std::map<std::string, keyword_validator_type> dependent_required;
            std::map<std::string, schema_validator_type> dependent_schemas;

            for (const auto& dep : sch.object_range())
            {
                switch (dep.value().type()) 
                {
                    case json_type::array_value:
                    {
                        auto location = context.make_schema_path_with("dependencies");
                        dependent_required.emplace(dep.key(), 
                            make_required_validator(compilation_context(nullptr, std::vector<schema_location>{{location}}),
                                dep.value()));
                        break;
                    }
                    case json_type::object_value:
                    {
                        std::string sub_keys[] = {"dependencies"};
                        dependent_schemas.emplace(dep.key(),
                            make_schema_validator(context, dep.value(), sub_keys));
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            

            return jsoncons::make_unique<dependencies_validator<Json>>( std::move(schema_path),
                std::move(dependent_required), std::move(dependent_schemas));
        }

        std::unique_ptr<dependent_required_validator<Json>> make_dependent_required_validator( 
            const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.get_absolute_uri();
            std::map<std::string, keyword_validator_type> dependent_required;

            for (const auto& dep : sch.object_range())
            {
                switch (dep.value().type()) 
                {
                    case json_type::array_value:
                    {
                        auto location = context.make_schema_path_with("dependentRequired");
                        dependent_required.emplace(dep.key(), 
                            make_required_validator(compilation_context(nullptr, std::vector<schema_location>{{location}}),
                                dep.value()));
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }

            return jsoncons::make_unique<dependent_required_validator<Json>>( std::move(schema_path),
                std::move(dependent_required));
        }

        std::unique_ptr<dependent_schemas_validator<Json>> make_dependent_schemas_validator( const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.get_absolute_uri();
            std::map<std::string, schema_validator_type> dependent_schemas;

            for (const auto& dep : sch.object_range())
            {
                switch (dep.value().type()) 
                {
                    case json_type::object_value:
                    {
                        std::string sub_keys[] = {"dependentSchemas"};
                        dependent_schemas.emplace(dep.key(),
                            make_schema_validator(context, dep.value(), sub_keys));
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            

            return jsoncons::make_unique<dependent_schemas_validator<Json>>( std::move(schema_path),
                std::move(dependent_schemas));
        }


        std::unique_ptr<property_names_validator<Json>> make_property_names_validator( 
            const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.get_absolute_uri();
            schema_validator_type property_names_schema_validator;

            std::string sub_keys[] = { "propertyNames"};
            property_names_schema_validator = make_schema_validator(context, sch, sub_keys);

            return jsoncons::make_unique<property_names_validator<Json>>( std::move(schema_path),
                std::move(property_names_schema_validator));
        }

        std::unique_ptr<unevaluated_properties_validator<Json>> make_unevaluated_properties_validator(
            const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.get_absolute_uri();

            std::string sub_keys[] = {"unevaluatedProperties"};

            return jsoncons::make_unique<unevaluated_properties_validator<Json>>( std::move(schema_path),
                make_schema_validator(context, sch, sub_keys));
        }

        void parse(const Json& sch) override
        {
            parse(sch, "#");
        }

        void parse(const Json& sch, const std::string& retrieval_uri) override
        {
            if (sch.is_object())
            {
                auto it = sch.find("$schema");
                if (it != sch.object_range().end())
                {
                    auto sv = it->value().as_string_view();
                    if (sv.find("https://json-schema.org/draft/2019-09/schema") == string_view::npos)
                    {
                        std::string message("Unsupported schema version ");
                        message.append(sv.data(), sv.size());
                        JSONCONS_THROW(schema_error(message));
                    }
                }
            }
            load(compilation_context(schema_location(retrieval_uri)), sch);
        }

        void load(const compilation_context& context, const Json& sch)
        {
            subschema_registries_.clear();
            root_ = make_schema_validator(context, sch, {});

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
                            subschemas_.emplace_back(make_schema_validator(compilation_context(schema_location(loc)), external_sch, {}));
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
        }

    private:

        void insert_schema(const schema_location& uri, schema_validator<Json>* s)
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
            //root_->resolve_recursive_refs(root_->schema_path(), root_->is_recursive_anchor(), root_.get());
        }

        void insert_unknown_keyword(const schema_location& uri, 
                                    const std::string& key, 
                                    const Json& value)
        {
            auto &file = get_or_create_file(uri.base().string());
            auto new_u = uri.append(key);
            schema_location new_uri(new_u);

            if (new_uri.has_fragment() && !new_uri.has_plain_name_fragment()) 
            {
                auto fragment = std::string(new_uri.fragment());
                // is there a reference looking for this unknown-keyword, which is thus no longer a unknown keyword but a schema
                auto unresolved = std::find_if(file.unresolved.begin(), file.unresolved.end(),
                    [fragment](const std::pair<std::string,ref_validator<Json>*>& pr) {return pr.first == fragment;});
                //auto unresolved = file.unresolved.find(fragment);
                if (unresolved != file.unresolved.end())
                    subschemas_.emplace_back(make_schema_validator(compilation_context(new_uri), value, {}));
                else // no, nothing ref'd it, keep for later
                    file.unknown_keywords.emplace(fragment, value);

                // recursively add possible subschemas of unknown keywords
                if (value.type() == json_type::object_value)
                    for (const auto& subsch : value.object_range())
                    {
                        insert_unknown_keyword(new_uri, subsch.key(), subsch.value());
                    }
            }
        }

        keyword_validator_type get_or_create_reference(const schema_location& uri)
        {
            auto &file = get_or_create_file(uri.base().string());

            // a schema already exists
            auto sch = file.schemas.find(std::string(uri.fragment()));
            if (sch != file.schemas.end())
            {
                return jsoncons::make_unique<ref_validator_type>(uri.base(), sch->second);
            }

            // referencing an unknown keyword, turn it into schema
            //
            // an unknown keyword can only be referenced by a JSONPointer,
            // not by a plain name identifier
            if (uri.has_fragment() && !uri.has_plain_name_fragment()) 
            {
                std::string fragment = std::string(uri.fragment());
                auto unprocessed_keywords_it = file.unknown_keywords.find(fragment);
                if (unprocessed_keywords_it != file.unknown_keywords.end()) 
                {
                    auto &subsch = unprocessed_keywords_it->second; 
                    auto s = make_schema_validator(compilation_context(uri), subsch, {}); 
                    file.unknown_keywords.erase(unprocessed_keywords_it);
                    auto orig = jsoncons::make_unique<ref_validator_type>(uri.base(), s.get());
                    subschemas_.emplace_back(std::move(s));
                    return orig;
                }
            }

            // get or create a ref_validator
            auto orig = jsoncons::make_unique<ref_validator_type>(uri.base());
            file.unresolved.emplace_back(std::string(uri.fragment()), orig.get());
            return orig;
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

        compilation_context make_compilation_context(const compilation_context& parent, 
            const Json& sch, jsoncons::span<const std::string> keys) const override
        {
            // Exclude uri's that are not plain name identifiers
            std::vector<schema_location> new_uris;
            for (const auto& uri : parent.uris())
            {
                if (!uri.has_plain_name_fragment())
                {
                    new_uris.push_back(uri);
                }
            }

            if (new_uris.empty())
            {
                new_uris.emplace_back("#");
            }

            // Append the keys for this sub-schema to the uri's
            for (const auto& key : keys)
            {
                for (auto& uri : new_uris)
                {
                    auto new_u = uri.append(key);
                    uri = schema_location(new_u);
                }
            }
            if (sch.is_object())
            {
                auto it = sch.find("$id"); // If $id is found, this schema can be referenced by the id
                if (it != sch.object_range().end()) 
                {
                    std::string id = it->value().template as<std::string>(); 
                    schema_location relative(id); 
                    schema_location new_uri = relative.resolve(parent.get_base_uri());
                    //std::cout << "$id: " << id << ", " << new_uri.string() << "\n";
                    // Add it to the list if it is not already there
                    if (std::find(new_uris.begin(), new_uris.end(), new_uri) == new_uris.end())
                    {
                        new_uris.emplace_back(new_uri); 
                    }
                }
            }
/*
            std::cout << "Absolute URI: " << parent.get_absolute_uri().string() << "\n";
            for (const auto& uri : new_uris)
            {
                std::cout << "    " << uri.string() << "\n";
            }
*/
            return compilation_context(std::addressof(parent), new_uris);
        }

    };

} // namespace draft201909
} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_DRAFT7_KEYWORD_FACTORY_HPP

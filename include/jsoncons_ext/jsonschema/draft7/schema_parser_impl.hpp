// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_DRAFT7_SCHEMA_PARSER_IMPL_HPP
#define JSONCONS_JSONSCHEMA_DRAFT7_SCHEMA_PARSER_IMPL_HPP

#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/common/compilation_context.hpp>
#include <jsoncons_ext/jsonschema/common/evaluation_context.hpp>
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

            JSONCONS_THROW(jsonschema::schema_error("Don't know how to load JSON Schema " + uri.base().string()));
        }
    };

    template <class Json>
    class schema_parser_impl : public schema_parser<Json> 
    {
    public:
        using validator_type = std::unique_ptr<validator_base<Json>>;
        using keyword_validator_wrapper_type = keyword_validator_wrapper<Json>;
        using keyword_validator_type = typename std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_pointer = schema_validator<Json>*;
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;
        using ref_validator_type = ref_validator<Json>;
    private:
        struct subschema_registry
        {
            std::map<std::string, schema_validator_pointer> schemas; // schemas
            std::map<std::string, ref_validator_type*> unresolved; // unresolved references
            std::map<std::string, Json> unknown_keywords;
        };

        uri_resolver<Json> resolver_;
        schema_validator_type root_;

        // Owns all subschemas
        std::vector<validator_type> subschemas_;

        // Map location to subschema_registry
        std::map<std::string, subschema_registry> subschema_registries_;

    public:
        schema_parser_impl(const uri_resolver<Json>& resolver = default_uri_resolver<Json>()) noexcept

            : resolver_(resolver)
        {
        }

        schema_parser_impl(uri_resolver<Json>&& resolver) noexcept

            : resolver_(std::move(resolver))
        {
        }

        schema_parser_impl(const schema_parser_impl&) = delete;
        schema_parser_impl& operator=(const schema_parser_impl&) = delete;
        schema_parser_impl(schema_parser_impl&&) = default;
        schema_parser_impl& operator=(schema_parser_impl&&) = default;

        std::shared_ptr<json_schema<Json>> get_schema() override
        {
            return std::make_shared<json_schema<Json>>(std::move(subschemas_), std::move(root_));
        }

        schema_validator_type make_schema_validator(const evaluation_context& eval_context, 
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
                        schema_validator_ptr = jsoncons::make_unique<boolean_schema_validator<Json>>(eval_context.eval_path(), 
                            schema_path, sch.template as<bool>());
                    }
                    else
                    {
                        schema_validator_ptr = jsoncons::make_unique<boolean_schema_validator<Json>>(eval_context.eval_path(), 
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
                            subschemas_.emplace_back(make_schema_validator(evaluation_context{}, 
                                new_context, def.value(), sub_keys));
                        }
                        known_keywords.insert("definitions");
                    }
                    it = sch.find("$ref");
                    if (it != sch.object_range().end()) // this schema is a reference
                    {
                        std::vector<keyword_validator_type> validators;
                        Json default_value{ jsoncons::null_type() };
                        schema_location relative(it->value().template as<std::string>()); 
                        auto id = relative.resolve(context.get_base_uri()); 
                        validators.push_back(get_or_create_reference(evaluation_context(eval_context, "$ref"), id));
                        known_keywords.insert("$ref");
                        schema_validator_ptr = jsoncons::make_unique<object_schema_validator<Json>>(eval_context.eval_path(),
                            new_context.get_absolute_uri(),
                            std::move(validators), std::move(default_value));
                    }
                    else
                    {
                        schema_validator_ptr = make_object_schema_validator(eval_context, new_context, sch);
                    }
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

        schema_validator_type make_object_schema_validator(const evaluation_context& eval_context, 
            const compilation_context& context, const Json& sch)
        {
            Json default_value{ jsoncons::null_type() };
            std::vector<keyword_validator_type> validators;
            std::set<std::string> known_keywords;

            auto it = sch.find("default");
            if (it != sch.object_range().end()) 
            {
                default_value = it->value();
                known_keywords.insert("default");
            }
            it = sch.find("type");
            if (it != sch.object_range().end()) 
            {
                validators.push_back(make_type_validator(evaluation_context(eval_context, "type"), context, it->value()));
                known_keywords.insert("type");
            }

            it = sch.find("enum");
            if (it != sch.object_range().end()) 
            {
                validators.push_back(make_enum_validator(evaluation_context(eval_context, "enum"), context, it->value()));
                known_keywords.insert("enum");
            }

            it = sch.find("const");
            if (it != sch.object_range().end()) 
            {
                validators.push_back(make_const_validator(evaluation_context{eval_context, "const"}, context, it->value()));
                known_keywords.insert("const");
            }

            it = sch.find("not");
            if (it != sch.object_range().end()) 
            {
                validators.push_back(make_not_validator(evaluation_context{eval_context, "not"}, 
                    context, it->value()));
                known_keywords.insert("not");
            }

            it = sch.find("allOf");
            if (it != sch.object_range().end()) 
            {
                validators.push_back(make_all_of_validator(evaluation_context{eval_context, "allOf"}, context, it->value()));
                known_keywords.insert("allOf");
            }

            it = sch.find("anyOf");
            if (it != sch.object_range().end()) 
            {
                validators.push_back(make_any_of_validator(evaluation_context{eval_context, "anyOf"}, context, it->value()));
                known_keywords.insert("anyOf");
            }

            it = sch.find("oneOf");
            if (it != sch.object_range().end()) 
            {
                validators.push_back(make_one_of_validator(evaluation_context{eval_context, "oneOf"}, context, it->value()));
                known_keywords.insert("oneOf");
            }

            it = sch.find("if");
            if (it != sch.object_range().end()) 
            {
                validators.push_back(make_conditional_validator(evaluation_context{eval_context, "if"}, context, it->value(), sch));
                known_keywords.insert("if");
                // sch["if"] is object and has id, can be looked up
            }
            else
            {
                auto then_it = sch.find("then");
                if (then_it != sch.object_range().end()) 
                {
                    std::string sub_keys[] = { "then" };
                    subschemas_.emplace_back(make_schema_validator(evaluation_context{eval_context, "then"}, 
                        context, then_it->value(), sub_keys));
                    known_keywords.insert("then");
                }

                auto else_it = sch.find("else");
                if (else_it != sch.object_range().end()) 
                {
                    std::string sub_keys[] = { "else" };
                    subschemas_.emplace_back(make_schema_validator(evaluation_context{evaluation_context{eval_context, "else"}, 
                        sub_keys}, context, else_it->value(), sub_keys));
                    known_keywords.insert("else");
                }
            }

            // Object validators

            it = sch.find("maxProperties");
            if (it != sch.object_range().end()) 
            {
                auto max_properties = it->value().template as<std::size_t>();
                validators.emplace_back(jsoncons::make_unique<max_properties_validator<Json>>(eval_context.eval_path(), 
                    context.make_schema_path_with("maxProperties"), max_properties));
            }

            it = sch.find("minProperties");
            if (it != sch.object_range().end()) 
            {
                auto min_properties = it->value().template as<std::size_t>();
                validators.emplace_back(jsoncons::make_unique<min_properties_validator<Json>>(eval_context.eval_path(), 
                    context.make_schema_path_with("minProperties"), min_properties));
            }

            it = sch.find("required");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(jsoncons::make_unique<required_validator<Json>>(eval_context.eval_path(), 
                    context.make_schema_path_with("required"), it->value().template as<std::vector<std::string>>()));
            }

            std::unique_ptr<properties_validator<Json>> properties;
            it = sch.find("properties");
            if (it != sch.object_range().end()) 
            {
                properties = make_properties_validator(evaluation_context(eval_context, "properties"),
                    context, it->value());
            }
            std::unique_ptr<pattern_properties_validator<Json>> pattern_properties;

    #if defined(JSONCONS_HAS_STD_REGEX)
            it = sch.find("patternProperties");
            if (it != sch.object_range().end())
            {
                pattern_properties = make_pattern_properties_validator(evaluation_context(eval_context, "patternProperties"),
                    context, it->value());
            }
    #endif

            it = sch.find("additionalProperties");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_additional_properties_validator(evaluation_context(eval_context, "additionalProperties"),
                    context, it->value(), std::move(properties), std::move(pattern_properties)));
            }
            else
            {
                validators.emplace_back(make_additional_properties_validator(evaluation_context(eval_context, "additionalProperties"),
                    context, Json(true), std::move(properties), std::move(pattern_properties)));
            }

            it = sch.find("dependencies");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_dependencies_validator(evaluation_context(eval_context, "dependencies"),
                    context, it->value()));
            }

            auto property_names_it = sch.find("propertyNames");
            if (property_names_it != sch.object_range().end()) 
            {
                validators.emplace_back(make_property_names_validator(evaluation_context(eval_context, "propertyNames"),
                    context, it->value()));
            }

            // Array validators

            it = sch.find("maxItems");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_max_items_validator(evaluation_context{eval_context, "maxItems"}, context, it->value()));
            }

            it = sch.find("minItems");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_min_items_validator(evaluation_context{eval_context, "minItems"}, context, it->value()));
            }

            it = sch.find("uniqueItems");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_unique_items_validator(evaluation_context{eval_context, "uniqueItems"}, context, it->value()));
            }

            it = sch.find("items");
            if (it != sch.object_range().end()) 
            {

                if (it->value().type() == json_type::array_value) 
                {
                    validators.emplace_back(make_items_array_validator(evaluation_context{eval_context, "items"}, context, sch, it->value()));
                } 
                else if (it->value().type() == json_type::object_value ||
                           it->value().type() == json_type::bool_value)
                {
                    validators.emplace_back(make_items_object_validator(evaluation_context{eval_context, "items"}, context, sch, it->value()));
                }
            }

            it = sch.find("contains");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_contains_validator(evaluation_context{eval_context, "contains"}, context, sch, it->value()));
            }

            // integer and number

            it = sch.find("maximum");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_maximum_validator(evaluation_context{eval_context, "maximum"}, context, it->value()));
            }

            it = sch.find("minimum");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_minimum_validator(evaluation_context{eval_context, "minimum"}, context, it->value()));
            }

            it = sch.find("exclusiveMaximum");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_exclusive_maximum_validator(evaluation_context{eval_context, "exclusiveMaximum"}, context, it->value()));
            }

            it = sch.find("exclusiveMinimum");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_exclusive_minimum_validator(evaluation_context{eval_context, "exclusiveMinimum"}, context, it->value()));
            }

            it = sch.find("multipleOf");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_multiple_of_validator(evaluation_context{eval_context, "multipleOf"}, context, it->value()));
            }

            // string validators

            it = sch.find("maxLength");
            if (it != sch.object_range().end())
            {
                validators.emplace_back(make_max_length_validator(evaluation_context{eval_context, "maxLength"}, context, it->value()));
            }

            it = sch.find("minLength");
            if (it != sch.object_range().end())
            {
                validators.emplace_back(make_min_length_validator(evaluation_context{eval_context, "minLength"}, context, it->value()));
            }

            it = sch.find("contentEncoding");
            if (it != sch.object_range().end())
            {
                validators.emplace_back(make_content_encoding_validator(evaluation_context{eval_context, "contentEncoding"}, context, it->value()));
                // If "contentEncoding" is set to "binary", a Json value
                // of type json_type::byte_string_value is accepted.
            }

            it = sch.find("contentMediaType");
            if (it != sch.object_range().end())
            {
                validators.emplace_back(make_content_media_type_validator(evaluation_context{eval_context, "contentMediaType"}, context, it->value()));
            }

#if defined(JSONCONS_HAS_STD_REGEX)
            it = sch.find("pattern");
            if (it != sch.object_range().end())
            {
                validators.emplace_back(make_pattern_validator(evaluation_context{eval_context, "pattern"}, context, it->value()));
            }
#endif

            it = sch.find("format");
            if (it != sch.object_range().end())
            {
                validators.emplace_back(make_format_validator(evaluation_context{eval_context, "format"}, context, it->value()));
            }
            
            return jsoncons::make_unique<object_schema_validator<Json>>(eval_context.eval_path(),
                context.get_absolute_uri(),
                std::move(validators), std::move(default_value));
        }

        std::unique_ptr<type_validator<Json>> make_type_validator(const evaluation_context& eval_context, const compilation_context& context, 
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

            return jsoncons::make_unique<type_validator<Json>>(eval_context.eval_path(), std::move(schema_path), 
                std::move(expected_types)
         );
        }

        std::unique_ptr<content_encoding_validator<Json>> make_content_encoding_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("contentEncoding");
            if (!sch.is_string())
            {
                std::string message("contentEncoding must be a string");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::string>();
            return jsoncons::make_unique<content_encoding_validator<Json>>(eval_context.eval_path(), schema_path, value);
        }

        std::unique_ptr<content_media_type_validator<Json>> make_content_media_type_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("contentMediaType");
            if (!sch.is_string())
            {
                std::string message("contentMediaType must be a string");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::string>();
            return jsoncons::make_unique<content_media_type_validator<Json>>(eval_context.eval_path(), schema_path, value);
        }

        std::unique_ptr<format_validator<Json>> make_format_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
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

            return jsoncons::make_unique<format_validator<Json>>(eval_context.eval_path(), schema_path, 
                format_check);
        }

        std::unique_ptr<pattern_validator<Json>> make_pattern_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("pattern");
            auto pattern_string = sch.template as<std::string>();
            auto regex = std::regex(pattern_string, std::regex::ECMAScript);
            return jsoncons::make_unique<pattern_validator<Json>>(eval_context.eval_path(), schema_path, 
                pattern_string, regex);
        }

        std::unique_ptr<max_length_validator<Json>> make_max_length_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("maxLength");
            if (!sch.is_number())
            {
                std::string message("maxLength must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<max_length_validator<Json>>(eval_context.eval_path(), schema_path, value);
        }

        std::unique_ptr<max_items_validator<Json>> make_max_items_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("maxItems");
            if (!sch.is_number())
            {
                std::string message("maxItems must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<max_items_validator<Json>>(eval_context.eval_path(), schema_path, value);
        }

        std::unique_ptr<min_items_validator<Json>> make_min_items_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("minItems");
            if (!sch.is_number())
            {
                std::string message("minItems must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<min_items_validator<Json>>(eval_context.eval_path(), schema_path, value);
        }

        std::unique_ptr<contains_validator<Json>> make_contains_validator(const evaluation_context& eval_context, const compilation_context& context,
            const Json& /*parent*/, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("contains");

            std::string sub_keys[] = { "contains" };

            return jsoncons::make_unique<contains_validator<Json>>(eval_context.eval_path(), schema_path, 
                make_schema_validator(evaluation_context{}, context, sch, sub_keys));
        }

        std::unique_ptr<items_array_validator<Json>> make_items_array_validator(const evaluation_context& eval_context, const compilation_context& context, 
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

                    item_validators.emplace_back(make_schema_validator(evaluation_context{}, context, subsch, sub_keys));
                }

                auto it = parent.find("additionalItems");
                if (it != parent.object_range().end()) 
                {
                    std::string sub_keys[] = {"additionalItems"};
                    additional_items_validator = make_schema_validator(evaluation_context{}, context, it->value(), sub_keys);
                }
            }

            return jsoncons::make_unique<items_array_validator<Json>>(eval_context.eval_path(), schema_path, 
                std::move(item_validators), std::move(additional_items_validator));
        }

        std::unique_ptr<items_object_validator<Json>> make_items_object_validator(const evaluation_context& eval_context, const compilation_context& context, 
            const Json& /* parent */, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("items");

            std::string sub_keys[] = {"items"};

            return jsoncons::make_unique<items_object_validator<Json>>(eval_context.eval_path(), schema_path, 
                make_schema_validator(evaluation_context{}, context, sch, sub_keys));
        }

        std::unique_ptr<unique_items_validator<Json>> make_unique_items_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("uniqueItems");
            bool are_unique = sch.template as<bool>();
            return jsoncons::make_unique<unique_items_validator<Json>>(eval_context.eval_path(), schema_path, are_unique);
        }

        std::unique_ptr<min_length_validator<Json>> make_min_length_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("minLength");
            if (!sch.is_number())
            {
                std::string message("minLength must be an integer value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<min_length_validator<Json>>(eval_context.eval_path(), schema_path, value);
        }

        std::unique_ptr<not_validator<Json>> make_not_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("not");
            std::string not_key[] = { "not" };
            return jsoncons::make_unique<not_validator<Json>>(eval_context.eval_path(), schema_path, 
                make_schema_validator(evaluation_context{}, context, sch, not_key));
        }

        std::unique_ptr<maximum_validator<Json>> make_maximum_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("maximum");
            if (!sch.is_number())
            {
                std::string message("maximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            return jsoncons::make_unique<maximum_validator<Json>>(eval_context.eval_path(), schema_path, sch);
        }

        std::unique_ptr<exclusive_maximum_validator<Json>> make_exclusive_maximum_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("exclusiveMaximum");
            if (!sch.is_number())
            {
                std::string message("exclusiveMaximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            return jsoncons::make_unique<exclusive_maximum_validator<Json>>(eval_context.eval_path(), schema_path, sch);
        }

        std::unique_ptr<minimum_validator<Json>> make_minimum_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("minimum");

            //std::cout << "make_minimum_validator uri: " << context.get_absolute_uri().string() << ", " << schema_path.string() << "\n";
                
            if (!sch.is_number())
            {
                std::string message("minimum must be an integer");
                JSONCONS_THROW(schema_error(message));
            }
            return jsoncons::make_unique<minimum_validator<Json>>(eval_context.eval_path(), schema_path, sch);
        }

        std::unique_ptr<exclusive_minimum_validator<Json>> make_exclusive_minimum_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("exclusiveMinimum");
            if (!sch.is_number())
            {
                std::string message("exclusiveMinimum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            return jsoncons::make_unique<exclusive_minimum_validator<Json>>(eval_context.eval_path(), schema_path, sch);
        }

        std::unique_ptr<multiple_of_validator<Json>> make_multiple_of_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("multipleOf");
            if (!sch.is_number())
            {
                std::string message("multipleOf must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<double>();
            return jsoncons::make_unique<multiple_of_validator<Json>>(eval_context.eval_path(), schema_path, value);
        }

        std::unique_ptr<const_validator<Json>> make_const_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("const");
            return jsoncons::make_unique<const_validator<Json>>(eval_context.eval_path(), schema_path, sch);
        }

        std::unique_ptr<enum_validator<Json>> make_enum_validator(const evaluation_context& eval_context, const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("enum");
            return jsoncons::make_unique<enum_validator<Json>>(eval_context.eval_path(), schema_path, sch);
        }

        std::unique_ptr<required_validator<Json>> make_required_validator(const evaluation_context& eval_context, const compilation_context& context,
            const std::vector<std::string>& items)
        {
            uri schema_path = context.make_schema_path_with("required");
            return jsoncons::make_unique<required_validator<Json>>(eval_context.eval_path(), schema_path, items);
        }


        std::unique_ptr<conditional_validator<Json>> make_conditional_validator(const evaluation_context& eval_context, const compilation_context& context,
            const Json& sch_if, const Json& sch)
        {
            std::string schema_path = context.get_absolute_uri().string();
            schema_validator_type if_validator(nullptr);
            schema_validator_type then_validator(nullptr);
            schema_validator_type else_validator(nullptr);

            std::string if_key[] = { "if" };
            if_validator = make_schema_validator(evaluation_context{}, context, sch_if, if_key);

            auto then_it = sch.find("then");
            if (then_it != sch.object_range().end()) 
            {
                std::string then_key[] = { "then" };
                then_validator = make_schema_validator(evaluation_context{}, context, then_it->value(), then_key);
            }

            auto else_it = sch.find("else");
            if (else_it != sch.object_range().end()) 
            {
                std::string else_key[] = { "else" };
                else_validator = make_schema_validator(evaluation_context{}, context, else_it->value(), else_key);
            }

            return jsoncons::make_unique<conditional_validator<Json>>(eval_context.eval_path(), std::move(schema_path),
                std::move(if_validator), std::move(then_validator), std::move(else_validator));
        }

        std::unique_ptr<combining_validator<Json,all_of_criterion<Json>>> make_all_of_validator(const evaluation_context& eval_context, const compilation_context& context,
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("allOf");
            std::vector<schema_validator_type> subschemas;

            size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                std::string sub_keys[] = { all_of_criterion<Json>::key(), std::to_string(c++) };
                subschemas.emplace_back(make_schema_validator(evaluation_context{}, context, subsch, sub_keys));
            }
            return jsoncons::make_unique<combining_validator<Json,all_of_criterion<Json>>>(eval_context.eval_path(), std::move(schema_path), std::move(subschemas));
        }

        std::unique_ptr<combining_validator<Json,any_of_criterion<Json>>> make_any_of_validator(const evaluation_context& eval_context, const compilation_context& context,
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("anyOf");
            std::vector<schema_validator_type> subschemas;

            size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                std::string sub_keys[] = { any_of_criterion<Json>::key(), std::to_string(c++) };
                subschemas.emplace_back(make_schema_validator(evaluation_context{}, context, subsch, sub_keys));
            }
            return jsoncons::make_unique<combining_validator<Json,any_of_criterion<Json>>>(eval_context.eval_path(), std::move(schema_path), std::move(subschemas));
        }

        std::unique_ptr<combining_validator<Json,one_of_criterion<Json>>> make_one_of_validator(const evaluation_context& eval_context, const compilation_context& context,
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("oneOf");
            std::vector<schema_validator_type> subschemas;

            size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                std::string sub_keys[] = { one_of_criterion<Json>::key(), std::to_string(c++) };
                subschemas.emplace_back(make_schema_validator(evaluation_context{}, context, subsch, sub_keys));
            }
            return jsoncons::make_unique<combining_validator<Json,one_of_criterion<Json>>>(eval_context.eval_path(), std::move(schema_path), std::move(subschemas));
        }

        std::unique_ptr<legacy_object_validator<Json>> make_legacy_object_validator(const evaluation_context& eval_context, const compilation_context& context,
            const Json& /*sch*/)
        {

            uri schema_path = context.get_absolute_uri();
            std::vector<keyword_validator_type> validators;
            std::map<std::string, schema_validator_type> properties;
        #if defined(JSONCONS_HAS_STD_REGEX)
            std::vector<std::pair<std::regex, schema_validator_type>> pattern_properties;
        #endif
            schema_validator_type additional_properties;
            std::map<std::string, keyword_validator_type> dependent_required;
            std::map<std::string, schema_validator_type> dependent_schemas;
            schema_validator_type property_names_validator;
/*
            auto it = sch.find("properties");
            if (it != sch.object_range().end()) 
            {
                for (const auto& prop : it->value().object_range())
                {
                    std::string sub_keys[] = {"properties", prop.key()};
                    properties.emplace(
                        std::make_pair(
                            prop.key(),
                            make_schema_validator(evaluation_context{}, context, prop.value(), sub_keys)));
                }
            }

    #if defined(JSONCONS_HAS_STD_REGEX)
            it = sch.find("patternProperties");
            if (it != sch.object_range().end()) 
            {
                for (const auto& prop : it->value().object_range())
                {
                    std::string sub_keys[] = {prop.key()};
                    pattern_properties.emplace_back(
                        std::make_pair(
                            std::regex(prop.key(), std::regex::ECMAScript),
                            make_schema_validator(evaluation_context{}, context, prop.value(), sub_keys)));
                }
            }
    #endif

            it = sch.find("additionalProperties");
            if (it != sch.object_range().end()) 
            {
                std::string sub_keys[] = {"additionalProperties"};
                additional_properties = make_schema_validator(evaluation_context{eval_context, "additionalProperties"}, context, it->value(), sub_keys);
            }

            it = sch.find("dependencies");
            if (it != sch.object_range().end()) 
            {
                for (const auto& dep : it->value().object_range())
                {
                    switch (dep.value().type()) 
                    {
                        case json_type::array_value:
                        {
                            auto location = context.make_schema_path_with("dependencies");
                            dependent_required.emplace(dep.key(), 
                                make_required_validator(evaluation_context{eval_context, "dependencies"}, compilation_context(nullptr, std::vector<schema_location>{{location}}),
                                    dep.value().template as<std::vector<std::string>>()));
                            break;
                        }
                        case json_type::object_value:
                        {
                            std::string sub_keys[] = {"dependencies"};
                            dependent_schemas.emplace(dep.key(),
                                make_schema_validator(evaluation_context{eval_context, "dependencies"}, context, dep.value(), sub_keys));
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
            }

            auto property_names_it = sch.find("propertyNames");
            if (property_names_it != sch.object_range().end()) 
            {
                std::string sub_keys[] = {"propertyNames"};
                property_names_validator = make_schema_validator(evaluation_context{}, context, property_names_it->value(), sub_keys);
            }
*/
            return jsoncons::make_unique<legacy_object_validator<Json>>(eval_context.eval_path(), std::move(schema_path),
                std::move(validators),
                std::move(properties),
#if defined(JSONCONS_HAS_STD_REGEX)
                std::move(pattern_properties),
#endif
                std::move(additional_properties),
                std::move(dependent_required),
                std::move(dependent_schemas),
                std::move(property_names_validator)
     );
        }
        
        std::unique_ptr<properties_validator<Json>> make_properties_validator(const evaluation_context& eval_context,
                                                                              const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.get_absolute_uri();
            std::map<std::string, schema_validator_type> properties;

            for (const auto& prop : sch.object_range())
            {
                std::string sub_keys[] =
                {"properties", prop.key()};
                properties.emplace(std::make_pair(prop.key(), 
                    make_schema_validator(eval_context, context, prop.value(), sub_keys)));
            }

            return jsoncons::make_unique<properties_validator<Json>>(eval_context.eval_path(),
                                                                     std::move(schema_path), std::move(properties));
        }

#if defined(JSONCONS_HAS_STD_REGEX)
                
        std::unique_ptr<pattern_properties_validator<Json>> make_pattern_properties_validator(const evaluation_context& eval_context, const compilation_context& context, 
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
                        make_schema_validator(eval_context, context, prop.value(), sub_keys)));
            }

            return jsoncons::make_unique<pattern_properties_validator<Json>>(eval_context.eval_path(), std::move(schema_path),
                std::move(pattern_properties));
        }
#endif
       

        std::unique_ptr<additional_properties_validator<Json>> make_additional_properties_validator(const evaluation_context& eval_context, 
            const compilation_context& context, const Json& sch, 
            std::unique_ptr<properties_validator<Json>>&& properties, std::unique_ptr<pattern_properties_validator<Json>>&& pattern_properties)
        {
            uri schema_path = context.get_absolute_uri();
            std::vector<keyword_validator_type> validators;
            schema_validator_type additional_properties;

            std::string sub_keys[] = {"additionalProperties"};
            additional_properties = make_schema_validator(evaluation_context{eval_context, "additionalProperties"}, context, sch, sub_keys);

            return jsoncons::make_unique<additional_properties_validator<Json>>(eval_context.eval_path(), std::move(schema_path),
                std::move(properties), std::move(pattern_properties),
                std::move(additional_properties));
        }
                

        std::unique_ptr<dependencies_validator<Json>> make_dependencies_validator(const evaluation_context& eval_context, const compilation_context& context, 
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
                            make_required_validator(evaluation_context{eval_context, "dependencies"}, compilation_context(nullptr, std::vector<schema_location>{{location}}),
                                dep.value().template as<std::vector<std::string>>()));
                        break;
                    }
                    case json_type::object_value:
                    {
                        std::string sub_keys[] = {"dependencies"};
                        dependent_schemas.emplace(dep.key(),
                            make_schema_validator(evaluation_context{eval_context, "dependencies"}, context, dep.value(), sub_keys));
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            

            return jsoncons::make_unique<dependencies_validator<Json>>(eval_context.eval_path(), std::move(schema_path),
                std::move(dependent_required), std::move(dependent_schemas));
        }


        std::unique_ptr<property_names_validator<Json>> make_property_names_validator(const evaluation_context& eval_context, 
            const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.get_absolute_uri();
            schema_validator_type property_names_schema_validator;

            std::string sub_keys[] = { "propertyNames"};
            property_names_schema_validator = make_schema_validator(eval_context, context, sch, sub_keys);

            return jsoncons::make_unique<property_names_validator<Json>>(eval_context.eval_path(), std::move(schema_path),
                std::move(property_names_schema_validator));
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
                    if (sv.find("json-schema.org/draft-07/schema#") == string_view::npos)
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
            root_ = make_schema_validator(evaluation_context{}, context, sch, {});

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
                            subschemas_.emplace_back(make_schema_validator(evaluation_context{}, compilation_context(schema_location(loc)), external_sch, {}));
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
                bool first = true;
                if (!file.second.unresolved.empty())
                {
                    std::string message = "After all files have been parsed, '" + 
                        (file.first == "" ? "<root>" : file.first) +"' still has " + std::to_string(file.second.unresolved.size()) + " undefined references ";
                    for (auto& item : file.second.unresolved)
                    {
                        if (!first)
                        {
                            message.append(", ");
                            first = false;
                        }
                        message.append("'" + item.first + "' ");
                    }
                    JSONCONS_THROW(schema_error(message));
                }
            }
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

            // is there an unresolved reference to this newly inserted schema?
            auto unresolved_it = file.unresolved.find(std::string(uri.fragment()));
            if (unresolved_it != file.unresolved.end()) 
            {
                unresolved_it->second->set_referred_schema(s->make_copy(unresolved_it->second->eval_path()));
                file.unresolved.erase(unresolved_it);
            }
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
                auto unresolved = file.unresolved.find(fragment);
                if (unresolved != file.unresolved.end())
                    subschemas_.emplace_back(make_schema_validator(evaluation_context{}, compilation_context(new_uri), value, {}));
                else // no, nothing ref'd it, keep for later
                    file.unknown_keywords[fragment] = value;

                // recursively add possible subschemas of unknown keywords
                if (value.type() == json_type::object_value)
                    for (const auto& subsch : value.object_range())
                    {
                        insert_unknown_keyword(new_uri, subsch.key(), subsch.value());
                    }
            }
        }

        keyword_validator_type get_or_create_reference(const evaluation_context& eval_context, const schema_location& uri)
        {
            auto &file = get_or_create_file(uri.base().string());

            // a schema already exists
            auto sch = file.schemas.find(std::string(uri.fragment()));
            if (sch != file.schemas.end())
            {
                return jsoncons::make_unique<ref_validator_type>(eval_context.eval_path(), uri.base(), sch->second->make_copy(eval_context.eval_path()));
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
                    auto s = make_schema_validator(evaluation_context{}, compilation_context(uri), subsch, {}); 
                    //auto p = s.get();
                    //subschemas_.emplace_back(std::move(s));
                    file.unknown_keywords.erase(unprocessed_keywords_it);
                    return jsoncons::make_unique<ref_validator_type>(eval_context.eval_path(), uri.base(), std::move(s));
                }
            }

            // get or create a ref_validator
            auto it = file.unresolved.find(std::string(uri.fragment()));
            if (it != file.unresolved.end()) 
            {
                return jsoncons::make_unique<keyword_validator_wrapper_type>(it->second);
            }
            else 
            {
                auto orig = jsoncons::make_unique<ref_validator_type>(eval_context.eval_path(), uri.base());
                auto p = orig.get();
                file.unresolved.insert(it, {std::string(uri.fragment()), p});

                subschemas_.emplace_back(std::move(orig));
                return jsoncons::make_unique<keyword_validator_wrapper_type>(p);
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

} // namespace draft7
} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_DRAFT7_KEYWORD_FACTORY_HPP

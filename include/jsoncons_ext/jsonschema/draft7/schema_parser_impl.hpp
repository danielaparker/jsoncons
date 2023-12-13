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
#include <jsoncons_ext/jsonschema/json_schema.hpp>
#include <jsoncons_ext/jsonschema/common/keywords.hpp>
#include <jsoncons_ext/jsonschema/schema_version.hpp>
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

        validator_type make_subschema_validator(const Json& schema,
            const compilation_context& context,
            const std::vector<std::string>& keys) //override
        {
            auto new_context = context.update_uris(schema, keys);

            validator_pointer sch = nullptr;

            bool is_ref = false;
            switch (schema.type())
            {
                case json_type::bool_value:
                    if (schema.template as<bool>())
                    {
                        auto ref =  make_true_validator(new_context);
                        sch = ref.get();
                        subschemas_.emplace_back(std::move(ref));
                    }
                    else
                    {
                        auto ref = make_false_validator(new_context);
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
                        auto ref = make_type_validator(schema, new_context);
                        sch = ref.get();
                        subschemas_.emplace_back(std::move(ref));
                    }
                    break;
                }
                default:
                    JSONCONS_THROW(schema_error("invalid JSON-type for a schema for " + new_context.get_absolute_uri().string() + ", expected: boolean or object"));
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

        void init_type_mapping(std::vector<validator_type>& type_mapping, const std::string& type,
            const Json& schema,
            const compilation_context& context,
            std::set<std::string>& keywords)
        {
            if (type == "null")
            {
                type_mapping[(uint8_t)json_type::null_value] = make_null_validator(context);
            }
            else if (type == "object")
            {
                type_mapping[(uint8_t)json_type::object_value] = make_object_validator(schema, context);
            }
            else if (type == "array")
            {
                type_mapping[(uint8_t)json_type::array_value] = make_array_validator(schema, context);
            }
            else if (type == "string")
            {
                type_mapping[(uint8_t)json_type::string_value] = make_string_validator(schema, context);
                // For binary types
                type_mapping[(uint8_t) json_type::byte_string_value] = make_string_validator(schema, context);
            }
            else if (type == "boolean")
            {
                type_mapping[(uint8_t)json_type::bool_value] = make_boolean_validator(context);
            }
            else if (type == "integer")
            {
                type_mapping[(uint8_t)json_type::int64_value] = make_integer_validator(schema, context, keywords);
                type_mapping[(uint8_t)json_type::uint64_value] = make_integer_validator(schema, context, keywords);
                type_mapping[(uint8_t)json_type::double_value] = make_integer_validator(schema, context, keywords);
            }
            else if (type == "number")
            {
                type_mapping[(uint8_t)json_type::double_value] = make_number_validator(schema, context, keywords);
                type_mapping[(uint8_t)json_type::int64_value] =  make_number_validator(schema, context, keywords);
                type_mapping[(uint8_t)json_type::uint64_value] =  make_number_validator(schema, context, keywords);
            }
            else if (type.empty())
            {
                type_mapping[(uint8_t)json_type::null_value] = make_null_validator(context);
                type_mapping[(uint8_t)json_type::object_value] = make_object_validator(schema, context);
                type_mapping[(uint8_t)json_type::array_value] = make_array_validator(schema, context);
                type_mapping[(uint8_t)json_type::string_value] = make_string_validator(schema, context);
                // For binary types
                type_mapping[(uint8_t) json_type::byte_string_value] = make_string_validator(schema, context);
                type_mapping[(uint8_t)json_type::bool_value] = make_boolean_validator(context);
                type_mapping[(uint8_t)json_type::int64_value] = make_integer_validator(schema, context, keywords);
                type_mapping[(uint8_t)json_type::uint64_value] = make_integer_validator(schema, context, keywords);
                type_mapping[(uint8_t)json_type::double_value] = make_integer_validator(schema, context, keywords);
                type_mapping[(uint8_t)json_type::double_value] = make_number_validator(schema, context, keywords);
                type_mapping[(uint8_t)json_type::int64_value] = make_number_validator(schema, context, keywords);
                type_mapping[(uint8_t)json_type::uint64_value] = make_number_validator(schema, context, keywords);
            }
        }

        std::unique_ptr<type_validator<Json>> make_type_validator(const Json& schema,
            const compilation_context& context)
        {
            std::string schema_path = context.get_absolute_uri().string();
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
                init_type_mapping(type_mapping, "", schema, context, known_keywords);
            }
            else 
            {
                switch (it->value().type()) 
                { 
                    case json_type::string_value: 
                    {
                        auto type = it->value().template as<std::string>();
                        init_type_mapping(type_mapping, type, schema, context, known_keywords);
                        expected_types.emplace_back(std::move(type));
                        break;
                    } 

                    case json_type::array_value: // "type": ["type1", "type2"]
                    {
                        for (const auto& item : it->value().array_range())
                        {
                            auto type = item.template as<std::string>();
                            init_type_mapping(type_mapping, type, schema, context, known_keywords);
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
                enumvalidator = enum_validator<Json>(context.get_absolute_uri().string(), it->value());
            }

            it = schema.find("const");
            if (it != schema.object_range().end()) 
            {
                const_validator = const_keyword<Json>(context.get_absolute_uri().string(), it->value());
            }

            it = schema.find("not");
            if (it != schema.object_range().end()) 
            {
                combined_validators.emplace_back(make_not_validator(it->value(), context));
            }

            it = schema.find("allOf");
            if (it != schema.object_range().end()) 
            {
                combined_validators.emplace_back(make_all_of_validator(it->value(), context));
            }

            it = schema.find("anyOf");
            if (it != schema.object_range().end()) 
            {
                combined_validators.emplace_back(make_any_of_validator(it->value(), context));
            }

            it = schema.find("oneOf");
            if (it != schema.object_range().end()) 
            {
                combined_validators.emplace_back(make_one_of_validator(it->value(), context));
            }

            it = schema.find("if");
            if (it != schema.object_range().end()) 
            {
                conditionalvalidator = make_conditional_validator(it->value(), schema, context);
                // schema["if"] is object and has id, can be looked up
            }
            else
            {
                auto then_it = schema.find("then");
                if (then_it != schema.object_range().end()) 
                {
                    make_subschema_validator(then_it->value(), context, {"then"});
                }

                auto else_it = schema.find("else");
                if (else_it != schema.object_range().end()) 
                {
                    make_subschema_validator(else_it->value(), context, {"else"});
                }
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

        std::unique_ptr<content_encoding_validator<Json>> make_content_encoding_validator(const Json& schema, const compilation_context& context)
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

        std::unique_ptr<string_validator<Json>> make_string_validator(const Json& schema,
            const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("string");
            auto new_context = context.update_uris(schema, schema_path);

            std::vector<validator_type> validators;
            auto it = schema.find("maxLength");
            if (it != schema.object_range().end())
            {
                validators.emplace_back(make_max_length_validator(it->value(), new_context));
            }

            it = schema.find("minLength");
            if (it != schema.object_range().end())
            {
                validators.emplace_back(make_min_length_validator(it->value(), new_context));
            }

            it = schema.find("contentEncoding");
            if (it != schema.object_range().end())
            {
                validators.emplace_back(make_content_encoding_validator(it->value(), new_context));
                // If "contentEncoding" is set to "binary", a Json value
                // of type json_type::byte_string_value is accepted.
            }

            it = schema.find("contentMediaType");
            if (it != schema.object_range().end())
            {
                validators.emplace_back(make_content_media_type_validator(it->value(), new_context));
            }

#if defined(JSONCONS_HAS_STD_REGEX)
            it = schema.find("pattern");
            if (it != schema.object_range().end())
            {
                validators.emplace_back(make_pattern_validator(it->value(), new_context));
            }
#endif

            it = schema.find("format");
            if (it != schema.object_range().end())
            {
                validators.emplace_back(make_format_validator(it->value(), new_context));
            }

            return jsoncons::make_unique<string_validator<Json>>(schema_path, std::move(validators));
        }

        std::unique_ptr<content_media_type_validator<Json>> make_content_media_type_validator(const Json& schema, const compilation_context& context)
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

        std::unique_ptr<format_validator<Json>> make_format_validator(const Json& schema, const compilation_context& context)
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

        std::unique_ptr<pattern_validator<Json>> make_pattern_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("pattern");
            auto pattern_string = schema.template as<std::string>();
            auto regex = std::regex(pattern_string, std::regex::ECMAScript);
            return jsoncons::make_unique<pattern_validator<Json>>(schema_path, 
                pattern_string, regex);
        }

        std::unique_ptr<max_length_validator<Json>> make_max_length_validator(const Json& schema, const compilation_context& context)
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

        std::unique_ptr<max_items_validator<Json>> make_max_items_validator(const Json& schema, const compilation_context& context)
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

        std::unique_ptr<min_items_validator<Json>> make_min_items_validator(const Json& schema, const compilation_context& context)
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

        std::unique_ptr<array_validator<Json>> make_array_validator(const Json& schema,
            const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("array");
            auto new_context = context.update_uris(schema, schema_path);

            std::vector<validator_type> validators;

            auto it = schema.find("maxItems");
            if (it != schema.object_range().end()) 
            {
                validators.emplace_back(make_max_items_validator(it->value(), context));
            }

            it = schema.find("minItems");
            if (it != schema.object_range().end()) 
            {
                validators.emplace_back(make_min_items_validator(it->value(), context));
            }

            it = schema.find("uniqueItems");
            if (it != schema.object_range().end()) 
            {
                validators.emplace_back(make_unique_items_validator(it->value(), context));
            }

            it = schema.find("items");
            if (it != schema.object_range().end()) 
            {

                if (it->value().type() == json_type::array_value) 
                {
                    validators.emplace_back(make_items_array_validator(schema, it->value(), 
                        context));
                } 
                else if (it->value().type() == json_type::object_value ||
                           it->value().type() == json_type::bool_value)
                {
                    validators.emplace_back(make_items_object_validator(schema, it->value(), 
                        context));
                }
            }

            it = schema.find("contains");
            if (it != schema.object_range().end()) 
            {
                validators.emplace_back(make_contains_validator(schema, it->value(), 
                    context));
            }

            return jsoncons::make_unique<array_validator<Json>>(schema_path, std::move(validators));
        }

        std::unique_ptr<contains_validator<Json>> make_contains_validator(const Json& /*parent*/, const Json& schema,
            const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("contains");

            return jsoncons::make_unique<contains_validator<Json>>(schema_path, 
                make_subschema_validator(schema, context, { "contains" }));
        }

        std::unique_ptr<items_array_validator<Json>> make_items_array_validator(const Json& parent, const Json& schema, 
            const compilation_context& context)
        {
            std::vector<validator_type> item_validators;
            validator_type additional_items_validator = nullptr;

            std::string schema_path = context.make_schema_path_with("items");

            if (schema.type() == json_type::array_value) 
            {
                size_t c = 0;
                for (const auto& subsch : schema.array_range())
                    item_validators.emplace_back(make_subschema_validator(subsch, context, {"items", std::to_string(c++)}));

                auto it = parent.find("additionalItems");
                if (it != parent.object_range().end()) 
                {
                    additional_items_validator = make_subschema_validator(it->value(), context, {"additionalItems"});
                }
            }

            return jsoncons::make_unique<items_array_validator<Json>>(schema_path, 
                std::move(item_validators), std::move(additional_items_validator));
        }

        std::unique_ptr<items_object_validator<Json>> make_items_object_validator(const Json& /* parent */, const Json& schema, 
            const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("items");

            return jsoncons::make_unique<items_object_validator<Json>>(schema_path, 
                make_subschema_validator(schema, context, { "items" }));
        }

        std::unique_ptr<unique_items_validator<Json>> make_unique_items_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("uniqueItems");
            bool are_unique = schema.template as<bool>();
            return jsoncons::make_unique<unique_items_validator<Json>>(schema_path, are_unique);
        }

        std::unique_ptr<min_length_validator<Json>> make_min_length_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("minLength");
            if (!schema.is_number())
            {
                std::string message("minLength must be an integer value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<std::size_t>();
            return jsoncons::make_unique<min_length_validator<Json>>(schema_path, value);
        }

        std::unique_ptr<not_validator<Json>> make_not_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("not");
            return jsoncons::make_unique<not_validator<Json>>(schema_path, 
                make_subschema_validator(schema, context, {"not"}));
        }

        std::unique_ptr<maximum_validator<Json,int64_t>> make_maximum_integer_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("maximum");
            if (!schema.is_number())
            {
                std::string message("maximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<int64_t>();
            return jsoncons::make_unique<maximum_validator<Json,int64_t>>(schema_path, value);
        }

        std::unique_ptr<maximum_validator<Json,double>> make_maximum_number_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("maximum");
            if (!schema.is_number())
            {
                std::string message("maximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<double>();
            return jsoncons::make_unique<maximum_validator<Json,double>>(schema_path, value);
        }

        std::unique_ptr<exclusive_maximum_validator<Json,int64_t>> make_exclusive_maximum_integer_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("exclusiveMaximum");
            if (!schema.is_number())
            {
                std::string message("exclusiveMaximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<int64_t>();
            return jsoncons::make_unique<exclusive_maximum_validator<Json,int64_t>>(schema_path, value);
        }

        std::unique_ptr<exclusive_maximum_validator<Json,double>> make_exclusive_maximum_number_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("exclusiveMaximum");
            if (!schema.is_number())
            {
                std::string message("exclusiveMaximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<double>();
            return jsoncons::make_unique<exclusive_maximum_validator<Json,double>>(schema_path, value);
        }

        std::unique_ptr<minimum_validator<Json,int64_t>> make_minimum_integer_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("minimum");
            if (!schema.is_number())
            {
                std::string message("minimum must be an integer");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<int64_t>();
            return jsoncons::make_unique<minimum_validator<Json,int64_t>>(schema_path, value);
        }

        std::unique_ptr<minimum_validator<Json,double>> make_minimum_number_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("minimum");
            if (!schema.is_number())
            {
                std::string message("minimum must be an number");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<double>();
            return jsoncons::make_unique<minimum_validator<Json,double>>(schema_path, value);
        }

        std::unique_ptr<exclusive_minimum_validator<Json,int64_t>> make_exclusive_minimum_integer_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("exclusiveMinimum");
            if (!schema.is_number())
            {
                std::string message("exclusiveMinimum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<int64_t>();
            return jsoncons::make_unique<exclusive_minimum_validator<Json,int64_t>>(schema_path, value);
        }

        std::unique_ptr<exclusive_minimum_validator<Json,double>> make_exclusive_minimum_number_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("exclusiveMinimum");
            if (!schema.is_number())
            {
                std::string message("exclusiveMinimum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<double>();
            return jsoncons::make_unique<exclusive_minimum_validator<Json,double>>(schema_path, value);
        }

        std::unique_ptr<multiple_of_validator<Json>> make_multiple_of_validator(const Json& schema, const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("multipleOf");
            if (!schema.is_number())
            {
                std::string message("multipleOf must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = schema.template as<double>();
            return jsoncons::make_unique<multiple_of_validator<Json>>(schema_path, value);
        }

        std::unique_ptr<integer_validator<Json>> make_integer_validator(const Json& schema,
            const compilation_context& context, std::set<std::string>& keywords)
        {
            std::string schema_path = context.make_schema_path_with("integer");
            auto new_context = context.update_uris(schema, schema_path);

            std::vector<validator_type> validators;

            auto it = schema.find("maximum");
            if (it != schema.object_range().end()) 
            {
                keywords.insert("maximum");
                validators.emplace_back(make_maximum_integer_validator(it->value(), new_context));
            }

            it = schema.find("minimum");
            if (it != schema.object_range().end()) 
            {
                keywords.insert("minimum");
                validators.emplace_back(make_minimum_integer_validator(it->value(), new_context));
            }

            it = schema.find("exclusiveMaximum");
            if (it != schema.object_range().end()) 
            {
                keywords.insert("exclusiveMaximum");
                validators.emplace_back(make_exclusive_maximum_integer_validator(it->value(), new_context));
            }

            it = schema.find("exclusiveMinimum");
            if (it != schema.object_range().end()) 
            {
                keywords.insert("exclusiveMinimum");
                validators.emplace_back(make_exclusive_minimum_integer_validator(it->value(), new_context));
            }

            it = schema.find("multipleOf");
            if (it != schema.object_range().end()) 
            {
                keywords.insert("multipleOf");
                validators.emplace_back(make_multiple_of_validator(it->value(), new_context));
            }


            return jsoncons::make_unique<integer_validator<Json>>(schema_path, std::move(validators));
        }

        std::unique_ptr<number_validator<Json>> make_number_validator(const Json& schema,
            const compilation_context& context, std::set<std::string>& keywords)
        {
            std::string schema_path = context.make_schema_path_with("number");
            auto new_context = context.update_uris(schema, schema_path);

            std::vector<validator_type> validators;

            auto it = schema.find("maximum");
            if (it != schema.object_range().end()) 
            {
                keywords.insert("maximum");
                validators.emplace_back(make_maximum_number_validator(it->value(), new_context));
            }

            it = schema.find("minimum");
            if (it != schema.object_range().end()) 
            {
                keywords.insert("minimum");
                validators.emplace_back(make_minimum_number_validator(it->value(), new_context));
            }

            it = schema.find("exclusiveMaximum");
            if (it != schema.object_range().end()) 
            {
                keywords.insert("exclusiveMaximum");
                validators.emplace_back(make_exclusive_maximum_number_validator(it->value(), new_context));
            }

            it = schema.find("exclusiveMinimum");
            if (it != schema.object_range().end()) 
            {
                keywords.insert("exclusiveMinimum");
                validators.emplace_back(make_exclusive_minimum_number_validator(it->value(), new_context));
            }

            it = schema.find("multipleOf");
            if (it != schema.object_range().end()) 
            {
                keywords.insert("multipleOf");
                validators.emplace_back(make_multiple_of_validator(it->value(), new_context));
            }


            return jsoncons::make_unique<number_validator<Json>>(schema_path, std::move(validators));
        }

        std::unique_ptr<null_validator<Json>> make_null_validator(const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("null");
            return jsoncons::make_unique<null_validator<Json>>(schema_path);
        }

        std::unique_ptr<boolean_validator<Json>> make_boolean_validator(const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("boolean");
            return jsoncons::make_unique<boolean_validator<Json>>(schema_path);
        }

        std::unique_ptr<true_validator<Json>> make_true_validator(const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("true");
            return jsoncons::make_unique<true_validator<Json>>(schema_path);
        }

        std::unique_ptr<false_validator<Json>> make_false_validator(const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("false");
            return jsoncons::make_unique<false_validator<Json>>(schema_path);
        }

        std::unique_ptr<required_validator<Json>> make_required_validator(const compilation_context& context,
            const std::vector<std::string>& items)
        {
            std::string schema_path = context.make_schema_path_with("required");
            return jsoncons::make_unique<required_validator<Json>>(schema_path, items);
        }

        conditional_validator<Json> make_conditional_validator(const Json& sch_if, const Json& schema,
            const compilation_context& context)
        {
            std::string schema_path = context.get_absolute_uri().string();
            validator_type if_validator(nullptr);
            validator_type then_validator(nullptr);
            validator_type else_validator(nullptr);

            if_validator = make_subschema_validator(sch_if, context, { "if" });

            auto then_it = schema.find("then");
            if (then_it != schema.object_range().end()) 
            {
                then_validator = make_subschema_validator(then_it->value(), context, {"then"});
            }

            auto else_it = schema.find("else");
            if (else_it != schema.object_range().end()) 
            {
                else_validator = make_subschema_validator(else_it->value(), context, {"else"});
            }

            return conditional_validator<Json>(std::move(schema_path),
                std::move(if_validator), std::move(then_validator), std::move(else_validator));
        }

        std::unique_ptr<combining_validator<Json,all_of_criterion<Json>>> make_all_of_validator(const Json& schema,
            const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("allOf");
            std::vector<validator_type> subschemas;

            size_t c = 0;
            for (const auto& subsch : schema.array_range())
            {
                subschemas.emplace_back(make_subschema_validator(subsch, context, {all_of_criterion<Json>::key(), std::to_string(c++)}));
            }
            return jsoncons::make_unique<combining_validator<Json,all_of_criterion<Json>>>(std::move(schema_path), std::move(subschemas));
        }

        std::unique_ptr<combining_validator<Json,any_of_criterion<Json>>> make_any_of_validator(const Json& schema,
            const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("anyOf");
            std::vector<validator_type> subschemas;

            size_t c = 0;
            for (const auto& subsch : schema.array_range())
            {
                subschemas.emplace_back(make_subschema_validator(subsch, context, {any_of_criterion<Json>::key(), std::to_string(c++)}));
            }
            return jsoncons::make_unique<combining_validator<Json,any_of_criterion<Json>>>(std::move(schema_path), std::move(subschemas));
        }

        std::unique_ptr<combining_validator<Json,one_of_criterion<Json>>> make_one_of_validator(const Json& schema,
            const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("oneOf");
            std::vector<validator_type> subschemas;

            size_t c = 0;
            for (const auto& subsch : schema.array_range())
            {
                subschemas.emplace_back(make_subschema_validator(subsch, context, {one_of_criterion<Json>::key(), std::to_string(c++)}));
            }
            return jsoncons::make_unique<combining_validator<Json,one_of_criterion<Json>>>(std::move(schema_path), std::move(subschemas));
        }

        std::unique_ptr<object_validator<Json>> make_object_validator(const Json& schema,
            const compilation_context& context)
        {
            std::string schema_path = context.make_schema_path_with("object");
            jsoncons::optional<std::size_t> max_properties;
            std::string absolute_max_properties_location;
            jsoncons::optional<std::size_t> min_properties;
            std::string absolute_min_properties_location;
            jsoncons::optional<required_validator<Json>> required;
            std::map<std::string, validator_type> properties;
        #if defined(JSONCONS_HAS_STD_REGEX)
            std::vector<std::pair<std::regex, validator_type>> pattern_properties;
        #endif
            validator_type additional_properties;
            std::map<std::string, validator_type> dependencies;
            validator_type property_name_validator;

            auto it = schema.find("maxProperties");
            if (it != schema.object_range().end()) 
            {
                max_properties = it->value().template as<std::size_t>();
                absolute_max_properties_location = context.make_schema_path_with("maxProperties");
            }

            it = schema.find("minProperties");
            if (it != schema.object_range().end()) 
            {
                min_properties = it->value().template as<std::size_t>();
                absolute_min_properties_location = context.make_schema_path_with("minProperties");
            }

            it = schema.find("required");
            if (it != schema.object_range().end()) 
            {
                auto location = context.make_schema_path_with("required");
                required = required_validator<Json>(location, 
                                                   it->value().template as<std::vector<std::string>>());
            }

            it = schema.find("properties");
            if (it != schema.object_range().end()) 
            {
                for (const auto& prop : it->value().object_range())
                    properties.emplace(
                        std::make_pair(
                            prop.key(),
                            make_subschema_validator(prop.value(), context, {"properties", prop.key()})));
            }

    #if defined(JSONCONS_HAS_STD_REGEX)
            it = schema.find("patternProperties");
            if (it != schema.object_range().end()) 
            {
                for (const auto& prop : it->value().object_range())
                    pattern_properties.emplace_back(
                        std::make_pair(
                            std::regex(prop.key(), std::regex::ECMAScript),
                            make_subschema_validator(prop.value(), context, {prop.key()})));
            }
    #endif

            it = schema.find("additionalProperties");
            if (it != schema.object_range().end()) 
            {
                additional_properties = make_subschema_validator(it->value(), context, {"additionalProperties"});
            }

            it = schema.find("dependencies");
            if (it != schema.object_range().end()) 
            {
                for (const auto& dep : it->value().object_range())
                {
                    switch (dep.value().type()) 
                    {
                        case json_type::array_value:
                        {
                            auto location = context.make_schema_path_with("dependencies");
                            dependencies.emplace(dep.key(), 
                                make_required_validator(compilation_context(std::vector<schema_location>{{location}}),
                                    dep.value().template as<std::vector<std::string>>()));
                            break;
                        }
                        default:
                        {
                            dependencies.emplace(dep.key(),
                                make_subschema_validator(dep.value(), context, {"dependencies", dep.key()}));
                            break;
                        }
                    }
                }
            }

            auto property_names_it = schema.find("propertyNames");
            if (property_names_it != schema.object_range().end()) 
            {
                property_name_validator = make_subschema_validator(property_names_it->value(), context, {"propertyNames"});
            }


            return jsoncons::make_unique<object_validator<Json>>(std::move(schema_path),
                std::move(max_properties), std::move(absolute_max_properties_location),
                std::move(min_properties), std::move(absolute_min_properties_location),
                std::move(required),
                std::move(properties),
#if defined(JSONCONS_HAS_STD_REGEX)
                std::move(pattern_properties),
#endif
                std::move(additional_properties),
                std::move(dependencies),
                std::move(property_name_validator)
            );
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
                    if (!schema_version::contains(sv))
                    {
                        std::string message("Unsupported schema version ");
                        message.append(sv.data(), sv.size());
                        JSONCONS_THROW(schema_error(message));
                    }
                }
            }
            load(sch, compilation_context(schema_location(retrieval_uri)));
        }

        void load(const Json& sch, const compilation_context& context)
        {
            subschema_registries_.clear();
            root_ = make_subschema_validator(sch, context, {});

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
            auto& file = get_or_create_file(uri.base().string());
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
            auto &file = get_or_create_file(uri.base().string());
            auto new_u = uri.append(key);
            schema_location new_uri(new_u);

            if (new_uri.has_fragment() && !new_uri.has_plain_name_fragment()) 
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
            auto &file = get_or_create_file(uri.base().string());

            // a schema already exists
            auto sch = file.schemas.find(std::string(uri.fragment()));
            if (sch != file.schemas.end())
                return jsoncons::make_unique<reference_validator_type>(sch->second);

            // referencing an unknown keyword, turn it into schema
            //
            // an unknown keyword can only be referenced by a JSONPointer,
            // not by a plain name identifier
            if (uri.has_fragment() && !uri.has_plain_name_fragment()) 
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

} // namespace draft7
} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_DRAFT7_KEYWORD_FACTORY_HPP

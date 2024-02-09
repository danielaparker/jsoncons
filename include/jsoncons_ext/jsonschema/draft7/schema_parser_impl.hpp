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

        schema_validator_type make_schema_validator(const compilation_context& context, const Json& sch,
            jsoncons::span<const std::string> keys) //override
        {
            auto new_context = make_compilation_context(context, sch, keys);
            //std::cout << "make_schema_validator " << context.get_absolute_uri().string() << ", " << new_context.get_absolute_uri().string() << "\n\n";

            Json default_value{jsoncons::null_type()};
            schema_validator_type schema_validator_ptr;

            bool is_ref = false;

            std::vector<keyword_validator_type> validators; 
            switch (sch.type())
            {
                case json_type::bool_value:
                {
                    uri schema_path = new_context.get_absolute_uri();
                    if (sch.template as<bool>())
                    {
                        schema_validator_ptr = jsoncons::make_unique<boolean_schema_validator<Json>>(jsonpointer::json_pointer{}, 
                            schema_path, sch.template as<bool>());
                    }
                    else
                    {
                        schema_validator_ptr = jsoncons::make_unique<boolean_schema_validator<Json>>(jsonpointer::json_pointer{}, 
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

                    auto it = sch.find("default");
                    if (it != sch.object_range().end()) 
                    {
                        default_value = it->value();
                        known_keywords.insert("default");
                    }
                    it = sch.find("$ref");
                    if (it != sch.object_range().end()) // this schema is a reference
                    {
                        is_ref = true;
                        schema_location relative(it->value().template as<std::string>()); 
                        auto id = relative.resolve(context.get_base_uri()); 
                        validators.push_back(get_or_create_reference(id));
                        known_keywords.insert("$ref");
                    }
                    it = sch.find("definitions");
                    if (it != sch.object_range().end()) 
                    {
                        for (const auto& def : it->value().object_range())
                        {
                            std::string k[] = { "definitions", def.key() };
                            subschemas_.emplace_back(make_schema_validator(new_context, def.value(), k));
                        }
                        known_keywords.insert("definitions");
                    }
                    if (!is_ref)
                    {
                        validators.push_back(make_type_validator(new_context, sch, known_keywords));

                        it = sch.find("enum");
                        if (it != sch.object_range().end()) 
                        {
                            validators.push_back(make_enum_validator(new_context, it->value()));
                            known_keywords.insert("enum");
                        }

                        it = sch.find("const");
                        if (it != sch.object_range().end()) 
                        {
                            validators.push_back(make_const_validator(new_context, it->value()));
                            known_keywords.insert("const");
                        }

                        it = sch.find("not");
                        if (it != sch.object_range().end()) 
                        {
                            validators.push_back(make_not_validator(new_context, it->value()));
                            known_keywords.insert("not");
                        }

                        it = sch.find("allOf");
                        if (it != sch.object_range().end()) 
                        {
                            validators.push_back(make_all_of_validator(new_context, it->value()));
                            known_keywords.insert("allOf");
                        }

                        it = sch.find("anyOf");
                        if (it != sch.object_range().end()) 
                        {
                            validators.push_back(make_any_of_validator(new_context, it->value()));
                            known_keywords.insert("anyOf");
                        }

                        it = sch.find("oneOf");
                        if (it != sch.object_range().end()) 
                        {
                            validators.push_back(make_one_of_validator(new_context, it->value()));
                            known_keywords.insert("oneOf");
                        }

                        it = sch.find("if");
                        if (it != sch.object_range().end()) 
                        {
                            validators.push_back(make_conditional_validator(new_context, it->value(), sch));
                            known_keywords.insert("if");
                            // sch["if"] is object and has id, can be looked up
                        }
                        else
                        {
                            auto then_it = sch.find("then");
                            if (then_it != sch.object_range().end()) 
                            {
                                std::string sub_keys[] = { "then" };
                                subschemas_.emplace_back(make_schema_validator(new_context, then_it->value(), sub_keys));
                                known_keywords.insert("then");
                            }

                            auto else_it = sch.find("else");
                            if (else_it != sch.object_range().end()) 
                            {
                                std::string sub_keys[] = { "else" };
                                subschemas_.emplace_back(make_schema_validator(new_context, else_it->value(), sub_keys));
                                known_keywords.insert("else");
                            }
                        }
                    }
                    schema_validator_ptr = jsoncons::make_unique<object_schema_validator<Json>>(jsonpointer::json_pointer{},
                        new_context.get_absolute_uri().string(),
                        std::move(validators), std::move(default_value));
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

        void init_type_mapping(std::vector<keyword_validator_type>& type_mapping, const std::string& type,
            const Json& sch,
            const compilation_context& context,
            std::set<std::string>& keywords)
        {
            if (type == "null")
            {
                type_mapping[(uint8_t)json_type::null_value] = make_null_validator(context);
            }
            else if (type == "object")
            {
                type_mapping[(uint8_t)json_type::object_value] = make_object_validator(context, sch);
            }
            else if (type == "array")
            {
                type_mapping[(uint8_t)json_type::array_value] = make_array_validator(context, sch);
            }
            else if (type == "string")
            {
                type_mapping[(uint8_t)json_type::string_value] = make_string_validator(context, sch);
                // For binary types
                type_mapping[(uint8_t) json_type::byte_string_value] = make_string_validator(context, sch);
            }
            else if (type == "boolean")
            {
                type_mapping[(uint8_t)json_type::bool_value] = make_boolean_validator(context);
            }
            else if (type == "integer")
            {
                type_mapping[(uint8_t)json_type::int64_value] = make_integer_validator(context, sch, keywords);
                type_mapping[(uint8_t)json_type::uint64_value] = make_integer_validator(context, sch, keywords);
                type_mapping[(uint8_t)json_type::double_value] = make_integer_validator(context, sch, keywords);
            }
            else if (type == "number")
            {
                type_mapping[(uint8_t)json_type::double_value] = make_number_validator(context, sch, keywords);
                type_mapping[(uint8_t)json_type::int64_value] =  make_number_validator(context, sch, keywords);
                type_mapping[(uint8_t)json_type::uint64_value] =  make_number_validator(context, sch, keywords);
            }
            else if (type.empty())
            {
                type_mapping[(uint8_t)json_type::null_value] = make_null_validator(context);
                type_mapping[(uint8_t)json_type::object_value] = make_object_validator(context, sch);
                type_mapping[(uint8_t)json_type::array_value] = make_array_validator(context, sch);
                type_mapping[(uint8_t)json_type::string_value] = make_string_validator(context, sch);
                // For binary types
                type_mapping[(uint8_t) json_type::byte_string_value] = make_string_validator(context, sch);
                type_mapping[(uint8_t)json_type::bool_value] = make_boolean_validator(context);
                type_mapping[(uint8_t)json_type::int64_value] = make_integer_validator(context, sch, keywords);
                type_mapping[(uint8_t)json_type::uint64_value] = make_integer_validator(context, sch, keywords);
                type_mapping[(uint8_t)json_type::double_value] = make_integer_validator(context, sch, keywords);
                type_mapping[(uint8_t)json_type::double_value] = make_number_validator(context, sch, keywords);
                type_mapping[(uint8_t)json_type::int64_value] = make_number_validator(context, sch, keywords);
                type_mapping[(uint8_t)json_type::uint64_value] = make_number_validator(context, sch, keywords);
            }
        }

        std::unique_ptr<type_validator<Json>> make_type_validator(const compilation_context& context, 
            const Json& sch, std::set<std::string> known_keywords)
        {
            std::string schema_path = context.get_absolute_uri().string();
            std::vector<std::string> expected_types;

            std::vector<keyword_validator_type> type_mapping{(uint8_t)(json_type::object_value)+1};

            auto it = sch.find("type");
            if (it == sch.object_range().end()) 
            {
                init_type_mapping(type_mapping, "", sch, context, known_keywords);
            }
            else 
            {
                switch (it->value().type()) 
                { 
                    case json_type::string_value: 
                    {
                        auto type = it->value().template as<std::string>();
                        init_type_mapping(type_mapping, type, sch, context, known_keywords);
                        expected_types.emplace_back(std::move(type));
                        break;
                    } 

                    case json_type::array_value: // "type": ["type1", "type2"]
                    {
                        for (const auto& item : it->value().array_range())
                        {
                            auto type = item.template as<std::string>();
                            init_type_mapping(type_mapping, type, sch, context, known_keywords);
                            expected_types.emplace_back(std::move(type));
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            return jsoncons::make_unique<type_validator<Json>>(jsonpointer::json_pointer{}, std::move(schema_path), 
                std::move(type_mapping), std::move(expected_types)
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
            return jsoncons::make_unique<content_encoding_validator<Json>>(jsonpointer::json_pointer{}, schema_path, value);
        }

        std::unique_ptr<string_validator<Json>> make_string_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.get_absolute_uri();

            std::vector<keyword_validator_type> validators;
            auto it = sch.find("maxLength");
            if (it != sch.object_range().end())
            {
                validators.emplace_back(make_max_length_validator(context, it->value()));
            }

            it = sch.find("minLength");
            if (it != sch.object_range().end())
            {
                validators.emplace_back(make_min_length_validator(context, it->value()));
            }

            it = sch.find("contentEncoding");
            if (it != sch.object_range().end())
            {
                validators.emplace_back(make_content_encoding_validator(context, it->value()));
                // If "contentEncoding" is set to "binary", a Json value
                // of type json_type::byte_string_value is accepted.
            }

            it = sch.find("contentMediaType");
            if (it != sch.object_range().end())
            {
                validators.emplace_back(make_content_media_type_validator(context, it->value()));
            }

#if defined(JSONCONS_HAS_STD_REGEX)
            it = sch.find("pattern");
            if (it != sch.object_range().end())
            {
                validators.emplace_back(make_pattern_validator(context, it->value()));
            }
#endif

            it = sch.find("format");
            if (it != sch.object_range().end())
            {
                validators.emplace_back(make_format_validator(context, it->value()));
            }

            return jsoncons::make_unique<string_validator<Json>>(jsonpointer::json_pointer{}, schema_path, std::move(validators));
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
            return jsoncons::make_unique<content_media_type_validator<Json>>(jsonpointer::json_pointer{}, schema_path, value);
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

            return jsoncons::make_unique<format_validator<Json>>(jsonpointer::json_pointer{}, schema_path, 
                format_check);
        }

        std::unique_ptr<pattern_validator<Json>> make_pattern_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("pattern");
            auto pattern_string = sch.template as<std::string>();
            auto regex = std::regex(pattern_string, std::regex::ECMAScript);
            return jsoncons::make_unique<pattern_validator<Json>>(jsonpointer::json_pointer{}, schema_path, 
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
            return jsoncons::make_unique<max_length_validator<Json>>(jsonpointer::json_pointer{}, schema_path, value);
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
            return jsoncons::make_unique<max_items_validator<Json>>(jsonpointer::json_pointer{}, schema_path, value);
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
            return jsoncons::make_unique<min_items_validator<Json>>(jsonpointer::json_pointer{}, schema_path, value);
        }

        std::unique_ptr<array_validator<Json>> make_array_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.get_absolute_uri();

            std::vector<keyword_validator_type> validators;

            auto it = sch.find("maxItems");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_max_items_validator(context, it->value()));
            }

            it = sch.find("minItems");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_min_items_validator(context, it->value()));
            }

            it = sch.find("uniqueItems");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_unique_items_validator(context, it->value()));
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

            it = sch.find("contains");
            if (it != sch.object_range().end()) 
            {
                validators.emplace_back(make_contains_validator(context, sch, it->value()));
            }

            return jsoncons::make_unique<array_validator<Json>>(jsonpointer::json_pointer{}, schema_path, std::move(validators));
        }

        std::unique_ptr<contains_validator<Json>> make_contains_validator(const compilation_context& context,
            const Json& /*parent*/, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("contains");

            std::string sub_keys[] = { "contains" };

            return jsoncons::make_unique<contains_validator<Json>>(jsonpointer::json_pointer{}, schema_path, 
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

            return jsoncons::make_unique<items_array_validator<Json>>(jsonpointer::json_pointer{}, schema_path, 
                std::move(item_validators), std::move(additional_items_validator));
        }

        std::unique_ptr<items_object_validator<Json>> make_items_object_validator(const compilation_context& context, 
            const Json& /* parent */, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("items");

            std::string sub_keys[] = {"items"};

            return jsoncons::make_unique<items_object_validator<Json>>(jsonpointer::json_pointer{}, schema_path, 
                make_schema_validator(context, sch, sub_keys));
        }

        std::unique_ptr<unique_items_validator<Json>> make_unique_items_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("uniqueItems");
            bool are_unique = sch.template as<bool>();
            return jsoncons::make_unique<unique_items_validator<Json>>(jsonpointer::json_pointer{}, schema_path, are_unique);
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
            return jsoncons::make_unique<min_length_validator<Json>>(jsonpointer::json_pointer{}, schema_path, value);
        }

        std::unique_ptr<not_validator<Json>> make_not_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("not");
            std::string not_key[] = { "not" };
            return jsoncons::make_unique<not_validator<Json>>(jsonpointer::json_pointer{}, schema_path, 
                make_schema_validator(context, sch, not_key));
        }

        std::unique_ptr<maximum_validator<Json,int64_t>> make_maximum_integer_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("maximum");
            if (!sch.is_number())
            {
                std::string message("maximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<int64_t>();
            return jsoncons::make_unique<maximum_validator<Json,int64_t>>(jsonpointer::json_pointer{}, schema_path, value);
        }

        std::unique_ptr<maximum_validator<Json,double>> make_maximum_number_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("maximum");
            if (!sch.is_number())
            {
                std::string message("maximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<double>();
            return jsoncons::make_unique<maximum_validator<Json,double>>(jsonpointer::json_pointer{}, schema_path, value);
        }

        std::unique_ptr<exclusive_maximum_validator<Json,int64_t>> make_exclusive_maximum_integer_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("exclusiveMaximum");
            if (!sch.is_number())
            {
                std::string message("exclusiveMaximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<int64_t>();
            return jsoncons::make_unique<exclusive_maximum_validator<Json,int64_t>>(jsonpointer::json_pointer{}, schema_path, value);
        }

        std::unique_ptr<exclusive_maximum_validator<Json,double>> make_exclusive_maximum_number_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("exclusiveMaximum");
            if (!sch.is_number())
            {
                std::string message("exclusiveMaximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<double>();
            return jsoncons::make_unique<exclusive_maximum_validator<Json,double>>(jsonpointer::json_pointer{}, schema_path, value);
        }

        std::unique_ptr<minimum_validator<Json,int64_t>> make_minimum_integer_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("minimum");

            //std::cout << "make_minimum_integer_validator uri: " << context.get_absolute_uri().string() << ", " << schema_path.string() << "\n";
                
            if (!sch.is_number())
            {
                std::string message("minimum must be an integer");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<int64_t>();
            return jsoncons::make_unique<minimum_validator<Json,int64_t>>(jsonpointer::json_pointer{}, schema_path, value);
        }

        std::unique_ptr<minimum_validator<Json,double>> make_minimum_number_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("minimum");
            if (!sch.is_number())
            {
                std::string message("minimum must be an number");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<double>();
            return jsoncons::make_unique<minimum_validator<Json,double>>(jsonpointer::json_pointer{}, schema_path, value);
        }

        std::unique_ptr<exclusive_minimum_validator<Json,int64_t>> make_exclusive_minimum_integer_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("exclusiveMinimum");
            if (!sch.is_number())
            {
                std::string message("exclusiveMinimum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<int64_t>();
            return jsoncons::make_unique<exclusive_minimum_validator<Json,int64_t>>(jsonpointer::json_pointer{}, schema_path, value);
        }

        std::unique_ptr<exclusive_minimum_validator<Json,double>> make_exclusive_minimum_number_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("exclusiveMinimum");
            if (!sch.is_number())
            {
                std::string message("exclusiveMinimum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<double>();
            return jsoncons::make_unique<exclusive_minimum_validator<Json,double>>(jsonpointer::json_pointer{}, schema_path, value);
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
            return jsoncons::make_unique<multiple_of_validator<Json>>(jsonpointer::json_pointer{}, schema_path, value);
        }

        std::unique_ptr<integer_validator<Json>> make_integer_validator(const compilation_context& context, 
            const Json& sch, std::set<std::string>& keywords) 
        {
            uri schema_path = context.get_absolute_uri();

            std::vector<keyword_validator_type> validators;

            auto it = sch.find("maximum");
            if (it != sch.object_range().end()) 
            {
                keywords.insert("maximum");
                validators.emplace_back(make_maximum_integer_validator(context, it->value()));
            }

            it = sch.find("minimum");
            if (it != sch.object_range().end()) 
            {
                keywords.insert("minimum");
                validators.emplace_back(make_minimum_integer_validator(context, it->value()));
            }

            it = sch.find("exclusiveMaximum");
            if (it != sch.object_range().end()) 
            {
                keywords.insert("exclusiveMaximum");
                validators.emplace_back(make_exclusive_maximum_integer_validator(context, it->value()));
            }

            it = sch.find("exclusiveMinimum");
            if (it != sch.object_range().end()) 
            {
                keywords.insert("exclusiveMinimum");
                validators.emplace_back(make_exclusive_minimum_integer_validator(context, it->value()));
            }

            it = sch.find("multipleOf");
            if (it != sch.object_range().end()) 
            {
                keywords.insert("multipleOf");
                validators.emplace_back(make_multiple_of_validator(context, it->value()));
            }


            return jsoncons::make_unique<integer_validator<Json>>(jsonpointer::json_pointer{}, schema_path, std::move(validators));
        }

        std::unique_ptr<number_validator<Json>> make_number_validator(const compilation_context& context, 
            const Json& sch, std::set<std::string>& keywords)
        {
            uri schema_path = context.get_absolute_uri();

            std::vector<keyword_validator_type> validators;

            auto it = sch.find("maximum");
            if (it != sch.object_range().end()) 
            {
                keywords.insert("maximum");
                validators.emplace_back(make_maximum_number_validator(context, it->value()));
            }

            it = sch.find("minimum");
            if (it != sch.object_range().end()) 
            {
                keywords.insert("minimum");
                validators.emplace_back(make_minimum_number_validator(context, it->value()));
            }

            it = sch.find("exclusiveMaximum");
            if (it != sch.object_range().end()) 
            {
                keywords.insert("exclusiveMaximum");
                validators.emplace_back(make_exclusive_maximum_number_validator(context, it->value()));
            }

            it = sch.find("exclusiveMinimum");
            if (it != sch.object_range().end()) 
            {
                keywords.insert("exclusiveMinimum");
                validators.emplace_back(make_exclusive_minimum_number_validator(context, it->value()));
            }

            it = sch.find("multipleOf");
            if (it != sch.object_range().end()) 
            {
                keywords.insert("multipleOf");
                validators.emplace_back(make_multiple_of_validator(context, it->value()));
            }


            return jsoncons::make_unique<number_validator<Json>>(jsonpointer::json_pointer{}, schema_path, std::move(validators));
        }

        std::unique_ptr<null_validator<Json>> make_null_validator(const compilation_context& context)
        {
            uri schema_path = context.make_schema_path_with("null");
            return jsoncons::make_unique<null_validator<Json>>(jsonpointer::json_pointer{}, schema_path);
        }

        std::unique_ptr<boolean_validator<Json>> make_boolean_validator(const compilation_context& context)
        {
            uri schema_path = context.get_absolute_uri();
            return jsoncons::make_unique<boolean_validator<Json>>(jsonpointer::json_pointer{}, schema_path);
        }

        std::unique_ptr<const_validator<Json>> make_const_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("const");
            return jsoncons::make_unique<const_validator<Json>>(jsonpointer::json_pointer{}, schema_path, sch);
        }

        std::unique_ptr<enum_validator<Json>> make_enum_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("enum");
            return jsoncons::make_unique<enum_validator<Json>>(jsonpointer::json_pointer{}, schema_path, sch);
        }

        std::unique_ptr<required_validator<Json>> make_required_validator(const compilation_context& context,
            const std::vector<std::string>& items)
        {
            uri schema_path = context.make_schema_path_with("required");
            return jsoncons::make_unique<required_validator<Json>>(jsonpointer::json_pointer{}, schema_path, items);
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

            return jsoncons::make_unique<conditional_validator<Json>>(jsonpointer::json_pointer{}, std::move(schema_path),
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
            return jsoncons::make_unique<combining_validator<Json,all_of_criterion<Json>>>(jsonpointer::json_pointer{}, std::move(schema_path), std::move(subschemas));
        }

        std::unique_ptr<combining_validator<Json,any_of_criterion<Json>>> make_any_of_validator(const compilation_context& context,
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
            return jsoncons::make_unique<combining_validator<Json,any_of_criterion<Json>>>(jsonpointer::json_pointer{}, std::move(schema_path), std::move(subschemas));
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
            return jsoncons::make_unique<combining_validator<Json,one_of_criterion<Json>>>(jsonpointer::json_pointer{}, std::move(schema_path), std::move(subschemas));
        }

        std::unique_ptr<object_validator<Json>> make_object_validator(const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.get_absolute_uri();
            std::vector<keyword_validator_type> general_validators;
            std::map<std::string, schema_validator_type> properties;
        #if defined(JSONCONS_HAS_STD_REGEX)
            std::vector<std::pair<std::regex, schema_validator_type>> pattern_properties;
        #endif
            schema_validator_type additional_properties;
            std::map<std::string, keyword_validator_type> dependent_required;
            std::map<std::string, schema_validator_type> dependent_schemas;
            schema_validator_type property_name_validator;

            auto it = sch.find("maxProperties");
            if (it != sch.object_range().end()) 
            {
                auto max_properties = it->value().template as<std::size_t>();
                general_validators.emplace_back(jsoncons::make_unique<max_properties_validator<Json>>(jsonpointer::json_pointer{}, context.make_schema_path_with("maxProperties"), max_properties));
            }

            it = sch.find("minProperties");
            if (it != sch.object_range().end()) 
            {
                auto min_properties = it->value().template as<std::size_t>();
                general_validators.emplace_back(jsoncons::make_unique<min_properties_validator<Json>>(jsonpointer::json_pointer{}, context.make_schema_path_with("minProperties"), min_properties));
            }

            it = sch.find("required");
            if (it != sch.object_range().end()) 
            {
                general_validators.emplace_back(jsoncons::make_unique<required_validator<Json>>(jsonpointer::json_pointer{}, context.make_schema_path_with("required"), 
                    it->value().template as<std::vector<std::string>>()));
            }

            it = sch.find("properties");
            if (it != sch.object_range().end()) 
            {
                for (const auto& prop : it->value().object_range())
                {
                    std::string sub_keys[] = {"properties", prop.key()};
                    properties.emplace(
                        std::make_pair(
                            prop.key(),
                            make_schema_validator(context, prop.value(), sub_keys)));
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
                            make_schema_validator(context, prop.value(), sub_keys)));
                }
            }
    #endif

            it = sch.find("additionalProperties");
            if (it != sch.object_range().end()) 
            {
                std::string sub_keys[] = {"additionalProperties"};
                additional_properties = make_schema_validator(context, it->value(), sub_keys);
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
                                make_required_validator(compilation_context(nullptr, std::vector<schema_location>{{location}}),
                                    dep.value().template as<std::vector<std::string>>()));
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
            }

            auto property_names_it = sch.find("propertyNames");
            if (property_names_it != sch.object_range().end()) 
            {
                std::string sub_keys[] = {"propertyNames"};
                property_name_validator = make_schema_validator(context, property_names_it->value(), sub_keys);
            }

            return jsoncons::make_unique<object_validator<Json>>(jsonpointer::json_pointer{}, std::move(schema_path),
                std::move(general_validators),
                std::move(properties),
#if defined(JSONCONS_HAS_STD_REGEX)
                std::move(pattern_properties),
#endif
                std::move(additional_properties),
                std::move(dependent_required),
                std::move(dependent_schemas),
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
                    subschemas_.emplace_back(make_schema_validator(compilation_context(new_uri), value, {}));
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

        keyword_validator_type get_or_create_reference(const schema_location& uri)
        {
            auto &file = get_or_create_file(uri.base().string());

            // a schema already exists
            auto sch = file.schemas.find(std::string(uri.fragment()));
            if (sch != file.schemas.end())
            {
                return jsoncons::make_unique<ref_validator_type>(jsonpointer::json_pointer{}, uri.base(), sch->second->make_copy(jsonpointer::json_pointer{}));
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
                    //auto p = s.get();
                    //subschemas_.emplace_back(std::move(s));
                    file.unknown_keywords.erase(unprocessed_keywords_it);
                    return jsoncons::make_unique<ref_validator_type>(jsonpointer::json_pointer{}, uri.base(), std::move(s));
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
                auto orig = jsoncons::make_unique<ref_validator_type>(jsonpointer::json_pointer{}, uri.base());
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

            std::cout << "Absolute URI: " << parent.get_absolute_uri().string() << "\n";
            for (const auto& uri : new_uris)
            {
                std::cout << "    " << uri.string() << "\n";
            }

            return compilation_context(std::addressof(parent), new_uris);
        }

    };

} // namespace draft7
} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_DRAFT7_KEYWORD_FACTORY_HPP

// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMMON_SCHEMA_BUILDER_HPP
#define JSONCONS_JSONSCHEMA_COMMON_SCHEMA_BUILDER_HPP

#include <memory>
#include <jsoncons_ext/jsonschema/common/schema_validators.hpp>
#include <jsoncons_ext/jsonschema/common/compilation_context.hpp>

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    using uri_resolver = std::function<Json(const jsoncons::uri & /*id*/)>;

    enum class spec_version{draft7, draft201909, draft202012};

    class evaluation_options
    {
        spec_version default_version_;
        bool require_format_validation_;
    public:
        evaluation_options()
            : default_version_{spec_version::draft7}, 
              require_format_validation_(false)
        {
        }

        bool require_format_validation() const
        {
            return require_format_validation_;
        }
        evaluation_options& require_format_validation(bool value) 
        {
            require_format_validation_ = value;
            return *this;
        }

        spec_version default_version() const
        {
            return default_version_;
        }
        evaluation_options& default_version(spec_version version) 
        {
            default_version_ = version;
            return *this;
        }
    };

    template <class Json>
    class schema_builder
    {
    public:
        using schema_store_type = std::map<jsoncons::uri, schema_validator<Json>*>;
        using schema_builder_factory_type = std::function<std::unique_ptr<schema_builder<Json>>(const Json&,
            const uri_resolver<Json>&, const evaluation_options&,schema_store_type*)>;
        using keyword_validator_type = typename std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;
        using ref_validator_type = ref_validator<Json>;
        using ref_type = ref<Json>;
        using anchor_uri_map_type = std::unordered_map<std::string,uri_wrapper>;

    private:
        std::string spec_version_;
        schema_builder_factory_type builder_factory_;
        uri_resolver<Json> resolver_;
        evaluation_options options_;
        schema_validator_type root_;

        // Owns external schemas
        std::vector<schema_validator_type> schemas_;
        schema_store_type* schema_store_ptr_;
    public:
        std::vector<std::pair<jsoncons::uri, ref_type*>> unresolved_refs_; 
        std::map<jsoncons::uri, Json> unknown_keywords_;

    public:

        schema_builder(const std::string& spec_version, const schema_builder_factory_type& builder_factory, 
            uri_resolver<Json> resolver, evaluation_options options, schema_store_type* schema_store_ptr)
            : spec_version_(spec_version), builder_factory_(builder_factory), resolver_(resolver), options_(std::move(options)),
              schema_store_ptr_(schema_store_ptr)
        {
            JSONCONS_ASSERT(schema_store_ptr != nullptr);
        }

        virtual ~schema_builder() = default;

        void save_schema(schema_validator_type&& schema)
        {
            schemas_.emplace_back(std::move(schema));
        }

        const std::string& spec_version() const
        {
            return spec_version_;
        }
        
        void build_schema(const Json& sch) 
        {
            anchor_uri_map_type anchor_dict;
            root_ = make_schema_validator(compilation_context{}, sch, {}, anchor_dict);
        }

        void build_schema(const Json& sch, const std::string& retrieval_uri) 
        {
            anchor_uri_map_type anchor_dict;
            root_ = make_schema_validator(compilation_context(uri_wrapper(retrieval_uri)), sch, {}, anchor_dict);
        }
        
        evaluation_options options() const
        {
            return options_;
        }

        std::unique_ptr<document_schema_validator<Json>> get_schema()
        {                        
            //std::cout << "schema_store:\n";
            //for (auto& member : schema_store_)
            //{
            //    std::cout << "    " << member.first.string() << "\n";
            //}

            // load all external schemas that have not already been loaded
            std::size_t loaded_count = 0;
            do
            {
                loaded_count = 0;

                for (std::size_t i = unresolved_refs_.size(); i-- > 0; )
                {
                    auto loc = unresolved_refs_[i].first;
                    //std::cout << "unresolved: " << loc.string() << "\n";
                    if (schema_store_ptr_->find(loc) == schema_store_ptr_->end()) // registry for this file is empty
                    {
                        if (resolver_)
                        {
                            Json external_sch = resolver_(loc.base());

                            anchor_uri_map_type anchor_dict2;
                            this->save_schema(make_cross_draft_schema_validator(compilation_context(uri_wrapper(loc.base())), 
                                external_sch, {}, anchor_dict2));
                            ++loaded_count;
                        }
                        else
                        {
                            JSONCONS_THROW(schema_error("External schema reference '" + loc.base().string() + "' needs to be loaded, but no resolver provided"));
                        }
                    }
                }
            } while (loaded_count > 0);

            resolve_references();

            return jsoncons::make_unique<document_schema_validator<Json>>(std::move(root_), std::move(schemas_));
        }

        void resolve_references()
        {
            for (auto& ref : unresolved_refs_)
            {
                auto it = schema_store_ptr_->find(ref.first);
                if (it == schema_store_ptr_->end())
                {
                    JSONCONS_THROW(schema_error("Undefined reference " + ref.first.string()));
                }
                if (it->second == nullptr)
                {
                    JSONCONS_THROW(schema_error("Null referred schema " + ref.first.string()));
                }
                ref.second->set_referred_schema(it->second);
            }
        }

        virtual compilation_context make_compilation_context(const compilation_context& parent,
            const Json& sch, jsoncons::span<const std::string> keys) const = 0;

        virtual schema_validator_type make_schema_validator(const compilation_context& context, 
            const Json& sch, jsoncons::span<const std::string> keys, anchor_uri_map_type& anchor_dict) = 0;

        schema_validator_type make_cross_draft_schema_validator(const compilation_context& context, 
            const Json& sch, jsoncons::span<const std::string> keys, anchor_uri_map_type& anchor_dict)
        {
            schema_validator_type schema_val = schema_validator_type{};
            switch (sch.type())
            {
                case json_type::object_value:
                {
                    auto it = sch.find("$schema");
                    if (it != sch.object_range().end())
                    {
                        if (it->value().as_string_view() == spec_version())
                        {
                            return make_schema_validator(context, sch, keys, anchor_dict);
                        }
                        else
                        {
                            auto schema_builder = builder_factory_(sch, resolver_, options_, schema_store_ptr_);
                            schema_builder->build_schema(sch);
                            schema_val = schema_builder->get_schema();
                        }
                    }
                    else
                    {
                        return make_schema_validator(context, sch, keys, anchor_dict);
                    }
                    break;
                }
                case json_type::bool_value:
                {
                    return make_schema_validator(context, sch, keys, anchor_dict);
                }
                default:
                    JSONCONS_THROW(schema_error("Schema must be object or boolean"));
            }
            return schema_val;
        }

        virtual std::unique_ptr<max_length_validator<Json>> make_max_length_validator(const compilation_context& context, 
            const Json& sch)
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

        virtual std::unique_ptr<min_length_validator<Json>> make_min_length_validator(const compilation_context& context, 
            const Json& sch)
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

        virtual std::unique_ptr<not_validator<Json>> make_not_validator(const compilation_context& context, 
            const Json& sch, anchor_uri_map_type& anchor_dict)
        {
            uri schema_path = context.make_schema_path_with("not");
            std::string not_key[] = { "not" };
            return jsoncons::make_unique<not_validator<Json>>( schema_path, 
                make_cross_draft_schema_validator(context, sch, not_key, anchor_dict));
        }

        virtual std::unique_ptr<const_validator<Json>> make_const_validator(const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("const");
            return jsoncons::make_unique<const_validator<Json>>( schema_path, sch);
        }

        virtual std::unique_ptr<enum_validator<Json>> make_enum_validator(const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("enum");
            return jsoncons::make_unique<enum_validator<Json>>( schema_path, sch);
        }

        virtual std::unique_ptr<required_validator<Json>> make_required_validator(const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("required");
            return jsoncons::make_unique<required_validator<Json>>( schema_path, sch.template as<std::vector<std::string>>());
        }

        virtual std::unique_ptr<maximum_validator<Json>> make_maximum_validator(const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("maximum");
            if (!sch.is_number())
            {
                std::string message("maximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            return jsoncons::make_unique<maximum_validator<Json>>( schema_path, sch);
        }

        virtual std::unique_ptr<exclusive_maximum_validator<Json>> make_exclusive_maximum_validator(const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("exclusiveMaximum");
            if (!sch.is_number())
            {
                std::string message("exclusiveMaximum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            return jsoncons::make_unique<exclusive_maximum_validator<Json>>( schema_path, sch);
        }

        virtual std::unique_ptr<keyword_validator<Json>> make_minimum_validator(const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("minimum");
                
            if (!sch.is_number())
            {
                std::string message("minimum must be an integer");
                JSONCONS_THROW(schema_error(message));
            }
            return jsoncons::make_unique<minimum_validator<Json>>( schema_path, sch);
        }

        virtual std::unique_ptr<exclusive_minimum_validator<Json>> make_exclusive_minimum_validator(const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("exclusiveMinimum");
            if (!sch.is_number())
            {
                std::string message("exclusiveMinimum must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            return jsoncons::make_unique<exclusive_minimum_validator<Json>>( schema_path, sch);
        }

        virtual std::unique_ptr<multiple_of_validator<Json>> make_multiple_of_validator(const compilation_context& context, 
            const Json& sch)
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


        virtual std::unique_ptr<type_validator<Json>> make_type_validator(const compilation_context& context,
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

        virtual std::unique_ptr<content_encoding_validator<Json>> make_content_encoding_validator(const compilation_context& context, 
            const Json& sch)
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

        virtual std::unique_ptr<content_media_type_validator<Json>> make_content_media_type_validator(const compilation_context& context, 
            const Json& sch)
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

        virtual std::unique_ptr<format_validator<Json>> make_format_validator(const compilation_context& context, 
            const Json& sch)
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

        virtual std::unique_ptr<pattern_validator<Json>> make_pattern_validator(const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("pattern");
            auto pattern_string = sch.template as<std::string>();
            auto regex = std::regex(pattern_string, std::regex::ECMAScript);
            return jsoncons::make_unique<pattern_validator<Json>>( schema_path, 
                pattern_string, regex);
        }

        virtual std::unique_ptr<max_items_validator<Json>> make_max_items_validator(const compilation_context& context, 
            const Json& sch)
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

        virtual std::unique_ptr<min_items_validator<Json>> make_min_items_validator(const compilation_context& context, 
            const Json& sch)
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

        virtual std::unique_ptr<max_properties_validator<Json>> make_max_properties_validator(const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("maxProperties");
            if (!sch.is_number())
            {
                std::string message("maxProperties must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<max_properties_validator<Json>>( schema_path, value);
        }

        virtual std::unique_ptr<min_properties_validator<Json>> make_min_properties_validator(const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("minProperties");
            if (!sch.is_number())
            {
                std::string message("minProperties must be a number value");
                JSONCONS_THROW(schema_error(message));
            }
            auto value = sch.template as<std::size_t>();
            return jsoncons::make_unique<min_properties_validator<Json>>( schema_path, value);
        }

        virtual std::unique_ptr<contains_validator<Json>> make_contains_validator(const compilation_context& context,
            const Json& sch, const Json& parent, anchor_uri_map_type& anchor_dict)
        {
            uri schema_path = context.make_schema_path_with("contains");

            std::string sub_keys[] = { "contains" };

            std::unique_ptr<max_contains_keyword<Json>> max_contains;
            auto it = parent.find("maxContains");
            if (it != parent.object_range().end()) 
            {
                uri path = context.make_schema_path_with("maxContains");
                auto value = it->value().template as<std::size_t>();
                max_contains = jsoncons::make_unique<max_contains_keyword<Json>>(path, value);
            }
            else
            {
                uri path = context.make_schema_path_with("maxContains");
                max_contains = jsoncons::make_unique<max_contains_keyword<Json>>(path, (std::numeric_limits<std::size_t>::max)());
            }

            std::unique_ptr<min_contains_keyword<Json>> min_contains;
            it = parent.find("minContains");
            if (it != parent.object_range().end()) 
            {
                uri path = context.make_schema_path_with("minContains");
                auto value = it->value().template as<std::size_t>();
                min_contains = jsoncons::make_unique<min_contains_keyword<Json>>(path, value);
            }
            else
            {
                uri path = context.make_schema_path_with("minContains");
                min_contains = jsoncons::make_unique<min_contains_keyword<Json>>(path, 1);
            }

            return jsoncons::make_unique<contains_validator<Json>>( schema_path, 
                make_cross_draft_schema_validator(context, sch, sub_keys, anchor_dict), std::move(max_contains), std::move(min_contains));
        }

        virtual std::unique_ptr<unique_items_validator<Json>> make_unique_items_validator(const compilation_context& context, 
            const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("uniqueItems");
            bool are_unique = sch.template as<bool>();
            return jsoncons::make_unique<unique_items_validator<Json>>( schema_path, are_unique);
        }

        virtual std::unique_ptr<all_of_validator<Json>> make_all_of_validator(const compilation_context& context,
            const Json& sch, anchor_uri_map_type& anchor_dict)
        {
            uri schema_path = context.make_schema_path_with("allOf");
            std::vector<schema_validator_type> subschemas;

            size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                std::string sub_keys[] = { "allOf", std::to_string(c++) };
                subschemas.emplace_back(make_cross_draft_schema_validator(context, subsch, sub_keys, anchor_dict));
            }
            return jsoncons::make_unique<all_of_validator<Json>>(std::move(schema_path), std::move(subschemas));
        }

        virtual std::unique_ptr<any_of_validator<Json>> make_any_of_validator(const compilation_context& context,
            const Json& sch, anchor_uri_map_type& anchor_dict)
        {
            uri schema_path = context.make_schema_path_with("anyOf");
            std::vector<schema_validator_type> subschemas;

            size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                std::string sub_keys[] = { "anyOf", std::to_string(c++) };
                subschemas.emplace_back(make_cross_draft_schema_validator(context, subsch, sub_keys, anchor_dict));
            }
            return jsoncons::make_unique<any_of_validator<Json>>(std::move(schema_path), std::move(subschemas));
        }

        virtual std::unique_ptr<one_of_validator<Json>> make_one_of_validator(const compilation_context& context,
            const Json& sch, anchor_uri_map_type& anchor_dict)
        {
            uri schema_path{ context.make_schema_path_with("oneOf") };
            std::vector<schema_validator_type> subschemas;

            size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                std::string sub_keys[] = { "oneOf", std::to_string(c++) };
                subschemas.emplace_back(make_cross_draft_schema_validator(context, subsch, sub_keys, anchor_dict));
            }
            return jsoncons::make_unique<one_of_validator<Json>>(std::move(schema_path), std::move(subschemas));
        }

        virtual std::unique_ptr<dependencies_validator<Json>> make_dependencies_validator(const compilation_context& context, 
            const Json& sch, anchor_uri_map_type& anchor_dict)
        {
            uri schema_path = context.get_absolute_uri();
            std::map<std::string, keyword_validator_type> dependent_required;
            std::map<std::string, schema_validator_type> dependent_schemas;

            //std::cout << "dependencies" << "\n" << pretty_print(sch) << "\n";

            for (const auto& dep : sch.object_range())
            {
                switch (dep.value().type()) 
                {
                    case json_type::array_value:
                    {
                        auto location = context.make_schema_path_with("dependencies");
                        dependent_required.emplace(dep.key(), 
                            this->make_required_validator(compilation_context(std::vector<uri_wrapper>{{uri_wrapper{ location }}}),
                                dep.value().template as<std::vector<std::string>>()));
                        break;
                    }
                    case json_type::bool_value:
                    case json_type::object_value:
                    {
                        std::string sub_keys[] = {"dependencies"};
                        dependent_schemas.emplace(dep.key(),
                            make_cross_draft_schema_validator(context, dep.value(), sub_keys, anchor_dict));
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

        virtual std::unique_ptr<property_names_validator<Json>> make_property_names_validator(const compilation_context& context, 
            const Json& sch, anchor_uri_map_type& anchor_dict)
        {
            uri schema_path = context.get_absolute_uri();
            schema_validator_type property_names_schema_validator;

            std::string sub_keys[] = { "propertyNames"};
            property_names_schema_validator = make_cross_draft_schema_validator(context, sch, sub_keys, anchor_dict);

            return jsoncons::make_unique<property_names_validator<Json>>( std::move(schema_path),
                std::move(property_names_schema_validator));
        }

        // 201909 and later
                
        virtual std::unique_ptr<dependent_required_validator<Json>> make_dependent_required_validator( 
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
                            this->make_required_validator(compilation_context(std::vector<uri_wrapper>{{uri_wrapper{ location }}}),
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

        virtual std::unique_ptr<dependent_schemas_validator<Json>> make_dependent_schemas_validator( const compilation_context& context, 
            const Json& sch, anchor_uri_map_type& anchor_dict)
        {
            uri schema_path = context.get_absolute_uri();
            std::map<std::string, schema_validator_type> dependent_schemas;

            for (const auto& dep : sch.object_range())
            {
                switch (dep.value().type()) 
                {
                    case json_type::bool_value:
                    case json_type::object_value:
                    {
                        std::string sub_keys[] = {"dependentSchemas"};
                        dependent_schemas.emplace(dep.key(),
                            make_cross_draft_schema_validator(context, dep.value(), sub_keys, anchor_dict));
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

        virtual std::unique_ptr<unevaluated_properties_validator<Json>> make_unevaluated_properties_validator(
            const compilation_context& context, const Json& sch, anchor_uri_map_type& anchor_dict)
        {
            uri schema_path = context.get_absolute_uri();

            std::string sub_keys[] = {"unevaluatedProperties"};

            return jsoncons::make_unique<unevaluated_properties_validator<Json>>( std::move(schema_path),
                make_cross_draft_schema_validator(context, sch, sub_keys, anchor_dict));
        }

        virtual std::unique_ptr<unevaluated_items_validator<Json>> make_unevaluated_items_validator(
            const compilation_context& context, const Json& sch, anchor_uri_map_type& anchor_dict)
        {
            uri schema_path = context.get_absolute_uri();

            std::string sub_keys[] = {"unevaluatedItems"};

            return jsoncons::make_unique<unevaluated_items_validator<Json>>( std::move(schema_path),
                make_cross_draft_schema_validator(context, sch, sub_keys, anchor_dict));
        }

        void insert_schema(const uri_wrapper& identifier, schema_validator<Json>* s)
        {
            this->schema_store_ptr_->emplace(identifier.uri(), s);
        }

        void insert_unknown_keyword(const uri_wrapper& uri, 
                                    const std::string& key, 
                                    const Json& value)
        {
            auto new_u = uri.append(key);
            uri_wrapper new_uri(new_u);

            if (new_uri.has_fragment() && !new_uri.has_plain_name_fragment()) 
            {
                // is there a reference looking for this unknown-keyword, which is thus no longer a unknown keyword but a schema
                auto unresolved_refs = std::find_if(this->unresolved_refs_.begin(), this->unresolved_refs_.end(),
                    [new_uri](const std::pair<jsoncons::uri,ref<Json>*>& pr) {return pr.first == new_uri.uri();});
                if (unresolved_refs != this->unresolved_refs_.end())
                {
                    anchor_uri_map_type anchor_dict2;
                    this->save_schema(make_cross_draft_schema_validator(compilation_context(new_uri), value, {}, anchor_dict2));
                }
                else // no, nothing ref'd it, keep for later
                {
                    //file.unknown_keywords.emplace(fragment, value);
                    this->unknown_keywords_.emplace(new_uri.uri(), value);
                }

                // recursively add possible subschemas of unknown keywords
                if (value.type() == json_type::object_value)
                {
                    for (const auto& subsch : value.object_range())
                    {
                        insert_unknown_keyword(new_uri, subsch.key(), subsch.value());
                    }
                }
            }
        }

        std::unique_ptr<ref_validator<Json>> get_or_create_reference(const uri_wrapper& identifier)
        {
            // a schema already exists
            auto it = this->schema_store_ptr_->find(identifier.uri());
            if (it != this->schema_store_ptr_->end())
            {
                return jsoncons::make_unique<ref_validator_type>(identifier.uri(), it->second);
            }

            // referencing an unknown keyword, turn it into schema
            //
            // an unknown keyword can only be referenced by a JSONPointer,
            // not by a plain name identifier
            if (identifier.has_fragment() && !identifier.has_plain_name_fragment()) 
            {
                //std::string fragment = std::string(identifier.fragment());

                auto it2 = this->unknown_keywords_.find(identifier.uri());
                if (it2 != this->unknown_keywords_.end())
                {
                    auto& subsch = it2->second;
                    anchor_uri_map_type anchor_dict2;
                    auto s = make_cross_draft_schema_validator(compilation_context(identifier), subsch, {}, anchor_dict2);
                    this->unknown_keywords_.erase(it2);
                    auto orig = jsoncons::make_unique<ref_validator_type>(identifier.uri(), s.get());
                    this->save_schema(std::move(s));
                    return orig;
                }
            }

            // get or create a ref_validator
            auto orig = jsoncons::make_unique<ref_validator_type>(identifier.uri());

            this->unresolved_refs_.emplace_back(identifier.uri(), orig.get());
            return orig;
        }

        static bool validate_anchor(const std::string& s)
        {
            if (s.empty())
            {
                return false;
            }
            if (!((s[0] >= 'a' && s[0] <= 'z') || (s[0] >= 'A' && s[0] <= 'Z')))
            {
                return false;
            }

            for (std::size_t i = 1; i < s.size(); ++i)
            {
                switch (s[i])
                {
                    case '-':
                    case '_':
                    case ':':
                    case '.':
                        break;
                    default:
                        if (!((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z') || (s[i] >= '0' && s[i] <= '9')))
                        {
                            return false;
                        }
                        break;
                }
            }
            return true;
        }

    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_COMMON_SCHEMA_HPP

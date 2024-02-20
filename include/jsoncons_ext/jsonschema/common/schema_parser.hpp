// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMMON_SCHEMA_PARSER_HPP
#define JSONCONS_JSONSCHEMA_COMMON_SCHEMA_PARSER_HPP

#include <memory>
#include <jsoncons_ext/jsonschema/json_schema.hpp>
#include <jsoncons_ext/jsonschema/common/keywords.hpp>

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    class schema_parser
    {
    public:
        using keyword_validator_type = typename std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;

        virtual ~schema_parser() = default;

        virtual void parse(const Json& sch) = 0;

        virtual void parse(const Json& sch, const std::string& retrieval_uri) = 0;

        virtual std::shared_ptr<json_schema<Json>> get_schema() = 0;

        virtual compilation_context make_compilation_context(const compilation_context& parent,
            const Json& sch, jsoncons::span<const std::string> keys) const = 0;

        virtual schema_validator_type make_schema_validator(const compilation_context& context, 
            const Json& sch, jsoncons::span<const std::string> keys) = 0;

        virtual std::unique_ptr<max_length_validator<Json>> make_max_length_validator(const compilation_context& context, const Json& sch)
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

        virtual std::unique_ptr<min_length_validator<Json>> make_min_length_validator(const compilation_context& context, const Json& sch)
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

        virtual std::unique_ptr<not_validator<Json>> make_not_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("not");
            std::string not_key[] = { "not" };
            return jsoncons::make_unique<not_validator<Json>>( schema_path, 
                make_schema_validator(context, sch, not_key));
        }

        virtual std::unique_ptr<const_validator<Json>> make_const_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("const");
            return jsoncons::make_unique<const_validator<Json>>( schema_path, sch);
        }

        virtual std::unique_ptr<enum_validator<Json>> make_enum_validator(const compilation_context& context, const Json& sch)
        {
            uri schema_path = context.make_schema_path_with("enum");
            return jsoncons::make_unique<enum_validator<Json>>( schema_path, sch);
        }

        virtual std::unique_ptr<required_validator<Json>> make_required_validator(const compilation_context& context, const Json& sch)
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


    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_COMMON_SCHEMA_HPP

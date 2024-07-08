// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_JSON_VALIDATOR_HPP
#define JSONCONS_JSONSCHEMA_JSON_VALIDATOR_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/json_schema.hpp>
#include <cassert>
#include <set>
#include <sstream>
#include <iostream>
#include <cassert>
#include <functional>
#include <unordered_set>

namespace jsoncons {
namespace jsonschema {

#if !defined(JSONCONS_NO_DEPRECATED)

class validation_output 
    {
        std::string keyword_;
        std::string schema_path_;
        std::string instance_location_;
        std::string message_;
        std::vector<validation_output> nested_errors_;
    public:
        validation_output(std::string keyword,
            std::string schema_path,
            std::string instance_location,
            std::string message)
            : keyword_(std::move(keyword)), 
              schema_path_(std::move(schema_path)),
              instance_location_(std::move(instance_location)),
              message_(std::move(message))
        {
        }

        validation_output(const std::string& keyword,
            const std::string& schema_path,
            const std::string& instance_location,
            const std::string& message,
            const std::vector<validation_output>& nested_errors)
            : keyword_(keyword),
              schema_path_(schema_path),
              instance_location_(instance_location), 
              message_(message),
              nested_errors_(nested_errors)
        {
        }

        const std::string& instance_location() const
        {
            return instance_location_;
        }

        const std::string& message() const
        {
            return message_;
        }

        const std::string& schema_path() const
        {
            return schema_path_;
        }

        const std::string& keyword() const
        {
            return keyword_;
        }

        const std::vector<validation_output>& nested_errors() const
        {
            return nested_errors_;
        }
    };

    struct validation_message_to_validation_output : public error_reporter
    {
        using validation_output_reporter_t = std::function<void(const validation_output& msg)>;

        validation_output_reporter_t reporter_;

        validation_message_to_validation_output(const validation_output_reporter_t& reporter)
            : reporter_(reporter)
        {
        }
    private:
        walk_result do_error(const validation_message& m) override
        {
            std::vector<validation_output> nested_errors;
            for (const auto& detail : m.details())
            {
                nested_errors.emplace_back(validation_output(detail.keyword(),
                    detail.schema_location().string(),
                    detail.instance_location().string(),
                    detail.message()));
            }
                
            reporter_(validation_output(m.keyword(),
                m.schema_location().string(),
                m.instance_location().string(),
                m.message(),
                std::move(nested_errors)));

            return walk_result::advance;
        }
    };

    template <typename Json>
    class json_validator
    {
        std::shared_ptr<json_schema<Json>> root_;

    public:
        json_validator(std::shared_ptr<json_schema<Json>> root)
            : root_(root)
        {
        }

        json_validator(json_validator &&) = default;
        json_validator &operator=(json_validator &&) = default;

        json_validator(json_validator const &) = delete;
        json_validator &operator=(json_validator const &) = delete;

        ~json_validator() = default;

        // Validate input JSON against a JSON Schema with a default throwing error reporter
        Json validate(const Json& instance) const
        {
            throwing_error_listener reporter;
            Json patch(json_array_arg);

            root_->validate2(instance, reporter, patch);
            return patch;
        }

        // Validate input JSON against a JSON Schema 
        bool is_valid(const Json& instance) const
        {
            fail_early_reporter reporter;
            Json patch(json_array_arg);

            root_->validate2(instance, reporter, patch);
            return reporter.error_count() == 0;
        }

        // Validate input JSON against a JSON Schema with a provided error reporter
        template <typename MsgReporter>
        typename std::enable_if<extension_traits::is_unary_function_object<MsgReporter,validation_output>::value,Json>::type
        validate(const Json& instance, MsgReporter&& reporter) const
        {
            Json patch(json_array_arg);

            validation_message_to_validation_output adaptor(reporter);

            root_->validate2(instance, adaptor, patch);
            return patch;
        }
    };
    
#endif    

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_JSON_VALIDATOR_HPP

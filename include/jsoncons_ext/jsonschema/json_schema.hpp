// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_JSONSCHEMA_JSON_SCHEMA_HPP
#define JSONCONS_EXT_JSONSCHEMA_JSON_SCHEMA_HPP

#include <functional>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/config/jsoncons_config.hpp>

#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/common/schema_validator.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>

namespace jsoncons {
namespace jsonschema {

class validation_message_to_json_events 
{
    json_visitor* visitor_ptr_;
public:
    validation_message_to_json_events(json_visitor& visitor)
        : visitor_ptr_(std::addressof(visitor))
    {
    }

    walk_state operator()(const validation_message& message)
    {
        write_error(message);
        return walk_state::advance;
    }

    void write_error(const validation_message& message)
    {
        visitor_ptr_->begin_object();

        visitor_ptr_->key("valid");
        visitor_ptr_->bool_value(false);

        visitor_ptr_->key("evaluationPath");
        visitor_ptr_->string_value(message.eval_path().string());

        visitor_ptr_->key("schemaLocation");
        visitor_ptr_->string_value(message.schema_location().string());

        visitor_ptr_->key("instanceLocation");
        visitor_ptr_->string_value(message.instance_location().string());

        visitor_ptr_->key("error");
        visitor_ptr_->string_value(message.message());

        if (!message.details().empty())
        {
            visitor_ptr_->key("details");
            visitor_ptr_->begin_array();
            for (const auto& detail : message.details())
            {
                write_error(detail);
            }
            visitor_ptr_->end_array();
        }

        visitor_ptr_->end_object();
    }
};

template <typename Json>
class throwing_error_listener : public error_reporter<Json>
{
    walk_state do_error(const validation_message& msg,
        jsoncons::optional<Json>&) override
    {
        JSONCONS_THROW(validation_error(msg.instance_location().string() + ": " + msg.message()));
    }
};

template <typename Json>
class fail_early_reporter : public error_reporter<Json>
{
    walk_state do_error(const validation_message&, 
        jsoncons::optional<Json>&) override
    {
        return walk_state::abort;
    }
};

template <typename Json,typename Reporter,typename Enable=void>
struct error_reporter_adaptor : public error_reporter<Json>
{
};

template <typename Json,typename Reporter>
struct error_reporter_adaptor<Json,Reporter,
        typename std::enable_if<ext_traits::is_function_object_1_exact<Reporter,walk_state,validation_message>::value>::type>
    : public error_reporter<Json>
{
    Reporter reporter_;

    error_reporter_adaptor(const Reporter& reporter)
        : reporter_(reporter)
    {
    }
private:
    walk_state do_error(const validation_message& msg, 
        jsoncons::optional<Json>& /*patch*/) override
    {
        return reporter_(msg);
    }
};


template <typename Json,typename Reporter>
struct error_reporter_adaptor<Json,Reporter,
        typename std::enable_if<ext_traits::is_function_object_2_exact<Reporter,walk_state,validation_message,jsoncons::optional<Json>&>::value>::type>
    : public error_reporter<Json>
{
    Reporter reporter_;

    error_reporter_adaptor(const Reporter& reporter)
        : reporter_(reporter)
    {
    }
private:
    walk_state do_error(const validation_message& msg, 
        jsoncons::optional<Json>& patch) override
    {
        return reporter_(msg, patch);
    }
};
   
template <typename Json>
class json_validator;

template <typename Json>
class json_schema
{
    using keyword_validator_ptr_type = std::unique_ptr<keyword_validator<Json>>;
    using document_schema_validator_type = std::unique_ptr<document_schema_validator<Json>>;

    document_schema_validator_type root_;
    
    friend class json_validator<Json>;
public:
    json_schema(document_schema_validator_type&& root)
        : root_(std::move(root))
    {
        if (root_ == nullptr)
            JSONCONS_THROW(schema_error("There is no root schema to validate an instance against"));
    }

    json_schema(const json_schema&) = delete;
    json_schema(json_schema&&) = default;
    json_schema& operator=(const json_schema&) = delete;
    json_schema& operator=(json_schema&&) = default;

    // Validate input JSON against a JSON Schema with a default throwing error reporter
    Json validate(const Json& instance) const
    {
        throwing_error_listener<Json> reporter;
        jsonpointer::json_pointer instance_location{};
        jsoncons::optional<Json> patch{jsoncons::in_place, jsoncons::json_array_arg};

        eval_context<Json> context;
        evaluation_results results;
        root_->validate(context, instance, instance_location, results, reporter, patch);
        return patch ? *patch : Json{json_array_arg};
    }

    // Validate input JSON against a JSON Schema 
    bool is_valid(const Json& instance) const
    {
        fail_early_reporter<Json> reporter;
        jsonpointer::json_pointer instance_location{};
        jsoncons::optional<Json> patch{};

        eval_context<Json> context;
        evaluation_results results;
        root_->validate(context, instance, instance_location, results, reporter, patch);
        return reporter.error_count() == 0;
    }

    // Validate input JSON against a JSON Schema with a provided error reporter
    template <typename CustomReporter>
    typename std::enable_if<ext_traits::is_function_object_1_exact<CustomReporter,walk_state,validation_message>::value,void>::type
    validate(const Json& instance, const CustomReporter& reporter) const
    {
        jsonpointer::json_pointer instance_location{};
        jsoncons::optional<Json> patch;

        error_reporter_adaptor<Json,CustomReporter> adaptor(reporter);
        eval_context<Json> context;
        evaluation_results results;
        root_->validate(context, instance, instance_location, results, adaptor, patch);
    }

    // Validate input JSON against a JSON Schema with a provided error reporter
    template <typename CustomReporter>
    typename std::enable_if<ext_traits::is_function_object_1_exact<CustomReporter,walk_state,validation_message>::value,void>::type
    validate(const Json& instance, CustomReporter&& reporter, Json& patch) const
    {
        jsonpointer::json_pointer instance_location{};

        error_reporter_adaptor<Json,CustomReporter> adaptor(std::forward<CustomReporter>(reporter));
        eval_context<Json> context;
        evaluation_results results;
        jsoncons::optional<Json> temp{jsoncons::in_place, jsoncons::json_array_arg};
        root_->validate(context, instance, instance_location, results, adaptor, temp);
        if (temp)
        {
            patch = std::move(*temp);
        }
        else
        {
            patch = Json(json_array_arg);
        }
    }

    // Validate input JSON against a JSON Schema with a provided error reporter
    template <typename CustomReporter>
    typename std::enable_if<ext_traits::is_function_object_2_exact<CustomReporter,walk_state,validation_message,jsoncons::optional<Json>&>::value,void>::type
    validate(const Json& instance, const CustomReporter& reporter) const
    {
        jsonpointer::json_pointer instance_location{};
        jsoncons::optional<Json> patch;

        error_reporter_adaptor<Json,CustomReporter> adaptor(reporter);
        eval_context<Json> context;
        evaluation_results results;
        root_->validate(context, instance, instance_location, results, adaptor, patch);
    }

    // Validate input JSON against a JSON Schema with a provided error reporter
    template <typename CustomReporter>
    typename std::enable_if<ext_traits::is_function_object_2_exact<CustomReporter,walk_state,validation_message, jsoncons::optional<Json>&>::value,void>::type
    validate(const Json& instance, CustomReporter&& reporter, Json& patch) const
    {
        jsonpointer::json_pointer instance_location{};

        error_reporter_adaptor<Json, CustomReporter> adaptor(std::forward<CustomReporter>(reporter));
        eval_context<Json> context;
        evaluation_results results;
        jsoncons::optional<Json> temp{jsoncons::in_place, jsoncons::json_array_arg};
        root_->validate(context, instance, instance_location, results, adaptor, temp);
        if (temp)
        {
            patch = std::move(*temp);
        }
        else
        {
            patch = Json(json_array_arg);
        }
    }

    // Validate input JSON against a JSON Schema with a provided error reporter
    void validate(const Json& instance, Json& patch) const
    {
        jsonpointer::json_pointer instance_location{};

        fail_early_reporter<Json> reporter;
        eval_context<Json> context;
        evaluation_results results;
        jsoncons::optional<Json> temp{jsoncons::in_place, jsoncons::json_array_arg};
        root_->validate(context, instance, instance_location, results, reporter, temp);
        if (temp)
        {
            patch = std::move(*temp);
        }
        else
        {
            patch = Json(json_array_arg);
        }
    }

    // Validate input JSON against a JSON Schema with a provided json_visitor
    void validate(const Json& instance, json_visitor& visitor) const
    {
        visitor.begin_array();
        jsonpointer::json_pointer instance_location{};
        jsoncons::optional<Json> patch;

        validation_message_to_json_events adaptor{ visitor };
        eval_context<Json> context;
        evaluation_results results;
        error_reporter_adaptor<Json, validation_message_to_json_events> reporter(adaptor);
        root_->validate(context, instance, instance_location, results, reporter, patch);
        visitor.end_array();
        visitor.flush();
    }
    
    template <typename WalkReporter>
    void walk(const Json& instance, const WalkReporter& reporter) const
    {
        jsonpointer::json_pointer instance_location{};

        root_->walk(eval_context<Json>{}, instance, instance_location, reporter);
    }
};


} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_EXT_JSONSCHEMA_SCHEMA_HPP

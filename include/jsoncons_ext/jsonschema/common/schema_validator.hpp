// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMMON_SCHEMA_VALIDATOR_HPP
#define JSONCONS_JSONSCHEMA_COMMON_SCHEMA_VALIDATOR_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/common/evaluation_context.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>
#include <unordered_set>

namespace jsoncons {
namespace jsonschema {

    // Interface for validation error handlers
    class error_reporter
    {
        bool fail_early_;
        std::size_t error_count_;
    public:
        error_reporter(bool fail_early = false)
            : fail_early_(fail_early), error_count_(0)
        {
        }

        virtual ~error_reporter() = default;

        void error(const validation_output& o)
        {
            ++error_count_;
            do_error(o);
        }

        std::size_t error_count() const
        {
            return error_count_;
        }

        bool fail_early() const
        {
            return fail_early_;
        }

    private:
        virtual void do_error(const validation_output& /* e */) = 0;
    };


    template <class Json>
    class schema_validator;

    template <class Json>
    class ref
    {
    public:
        virtual ~ref() = default;
        virtual void set_referred_schema(const schema_validator<Json>* target) = 0;
    };

    template <class Json>
    class schema_registry
    {
    public:
        virtual ~schema_registry() = default;
        virtual schema_validator<Json>* get_schema(const jsoncons::uri& uri) = 0;
    };

    template <class Json>
    class validator_base 
    {
    public:
        virtual ~validator_base() = default;

        virtual const uri& schema_path() const = 0;

        void validate(const evaluation_context<Json>& eval_context,
            const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const 
        {
            do_validate(eval_context, instance, instance_location, evaluated_properties, reporter, patch);
        }

    private:
        virtual void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const = 0;
    };

    template <class Json>
    class keyword_validator : public validator_base<Json> 
    {
    public:
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        virtual const std::string& keyword_name() const = 0;
    };

    template <class Json>
    class keyword_validator_base : public keyword_validator<Json>
    {
        std::string keyword_name_;
        uri schema_path_;
    public:
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        keyword_validator_base(const std::string& keyword_name, const uri& schema_path)
            : keyword_name_(keyword_name), schema_path_(schema_path)
        {
        }

        keyword_validator_base(const keyword_validator_base&) = delete;
        keyword_validator_base(keyword_validator_base&&) = default;
        keyword_validator_base& operator=(const keyword_validator_base&) = delete;
        keyword_validator_base& operator=(keyword_validator_base&&) = default;

        const std::string& keyword_name() const override
        {
            return keyword_name_;
        }

        const uri& schema_path() const override
        {
            return schema_path_;
        }
    };

    template <class Json>
    class keyword_base 
    {
        std::string keyword_name_;
        uri schema_path_;
    public:

        keyword_base(const std::string& keyword_name, const uri& schema_path)
            : keyword_name_(keyword_name), schema_path_(schema_path)
        {
        }

        virtual ~keyword_base() = default;

        keyword_base(const keyword_base&) = delete;
        keyword_base(keyword_base&&) = default;
        keyword_base& operator=(const keyword_base&) = delete;
        keyword_base& operator=(keyword_base&&) = default;

        const std::string& keyword_name() const 
        {
            return keyword_name_;
        }

        const uri& schema_path() const 
        {
            return schema_path_;
        }

    private:
    };


    template <class Json>
    class dynamic_anchor_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = std::unique_ptr<schema_validator<Json>>;

        jsoncons::uri value_;

    public:
        dynamic_anchor_validator(const uri& schema_path, const jsoncons::uri& value) 
            : keyword_validator_base<Json>("$dynamicAnchor", schema_path), value_(value)
        {
            std::cout << "dynamic_anchor_validator path: " << this->schema_path().string() << ", value: " << value_.string() << "\n";
        }

        const jsoncons::uri value() const
        {
            return value_;
        }

        uri get_base_uri() const
        {
            return this->schema_path();
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const override
        {
        }
    };


    template <class Json>
    using uri_resolver = std::function<Json(const jsoncons::uri & /*id*/)>;

    template <class Json>
    class schema_validator : public validator_base<Json>
    {
    public:
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;
        using keyword_validator_type = typename std::unique_ptr<keyword_validator<Json>>;

    public:
        schema_validator()
        {}

        virtual jsoncons::optional<Json> get_default_value() const = 0;

        virtual bool recursive_anchor() const = 0;

        virtual const std::unique_ptr<dynamic_anchor_validator<Json>>& dynamic_anchor() const = 0;
    };

    template <class Json>
    class boolean_schema_validator : public schema_validator<Json>
    {
    public:
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;
        using keyword_validator_type = typename std::unique_ptr<keyword_validator<Json>>;

        uri schema_path_;
        bool value_;

        std::unique_ptr<dynamic_anchor_validator<Json>> dynamic_anchor_;

    public:
        boolean_schema_validator(const uri& schema_path, bool value)
            : schema_path_(schema_path), value_(value)
        {
        }

        jsoncons::optional<Json> get_default_value() const override
        {
            return jsoncons::optional<Json>{};
        }

        const uri& schema_path() const override
        {
            return schema_path_;
        }

        bool recursive_anchor() const final
        {
            return false;
        }

        const std::unique_ptr<dynamic_anchor_validator<Json>>& dynamic_anchor() const final
        {
            return dynamic_anchor_;
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json&, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter, 
            Json&) const final
        {
            if (!value_)
            {
                reporter.error(validation_output("false", 
                    eval_context.eval_path(),
                    this->schema_path(), 
                    instance_location.to_string(), 
                    "False schema always fails"));
            }
        }
    };

    template <class Json>
    class object_schema_validator : public schema_validator<Json>
    {
    public:
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;
        using keyword_validator_type = typename std::unique_ptr<keyword_validator<Json>>;

        uri schema_path_;
        std::vector<keyword_validator_type> validators_;
        Json default_value_;
        bool recursive_anchor_;
        std::unique_ptr<dynamic_anchor_validator<Json>> dynamic_anchor_;

    public:
        object_schema_validator(const uri& schema_path, std::vector<keyword_validator_type>&& validators, Json&& default_value,
            bool recursive_anchor = false)
            : schema_path_(schema_path),
              validators_(std::move(validators)),
              default_value_(std::move(default_value)),
              recursive_anchor_(recursive_anchor)
        {
        }
        object_schema_validator(const uri& schema_path, std::vector<keyword_validator_type>&& validators, Json&& default_value,
            std::unique_ptr<dynamic_anchor_validator<Json>>&& dynamic_anchor)
            : schema_path_(schema_path),
              validators_(std::move(validators)),
              default_value_(std::move(default_value)),
              recursive_anchor_(false),
              dynamic_anchor_(std::move(dynamic_anchor))
        {
        }

        jsoncons::optional<Json> get_default_value() const override
        {
            return default_value_;
        }

        const uri& schema_path() const override
        {
            return schema_path_;
        }

        bool recursive_anchor() const final
        {
            return recursive_anchor_;
        }

        const std::unique_ptr<dynamic_anchor_validator<Json>>& dynamic_anchor() const final
        {
            return dynamic_anchor_;
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            //std::cout << "object_schema_validator: " << eval_context.eval_path().to_string() << " " << instance << "\n\n";

            std::unordered_set<std::string> local_evaluated_properties;

            evaluation_context<Json> this_context{eval_context, this};
            for (auto& validator : validators_)
            {
                //std::cout << "    " << validator->keyword_name() << "\n";
                validator->validate(this_context, instance, instance_location, local_evaluated_properties, reporter, patch);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }

            for (auto&& name : local_evaluated_properties)
            {
                evaluated_properties.emplace(std::move(name));
            }
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_KEYWORD_VALIDATOR_HPP

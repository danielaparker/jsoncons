// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMMON_VALIDATOR_HPP
#define JSONCONS_JSONSCHEMA_COMMON_VALIDATOR_HPP

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


    struct collecting_error_reporter : public error_reporter
    {
        std::vector<validation_output> errors;

    private:
        void do_error(const validation_output& o) final
        {
            errors.push_back(o);
        }
    };
    
    struct evaluation_results
    {
        std::unordered_set<std::string> evaluated_properties;
        std::unordered_set<std::size_t> evaluated_items;

        void merge(const evaluation_results& results)
        {
            for (auto&& name : results.evaluated_properties)
            {
                evaluated_properties.insert(name);
            }
            for (auto index : results.evaluated_items)
            {
                evaluated_items.insert(index);
            }
        }
        void merge(std::unordered_set<std::string>&& properties)
        {
            for (auto&& name : properties)
            {
                evaluated_properties.insert(std::move(name));
            }
        }
        void merge(const std::unordered_set<std::size_t>& items)
        {
            for (auto index : items)
            {
                evaluated_items.insert(index);
            }
        }
    };
    
    class evaluation_options
    {
        bool require_format_validation_;
    public:
        evaluation_options()
            : require_format_validation_(false)
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
    class validator_base 
    {
    public:
        virtual ~validator_base() = default;

        virtual const uri& schema_path() const = 0;

        void validate(const evaluation_context<Json>& context,
            const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch, 
            const evaluation_options& options) const 
        {
            do_validate(context, instance, instance_location, results, reporter, patch, options);
        }

    private:
        virtual void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch, const evaluation_options& options) const = 0;
    };

    template <class Json>
    class keyword_validator : public validator_base<Json> 
    {
    public:
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        virtual const std::string& keyword_name() const = 0;

        virtual const schema_validator<Json>* match_dynamic_anchor(const std::string& s) const = 0;
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
    class ref_validator : public keyword_validator_base<Json>, public virtual ref<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = std::unique_ptr<schema_validator<Json>>;

        const schema_validator<Json>* referred_schema_;

    public:
        ref_validator(const uri& schema_path) 
            : keyword_validator_base<Json>("$ref", schema_path), referred_schema_{nullptr}
        {
            //std::cout << "ref_validator: " << this->schema_path().string() << "\n";
        }

        ref_validator(const uri& schema_path, const schema_validator<Json>* referred_schema)
            : keyword_validator_base<Json>("$ref", schema_path), referred_schema_(referred_schema) 
        {
            //std::cout << "ref_validator2: " << this->schema_path().string() << "\n";
        }

        const schema_validator<Json>* referred_schema() const {return referred_schema_;}
        
        void set_referred_schema(const schema_validator<Json>* target) final { referred_schema_ = target; }

        uri get_base_uri() const
        {
            return this->schema_path();
        }

        const schema_validator<Json>* match_dynamic_anchor(const std::string& s) const final
        {
            return referred_schema_->id() ? nullptr: referred_schema_->match_dynamic_anchor(s);
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch, 
            const evaluation_options& options) const override
        {
            evaluation_context<Json> this_context(context, this->keyword_name());

            if (!referred_schema_)
            {
                reporter.error(validation_output(this->keyword_name(), 
                    this_context.eval_path(),
                    this->schema_path(), 
                    instance_location.to_string(), 
                    "Unresolved schema reference " + this->schema_path().string()));
                return;
            }

            referred_schema_->validate(this_context, instance, instance_location, results, reporter, patch, options);
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

        virtual const jsoncons::optional<jsoncons::uri>& id() const = 0;

        virtual bool has_dynamic_anchor(const std::string& anchor) const = 0;

        virtual const jsoncons::optional<jsoncons::uri>& dynamic_anchor() const = 0;

        virtual const schema_validator<Json>* match_dynamic_anchor(const std::string& s) const = 0;
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_KEYWORD_VALIDATOR_HPP

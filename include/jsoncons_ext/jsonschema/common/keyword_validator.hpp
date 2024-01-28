// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMMON_KEYWORD_VALIDATOR_HPP
#define JSONCONS_JSONSCHEMA_COMMON_KEYWORD_VALIDATOR_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
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

        void resolve_recursive_refs(const uri& base, bool has_recursive_anchor, schema_registry<Json>& schemas)
        {
            do_resolve_recursive_refs(base, has_recursive_anchor, schemas);
        }

        void validate(const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const 
        {
            do_validate(instance, instance_location, evaluated_properties, reporter, patch);
        }

    private:
        virtual void do_resolve_recursive_refs(const uri& base, bool has_recursive_anchor, schema_registry<Json>& schemas) = 0;

        virtual void do_validate(const Json& instance, 
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

        virtual keyword_validator_type clone(const uri& base_uri) const = 0;
    };

    template <class Json>
    class keyword_validator_base : public keyword_validator<Json>
    {
        uri schema_path_;
    public:
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;

        keyword_validator_base(const uri& schema_path)
            : schema_path_(schema_path)
        {
        }

        keyword_validator_base(const keyword_validator_base&) = delete;
        keyword_validator_base(keyword_validator_base&&) = default;
        keyword_validator_base& operator=(const keyword_validator_base&) = delete;
        keyword_validator_base& operator=(keyword_validator_base&&) = default;

        const uri& schema_path() const override
        {
            return schema_path_;
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

        virtual bool is_recursive_anchor() const = 0;

        virtual schema_validator_type clone(const uri& base_uri) const = 0;
    };

    template <class Json>
    class ref_validator : public keyword_validator<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = std::unique_ptr<schema_validator<Json>>;

        uri base_uri_;
        schema_validator_type referred_schema_;

    public:
        ref_validator(const uri& base_uri) : base_uri_(base_uri)
        {}

        ref_validator(const uri& base_uri, schema_validator_type&& target)
            : base_uri_(base_uri), referred_schema_(std::move(target)) {}

        void set_referred_schema(schema_validator_type&& target) { referred_schema_ = std::move(target); }

        uri get_base_uri() const
        {
            return base_uri_;
        }

        const uri& schema_path() const override
        {
            static uri s = uri("#");
            return referred_schema_ ? referred_schema_->schema_path() : s;
        }

        keyword_validator_type clone(const uri& base_uri) const override 
        {
            static uri s = uri("#");
            uri abs{s};

            schema_validator_type referred_schema;
            if (referred_schema_)
            {
                referred_schema = referred_schema_->clone(base_uri);
                abs = referred_schema_->schema_path().resolve(base_uri);
            }

            return jsoncons::make_unique<ref_validator>(abs, std::move(referred_schema));
        }

    private:

        void do_resolve_recursive_refs(const uri& base, bool has_recursive_anchor, schema_registry<Json>& schemas) override
        {
            std::cout << "ref_validator location::do_resolve_recursive_refs: " << schema_path().string()
                << "\n  base: " << base.string() << ", has_recursive_anchor: " << has_recursive_anchor << "\n\n";

            JSONCONS_ASSERT(referred_schema_)

            if (has_recursive_anchor)
            {
                referred_schema_->resolve_recursive_refs(base, has_recursive_anchor, schemas);
            }
            else
            {
                referred_schema_->resolve_recursive_refs(referred_schema_->schema_path(), referred_schema_->is_recursive_anchor(), schemas);
            }
        }

        void do_validate(const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const override
        {
            if (!referred_schema_)
            {
                reporter.error(validation_output("", 
                                                 this->schema_path(), 
                                                 instance_location.to_uri_fragment(), 
                                                 "Unresolved schema reference " + this->schema_path().string()));
                return;
            }

            referred_schema_->validate(instance, instance_location, evaluated_properties, reporter, patch);
        }
    };

    template <class Json>
    class recursive_ref_validator : public keyword_validator<Json>
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = std::unique_ptr<schema_validator<Json>>;

        uri base_uri_;
        schema_validator_type referred_schema_;

    public:
        recursive_ref_validator(const uri& base_uri) : base_uri_(base_uri), referred_schema_(nullptr)
        {}

        recursive_ref_validator(const uri& base_uri, schema_validator_type&& target)
            : base_uri_(base_uri), referred_schema_(std::move(target)) {}

        uri get_base_uri() const
        {
            return base_uri_;
        }

        const uri& schema_path() const override
        {
            return referred_schema_ ? referred_schema_->schema_path() : base_uri_;
        }

        keyword_validator_type clone(const jsoncons::uri& base_uri) const override 
        {
            std::cout << "recursive_ref_validator.clone " << "base_uri: << " << base_uri.string() << ", schema_path: " << this->schema_path().string() << "\n\n";

            auto uri = base_uri_.resolve(base_uri);
            return jsoncons::make_unique<recursive_ref_validator>(uri, referred_schema_ ? referred_schema_->clone(base_uri) : nullptr);
        }

    private:

        void do_resolve_recursive_refs(const uri& base, bool has_recursive_anchor, schema_registry<Json>& schemas) override
        {
            uri relative("#");
            uri location;
            if (has_recursive_anchor)
            {
                location = relative.resolve(base);
            }
            else
            {
                location = relative.resolve(base_uri_);
            }
            referred_schema_ = schemas.get_schema(location)->clone(location);
            referred_schema_->resolve_recursive_refs(base, has_recursive_anchor, schemas);
            std::cout << "recursive_ref_validator::do_resolve_recursive_refs location: " << schema_path().string()
                << "\n  base: " << base.string() << ", has_recursive_anchor: " << has_recursive_anchor 
                << "\n  location: " << location.string() << "\n\n";
        }

        void do_validate(const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const override
        {
            std::cout << "recursive_ref_validator.do_validate " << "keywordLocation: << " << this->schema_path().string() << ", instanceLocation:" << instance_location.to_string() << "\n";

            if (referred_schema_ == nullptr)
            {
                reporter.error(validation_output("", 
                                                 this->schema_path(), 
                                                 instance_location.to_uri_fragment(), 
                                                 "Unresolved schema reference " + this->schema_path().string()));
                return;
            }

            referred_schema_->validate(instance, instance_location, evaluated_properties, reporter, patch);
        }
    };

    template <class Json>
    class boolean_schema_validator : public schema_validator<Json>
    {
    public:
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;
        using keyword_validator_type = typename std::unique_ptr<keyword_validator<Json>>;

        uri schema_path_;
        bool value_;

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

        bool is_recursive_anchor() const final
        {
            return false;
        }

        schema_validator_type clone(const uri& base_uri) const final
        {
            return jsoncons::make_unique<boolean_schema_validator>(schema_path_.resolve(base_uri), value_);
        }

    private:

        void do_resolve_recursive_refs(const uri& /*base*/, bool /*has_recursive_anchor*/, schema_registry<Json>& /*schemas*/) override
        {
        }

        void do_validate(const Json&, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>&, 
            error_reporter& reporter, 
            Json&) const final
        {
            if (!value_)
            {
                reporter.error(validation_output("false", 
                    this->schema_path(), 
                    instance_location.to_uri_fragment(), 
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
        bool is_recursive_anchor_;

    public:
        object_schema_validator(const uri& schema_path, std::vector<keyword_validator_type>&& validators, Json&& default_value,
            bool is_recursive_anchor = false)
            : schema_path_(schema_path),
              validators_(std::move(validators)),
              default_value_(std::move(default_value)),
              is_recursive_anchor_(is_recursive_anchor)
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

        bool is_recursive_anchor() const final
        {
            return is_recursive_anchor_;
        }

        schema_validator_type clone(const uri& base_uri) const final
        {
            std::vector<keyword_validator_type> validators;
            for (auto& validator : validators_)
            {
                validators.push_back(validator->clone(base_uri));
            }

            return jsoncons::make_unique<object_schema_validator>(schema_path_.resolve(base_uri), std::move(validators), Json(default_value_), is_recursive_anchor_);
        }

    private:
        void do_resolve_recursive_refs(const uri& base, bool has_recursive_anchor, schema_registry<Json>& schemas) override
        {
            if (has_recursive_anchor)
            {
                for (auto& validator : validators_)
                {
                    validator->resolve_recursive_refs(base, has_recursive_anchor, schemas);
                }
            }
            else if (is_recursive_anchor_)
            {
                for (auto& validator : validators_)
                {
                    validator->resolve_recursive_refs(schema_path(), is_recursive_anchor_, schemas);
                }
            }
            else
            {
                for (auto& validator : validators_)
                {
                    validator->resolve_recursive_refs(schema_path(), is_recursive_anchor_, schemas);
                }
            }
            std::cout << "object_schema_validator location::do_resolve_recursive_refs: " << schema_path().string() << ", " << is_recursive_anchor_
                     << "\n  base: " << base.string() << ", has_recursive_anchor: " << has_recursive_anchor << "\n\n";
        }

        void do_validate(const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter, 
            Json& patch) const final
        {
            std::unordered_set<std::string> local_evaluated_properties;

            for (auto& validator : validators_)
            {
                validator->validate(instance, instance_location, local_evaluated_properties, reporter, patch);
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

    // keyword_validator_wrapper

    template <class Json>
    class keyword_validator_wrapper : public keyword_validator<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        keyword_validator<Json>* validator_;
    public:
        keyword_validator_wrapper(keyword_validator<Json>* validator)
            : validator_(validator)
        {
        }

        const uri& schema_path() const override
        {
            static uri s("#");
            return validator_ != nullptr ? validator_->schema_path() : s;
        }

        keyword_validator_type clone(const uri&) const final 
        {
            return jsoncons::make_unique<keyword_validator_wrapper>(validator_);
        }

    private:
        void do_resolve_recursive_refs(const uri& base, bool has_recursive_anchor, schema_registry<Json>& schemas) override
        {
            validator_->resolve_recursive_refs(base, has_recursive_anchor, schemas);
        }

        void do_validate(const Json& instance, 
            const jsonpointer::json_pointer& instance_location, 
            std::unordered_set<std::string>& evaluated_properties, 
            error_reporter& reporter,
            Json& patch) const override
        {
            validator_->validate(instance, instance_location, evaluated_properties, reporter, patch);
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_KEYWORD_VALIDATOR_HPP

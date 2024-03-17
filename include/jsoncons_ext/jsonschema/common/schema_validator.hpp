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
        void merge(evaluation_results&& results)
        {
            for (auto&& name : results.evaluated_properties)
            {
                evaluated_properties.insert(std::move(name));
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

        void validate(const evaluation_context<Json>& eval_context,
            const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch, 
            const evaluation_options& options) const 
        {
            do_validate(eval_context, instance, instance_location, results, reporter, patch, options);
        }

    private:
        virtual void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
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

        virtual const jsoncons::optional<jsoncons::uri>& dynamic_anchor() const = 0;

        virtual const schema_validator<Json>* match_dynamic_anchor(const std::string& s) const = 0;
    };

    template <class Json>
    class boolean_schema_validator : public schema_validator<Json>
    {
    public:
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;
        using keyword_validator_type = typename std::unique_ptr<keyword_validator<Json>>;

        uri schema_path_;
        bool value_;

        jsoncons::optional<jsoncons::uri> id_;

        jsoncons::optional<jsoncons::uri> dynamic_anchor_;

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

        const jsoncons::optional<jsoncons::uri>& id() const final
        {
            return id_;
        }

        const jsoncons::optional<jsoncons::uri>& dynamic_anchor() const final
        {
            return dynamic_anchor_;
        }

        const schema_validator<Json>* match_dynamic_anchor(const std::string& /*s*/) const final
        {
            return nullptr;
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json&, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter, 
            Json& /*patch*/, 
            const evaluation_options& /*options*/) const final
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
    class unevaluated_properties_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        schema_validator_type validator_;

    public:
        unevaluated_properties_validator(const uri& schema_path,
            schema_validator_type&& val
        )
            : keyword_validator_base<Json>("unevaluatedProperties", std::move(schema_path)), 
              validator_(std::move(val))
        {
        }

        const schema_validator<Json>* match_dynamic_anchor(const std::string& s) const final
        {
            if (validator_ != nullptr && !validator_->id()) 
            {
                auto p = validator_->match_dynamic_anchor(s);
                if (p != nullptr)
                {
                    return p;
                }
            }
            return nullptr;
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch, 
            const evaluation_options& options) const final
        {
            //std::cout << "unevaluated_properties_validator [" << eval_context.eval_path().to_string() << "," << this->schema_path().string() << "]";
            //std::cout << "results:\n";
            //for (const auto& s : results.evaluated_properties_)
            //{
            //    std::cout << "    " << s << "\n";
            //}
            //std::cout << "\n";
            if (!instance.is_object())
            {
                return;
            }

            if (validator_)
            {
                evaluation_context<Json> this_context(eval_context, this->keyword_name());

                for (const auto& prop : instance.object_range()) 
                {
                    auto prop_it = results.evaluated_properties.find(prop.key());

                    // check if it is in "results"
                    if (prop_it == results.evaluated_properties.end()) 
                    {
                        //std::cout << "Not in evaluated properties: " << prop.key() << "\n";
                        std::size_t error_count = reporter.error_count();
                        validator_->validate(this_context, prop.value() , instance_location, results, reporter, patch, options);
                        if (reporter.error_count() == error_count)
                        {
                            results.evaluated_properties.insert(prop.key());
                        }
                    }
                }
            }
        }
    };

    template <class Json>
    class unevaluated_items_validator : public keyword_validator_base<Json>
    {
        using keyword_validator_type = typename keyword_validator<Json>::keyword_validator_type;
        using schema_validator_type = typename schema_validator<Json>::schema_validator_type;

        schema_validator_type validator_;

    public:
        unevaluated_items_validator(const uri& schema_path,
            schema_validator_type&& val
        )
            : keyword_validator_base<Json>("unevaluatedProperties", std::move(schema_path)), 
              validator_(std::move(val))
        {
        }

        const schema_validator<Json>* match_dynamic_anchor(const std::string& s) const final
        {
            if (validator_ != nullptr && !validator_->id()) 
            {
                auto p = validator_->match_dynamic_anchor(s);
                if (p != nullptr)
                {
                    return p;
                }
            }
            return nullptr;
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch, 
            const evaluation_options& options) const final
        {
            //std::cout << this->keyword_name() << " [" << eval_context.eval_path().to_string() << ", " << this->schema_path().string() << "]";
            //std::cout << "results:\n";
            //for (const auto& s : results.evaluated_items)
            //{
            //    std::cout << "    " << s << "\n";
            //}
            //std::cout << "\n";
            if (!instance.is_array())
            {
                return;
            }

            if (validator_)
            {
                evaluation_context<Json> this_context(eval_context, this->keyword_name());

                std::size_t index = 0;
                for (const auto& item : instance.array_range()) 
                {
                    auto item_it = results.evaluated_items.find(index);

                    // check if it is in "results"
                    if (item_it == results.evaluated_items.end()) 
                    {
                        //std::cout << "Not in evaluated properties: " << item.key() << "\n";
                        std::size_t error_count = reporter.error_count();
                        validator_->validate(this_context, item, instance_location, results, reporter, patch, options);
                        if (reporter.error_count() == error_count)
                        {
                            results.evaluated_items.insert(index);
                        }
                    }
                    ++index;
                }
            }

            /*if (validator_)
            {
                evaluation_context<Json> this_context(eval_context, this->keyword_name());

                for (auto index : results.unevaluated_items())
                {
                    JSONCONS_ASSERT(index < instance.size());
                    validator_->validate(this_context, instance[index], instance_location, results, reporter, patch, options);
                }
            }*/
        }
    };

    template <class Json>
    class object_schema_validator : public schema_validator<Json>
    {
    public:
        using schema_validator_type = typename std::unique_ptr<schema_validator<Json>>;
        using keyword_validator_type = typename std::unique_ptr<keyword_validator<Json>>;

        uri schema_path_;
        jsoncons::optional<jsoncons::uri> id_;
        std::vector<keyword_validator_type> validators_; 
        std::unique_ptr<unevaluated_properties_validator<Json>> unevaluated_properties_val_;
        std::unique_ptr<unevaluated_items_validator<Json>> unevaluated_items_val_;
        std::map<std::string,schema_validator_type> defs_;
        Json default_value_;
        bool recursive_anchor_;
        jsoncons::optional<jsoncons::uri> dynamic_anchor_;

    public:
        object_schema_validator(const uri& schema_path, 
            const jsoncons::optional<jsoncons::uri>& id,
            std::vector<keyword_validator_type>&& validators, 
            std::map<std::string,schema_validator_type>&& defs,
            Json&& default_value)
            : schema_path_(schema_path),
              id_(id),
              validators_(std::move(validators)),
              defs_(std::move(defs)),
              default_value_(std::move(default_value)),
              recursive_anchor_(false)
        {
        }
        object_schema_validator(const uri& schema_path, 
            const jsoncons::optional<jsoncons::uri>& id,
            std::vector<keyword_validator_type>&& validators,
            std::unique_ptr<unevaluated_properties_validator<Json>>&& unevaluated_properties_val, 
            std::unique_ptr<unevaluated_items_validator<Json>>&& unevaluated_items_val, 
            std::map<std::string,schema_validator_type>&& defs,
            Json&& default_value, bool recursive_anchor)
            : schema_path_(schema_path),
              id_(id),
              validators_(std::move(validators)),
              unevaluated_properties_val_(std::move(unevaluated_properties_val)),
              unevaluated_items_val_(std::move(unevaluated_items_val)),
              defs_(std::move(defs)),
              default_value_(std::move(default_value)),
              recursive_anchor_(recursive_anchor)
        {
        }
        object_schema_validator(const uri& schema_path, 
            const jsoncons::optional<jsoncons::uri>& id,
            std::vector<keyword_validator_type>&& validators, 
            std::unique_ptr<unevaluated_properties_validator<Json>>&& unevaluated_properties_val, 
            std::unique_ptr<unevaluated_items_validator<Json>>&& unevaluated_items_val, 
            std::map<std::string,schema_validator_type>&& defs,
            Json&& default_value,
            jsoncons::optional<jsoncons::uri>&& dynamic_anchor)
            : schema_path_(schema_path),
              id_(std::move(id)),
              validators_(std::move(validators)),
              unevaluated_properties_val_(std::move(unevaluated_properties_val)),
              unevaluated_items_val_(std::move(unevaluated_items_val)),
              defs_(std::move(defs)),
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

        const jsoncons::optional<jsoncons::uri>& id() const final
        {
            return id_;
        }

        const jsoncons::optional<jsoncons::uri>& dynamic_anchor() const final
        {
            return dynamic_anchor_;
        }

        const schema_validator<Json>* match_dynamic_anchor(const std::string& s) const final
        {
            //std::cout << "match_dynamic_anchor " << s << ", " << this->schema_path().string() << ", id: " << (id_ ? id_->string() : "") << "\n";
            if (dynamic_anchor_)
            {
                //std::cout << ", fragment: " << dynamic_anchor_.value().fragment() << "\n";
                if (s == dynamic_anchor_.value().fragment())
                {
                    return this;
                }
            }

            //std::cout << "    " << "checking validators\n";
            for (const auto& val : validators_)
            {
                const schema_validator<Json>* p = val->match_dynamic_anchor(s);
                if (p != nullptr)
                {
                    return p;
                }
            }
            
            //std::cout << "    " << "checking defs\n";
            for (const auto& member : defs_)
            {
                //std::cout << "    " << member.first << "\n";
                if (!member.second->id())
                {
                    const schema_validator<Json>* p = member.second->match_dynamic_anchor(s);
                    if (p != nullptr)
                    {
                        return p;
                    }
                }
                //else
                //{
                //    std::cout << "    Skipping " << member.second->id()->string() << "\n"; 
                //}
            }
            
            return nullptr;
        }

    private:

        void do_validate(const evaluation_context<Json>& eval_context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch, 
            const evaluation_options& options) const final
        {
            //std::cout << "object_schema_validator begin[" << eval_context.eval_path().to_string() << "," << this->schema_path().string() << "]";
            //std::cout << "results:\n";
            //for (const auto& s : results)
            //{
            //    std::cout << "    " << s << "\n";
            //}
            //std::cout << "\n";
          
            
            evaluation_results local_results;

            bool require_evaluated_properties = eval_context.require_evaluated_properties() || unevaluated_properties_val_ != nullptr;
            bool require_evaluated_items = eval_context.require_evaluated_items() || unevaluated_items_val_ != nullptr;
            evaluation_context<Json> this_context{eval_context, this, require_evaluated_properties, require_evaluated_items};
            
            //std::cout << "validators:\n";
            for (auto& val : validators_)
            {               
                //std::cout << "    " << val->keyword_name() << "\n";
                val->validate(this_context, instance, instance_location, local_results, reporter, patch, options);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }
            
            if (unevaluated_properties_val_)
            {
                unevaluated_properties_val_->validate(this_context, instance, instance_location, local_results, reporter, patch, options);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }

            if (unevaluated_items_val_)
            {
                unevaluated_items_val_->validate(this_context, instance, instance_location, local_results, reporter, patch, options);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }

            if (eval_context.require_evaluated_properties())
            {
                results.merge(std::move(local_results.evaluated_properties));
            }
            if (eval_context.require_evaluated_items())
            {
                results.merge(std::move(local_results.evaluated_items));
            }
            
            //std::cout << "object_schema_validator end[" << eval_context.eval_path().to_string() << "," << this->schema_path().string() << "]";
            //std::cout << "results:\n";
            //for (const auto& s : results)
            //{
            //    std::cout << "    " << s << "\n";
            //}
            //std::cout << "\n";
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_KEYWORD_VALIDATOR_HPP

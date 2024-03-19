// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMMON_SCHEMA_VALIDATORS_HPP
#define JSONCONS_JSONSCHEMA_COMMON_SCHEMA_VALIDATORS_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/common/evaluation_context.hpp>
#include <jsoncons_ext/jsonschema/common/keyword_validators.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>
#include <unordered_set>

namespace jsoncons {
namespace jsonschema {

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

        bool has_dynamic_anchor(const std::string& /*anchor*/) const final
        {
            return false;
        }

        const schema_validator<Json>* match_dynamic_anchor(const std::string& /*s*/) const final
        {
            return nullptr;
        }

    private:

        void do_validate(const evaluation_context<Json>& context, const Json&, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& /*results*/, 
            error_reporter& reporter, 
            Json& /*patch*/, 
            const evaluation_options& /*options*/) const final
        {
            if (!value_)
            {
                reporter.error(validation_output("false", 
                    context.eval_path(),
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
        using anchor_dictionary_type = std::unordered_map<std::string,std::unique_ptr<ref_validator<Json>>>;

        uri schema_path_;
        jsoncons::optional<jsoncons::uri> id_;
        std::vector<keyword_validator_type> validators_; 
        std::unique_ptr<unevaluated_properties_validator<Json>> unevaluated_properties_val_;
        std::unique_ptr<unevaluated_items_validator<Json>> unevaluated_items_val_;
        std::map<std::string,schema_validator_type> defs_;
        Json default_value_;
        bool recursive_anchor_;
        jsoncons::optional<jsoncons::uri> dynamic_anchor_;
        anchor_dictionary_type anchor_dict_;

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
            jsoncons::optional<jsoncons::uri>&& dynamic_anchor,
            anchor_dictionary_type&& anchor_dict)
            : schema_path_(schema_path),
              id_(std::move(id)),
              validators_(std::move(validators)),
              unevaluated_properties_val_(std::move(unevaluated_properties_val)),
              unevaluated_items_val_(std::move(unevaluated_items_val)),
              defs_(std::move(defs)),
              default_value_(std::move(default_value)),
              recursive_anchor_(false),
              dynamic_anchor_(std::move(dynamic_anchor)),
              anchor_dict_(std::move(anchor_dict))
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

        bool has_dynamic_anchor(const std::string& anchor) const final
        {
            auto it = anchor_dict_.find(anchor);
            return it == anchor_dict_.end() ? false : true;
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

        void do_validate(const evaluation_context<Json>& context, const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results, 
            error_reporter& reporter, 
            Json& patch, 
            const evaluation_options& options) const final
        {
            //std::cout << "object_schema_validator begin[" << context.eval_path().to_string() << "," << this->schema_path().string() << "]";
            //std::cout << "results:\n";
            //for (const auto& s : results)
            //{
            //    std::cout << "    " << s << "\n";
            //}
            //std::cout << "\n";
          
            
            evaluation_results local_results;

            evaluation_flags flags = context.eval_flags();
            if (unevaluated_properties_val_)
            {
                flags |= evaluation_flags::require_evaluated_properties;
            }
            if (unevaluated_items_val_)
            {
                flags |= evaluation_flags::require_evaluated_items;
            }

            evaluation_context<Json> this_context{context, this, flags};
            
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

            if ((context.eval_flags() & evaluation_flags::require_evaluated_properties)
                 == evaluation_flags::require_evaluated_properties)
            {
                results.merge(std::move(local_results.evaluated_properties));
            }
            if ((context.eval_flags() & evaluation_flags::require_evaluated_items)
                 == evaluation_flags::require_evaluated_items)
            {
                results.merge(std::move(local_results.evaluated_items));
            }
            
            //std::cout << "object_schema_validator end[" << context.eval_path().to_string() << "," << this->schema_path().string() << "]";
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

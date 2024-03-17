// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMMON_EVALUATION_CONTEXT_HPP
#define JSONCONS_JSONSCHEMA_COMMON_EVALUATION_CONTEXT_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    class schema_validator;

    template<class Json>
    class evaluation_context
    {
    private:
        std::vector<const schema_validator<Json> *> dynamic_scope_;
        jsonpointer::json_pointer eval_path_;
        bool require_evaluated_properties_;
        bool require_evaluated_items_;
    public:
        evaluation_context()
            : require_evaluated_properties_(false), require_evaluated_items_(false)
        {
        }

        evaluation_context(const evaluation_context& other)
            : dynamic_scope_ { other.dynamic_scope_}, eval_path_{other.eval_path_},
              require_evaluated_properties_(other.require_evaluated_properties_), 
              require_evaluated_items_(other.require_evaluated_items_)
        {
        }

        evaluation_context(evaluation_context&& other)
            : dynamic_scope_{std::move(other.dynamic_scope_)},eval_path_{std::move(other.eval_path_)},
              require_evaluated_properties_(other.require_evaluated_properties_), 
              require_evaluated_items_(other.require_evaluated_items_)
        {
        }

        evaluation_context(const evaluation_context& parent, const schema_validator<Json> *validator)
            : dynamic_scope_ { parent.dynamic_scope_ }, eval_path_{ parent.eval_path_ },
              require_evaluated_properties_(parent.require_evaluated_properties_), 
              require_evaluated_items_(parent.require_evaluated_items_)
        {
            if (validator->id()|| dynamic_scope_.empty())
            {
                dynamic_scope_.push_back(validator);
            }
        }

        evaluation_context(const evaluation_context& parent, const schema_validator<Json> *validator,
            bool require_evaluated_properties, bool require_evaluated_items)
            : dynamic_scope_ { parent.dynamic_scope_ }, eval_path_{ parent.eval_path_ },
              require_evaluated_properties_(require_evaluated_properties), 
              require_evaluated_items_(require_evaluated_items)
        {
            if (validator->id()|| dynamic_scope_.empty())
            {
                dynamic_scope_.push_back(validator);
            }
        }

        evaluation_context(const evaluation_context& parent, const std::string& name)
            : dynamic_scope_{parent.dynamic_scope_}, eval_path_(parent.eval_path() / name),
              require_evaluated_properties_(parent.require_evaluated_properties_), 
              require_evaluated_items_(parent.require_evaluated_items_)
              
        {
        }

        evaluation_context(const evaluation_context& parent, std::size_t index)
            : dynamic_scope_{parent.dynamic_scope_}, eval_path_(parent.eval_path() / index),
              require_evaluated_properties_(parent.require_evaluated_properties_), 
              require_evaluated_items_(parent.require_evaluated_items_)
        {
        }

        const std::vector<const schema_validator<Json>*>& dynamic_scope() const
        {
            return dynamic_scope_;
        }

        const jsonpointer::json_pointer& eval_path() const
        {
            return eval_path_;
        }
        
        bool require_evaluated_properties() const
        {
            return require_evaluated_properties_;
        }

        bool require_evaluated_items() const
        {
            return require_evaluated_items_;
        }
    }; 

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_COMPILATION_CONTEXT_HPP

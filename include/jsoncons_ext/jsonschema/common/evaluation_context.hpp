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

    template <class Json>
    class evaluation_context
    {
    private:
        std::vector<const schema_validator<Json>*> dynamic_scope_;
        jsonpointer::json_pointer eval_path_;
    public:
        evaluation_context()
        {
        }

        evaluation_context(const evaluation_context& other)
            : dynamic_scope_{other.dynamic_scope_}, eval_path_{other.eval_path_}
        {
        }

        evaluation_context(evaluation_context&& other)
            : dynamic_scope_{std::move(other.dynamic_scope_)}, eval_path_{std::move(other.eval_path_)}
        {
        }

        evaluation_context(const evaluation_context& parent, const schema_validator<Json>* validator)
            : dynamic_scope_{ parent.dynamic_scope_ }, eval_path_{ parent.eval_path_ }
        {
            dynamic_scope_.push_back(validator);
        }

        evaluation_context(const evaluation_context& parent, const std::string& name)
            : dynamic_scope_{parent.dynamic_scope_}, eval_path_(parent.eval_path() / name)
        {
        }

        evaluation_context(const evaluation_context& parent, std::size_t index)
            : dynamic_scope_{parent.dynamic_scope_}, eval_path_(parent.eval_path() / index)
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
    }; 

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_COMPILATION_CONTEXT_HPP

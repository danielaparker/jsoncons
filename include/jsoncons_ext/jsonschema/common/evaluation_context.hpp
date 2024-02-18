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

    class evaluation_context
    {
    private:
        jsonpointer::json_pointer eval_path_;
    public:
        evaluation_context()
        {
        }

        evaluation_context(const evaluation_context& other)
        {
            eval_path_ = other.eval_path_;
        }

        evaluation_context(evaluation_context&& other)
        {
            eval_path_ = std::move(other.eval_path_);
        }

        evaluation_context(const evaluation_context& parent, const std::string& name)
            : eval_path_(parent.eval_path() / name)
        {
        }

        evaluation_context(const evaluation_context& parent, std::size_t index)
            : eval_path_(parent.eval_path() / index)
        {
        }

        const jsonpointer::json_pointer eval_path() const
        {
            return eval_path_;
        }
    }; 

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_COMPILATION_CONTEXT_HPP

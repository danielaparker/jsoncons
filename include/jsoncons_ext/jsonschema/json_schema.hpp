// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_JSON_SCHEMA_HPP
#define JSONCONS_JSONSCHEMA_JSON_SCHEMA_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>
#include <jsoncons_ext/jsonschema/common/schema_validators.hpp>
#include <unordered_set>

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    class json_schema
    {
        using keyword_validator_type = std::unique_ptr<keyword_validator<Json>>;
        using schema_validator_type = std::unique_ptr<schema_validator<Json>>;

        schema_validator_type root_;
    public:
        json_schema(schema_validator_type&& root)
            : root_(std::move(root))
        {
            if (root_ == nullptr)
                JSONCONS_THROW(schema_error("There is no root schema to validate an instance against"));
        }

        json_schema(const json_schema&) = delete;
        json_schema(json_schema&&) = default;
        json_schema& operator=(const json_schema&) = delete;
        json_schema& operator=(json_schema&&) = default;

        void validate(const Json& instance, 
            const jsonpointer::json_pointer& instance_location,
            evaluation_results& results,
            error_reporter& reporter, 
            Json& patch) const 
        {
            JSONCONS_ASSERT(root_ != nullptr);
            root_->validate(evaluation_context<Json>{}, instance, instance_location, results, reporter, patch);
        }
    };


} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_HPP

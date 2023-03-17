// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_SUBSCHEMA_HPP
#define JSONCONS_JSONSCHEMA_SUBSCHEMA_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>
#include <jsoncons_ext/jsonschema/schema_location.hpp>

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
    class keyword_validator 
    {
        std::string schema_path_;
    public:
        using self_pointer = keyword_validator<Json>*;

        keyword_validator(const std::string& schema_path)
            : schema_path_(schema_path)
        {
        }

        keyword_validator(const keyword_validator&) = delete;
        keyword_validator(keyword_validator&&) = default;
        keyword_validator& operator=(const keyword_validator&) = delete;
        keyword_validator& operator=(keyword_validator&&) = default;

        virtual ~keyword_validator() = default;

        const std::string& schema_path() const
        {
            return schema_path_;
        }

        void validate(const Json& instance, 
                      const jsonpointer::json_pointer& instance_location, 
                      error_reporter& reporter, 
                      Json& patch) const 
        {
            do_validate(instance, 
                        instance_location,
                        reporter,
                        patch);
        }

        virtual jsoncons::optional<Json> get_default_value(const jsonpointer::json_pointer&, const Json&, error_reporter&) const
        {
            return jsoncons::optional<Json>();
        }

    private:
        virtual void do_validate(const Json& instance, 
                                 const jsonpointer::json_pointer& instance_location, 
                                 error_reporter& reporter, 
                                 Json& patch) const = 0;
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_RULE_HPP

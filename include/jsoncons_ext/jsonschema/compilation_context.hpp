// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMPILATION_CONTEXT_HPP
#define JSONCONS_JSONSCHEMA_COMPILATION_CONTEXT_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>
#include <jsoncons_ext/jsonschema/schema_location.hpp>

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    class compilation_context
    {
        std::vector<schema_location> uris_;
    public:
        compilation_context(const std::vector<schema_location>& uris)
            : uris_(uris)
        {
        }

        const std::vector<schema_location>& uris() const {return uris_;}

        std::string get_absolute_keyword_location() const
        {
            return (!uris_.empty() && uris_.back().is_absolute()) ? uris_.back().string() : "";
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_RULE_HPP

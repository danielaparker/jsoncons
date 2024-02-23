// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMMON_COMPILATION_CONTEXT_HPP
#define JSONCONS_JSONSCHEMA_COMMON_COMPILATION_CONTEXT_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>
#include <jsoncons_ext/jsonschema/common/schema_identifier.hpp>

namespace jsoncons {
namespace jsonschema {

    class compilation_context
    {
        uri absolute_uri_;
        std::vector<schema_identifier> uris_;
    public:
        explicit compilation_context(const schema_identifier& location)
            : absolute_uri_(location.uri()), 
              uris_(std::vector<schema_identifier>{{location}})
        {
        }

        explicit compilation_context(const std::vector<schema_identifier>& uris)
            : uris_(uris)
        {
            absolute_uri_ = !uris.empty() ? uris.back().uri() : uri{ "#" };
        }

        const std::vector<schema_identifier>& uris() const {return uris_;}

        const uri& get_absolute_uri() const
        {
            return absolute_uri_;
        }

        uri get_base_uri() const
        {
            return absolute_uri_.base();
        }

        std::string make_schema_path_with(const std::string& keyword) const
        {
            for (auto it = uris_.rbegin(); it != uris_.rend(); ++it)
            {
                if (!it->has_plain_name_fragment())
                {
                    return it->append(keyword).string();
                }
            }
            return "#";
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_COMPILATION_CONTEXT_HPP

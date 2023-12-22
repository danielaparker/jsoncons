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
#include <jsoncons_ext/jsonschema/common/schema_location.hpp>

namespace jsoncons {
namespace jsonschema {

    class compilation_context
    {
        uri absolute_uri_;
        std::vector<schema_location> uris_;
    public:
        compilation_context(const schema_location& location)
            : absolute_uri_(location.uri().is_absolute() ? location.uri() : uri{}), 
              uris_(std::vector<schema_location>{{location}})
        {
        }

        compilation_context(schema_location&& location)
            : absolute_uri_(location.uri().is_absolute() ? location.uri() : uri{}), 
              uris_(std::vector<schema_location>{{std::move(location)}})
        {
        }

        explicit compilation_context(const std::vector<schema_location>& uris)
            : uris_(uris)
        {
            for (auto it = uris_.rbegin();
                 it != uris_.rend();
                 ++it)
            {
                if (it->uri().is_absolute())
                {
                    absolute_uri_ = it->uri();
                    break;
                }
            }
        }
        explicit compilation_context(std::vector<schema_location>&& uris)
            : uris_(std::move(uris))
        {
            for (auto it = uris_.rbegin();
                 it != uris_.rend();
                 ++it)
            {
                if (it->uri().is_absolute())
                {
                    absolute_uri_ = it->uri();
                    break;
                }
            }
        }

        const std::vector<schema_location>& uris() const {return uris_;}

        const uri& get_absolute_uri() const
        {
            return absolute_uri_;
        }

        uri get_base_uri() const
        {
            return absolute_uri_.base();
        }

        schema_location resolve_back(const schema_location& relative) const
        {
            return relative.resolve(uris_.back());
        }

        std::string make_schema_path_with(const std::string& keyword) const
        {
            for (auto it = uris_.rbegin(); it != uris_.rend(); ++it)
            {
                if (!it->has_plain_name_fragment() && it->is_absolute())
                {
                    return it->append(keyword).string();
                }
            }
            return "";
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_COMPILATION_CONTEXT_HPP

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
#include <jsoncons_ext/jsonschema/common/uri_wrapper.hpp>

namespace jsoncons {
namespace jsonschema {

    class compilation_context
    {
        uri absolute_uri_;
        std::vector<uri_wrapper> uris_;
        jsoncons::optional<uri> id_;
    public:

        compilation_context()
        {
        }

        explicit compilation_context(const uri_wrapper& location)
            : absolute_uri_(location.uri()), 
              uris_(std::vector<uri_wrapper>{{location}})
        {
        }

        explicit compilation_context(const std::vector<uri_wrapper>& uris)
            : uris_(uris)
        {
            absolute_uri_ = !uris.empty() ? uris.back().uri() : uri{ "#" };
        }

        explicit compilation_context(const std::vector<uri_wrapper>& uris, const jsoncons::optional<uri>& id)
            : uris_(uris), id_(id) 
        {
            absolute_uri_ = !uris.empty() ? uris.back().uri() : uri{ "#" };
        }

        const std::vector<uri_wrapper>& uris() const {return uris_;}
        
        const jsoncons::optional<uri>& id() const
        {
            return id_;
        }

        const uri& get_absolute_uri() const
        {
            return absolute_uri_;
        }

        uri get_base_uri() const
        {
            return absolute_uri_;
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

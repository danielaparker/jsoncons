// Copyright 2013-2024 Daniel Parker
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
#include <random>

namespace jsoncons {
namespace jsonschema {

    class compilation_context
    {
        uri_wrapper base_uri_;
        std::vector<uri_wrapper> uris_;
        jsoncons::optional<uri> id_;
    public:

        compilation_context()
        {
        }

        explicit compilation_context(const uri_wrapper& retrieval_uri)
            : base_uri_(retrieval_uri), 
              uris_(std::vector<uri_wrapper>{{retrieval_uri}})
        {
        }

        explicit compilation_context(const std::vector<uri_wrapper>& uris)
            : uris_(uris)
        {
            if (uris_.empty())
            {
                uris_.push_back(uri_wrapper{"#"});
            }
            base_uri_ = uris_.back();
        }

        explicit compilation_context(const std::vector<uri_wrapper>& uris, const jsoncons::optional<uri>& id)
            : uris_(uris), id_(id)
        {
            if (uris_.empty())
            {
                uris_.push_back(uri_wrapper{"#"});
            }
            base_uri_ = uris_.back();
        }

        const std::vector<uri_wrapper>& uris() const {return uris_;}
        
        const jsoncons::optional<uri>& id() const
        {
            return id_;
        }

        uri get_base_uri() const
        {
            return base_uri_.uri();
        }

        jsoncons::uri make_schema_location(const std::string& keyword) const
        {
            for (auto it = uris_.rbegin(); it != uris_.rend(); ++it)
            {
                if (!it->has_plain_name_fragment())
                {
                    return it->append(keyword).uri();
                }
            }
            return uri{"#"};
        }
        
        static jsoncons::uri make_random_uri()
        {
            std::random_device dev;
            std::mt19937 gen{ dev() };
            std::uniform_int_distribution<std::mt19937::result_type> dist(1, 10000);

            std::string str = "https:://jsoncons.com/" + std::to_string(dist(gen));
            jsoncons::uri uri{str};
            return uri;
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_COMPILATION_CONTEXT_HPP

// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_DRAFT7_COMPILATION_CONTEXT_HPP
#define JSONCONS_JSONSCHEMA_DRAFT7_COMPILATION_CONTEXT_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>
#include <jsoncons_ext/jsonschema/common/schema_location.hpp>

namespace jsoncons {
namespace jsonschema {
namespace draft7 {

    class compilation_context
    {
        uri absolute_uri_;
        std::vector<schema_location> uris_;
    public:
        explicit compilation_context(const schema_location& location)
            : absolute_uri_(location.uri()), 
              uris_(std::vector<schema_location>{{location}})
        {
        }

        explicit compilation_context(schema_location&& location)
            : absolute_uri_(location.uri()), 
              uris_(std::vector<schema_location>{{std::move(location)}})
        {
        }

        explicit compilation_context(const std::vector<schema_location>& uris)
            : uris_(uris)
        {
            absolute_uri_ = !uris.empty() ? uris.back().uri() : uri{"#"};
        }
        explicit compilation_context(std::vector<schema_location>&& uris)
            : uris_(std::move(uris))
        {
            absolute_uri_ = !uris.empty() ? uris.back().uri() : uri{"#"};
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

        template <class Json>
        compilation_context update_uris(const Json& sch, const std::string& key) const
        {
            std::string sub_keys[] = {key};
            return update_uris(sch, sub_keys);
        }

        template <class Json>
        compilation_context update_uris(const Json& sch, jsoncons::span<const std::string> keys) const
        {
            // Exclude uri's that are not plain name identifiers
            std::vector<schema_location> new_uris;
            for (const auto& uri : uris_)
            {
                if (!uri.has_plain_name_fragment())
                {
                    new_uris.push_back(uri);
                }
            }

            if (new_uris.empty())
            {
                new_uris.emplace_back("#");
            }

            // Append the keys for this sub-schema to the uri's
            for (const auto& key : keys)
            {
                for (auto& uri : new_uris)
                {
                    auto new_u = uri.append(key);
                    uri = schema_location(new_u);
                }
            }
            if (sch.is_object())
            {
                auto it = sch.find("$id"); // If $id is found, this schema can be referenced by the id
                if (it != sch.object_range().end()) 
                {
                    std::string id = it->value().template as<std::string>(); 
                    // Add it to the list if it is not already there
                    if (std::find(new_uris.begin(), new_uris.end(), id) == new_uris.end())
                    {
                        schema_location relative(id); 
                        schema_location new_uri = relative.resolve(new_uris.back());
                        new_uris.emplace_back(new_uri); 
                    }
                }
            }

            /*std::cout << "\ncontext\n";
            for (const auto& uri : new_uris)
            {
                std::cout << "    " << uri.string() << "\n";
            }*/

            return compilation_context(new_uris);
        }

        compilation_context update_uris(jsoncons::span<const std::string> keys) const
        {
            // Exclude uri's that are not plain name identifiers
            std::vector<schema_location> new_uris;
            for (const auto& uri : uris_)
            {
                if (!uri.has_plain_name_fragment())
                {
                    new_uris.push_back(uri);
                }
            }

            if (new_uris.empty())
            {
                new_uris.emplace_back("#");
            }

            // Append the keys for this sub-schema to the uri's
            for (const auto& key : keys)
            {
                for (auto& uri : new_uris)
                {
                    auto new_u = uri.append(key);
                    uri = schema_location(new_u);
                }
            }

            //for (const auto& uri : new_uris)
            //{
            //    std::cout << "    " << uri.string() << "\n";
            //}

            return compilation_context(new_uris);
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

} // namespace draft7
} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_COMPILATION_CONTEXT_HPP

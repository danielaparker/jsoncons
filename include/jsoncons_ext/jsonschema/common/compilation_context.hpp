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

        template <class Json>
        compilation_context update_uris(const Json& schema, const std::string& key) const
        {
            return update_uris(schema, std::vector<std::string>{{key}});
        }

        template <class Json>
        compilation_context update_base_uri(const Json& schema) const
        {
            if (schema.type() == json_type::object_value)
            {
                auto it = schema.find("$ref");
                if (it == schema.object_range().end()) // this schema is not a reference
                { 
                    it = schema.find("$id"); // If $id is found, this schema can be referenced by the id
                    if (it != schema.object_range().end()) 
                    {
                        std::string id = it->value().template as<std::string>(); 
                        schema_location relative(id); 
                        schema_location new_uri = relative.resolve(get_base_uri());
                        return compilation_context(new_uri); 
                    }
                }
            }

            return *this;
        }

        template <class Json>
        compilation_context update_uris(const Json& schema, const std::vector<std::string>& keys) const
        {
            bool has_plain_name_fragment = false;
            // Exclude uri's that are not plain name identifiers
            std::vector<schema_location> new_uris;
            for (const auto& uri : uris_)
            {
                if (!uri.has_plain_name_fragment())
                {
                    new_uris.push_back(uri);
                }
                else
                {
                    has_plain_name_fragment = true;
                }
            }

            if (has_plain_name_fragment)
            {
                //std::cout << "update_uris\n";
                for (const auto& uri : uris_)
                {
                    if (!uri.has_plain_name_fragment())
                    {
                        //std::cout << "    not has_plain_name_fragment " << uri.string() << std::endl;
                    }
                    else
                    {
                        //std::cout << "    has_plain_name_fragment " << uri.string() << std::endl;
                    }
                }
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
            if (schema.type() == json_type::object_value)
            {
                auto it = schema.find("$ref");
                if (it == schema.object_range().end()) // this schema is not a reference
                { 
                    it = schema.find("$id"); // If $id is found, this schema can be referenced by the id
                    if (it != schema.object_range().end()) 
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
            }

            return compilation_context(new_uris);
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

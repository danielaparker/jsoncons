// Copyright 2013-2026 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_EXT_JSONSCHEMA_COMMON_SCHEMA_READERS_HPP
#define JSONCONS_EXT_JSONSCHEMA_COMMON_SCHEMA_READERS_HPP

#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

#include <jsoncons/config/compiler_support.hpp>
#include <jsoncons/utility/string_utils.hpp>

#include <jsoncons_ext/jsonschema/common/compilation_context.hpp>
#include <jsoncons_ext/jsonschema/common/schema_validator.hpp>
#include <jsoncons_ext/jsonschema/evaluation_options.hpp>

namespace jsoncons {
namespace jsonschema {

template <typename Json>
class schema_readers
{
public:

    static void read_id_4(const compilation_context<Json>& parent, const Json& sch, 
        jsoncons::optional<uri>& id,
        std::vector<uri_wrapper>& new_uris)
    {
        auto it = sch.find("id"); // If id is found, this schema can be referenced by the id
        if (it != sch.object_range().end()) 
        {
            if (!(*it).value().is_string())
            {
                JSONCONS_THROW(schema_error("id must be string"));
            }
            uri relative((*it).value().as_string_view()); 
            auto resolved = parent.get_base_uri().resolve(relative);
            id = resolved;
            //std::cout << "$id: " << id << ", " << new_uri.string() << "\n";
            // Add it to the list if it is not already there

            uri_wrapper new_uri{resolved};
            if (std::find(new_uris.begin(), new_uris.end(), new_uri) == new_uris.end())
            {
                new_uris.emplace_back(new_uri); 
            }
        }
    }

    static void read_id_6_7(const compilation_context<Json>& parent, const Json& sch, 
        jsoncons::optional<uri>& id,
        std::vector<uri_wrapper>& new_uris)
    {
        auto it = sch.find("$id"); // If $id is found, this schema can be referenced by the id
        if (it != sch.object_range().end()) 
        {
            if (!(*it).value().is_string())
            {
                JSONCONS_THROW(schema_error("$id must be string"));
            }
            uri relative((*it).value().as_string_view()); 
            auto resolved = parent.get_base_uri().resolve(relative);
            id = resolved;
            //std::cout << "$id: " << id << ", " << new_uri.string() << "\n";
            // Add it to the list if it is not already there

            uri_wrapper new_uri{resolved};
            if (std::find(new_uris.begin(), new_uris.end(), new_uri) == new_uris.end())
            {
                new_uris.emplace_back(new_uri); 
            }
        }
    }

    static void read_id_201909_latest(const compilation_context<Json>& parent, const Json& sch, 
        jsoncons::optional<uri>& id,
        std::vector<uri_wrapper>& new_uris)
    {
        auto it = sch.find("$id"); // If $id is found, this schema can be referenced by the id
        if (it != sch.object_range().end()) 
        {
            if (!(*it).value().is_string())
            {
                JSONCONS_THROW(schema_error("$id must be string"));
            }
            auto sv = (*it).value().as_string_view();
            uri relative(sv); 
            if (relative.has_fragment())
            {
                std::string message{sv};
                message.append(": Drafts 2019-09 and later do not allow $id with fragment");
                JSONCONS_THROW(schema_error(message));
            }
            auto resolved = parent.get_base_uri().resolve(relative);
            id = resolved;
            uri_wrapper new_uri{resolved};
            //std::cout << "$id: " << id << ", " << new_uri.string() << "\n";
            // Add it to the list if it is not already there
            if (std::find(new_uris.begin(), new_uris.end(), new_uri) == new_uris.end())
            {
                new_uris.emplace_back(new_uri); 
            }
        }
    }

    static void read_anchor(const Json& sch, 
        std::vector<uri_wrapper>& new_uris)
    {
        auto it = sch.find("$anchor"); 
        if (it != sch.object_range().end()) 
        {
            if (!(*it).value().is_string())
            {
                JSONCONS_THROW(schema_error("$anchor must be string"));
            }
            auto anchor = (*it).value().as_string_view();
            if (!validate_anchor(anchor))
            {
                std::string message{"Invalid $anchor "};
                message.append(anchor.data(), anchor.size());
                JSONCONS_THROW(schema_error(message));
            }
            auto uri = !new_uris.empty() ? new_uris.back().uri() : jsoncons::uri{"#"};
            jsoncons::uri new_uri(uri, uri_fragment_part, anchor);
            uri_wrapper identifier{ new_uri };
            if (std::find(new_uris.begin(), new_uris.end(), identifier) == new_uris.end())
            {
                new_uris.emplace_back(std::move(identifier)); 
            }
        }
    }

    static void read_dynamic_anchor(const Json& sch, 
        std::vector<uri_wrapper>& new_uris)
    {
        auto it = sch.find("$dynamicAnchor"); 
        if (it != sch.object_range().end()) 
        {
            if (!(*it).value().is_string())
            {
                JSONCONS_THROW(schema_error("$dynamicAnchor must be string"));
            }
            auto anchor = (*it).value().as_string_view();
            if (!validate_anchor(anchor))
            {
                std::string message{"Invalid $dynamicAnchor "};
                message.append(anchor.data(), anchor.size());
                JSONCONS_THROW(schema_error(message));
            }
            auto uri = !new_uris.empty() ? new_uris.back().uri() : jsoncons::uri{"#"};
            jsoncons::uri new_uri(uri, uri_fragment_part, anchor);
            uri_wrapper identifier{ new_uri };
            if (std::find(new_uris.begin(), new_uris.end(), identifier) == new_uris.end())
            {
                new_uris.emplace_back(std::move(identifier)); 
            }
        }
    }

    static void read_custom_error_message(const Json& sch, 
        std::unordered_map<std::string, std::string>& custom_messages,
        std::string& custom_message) 
    {
        auto it = sch.find("errorMessage"); 
        if (it != sch.object_range().end()) 
        {
            const auto& value = it->value();
            if (value.is_object())
            {
                for (const auto& item : value.object_range())
                {
                    if (item.value().is_string())
                    {
                        custom_messages.emplace(item.name(), item.value().as_string_view());
                    }
                }
            }
            else if (value.is_string())
            {
                custom_message = value.template as<std::string>();
            }
        }
    }
private:

    static bool validate_anchor(jsoncons::string_view sv)
    {
        if (sv.empty())
        {
            return false;
        }
        if (!((sv[0] >= 'a' && sv[0] <= 'z') || (sv[0] >= 'A' && sv[0] <= 'Z')))
        {
            return false;
        }

        for (std::size_t i = 1; i < sv.size(); ++i)
        {
            switch (sv[i])
            {
                case '-':
                case '_':
                case ':':
                case '.':
                    break;
                default:
                    if (!((sv[i] >= 'a' && sv[i] <= 'z') || (sv[i] >= 'A' && sv[i] <= 'Z') || (sv[i] >= '0' && sv[i] <= '9')))
                    {
                        return false;
                    }
                    break;
            }
        }
        return true;
    }
};

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_EXT_JSONSCHEMA_COMMON_SCHEMA_READERS_HPP

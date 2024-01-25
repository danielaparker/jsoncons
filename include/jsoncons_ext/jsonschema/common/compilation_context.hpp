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

    enum class uri_anchor_flags : int {recursive_anchor=1};

    inline uri_anchor_flags operator|(const uri_anchor_flags &lhs, const uri_anchor_flags &rhs) 
    {
        return static_cast<uri_anchor_flags>(std::underlying_type<uri_anchor_flags>::type(lhs)
                                            | std::underlying_type<uri_anchor_flags>::type(rhs));
    }

    inline uri_anchor_flags operator&(const uri_anchor_flags &lhs, const uri_anchor_flags &rhs) 
    {
        return static_cast<uri_anchor_flags>(std::underlying_type<uri_anchor_flags>::type(lhs)
                                            & std::underlying_type<uri_anchor_flags>::type(rhs));
    }

    class compilation_context
    {
        const compilation_context* parent_;
        uri absolute_uri_;
        std::vector<schema_location> uris_;
        uri_anchor_flags anchor_flags_;
    public:
        explicit compilation_context(const schema_location& location)
            : parent_(nullptr), absolute_uri_(location.uri()), 
              uris_(std::vector<schema_location>{{location}}),
              anchor_flags_{}
        {
        }

        explicit compilation_context(const compilation_context* parent, const std::vector<schema_location>& uris,
            uri_anchor_flags flags = uri_anchor_flags{})
            : parent_(parent), uris_(uris), anchor_flags_{flags}
        {
            absolute_uri_ = !uris.empty() ? uris.back().uri() : uri{ "#" };
        }

        const std::vector<schema_location>& uris() const {return uris_;}

        const uri& get_absolute_uri() const
        {
            return absolute_uri_;
        }

        uri get_base_uri(uri_anchor_flags anchor_flags=uri_anchor_flags()) const
        {
            uri base = absolute_uri_.base();

            if (parent_ != nullptr && parent_->anchor_flags_ == uri_anchor_flags::recursive_anchor)
            {
                switch (anchor_flags)
                {
                    case uri_anchor_flags::recursive_anchor:
                    {
                        auto parent = parent_;
                        while (parent != nullptr)
                        {
                            if (parent->anchor_flags_ == uri_anchor_flags::recursive_anchor)
                            {
                                base = parent->get_base_uri();
                            }
                            parent = parent->parent_;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
            return base;
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

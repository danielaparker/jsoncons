// Copyright 2013-2024 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_EVALUATION_OPTIONS_HPP
#define JSONCONS_JSONSCHEMA_EVALUATION_OPTIONS_HPP

#include <string>

namespace jsoncons {
namespace jsonschema {

    struct schema_dialect
    {
        static std::string draft4() 
        {
            static std::string s{"http://json-schema.org/draft-04/schema#"};
            return s;
        }
        static std::string draft6() 
        {
            static std::string s{"http://json-schema.org/draft-06/schema#"};
            return s;
        }
        static std::string draft7() 
        {
            static std::string s{"http://json-schema.org/draft-07/schema#"};
            return s;
        }
        static std::string draft201909() 
        {
            static std::string s{"https://json-schema.org/draft/2019-09/schema"};
            return s;
        }
        static std::string draft202012() 
        {
            static std::string s{"https://json-schema.org/draft/2020-12/schema"};
            return s;
        }
    };

    class evaluation_options
    {
        std::string default_version_;
        bool require_format_validation_;
        bool compatibility_mode_;
    public:
        evaluation_options()
            : default_version_{schema_dialect::draft202012()}, 
              require_format_validation_(false), compatibility_mode_(false)
        {
        }

        bool require_format_validation() const
        {
            return require_format_validation_;
        }
        evaluation_options& require_format_validation(bool value) 
        {
            require_format_validation_ = value;
            return *this;
        }

        bool compatibility_mode() const
        {
            return compatibility_mode_;
        }
        evaluation_options& compatibility_mode(bool value) 
        {
            compatibility_mode_ = value;
            return *this;
        }

        const std::string& default_version() const
        {
            return default_version_;
        }
        evaluation_options& default_version(const std::string& version) 
        {
            default_version_ = version;
            return *this;
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_COMMON_SCHEMA_HPP

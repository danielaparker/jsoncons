// Copyright 2013-2023 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_COMMON_SCHEMA_IDENTIFIER_HPP
#define JSONCONS_JSONSCHEMA_COMMON_SCHEMA_IDENTIFIER_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>

namespace jsoncons {
namespace jsonschema {

    class schema_identifier
    {
        jsoncons::uri uri_;
        std::string identifier_;
    public:
        schema_identifier()
        {
        }

        explicit schema_identifier(const std::string& uri)
        {
            uri_ = jsoncons::uri(uri);
            identifier_ = uri_.fragment();
        }

        explicit schema_identifier(const uri& uri)
            : uri_{uri}
        {
            uri_ = jsoncons::uri(uri);
            identifier_ = uri_.fragment();
        }

        jsoncons::uri uri() const
        {
            return uri_;
        }

        bool has_fragment() const
        {
            return !uri_.encoded_fragment().empty();
        }

        bool has_plain_name_fragment() const
        {
            if (!has_fragment())
            {
                return false;
            }
            std::string identifier = uri_.fragment();
            std::error_code ec;
            jsonpointer::json_pointer::parse(identifier, ec);
            return ec ? true : false;
        }

        jsoncons::uri base() const
        {
            return uri_.base();
        }

        std::string path() const
        {
            return uri_.path();
        }

        bool is_absolute() const
        {
            return uri_.is_absolute();
        }

        std::string fragment() const
        {
            return identifier_;
        }

        schema_identifier resolve(const schema_identifier& uri) const
        {
            return schema_identifier(uri_.resolve(uri.uri_));
        }

        int compare(const schema_identifier& other) const
        {
            int result = uri_.compare(other.uri_);
            if (result != 0) 
            {
                return result;
            }
            return result; 
        }

        schema_identifier append(const std::string& field) const
        {
            if (has_plain_name_fragment())
                return *this;

            jsoncons::jsonpointer::json_pointer pointer(std::string(uri_.encoded_fragment()));
            pointer /= field;

            jsoncons::uri new_uri(uri_, uri_fragment_part, pointer.to_string());

            return schema_identifier(std::move(new_uri));
        }

        schema_identifier append(std::size_t index) const
        {
            if (has_plain_name_fragment())
                return *this;

            jsoncons::jsonpointer::json_pointer pointer(std::string(uri_.encoded_fragment()));
            pointer /= index;

            jsoncons::uri new_uri(uri_, uri_fragment_part, pointer.to_string());

            return schema_identifier(std::move(new_uri));
        }

        std::string string() const
        {
            std::string s = uri_.string();
            return s;
        }

        friend bool operator==(const schema_identifier& lhs, const schema_identifier& rhs)
        {
            return lhs.compare(rhs) == 0;
        }

        friend bool operator!=(const schema_identifier& lhs, const schema_identifier& rhs)
        {
            return lhs.compare(rhs) != 0;
        }

        friend bool operator<(const schema_identifier& lhs, const schema_identifier& rhs)
        {
            return lhs.compare(rhs) < 0;
        }

        friend bool operator<=(const schema_identifier& lhs, const schema_identifier& rhs)
        {
            return lhs.compare(rhs) <= 0;
        }

        friend bool operator>(const schema_identifier& lhs, const schema_identifier& rhs)
        {
            return lhs.compare(rhs) > 0;
        }

        friend bool operator>=(const schema_identifier& lhs, const schema_identifier& rhs)
        {
            return lhs.compare(rhs) >= 0;
        }
    private:
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_LOCATION_HPP

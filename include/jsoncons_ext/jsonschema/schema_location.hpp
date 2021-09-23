// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_SCHEMA_LOCATION_HPP
#define JSONCONS_JSONSCHEMA_SCHEMA_LOCATION_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/jsonschema_error.hpp>

namespace jsoncons {
namespace jsonschema {

    class schema_location
    {
        jsoncons::uri uri_;
        std::string identifier_;
    public:
        schema_location()
        {
        }

        schema_location(const std::string& uri)
        {
            auto pos = uri.find('#');
            if (pos != std::string::npos)
            {
                identifier_ = uri.substr(pos + 1); 
                unescape_percent(identifier_);
            }
            uri_ = jsoncons::uri(uri);
        }

        jsoncons::uri uri() const
        {
            return uri_;
        }

        bool has_fragment() const
        {
            return !identifier_.empty();
        }

        bool has_identifier() const
        {
            return !identifier_.empty() && identifier_.front() != '/';
        }

        jsoncons::string_view base() const
        {
            return uri_.base();
        }

        jsoncons::string_view path() const
        {
            return uri_.path();
        }

        bool is_absolute() const
        {
            return uri_.is_absolute();
        }

        std::string identifier() const
        {
            return identifier_;
        }

        std::string fragment() const
        {
            return identifier_;
        }

        schema_location resolve(const schema_location& uri) const
        {
            schema_location new_uri;
            new_uri.identifier_ = identifier_;
            new_uri.uri_ = uri_.resolve(uri.uri_);
            return new_uri;
        }

        int compare(const schema_location& other) const
        {
            int result = uri_.compare(other.uri_);
            if (result != 0) 
            {
                return result;
            }
            return result; 
        }

        schema_location append(const std::string& field) const
        {
            if (has_identifier())
                return *this;

            jsoncons::jsonpointer::json_pointer pointer(std::string(uri_.fragment()));
            pointer /= field;

            jsoncons::uri new_uri(uri_.scheme(),
                                  uri_.userinfo(),
                                  uri_.host(),
                                  uri_.port(),
                                  uri_.path(),
                                  uri_.query(),
                                  pointer.to_string());

            schema_location wrapper;
            wrapper.uri_ = new_uri;
            wrapper.identifier_ = pointer.to_string();

            return wrapper;
        }

        schema_location append(std::size_t index) const
        {
            if (has_identifier())
                return *this;

            jsoncons::jsonpointer::json_pointer pointer(std::string(uri_.fragment()));
            pointer /= index;

            jsoncons::uri new_uri(uri_.scheme(),
                                  uri_.userinfo(),
                                  uri_.host(),
                                  uri_.port(),
                                  uri_.path(),
                                  uri_.query(),
                                  pointer.to_string());

            schema_location wrapper;
            wrapper.uri_ = new_uri;
            wrapper.identifier_ = pointer.to_string();

            return wrapper;
        }

        std::string string() const
        {
            std::string s = uri_.string();
            return s;
        }

        friend bool operator==(const schema_location& lhs, const schema_location& rhs)
        {
            return lhs.compare(rhs) == 0;
        }

        friend bool operator!=(const schema_location& lhs, const schema_location& rhs)
        {
            return lhs.compare(rhs) != 0;
        }

        friend bool operator<(const schema_location& lhs, const schema_location& rhs)
        {
            return lhs.compare(rhs) < 0;
        }

        friend bool operator<=(const schema_location& lhs, const schema_location& rhs)
        {
            return lhs.compare(rhs) <= 0;
        }

        friend bool operator>(const schema_location& lhs, const schema_location& rhs)
        {
            return lhs.compare(rhs) > 0;
        }

        friend bool operator>=(const schema_location& lhs, const schema_location& rhs)
        {
            return lhs.compare(rhs) >= 0;
        }
    private:
        static void unescape_percent(std::string& s)
        {
            if (s.size() >= 3)
            {
                std::size_t pos = s.size() - 2;
                while (pos-- >= 1)
                {
                    if (s[pos] == '%')
                    {
                        std::string hex = s.substr(pos + 1, 2);
                        char ch = (char) std::strtoul(hex.c_str(), nullptr, 16);
                        s.replace(pos, 3, 1, ch);
                    }
                }
            }
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_RULE_HPP

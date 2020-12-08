/// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_JSONSCHEMA_ERROR_HPP
#define JSONCONS_JSONSCHEMA_JSONSCHEMA_ERROR_HPP

#include <jsoncons/json_exception.hpp>
#include <system_error>

namespace jsoncons {
namespace jsonschema {

    class schema_error : public std::runtime_error, public virtual json_exception
    {
    public:
        schema_error(const std::string& message)
            : std::runtime_error(message)
        {
        }

        const char* what() const noexcept override
        {
            return std::runtime_error::what();
        }  
    };

    class validation_error : public std::runtime_error, public virtual json_exception
    {
        std::string instance_location_;
        std::string keyword_;
        std::string absolute_keyword_location_;
        std::vector<validation_error> nested_errors_;
        mutable std::string what_;
    public:
        validation_error(const std::string& instance_location,
                         const std::string& message,
                         const std::string& keyword,
                         const std::string& location)
            : std::runtime_error(message),
            instance_location_(instance_location), 
            keyword_(keyword),
            absolute_keyword_location_(location)
        {
        }
        validation_error(const std::string& instance_location,
                         const std::string& message,
                         const std::string& keyword,
                         const std::string& location,
                         const std::vector<validation_error>& nested_errors)
            : std::runtime_error(message),
              instance_location_(instance_location), 
              keyword_(keyword),
              absolute_keyword_location_(location),
              nested_errors_(nested_errors)
        {
        }

        std::string message() const
        {
            return std::runtime_error::what();
        }

        const std::string& absolute_keyword_location() const
        {
            return absolute_keyword_location_;
        }

        const std::vector<validation_error>& nested_errors() const
        {
            return nested_errors_;
        }

        const char* what() const noexcept override
        {
            if (what_.empty())
            {
                JSONCONS_TRY
                {
                    what_.append(instance_location_);
                    what_.append(": ");
                    what_.append(std::runtime_error::what());
                    return what_.c_str();
                }
                JSONCONS_CATCH(...)
                {
                    return std::runtime_error::what();
                }
            }
            else
            {
                return what_.c_str();
            }
        }

        const std::string& keyword() const
        {
            return keyword_;
        }

    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_JSONSCHEMA_ERROR_HPP
